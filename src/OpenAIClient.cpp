// filepath: src/OpenAIClient.cpp
/**
 * OpenAIClient.cpp - Implementation of OpenAI API client functionality
 *
 * This file handles communication with the OpenAI API and manages the
 * request/response cycle. It includes functions for making HTTP calls via cURL,
 * handling responses, and updating the Notepad++ editor with AI-generated content.
 */

#include <windows.h>
#include "OpenAIClient.h"
#include "external_globals.h"
#include "EncodingUtils.h" // for toUTF8
#include <curl/curl.h>
#include "Sci_Position.h"
#include "Scintilla.h"
#include "PromptManager.h"     // for Prompt struct and related functions
#include "ResponseParsers.h"   // for different API response parsers and thinking section processing
#include "RequestFormatters.h" // for different API request formatters
#include <chrono>              // For timing API calls
#include <future>              // for async spinner responsiveness
#include <sstream>             // For string stream processing

/**
 * Streaming API response handling
 *
 * The plugin supports streaming responses from LLMs, where text is returned incrementally
 * rather than all at once. This provides a more interactive experience as the user can
 * see the response being generated in real-time.
 *
 * Streaming implementation uses Windows messages to safely communicate between the
 * background network thread and the UI thread. Each chunk of text is processed and
 * sent via the WM_OPENAI_STREAM_CHUNK message.
 */

// define custom message for streaming chunks
#define WM_OPENAI_STREAM_CHUNK (WM_APP + 100)

// static handle to direct streaming chunks
static HWND s_streamTargetScintilla = nullptr;

/**
 * Makes a POST request to the OpenAI API via cURL
 *
 * @param url The full API endpoint URL to call
 * @param proxy Optional proxy server to use (or "0" for no proxy)
 * @param request The JSON request body as a string
 * @param response Output parameter that will store the API response
 * @return true if the request was successful (200-level response), false otherwise
 */
