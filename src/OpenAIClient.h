/**
 * OpenAIClient.h - Interface for communicating with OpenAI API
 *
 * This file declares functions for sending requests to the OpenAI API,
 * processing responses, and updating the editor with the generated content.
 * It provides both high-level functions for plugin commands and low-level
 * utilities for HTTP communication.
 */

#pragma once
#include <windows.h>
#include <string>

#ifdef RC_INVOKED
// Skip JSON library for Resource Compiler
#else
#include <nlohmann/json.hpp>
#endif
using json = nlohmann::json;

namespace OpenAIClientImpl
{
    /**
     * Main plugin function that sends current text selection to OpenAI and processes the response
     *
     * This function:
     * 1. Gets the currently selected text in Notepad++
     * 2. Shows a loading dialog
     * 3. Prepares an API request with the selected text and configured parameters
     * 4. Sends the request to the OpenAI API
     * 5. Processes the response
     * 6. Updates the editor with the generated text
     * 7. Updates chat history if chat mode is enabled
     */
    void askChatGPT();
}

/**
 * Low-level function to make HTTP requests to the OpenAI API using cURL
 *
 * @param url The full API endpoint URL
 * @param proxy Proxy URL if needed (or "0" for no proxy)
 * @param request The JSON request body as a string
 * @param response Output parameter that will be filled with the API response
 * @return true if request was successful (HTTP 2xx), false otherwise
 */
bool callOpenAI(const std::string &url, const std::string &proxy, const std::string &request, std::string &response);

/**
 * Callback function used by cURL to receive HTTP response data
 *
 * @param contents Pointer to the response data buffer
 * @param size Always 1
 * @param nmemb Number of bytes in the data buffer
 * @param userp User-provided pointer (used for the response string)
 * @return Number of bytes processed
 */
size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * Replaces the currently selected text in a Scintilla editor instance
 *
 * @param curScintilla Handle to the current Scintilla editor
 * @param responseText Text to replace the current selection with
 */
void replaceSelected(HWND curScintilla, const std::string &responseText);

/**
 * Shows an error message when the instructions file cannot be accessed
 *
 * @param errorMessage Error message to display
 * @param errorCaption Caption for the error dialog
 */
void instructionsFileError(const WCHAR *errorMessage, const WCHAR *errorCaption);
