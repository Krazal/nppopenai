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
#include "PromptManager.h" // for Prompt struct and related functions
#include <chrono>          // For timing API calls

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

    struct curl_slist *headers = nullptr;
    // Authorization header
    std::string authHeader = "Authorization: Bearer " + toUTF8(configAPIValue_secretKey);
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

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

    CURLcode res = curl_easy_perform(curl);

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

        // Build OpenAI API request JSON
        json reqJson;
        reqJson["model"] = toUTF8(configAPIValue_model);
        reqJson["messages"] = json::array();
        if (!systemPrompt.empty())
        {
            reqJson["messages"].push_back({{"role", "system"}, {"content", toUTF8(systemPrompt)}});
        }
        reqJson["messages"].push_back({{"role", "user"}, {"content", selectedText}});
        if (configAPIValue_temperature != L"0")
            reqJson["temperature"] = std::stod(toUTF8(configAPIValue_temperature));
        if (configAPIValue_topP != L"0")
            reqJson["top_p"] = std::stod(toUTF8(configAPIValue_topP));
        if (configAPIValue_maxTokens != L"0")
            reqJson["max_tokens"] = std::stoi(toUTF8(configAPIValue_maxTokens));
        if (configAPIValue_frequencyPenalty != L"0")
            reqJson["frequency_penalty"] = std::stod(toUTF8(configAPIValue_frequencyPenalty));
        if (configAPIValue_presencePenalty != L"0")
            reqJson["presence_penalty"] = std::stod(toUTF8(configAPIValue_presencePenalty));

        // Prepare URL and proxy
        std::string url = toUTF8(configAPIValue_baseURL);
        if (url.back() != '/')
            url += '/';
        url += "v1/chat/completions";
        std::string proxy = toUTF8(configAPIValue_proxyURL);

        // Call OpenAI API
        std::string response;
        bool ok = callOpenAI(url, proxy, reqJson.dump(), response);
        if (!ok)
        {
            _loaderDlg.display(false);

            // Try to extract error details from the response JSON
            std::wstring errorMsg = L"Failed to contact OpenAI API.";
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
                // If we can't parse the response as JSON, and debug mode is on,
                // show the raw response
                if (debugMode && !response.empty())
                {
                    std::wstring wideResponse = multiByteToWideChar(response.c_str());
                    errorMsg += L"\nRaw response: " + wideResponse;
                }
            }

            instructionsFileError(errorMsg.c_str(), L"NppOpenAI Error");
            return;
        }

        // Parse response JSON
        std::string replyText;
        try
        {
            auto respJson = json::parse(response);
            if (respJson.contains("choices") && respJson["choices"].is_array() && !respJson["choices"].empty())
            {
                replyText = respJson["choices"][0]["message"]["content"].get<std::string>();
            }
            else
            {
                // Show more detailed error in debug mode
                if (debugMode)
                {
                    replyText = "[No valid response from OpenAI. Response: " + response + "]";
                }
                else
                {
                    replyText = "[No response from OpenAI]";
                }
            }
        }
        catch (const std::exception &e)
        {
            if (debugMode)
            {
                replyText = "[Failed to parse OpenAI response: ";
                replyText += e.what();
                replyText += "\nResponse: " + response + "]";
            }
            else
            {
                replyText = "[Failed to parse OpenAI response]";
            }
        }

        // Format the response based on "keep my question" preference
        std::string finalText;
        if (isKeepQuestion)
        {
            finalText = selectedText + "\n\n" + replyText;
        }
        else
        {
            finalText = replyText;
        }

        // Replace selected text with response
        replaceSelected(curScintilla, finalText); // Calculate elapsed time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        double elapsedSeconds = elapsedMilliseconds / 1000.0;

        // If in debug mode, show elapsed time in a status message
        if (debugMode)
        {
            TCHAR timeMsg[128];
            swprintf(timeMsg, 128, TEXT("API call completed in %.1f seconds"), elapsedSeconds);
            ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)timeMsg);
        }

        // Close the loader dialog when done
        _loaderDlg.display(false);
    }
} // namespace OpenAIClientImpl