bool callOpenAI(const std::string &url, const std::string &proxy, const std::string &request, std::string &response)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    struct curl_slist *headers = nullptr; // Headers vary by API
    std::string apiType = toUTF8(configAPIValue_responseType);

    // Content-Type is always JSON
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Add authentication headers based on API type
    if (apiType == "claude")
    {
        // Claude API uses x-api-key header
        std::string authHeader = "x-api-key: " + toUTF8(configAPIValue_secretKey);
        headers = curl_slist_append(headers, authHeader.c_str());
        // Claude also requires API version header
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    }
    else
    {
        // OpenAI and most others use Bearer token
        std::string authHeader = "Authorization: Bearer " + toUTF8(configAPIValue_secretKey);
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
 * Callback function for cURL to write response data
 *
 * This function is called by cURL when response data is received.
 * It appends the received data to the std::string pointer passed as userp.
 *
 * @param contents The received data buffer
 * @param size Always 1
 * @param nmemb The size of the data received
 * @param userp User-provided pointer (std::string for response data)
 * @return The number of bytes processed (should match nmemb on success)
 */
size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    std::string *pResponse = static_cast<std::string *>(userp);
    pResponse->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

/**
 * CURL write callback for streaming: posts each chunk to the main thread
 *
 * @param contents The received data buffer
 * @param size Always 1
 * @param nmemb The size of the data received
 * @param userp User-provided pointer (unused in this case)
 * @return The number of bytes processed (should match nmemb on success)
 */
size_t OpenAIStreamCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    // Avoid unused parameter warning
    (void)userp;

    size_t totalSize = size * nmemb;
    std::string chunk(static_cast<char *>(contents), totalSize);
    std::string content;

    try
    {
        // Handle streaming responses for different API formats
        if (!chunk.empty())
        {
            std::string apiType = toUTF8(configAPIValue_responseType);

            if (apiType == "openai")
            {
                // OpenAI format can have multiple lines, each starting with data:
                std::istringstream stream(chunk);
                std::string line;

                while (std::getline(stream, line))
                {
                    // Skip empty lines
                    if (line.empty() || line == "\r")
                    {
                        continue;
                    } // Process data: lines
                    if (line.find("data:") == 0)
                    {
                        // Extract the JSON part after "data: "
                        std::string jsonStr = line.substr(5);
                        // Remove any trailing \r if present
                        if (!jsonStr.empty() && jsonStr.back() == '\r')
                        {
                            jsonStr.pop_back();
                        }
                        if (jsonStr == " [DONE]" || jsonStr == "[DONE]" ||
                            jsonStr.find("[DONE]") != std::string::npos)
                        {
                            // End of stream marker for OpenAI
                            continue;
                        }

                        try
                        {
                            auto json = json::parse(jsonStr);
                            if (json.contains("choices") && json["choices"].size() > 0 &&
                                json["choices"][0].contains("delta") &&
                                json["choices"][0]["delta"].contains("content"))
                            {
                                content += json["choices"][0]["delta"]["content"].get<std::string>();
                            }
                        }
                        catch (...)
                        {
                            // Ignore parse errors for OpenAI stream chunks
                        }
                    }
                }

                // If we processed any content in the loop, return now
                if (!content.empty())
                {
                    auto *pChunk = new std::string(content);
                    PostMessage(nppData._nppHandle, WM_OPENAI_STREAM_CHUNK, 0, reinterpret_cast<LPARAM>(pChunk));
                    return totalSize;
                }
            }
            else if (apiType == "ollama")
            {
                // Ollama format: {"response":"text"}
                try
                {
                    auto json = json::parse(chunk);
                    if (json.contains("response"))
                    {
                        content = json["response"].get<std::string>();

                        // Direct handling for Ollama streaming to ensure delivery
                        if (!content.empty())
                        {
                            auto *pChunk = new std::string(content);
                            PostMessage(nppData._nppHandle, WM_OPENAI_STREAM_CHUNK, 0, reinterpret_cast<LPARAM>(pChunk));
                            return totalSize; // Return early after processing Ollama chunk
                        }
                    }

                    // Check for Ollama stream completion marker
                    if (json.contains("done") && json["done"].is_boolean() && json["done"].get<bool>())
                    {
                        // This is the final message indicating completion
                        // We can just return without posting a message
                        return totalSize;
                    }
                }
                catch (...)
                {
                    // Ignore parse errors for Ollama stream chunks
                }
            }
            else if (apiType == "claude")
            {
                // Claude format for events: {"type":"content_block_delta","delta":{"type":"text", "text":"text"}}
                try
                {
                    auto json = json::parse(chunk);
                    if (json.contains("type") && json["type"] == "content_block_delta" &&
                        json.contains("delta") && json["delta"].contains("text"))
                    {
                        content = json["delta"]["text"].get<std::string>();
                    }
                }
                catch (...)
                {
                    // Ignore parse errors for Claude stream chunks
                }
            } // If we successfully parsed content or this is raw text (and we haven't already handled it)
            if (!content.empty() || chunk.size() < 100)
            {
                // If we couldn't extract specific content, use the raw chunk (might be plain text)
                if (content.empty())
                {
                    content = chunk;

                    // Check for "data: [DONE]" in raw chunk and skip it
                    if (content == "data: [DONE]" || content == "data: [DONE]\n" ||
                        content == "data: [DONE]\r\n" || content.find("data: [DONE]") == 0)
                    {
                        // This is the OpenAI end marker, don't display it
                        return totalSize;
                    }
                }

                // Allocate and post to our message loop
                auto *pChunk = new std::string(content);
                PostMessage(nppData._nppHandle, WM_OPENAI_STREAM_CHUNK, 0, reinterpret_cast<LPARAM>(pChunk));
            }
        }
    }
    catch (...)
    {
        // Fallback for any unexpected errors - just send the raw chunk
        // But first check if it's a "[DONE]" marker
        if (chunk == "data: [DONE]" || chunk == "data: [DONE]\n" ||
            chunk == "data: [DONE]\r\n" || chunk.find("data: [DONE]") == 0)
        {
            // This is the OpenAI end marker, don't display it
            return totalSize;
        }

        auto *pChunk = new std::string(chunk);
        PostMessage(nppData._nppHandle, WM_OPENAI_STREAM_CHUNK, 0, reinterpret_cast<LPARAM>(pChunk));
    }

    return totalSize;
}

/**
 * Replace the currently selected text in a Scintilla editor
 *
 * @param curScintilla Handle to the current Scintilla editor instance
 * @param responseText The text that will replace the selected text
 */
void replaceSelected(HWND curScintilla, const std::string &responseText)
{
    // Get current selection range
    Sci_Position selStart = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
    Sci_Position selEnd = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
    // Set target range for replacement
    ::SendMessage(curScintilla, SCI_SETTARGETSTART, selStart, 0);
    ::SendMessage(curScintilla, SCI_SETTARGETEND, selEnd, 0);
    // Replace target with new text
    ::SendMessageA(curScintilla, SCI_REPLACETARGET, static_cast<WPARAM>(responseText.size()), reinterpret_cast<LPARAM>(responseText.c_str()));
}

/**
 * Display an error message when the instructions file cannot be read or found
 *
 * @param errorMessage The error message to display
 * @param errorCaption The caption for the error dialog
 */
