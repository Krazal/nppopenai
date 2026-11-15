#include "HTTPClient.h"
#include <curl/curl.h>
#include "OpenAIClient.h"  // For callback functions
#include "EncodingUtils.h" // for toUTF8
#include "editor/EditorInterface.h" // For getCurrentScintilla
#include "core/external_globals.h"
#include "npp/Notepad_plus_msgs.h"
#include <future>
#include <chrono>
#include <windows.h>

#define LLM_RETRY_IMPLEMENTATION
#include "llm-retry/llm_retry.hpp" // Backoff for transient API failures (MIT, vs.proj/include/llm-retry)

namespace
{
    // Longest Retry-After (in seconds) we are willing to wait for behind the loader dialog.
    // A server asking for more than this fails right away, as before.
    const double MAX_RETRY_AFTER_SECONDS = 10.0;

    /**
     * cURL errors where the same request may well succeed a moment later: a
     * connection the server or a proxy dropped. A refused connection (e.g. Ollama
     * not running), DNS, TLS and URL errors are reported immediately, as before.
     */
    bool isTransientCurlError(CURLcode res)
    {
        switch (res)
        {
        case CURLE_SEND_ERROR:
        case CURLE_RECV_ERROR:
        case CURLE_GOT_NOTHING:
            return true;
        default:
            return false;
        }
    }

    // Seconds from the server's Retry-After header, or -1 if it sent none
    double retryAfterSeconds(CURL *curl)
    {
        curl_off_t wait = 0;
        if (curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &wait) == CURLE_OK && wait > 0)
            return static_cast<double>(wait);
        return -1.0;
    }

    // true for an HTTP status that is worth another attempt (429, 5xx, 529, ...)
    bool isRetryableStatus(CURL *curl, long httpCode, const llm::RetryConfig &config)
    {
        for (int code : config.retry_on_http_codes)
        {
            if (code == httpCode)
                return retryAfterSeconds(curl) <= MAX_RETRY_AFTER_SECONDS;
        }
        return false;
    }

    struct AttemptResult
    {
        CURLcode res = CURLE_OK;
        long httpCode = 0;
    };

    /**
     * Runs one cURL request with llm::with_retry: up to 3 attempts, waiting
     * 0.5 s and then 1 s (or the server's Retry-After) between them.
     *
     * @param perform Performs one attempt; receives true on the last attempt
     * @param canRetry Returns false once a failed attempt must not be repeated
     * @return The outcome of the last attempt made
     */
    template <typename Perform, typename CanRetry>
    AttemptResult performWithRetry(CURL *curl, const llm::RetryConfig &config, Perform perform, CanRetry canRetry)
    {
        AttemptResult last;
        int attempt = 0;
        try
        {
            llm::with_retry<AttemptResult>([&]() -> AttemptResult
                                           {
                // Don't send the request again once the user has cancelled
                if (attempt > 0 && _loaderDlg.isCancelled())
                    throw llm::LLMError{0, "cancelled", false};

                ++attempt;
                if (attempt > 1 && debugMode)
                {
                    std::wstring retryMsg = L"Request failed, retrying (attempt " + std::to_wstring(attempt) + L" of " + std::to_wstring(config.max_attempts) + L")...";
                    ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)retryMsg.c_str());
                }

                last = AttemptResult{};
                last.res = perform(attempt == config.max_attempts);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &last.httpCode);

                if (_loaderDlg.isCancelled() || !canRetry())
                    return last;
                if (last.res != CURLE_OK)
                {
                    if (isTransientCurlError(last.res))
                        throw llm::LLMError{0, curl_easy_strerror(last.res), true};
                }
                else if (isRetryableStatus(curl, last.httpCode, config))
                {
                    throw llm::LLMError{static_cast<int>(last.httpCode), "HTTP error", true, retryAfterSeconds(curl)};
                }
                return last; },
                                           config);
        }
        catch (const llm::LLMError &)
        {
            // Out of attempts (or cancelled): report the last attempt as before
        }
        return last;
    }

    /**
     * Sits in front of OpenAIStreamCallback. While another attempt is still
     * possible, it keeps the error body of a retryable HTTP status (e.g. 429 or
     * 503) out of the editor, and it records whether any text reached the
     * editor, since a request that already wrote text must not be repeated.
     */
    struct StreamGate
    {
        CURL *curl = nullptr;
        void *targetWindow = nullptr;
        const llm::RetryConfig *config = nullptr;
        bool holdRetryableErrors = false;
        bool delivered = false;
    };

    size_t gatedStreamCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        StreamGate *gate = static_cast<StreamGate *>(userp);
        if (gate->holdRetryableErrors)
        {
            long httpCode = 0;
            curl_easy_getinfo(gate->curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if (isRetryableStatus(gate->curl, httpCode, *gate->config))
                return _loaderDlg.isCancelled() ? CURL_READFUNC_ABORT : size * nmemb;
        }
        gate->delivered = true;
        return OpenAIStreamCallback(contents, size, nmemb, gate->targetWindow);
    }
}

