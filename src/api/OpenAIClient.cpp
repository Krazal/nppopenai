/**
 * OpenAIClient.cpp - Implementation of OpenAI API client functionality
 *
 * This file handles communication with various LLM APIs and manages the
 * request/response cycle. It includes functions for making HTTP calls via cURL,
 * handling responses, and updating the Notepad++ editor with AI-generated content.
 * The file implements text replacement behavior based on the "Keep my question" setting,
 * which determines whether the original user query is preserved in the response output.
 */

#include <windows.h>
#include "OpenAIClient.h"
#include "core/external_globals.h"
#include "EncodingUtils.h" // for toUTF8
#include <curl/curl.h>
#include "Sci_Position.h"
#include "Scintilla.h"
#include "config/PromptManager.h" // for Prompt struct and related functions
#include "ResponseParsers.h"      // for different API response parsers and thinking section processing
#include "RequestFormatters.h"    // for different API request formatters
#include <chrono>                 // For timing API calls
#include <future>                 // for async spinner responsiveness
#include <sstream>                // For string stream processing

// New modular components
#include "HTTPClient.h"
#include "StreamParser.h"
#include "APIUtils.h"
#include "editor/EditorInterface.h"

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

// Global handle to direct streaming chunks (defined in external_globals.h)
HWND s_streamTargetScintilla = nullptr;

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
 * @param userp User-provided pointer (window handle)
 * @return The number of bytes processed (should match nmemb on success)
 */