void instructionsFileError(const WCHAR *errorMessage, const WCHAR *errorCaption)
{
    ::MessageBox(nppData._nppHandle, errorMessage, errorCaption, MB_ICONERROR);
}

/**
 * Implementation of the askChatGPT function
 *
 * This function is called from PluginDefinition.cpp. It handles the process of
 * sending a user-selected text to the OpenAI API and replacing the selection
 * with the AI-generated response.
 */
namespace OpenAIClientImpl
{
    void askChatGPT()
    {
        // Get the current Scintilla window
        int which = -1;
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
        if (which == -1)
            return;
        HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

        // Record start time for elapsed time calculation
        auto startTime = std::chrono::high_resolution_clock::now();

        // Show the loader dialog and make sure it's displayed before continuing
        _loaderDlg.doDialog();

        // Force a repaint and ensure dialog is visible by processing messages
        ::UpdateWindow(_loaderDlg.getHSelf());

        // Process all pending messages to make the dialog visible
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // Allow additional time for window to be shown
        ::Sleep(10);

        // Get selected text
        Sci_Position selStart = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
        Sci_Position selEnd = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
        Sci_Position selLen = selEnd - selStart;
        if (selLen <= 0)
        {
            _loaderDlg.display(false);
            instructionsFileError(L"No text selected.", L"NppOpenAI Error");
            return;
        }
        std::string selectedText(selLen, '\0');
        Sci_TextRangeFull tr;
        tr.chrg.cpMin = selStart;
        tr.chrg.cpMax = selEnd;
        tr.lpstrText = &selectedText[0];
        ::SendMessage(curScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

        // Parse prompts from instructions file
        std::vector<Prompt> prompts;
        parseInstructionsFile(instructionsFilePath, prompts);
        int promptIndex = 0;
        if (prompts.size() > 1)
        {
            // Before showing prompt dialog, hide the loader temporarily
            _loaderDlg.display(false);

            promptIndex = choosePrompt(nppData._nppHandle, prompts, -1);

            if (promptIndex < 0)
            {
                // User canceled, no need to show loader again
                return;
            }

            // Re-show the loader dialog as we're continuing
            _loaderDlg.doDialog();
            ::UpdateWindow(_loaderDlg.getHSelf());
        }
        std::wstring systemPrompt;
        if (!prompts.empty())
            systemPrompt = prompts[promptIndex].content;
        else
            systemPrompt = configAPIValue_instructions;

        // Parse parameters from configuration
        float temperature = configAPIValue_temperature != L"0" ? std::stof(toUTF8(configAPIValue_temperature)) : 1.0f;
        float topP = configAPIValue_topP != L"0" ? std::stof(toUTF8(configAPIValue_topP)) : 1.0f;
        int maxTokens = configAPIValue_maxTokens != L"0" ? std::stoi(toUTF8(configAPIValue_maxTokens)) : 0;
        float frequencyPenalty = configAPIValue_frequencyPenalty != L"0" ? std::stof(toUTF8(configAPIValue_frequencyPenalty)) : 0.0f;
        float presencePenalty = configAPIValue_presencePenalty != L"0" ? std::stof(toUTF8(configAPIValue_presencePenalty)) : 0.0f; // Get the appropriate request formatter based on the configured response type
        auto formatter = RequestFormatters::getFormatterForEndpoint(configAPIValue_responseType);

        // Format request using the selected formatter
        std::string request = formatter(
            configAPIValue_model,
            multiByteToWideChar(selectedText.c_str()),
            systemPrompt,
            temperature,
            maxTokens,
            topP,
            frequencyPenalty,
            presencePenalty); // Check if streaming is enabled
        bool streaming = (configAPIValue_streaming == L"1");
        if (streaming)
        {
            auto pos = request.rfind('}');
            if (pos != std::string::npos)
            {
                if (configAPIValue_responseType == L"openai")
                {
                    request.insert(pos, ",\"stream\":true");
                }
                else if (configAPIValue_responseType == L"ollama")
                {
                    // Make sure it's exactly stream:true as required by Ollama
                    request.insert(pos, ",\"stream\":true");
                }
                else if (configAPIValue_responseType == L"claude")
                {
                    request.insert(pos, ",\"stream\":true");
                }
            }
        }
        else if (configAPIValue_responseType == L"ollama")
        {
            // Ensure stream is explicitly set to false for non-streaming Ollama requests
            auto pos = request.rfind('}');
            if (pos != std::string::npos)
            {
                request.insert(pos, ",\"stream\":false");
            }
        }

        if (debugMode)
        {
            // Log request for debugging
            std::wstring debugMsg = L"Streaming request: ";
            debugMsg += multiByteToWideChar(request.c_str());
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)L"Making streaming request...");
        }