/**
 * Performs a standard HTTP request to an LLM API
 *
 * @param url The full API endpoint URL to call
 * @param request The JSON request body as a string
 * @param response Output parameter that will store the API response
 * @param apiType The type of API (openai, claude, ollama, etc.)
 * @param secretKey The API key for authentication
 * @param proxy Optional proxy server to use (or "0" for no proxy)
 * @return true if the request was successful (200-level response), false otherwise
 */
bool HTTPClient::performRequest(
    const std::string &url,
    const std::string &request,
    std::string &response,
    const std::string &apiType,
    const std::string &secretKey,
    const std::string &proxy)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    struct curl_slist *headers = nullptr;

    // Content-Type is always JSON
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Add authentication headers based on API type
    if (apiType == "claude")
    {
        std::string authHeader = "x-api-key: " + secretKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    }
    else
    {
        std::string authHeader = "Authorization: Bearer " + secretKey;
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());

    // Set proxy if provided
    if (!proxy.empty() && proxy != "0")
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
    }

    // Setup callback to capture response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIcURLCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request asynchronously to allow UI timers to run,
    // retrying transient failures (dropped connections, 429, 5xx)
    llm::RetryConfig retryConfig;
    auto futureRes = std::async(std::launch::async, [&]()
                                { return performWithRetry(
                                      curl, retryConfig,
                                      [&](bool)
                                      {
                                          response.clear(); // Keep only the last attempt's body
                                          return curl_easy_perform(curl);
                                      },
                                      []()
                                      { return true; }); });
    AttemptResult res;

    // Pump UI message loop until request completes
    while (futureRes.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        ::Sleep(10);
    }
    res = futureRes.get();

    // HTTP status code of the last attempt
    long http_code = res.httpCode;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Return true only if both curl succeeded and HTTP status is 2xx
    return (res.res == CURLE_OK && (http_code >= 200 && http_code < 300));
}

/**
 * Performs a streaming HTTP request to an LLM API
 *
 * @param url The full API endpoint URL to call
 * @param request The JSON request body as a string
 * @param apiType The type of API (openai, claude, ollama, etc.)
 * @param secretKey The API key for authentication
 * @param targetWindow The window handle to receive streaming chunks
 * @param streamMessageType The Windows message type for streaming chunks
 * @param proxy Optional proxy server to use (or "0" for no proxy)
 * @return true if the request was successful (200-level response), false otherwise
 */
bool HTTPClient::performStreamingRequest(
    const std::string &url,
    const std::string &request,
    const std::string &apiType,
    const std::string &secretKey,
    void *targetWindow,
    unsigned int streamMessageType,
    const std::string &proxy)
{
    // The streamMessageType parameter contains the message ID to use for posting chunks
    // We still need to cast it to void to suppress any unused parameter warnings
    (void)streamMessageType; // Will actually use WM_OPENAI_STREAM_CHUNK from OpenAIClient.cpp

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    struct curl_slist *headers = nullptr;

    // Set required headers
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Add Accept header for handling streaming response
    if (apiType == "openai" || apiType == "ollama")
    {
        headers = curl_slist_append(headers, "Accept: text/event-stream");
    }

    // Add authentication headers based on API type
    if (apiType == "claude")
    {
        std::string authHeader = "x-api-key: " + secretKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    }
    else
    {
        std::string authHeader = "Authorization: Bearer " + secretKey;
        headers = curl_slist_append(headers, authHeader.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set URL before async call
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    llm::RetryConfig retryConfig;
    StreamGate gate;
    gate.curl = curl;
    gate.targetWindow = targetWindow;
    gate.config = &retryConfig;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, gatedStreamCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &gate);

    // Important: Set these options for proper streaming behavior
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // Set proxy if provided before async call
    if (!proxy.empty() && proxy != "0")
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
    } // Add debugging for streaming requests
    if (debugMode)
    {
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)L"Starting streaming request...");
    }

	// Begin undo action in Scintilla editor to allow for proper undo/redo behavior
    HWND curScintilla = EditorInterface::getCurrentScintilla();
    ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

    // Perform the request asynchronously. A failed attempt is retried only
    // while no text has reached the editor yet.
    auto futureRes = std::async(std::launch::async, [&]()
                                { return performWithRetry(
                                      curl, retryConfig,
                                      [&](bool lastAttempt)
                                      {
                                          gate.holdRetryableErrors = !lastAttempt;
                                          return curl_easy_perform(curl);
                                      },
                                      [&]()
                                      { return !gate.delivered; }); });

    // Process message queue while request is in progress
    AttemptResult res;
    while (futureRes.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready)
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        ::Sleep(10);
    }
    res = futureRes.get(); // Get HTTP status code of the last attempt
    long http_code = res.httpCode;

	// End undo action in Scintilla editor
    ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);

    // Add debugging for the HTTP response
    if (debugMode)
    {
        std::wstring statusMsg = L"HTTP " + std::to_wstring(http_code) + L", cURL: " + std::to_wstring(res.res);
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)statusMsg.c_str());
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Return true only if both curl succeeded and HTTP status is 2xx
    return (res.res == CURLE_OK && (http_code >= 200 && http_code < 300));
}
