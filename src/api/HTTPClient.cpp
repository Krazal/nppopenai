#include "HTTPClient.h"
#include <curl/curl.h>
#include "OpenAIClient.h"  // For callback functions
#include "EncodingUtils.h" // for toUTF8
#include "core/external_globals.h"
#include "npp/Notepad_plus_msgs.h"
#include <future>
#include <chrono>
#include <windows.h>

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

    // Perform the request asynchronously to allow UI timers to run
    auto futureRes = std::async(std::launch::async, [curl]()
                                { return curl_easy_perform(curl); });
    CURLcode res;

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

    // Get HTTP status code
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Return true only if both curl succeeded and HTTP status is 2xx
    return (res == CURLE_OK && (http_code >= 200 && http_code < 300));
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIStreamCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, targetWindow);

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

    // Perform the request asynchronously
    auto futureRes = std::async(std::launch::async, [curl]()
                                {
                                   CURLcode r = curl_easy_perform(curl);
                                   return r; });

    // Process message queue while request is in progress
    CURLcode res;
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
    res = futureRes.get(); // Get HTTP status code
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    // Add debugging for the HTTP response
    if (debugMode)
    {
        std::wstring statusMsg = L"HTTP " + std::to_wstring(http_code) + L", cURL: " + std::to_wstring(res);
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)statusMsg.c_str());
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Return true only if both curl succeeded and HTTP status is 2xx
    return (res == CURLE_OK && (http_code >= 200 && http_code < 300));
}