        // Prepare URL and proxy, supporting custom endpoints for self-hosted LLMs
        std::string url = toUTF8(configAPIValue_baseURL);
        std::string chatRoute = toUTF8(configAPIValue_chatRoute);

        // Ensure base URL ends with a trailing slash
        if (!url.empty() && url.back() != '/')
        {
            url += '/';
        }

        // Check if URL already includes the chat completions endpoint
        bool hasCompletionsEndpoint =
            url.find(chatRoute) != std::string::npos;

        if (!hasCompletionsEndpoint && !chatRoute.empty())
        {
            url += chatRoute;
        }
        else if (hasCompletionsEndpoint)
        {
            if (url.size() > 1 && url.back() == '/' &&
                url.substr(url.size() - chatRoute.size() - 1).find(chatRoute + "/") != std::string::npos)
            {
                url.pop_back();
            }
        }
        std::string proxy = toUTF8(configAPIValue_proxyURL); // Stream or standard call
        std::string response;
        bool ok = true;
        if (streaming)
        {
            // prepare streaming curl
            CURL *curl = curl_easy_init();
            struct curl_slist *headers = nullptr;
            std::string apiType = toUTF8(configAPIValue_responseType);

            // Set required headers
            headers = curl_slist_append(headers, "Content-Type: application/json"); // Add Accept header for handling streaming response
            if (apiType == "openai" || apiType == "ollama")
            {
                headers = curl_slist_append(headers, "Accept: text/event-stream");
            }

            if (apiType == "claude")
            {
                std::string authHeader = "x-api-key: " + toUTF8(configAPIValue_secretKey);
                headers = curl_slist_append(headers, authHeader.c_str());
                headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
            }
            else
            {
                std::string authHeader = "Authorization: Bearer " + toUTF8(configAPIValue_secretKey);
                headers = curl_slist_append(headers, authHeader.c_str());
            }

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
            s_streamTargetScintilla = curScintilla;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIStreamCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);

            // Add proper headers for event-stream handling
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