size_t OpenAIStreamCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    // We need to use userp to know where to send messages
    HWND targetWindow = static_cast<HWND>(userp);
    if (!targetWindow)
    {
        targetWindow = nppData._nppHandle; // Fallback to main Notepad++ window
    }

    size_t totalSize = size * nmemb;
    std::string chunk(static_cast<char *>(contents), totalSize);
    std::string content;

    try
    {
        // Handle streaming responses for different API formats
        if (!chunk.empty())
        {
            std::string apiType = toUTF8(configAPIValue_responseType);
            content = StreamParser::extractContent(chunk, apiType);
        }

        // If we have content to display or this might be plain text (and short)
        if (!content.empty() || (chunk.size() < 100 && !StreamParser::isCompletionMarker(chunk)))
        {
            // If we couldn't extract content but have a small chunk, use the raw chunk
            if (content.empty() && chunk.size() < 100 && !StreamParser::isCompletionMarker(chunk))
            {
                content = chunk;
            }

            // Only send non-empty content
            if (!content.empty())
            {
                // Send content directly to the Scintilla editor
                // This bypasses the problematic PostMessage approach
                if (s_streamTargetScintilla && IsWindow(s_streamTargetScintilla))
                {
                    // Use SCI_REPLACESEL to insert the content at the current position
                    ::SendMessage(s_streamTargetScintilla, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(content.c_str()));
                }
            }
        }
    }
    catch (...)
    {
        // Fallback for any unexpected errors - just send the raw chunk
        // But first check if it's a completion marker
        if (StreamParser::isCompletionMarker(chunk))
        {
            return totalSize;
        }

        // Send content directly to the Scintilla editor (fallback path)
        if (s_streamTargetScintilla && IsWindow(s_streamTargetScintilla))
        {
            ::SendMessage(s_streamTargetScintilla, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(chunk.c_str()));
        }
    }

    return totalSize;
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
    /**
     * Display an error message with API error details
     */
    void displayApiError(const std::string &response)
    {
        std::wstring errorMsg = L"Failed to connect to API.";
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
            }
        }
        catch (...)
        {
            // Error parsing response - use default error message
        }

        instructionsFileError(errorMsg.c_str(), L"NppOpenAI Error");
    }

    void askChatGPT()
    {
        // Record start time for elapsed time calculation
        auto startTime = std::chrono::high_resolution_clock::now();

        // Get current editor
        HWND curScintilla = EditorInterface::getCurrentScintilla();
        if (!curScintilla)
        {
            return;
        }

        // Get selected text
        std::string selectedText = EditorInterface::getSelectedText(curScintilla);
        if (selectedText.empty())
        {
            instructionsFileError(L"No text selected.", L"NppOpenAI Error");
            return;
        } // Get system prompt - handle multiple prompts case BEFORE showing loader
        std::wstring systemPrompt = APIUtils::getSystemPrompt();

        // If multiple prompts are available, show selection dialog
        if (systemPrompt == L"MULTIPLE_PROMPTS_AVAILABLE")
        {
            // Parse prompts from instructions file
            std::vector<Prompt> prompts;
            parseInstructionsFile(instructionsFilePath, prompts);

            if (prompts.size() > 1)
            {
                // Show prompt selection dialog
                static int lastUsedPromptIndex = -1; // Local static variable to remember last choice
                int selectedPromptIndex = choosePrompt(nppData._nppHandle, prompts, lastUsedPromptIndex);

                if (selectedPromptIndex == -1)
                {
                    // User canceled the dialog
                    return;
                }

                // Remember the user's choice for next time
                lastUsedPromptIndex = selectedPromptIndex;
                systemPrompt = prompts[selectedPromptIndex].content;
            }
            else
            {
                // Fallback to default if something went wrong
                systemPrompt = configAPIValue_instructions;
            }
        }

        // NOW show the loader dialog after prompt selection is complete
        _loaderDlg.doDialog();
        _loaderDlg.resetTimer();
        ::UpdateWindow(_loaderDlg.getHSelf());

        // Process pending messages to make dialog visible
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        ::Sleep(10);

        // Check if streaming is enabled
        bool streaming = (configAPIValue_streaming == L"1"); // Prepare API request with all necessary parameters
        std::string request = APIUtils::prepareApiRequest(
            selectedText,
            systemPrompt,
            configAPIValue_model,
            configAPIValue_responseType,
            std::stof(toUTF8(configAPIValue_temperature)),
            std::stoi(toUTF8(configAPIValue_maxTokens)),
            std::stof(toUTF8(configAPIValue_topP)),
            std::stof(toUTF8(configAPIValue_frequencyPenalty)),
            std::stof(toUTF8(configAPIValue_presencePenalty)), streaming);

        // Build API URL with base URL and chat route
        std::string baseUrl = toUTF8(configAPIValue_baseURL);
        std::string chatRoute = toUTF8(configAPIValue_chatRoute);
        std::string url = APIUtils::buildApiUrl(baseUrl, chatRoute);
        std::string proxy = toUTF8(configAPIValue_proxyURL);
        std::string apiType = toUTF8(configAPIValue_responseType);
        std::string secretKey = toUTF8(configAPIValue_secretKey);

        std::string response;
        bool ok = true;
        if (streaming)
        { // Debug streaming request details
            if (debugMode)
            {
                std::wstring debugMsg = L"Streaming enabled, URL: ";
                debugMsg += multiByteToWideChar(url.c_str());
                ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)debugMsg.c_str());
            }

            // We no longer need to set s_streamTargetScintilla, the message handler gets the current Scintilla            // Prepare editor for streaming response using the proper interface
            EditorInterface::prepareForStreamingResponse(curScintilla, selectedText, isKeepQuestion, configAPIValue_responseType);

            // Store the current Scintilla handle for the streaming process
            s_streamTargetScintilla = curScintilla; // Perform streaming request with the correct message type
            ok = HTTPClient::performStreamingRequest(url, request, apiType, secretKey,
                                                     nppData._nppHandle, // Use nppData._nppHandle as target for messages
                                                     WM_OPENAI_STREAM_CHUNK, proxy);
        }
        else
        {
            // For non-streaming mode, perform regular request
            ok = HTTPClient::performRequest(url, request, response, apiType, secretKey, proxy);
        }
        if (!ok)
        {
            _loaderDlg.display(false);

            // Parse and display API error
            std::wstring errorMsg = L"Request failed";
            try
            {
                if (!response.empty())
                {
                    nlohmann::json errorJson = nlohmann::json::parse(response);
                    if (errorJson.contains("error"))
                    {
                        if (errorJson["error"].contains("message"))
                        {
                            std::string errorDetails = errorJson["error"]["message"].get<std::string>();
                            std::wstring wideError = multiByteToWideChar(errorDetails.c_str());
                            errorMsg = L"API Error: " + wideError;
                        }
                    }
                }
            }
            catch (...)
            {
                // Error parsing response - use default error message
            }

            instructionsFileError(errorMsg.c_str(), L"NppOpenAI Error");
            return;
        } // Handle non-streaming response
        if (!streaming)
        {
            // Parse response and extract content using the correct parser
            auto parser = ResponseParsers::getParserForEndpoint(configAPIValue_responseType);
            std::string extractedContent = parser(response);
            if (!extractedContent.empty())
            {
                // For non-streaming with keepQuestion, mimic streaming behavior:
                // Keep the question in place and append the response after it
                if (isKeepQuestion)
                {
                    // Move cursor to end of selection (after the question)
                    Sci_Position selEnd = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
                    ::SendMessage(curScintilla, SCI_SETSEL, selEnd, selEnd);

                    // Add appropriate spacing and the response after the question
                    std::string responseText;
                    if (configAPIValue_responseType == L"ollama")
                    {
                        responseText = "\n" + extractedContent;
                    }
                    else
                    {
                        responseText = "\n\n" + extractedContent;
                    }

                    // Insert the response after the question
                    EditorInterface::insertTextAtCursor(curScintilla, responseText);

                    // Debug output to verify behavior
                    if (debugMode)
                    {
                        std::string debugMsg = "Non-streaming: Inserted response after question (like streaming mode)";
                        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)debugMsg.c_str());
                    }
                }
                else
                {
                    // Replace the selected text entirely with the response
                    EditorInterface::replaceSelectedText(curScintilla, extractedContent);
                }
            }
            else
            {
                instructionsFileError(L"Failed to parse API response", L"NppOpenAI Error");
                _loaderDlg.display(false);
                return;
            }
        }
        // For streaming mode, the text is already in the editor through the callback        // Calculate and display elapsed time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        double elapsedSeconds = elapsedMilliseconds / 1000.0;

        // Show timing in status bar
        TCHAR timeMsg[128];
        swprintf(timeMsg, 128, TEXT("API call completed in %.1f seconds"), elapsedSeconds);
        ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)timeMsg);

        _loaderDlg.display(false);
    }
} // namespace OpenAIClientImpl