            auto futureRes = std::async(std::launch::async, [curl, &proxy, &url]()
                                        {
                                            // Set proxy if provided
                                            if (!proxy.empty() && proxy != "0") {
                                                curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
                                            }
                                            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                                            CURLcode r = curl_easy_perform(curl);
                                            return r; }); // Reuse the existing selStart variable instead of declaring a new one
            selStart = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
            ::SendMessage(curScintilla, SCI_SETTARGETSTART, selStart, 0);
            ::SendMessage(curScintilla, SCI_SETTARGETEND, ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0), 0); // Handle "keep my question" option
            std::string initialText = "";
            if (isKeepQuestion)
            {
                // Use only a single newline for Ollama responses to reduce excessive spacing
                if (configAPIValue_responseType == L"ollama")
                {
                    initialText = selectedText + "\n";
                }
                else
                {
                    initialText = selectedText + "\n\n";
                }
            }
            ::SendMessage(curScintilla, SCI_REPLACETARGET, initialText.length(), reinterpret_cast<LPARAM>(initialText.c_str()));

            // Set cursor at the end of the replacement text to ensure streaming content is appended correctly
            Sci_Position textEnd = selStart + initialText.length();
            ::SendMessage(curScintilla, SCI_SETSEL, textEnd, textEnd);
            while (futureRes.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready)
            {
                MSG msgLocal; // Renamed to avoid shadowing the outer msg variable
                while (::PeekMessage(&msgLocal, NULL, 0, 0, PM_REMOVE))
                {
                    if (msgLocal.message == WM_OPENAI_STREAM_CHUNK)
                    {
                        auto *p = reinterpret_cast<std::string *>(msgLocal.lParam);

                        /**
                         * Handle thinking sections in streaming responses
                         *
                         * When streaming is enabled, each chunk is processed separately
                         * to filter out <think>...</think> sections. Due to the nature of
                         * streaming, sections that span multiple chunks may be partially retained.
                         *
                         * If complete filtering is needed, use non-streaming mode instead.
                         * The user can control this behavior with the show_reasoning=0|1 setting.
                         */
                        std::string processedChunk = ResponseParsers::processThinkingSections(*p);

                        // Get current cursor position
                        Sci_Position currentPos = ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
                        // Insert text at cursor position
                        ::SendMessageA(curScintilla, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(processedChunk.c_str()));
                        // Move cursor to the end of the newly inserted text
                        Sci_Position newPos = currentPos + processedChunk.length();
                        ::SendMessage(curScintilla, SCI_GOTOPOS, newPos, 0);
                        delete p;
                    }
                    ::TranslateMessage(&msgLocal);
                    ::DispatchMessage(&msgLocal);
                }
                ::Sleep(10);
            }
            CURLcode res = futureRes.get();

            // Give a small delay to ensure all chunks have been received and queued
            ::Sleep(50);

            // After curl finishes, process any remaining message chunks that might be in the queue
            // This addresses an issue where the last part of streaming responses can be cut off
            MSG finalChunks;
            while (::PeekMessage(&finalChunks, NULL, WM_OPENAI_STREAM_CHUNK, WM_OPENAI_STREAM_CHUNK, PM_REMOVE))
            {
                if (finalChunks.message == WM_OPENAI_STREAM_CHUNK)
                {
                    auto *p = reinterpret_cast<std::string *>(finalChunks.lParam);
                    std::string processedChunk = ResponseParsers::processThinkingSections(*p);

                    // Get current cursor position
                    Sci_Position currentPos = ::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
                    // Insert text at cursor position
                    ::SendMessageA(curScintilla, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(processedChunk.c_str()));
                    // Move cursor to the end of the newly inserted text
                    Sci_Position newPos = currentPos + processedChunk.length();
                    ::SendMessage(curScintilla, SCI_GOTOPOS, newPos, 0);
                    delete p;
                }
            }

            // Get the HTTP status code to check for API errors
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            // Check both CURL status and HTTP status code
            ok = (res == CURLE_OK && (http_code >= 200 && http_code < 300));

            // If there was an HTTP error but CURL succeeded, capture the error response
            if (res == CURLE_OK && (http_code < 200 || http_code >= 300))
            {
                // For streaming errors, we might have received an error message
                // Check message queue for any error content
                MSG errorMsg;
                while (::PeekMessage(&errorMsg, NULL, WM_OPENAI_STREAM_CHUNK, WM_OPENAI_STREAM_CHUNK, PM_REMOVE))
                {
                    auto *p = reinterpret_cast<std::string *>(errorMsg.lParam);
                    response += *p;
                    delete p;
                }
            }
        }
        else
        {
            ok = callOpenAI(url, proxy, request, response);
        }
        if (!ok)
        {
            _loaderDlg.display(false);

            std::wstring errorMsg = L"Failed to connect OpenAI API.";
            try
            {
                auto errorJson = json::parse(response);
                if (errorJson.contains("error"))
                {
                    if (errorJson["error"].contains("message"))
                    {
                        std::string errorDetails = errorJson["error"]["message"].get<std::string>();
                        std::wstring wideError = multiByteToWideChar(errorDetails.c_str());
                        errorMsg = L"API Error: " + wideError;
                    }
                    if (debugMode && errorJson["error"].contains("type"))
                    {
                        std::string errorType = errorJson["error"]["type"].get<std::string>();
                        std::wstring wideType = multiByteToWideChar(errorType.c_str());
                        errorMsg += L"\nType: " + wideType;
                    }
                }
            }
            catch (...)
            {
                if (debugMode && !response.empty())
                {
                    std::wstring wideResponse = multiByteToWideChar(response.c_str());
                    errorMsg += L"\nRaw response: " + wideResponse;
                }
            }

            instructionsFileError(errorMsg.c_str(), L"NppOpenAI Error");
            return;
        }

        // For non-streaming mode, we need to parse the response and update the text
        if (!streaming)
        {
            auto parser = ResponseParsers::getParserForEndpoint(configAPIValue_responseType);
            std::string replyText = parser(response);

            if (debugMode && replyText.find("[Failed to parse") == 0)
            {
                replyText += "\nRaw response: " + response;
            }

            std::string finalText;
            if (isKeepQuestion)
            {
                // Use only a single newline for Ollama responses to reduce excessive spacing
                if (configAPIValue_responseType == L"ollama")
                {
                    finalText = selectedText + "\n" + replyText;
                }
                else
                {
                    finalText = selectedText + "\n\n" + replyText;
                }
            }
            else
            {
                finalText = replyText;
            }

            // Replace selected text with the response
            replaceSelected(curScintilla, finalText);
        } // In streaming mode, the text is already in the editor

        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        double elapsedSeconds = elapsedMilliseconds / 1000.0;

        if (debugMode)
        {
            TCHAR timeMsg[128];
            swprintf(timeMsg, 128, TEXT("API call completed in %.1f seconds"), elapsedSeconds);
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)timeMsg);
        }

        _loaderDlg.display(false);
    }
} // namespace OpenAIClientImpl
