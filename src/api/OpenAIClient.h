/**
 * OpenAIClient.h - Interface for communicating with LLM APIs
 *
 * This file declares functions for sending requests to various LLM APIs (OpenAI, Claude, Ollama),
 * processing responses, and updating the editor with the generated content.
 * It provides high-level functions for plugin commands and callback functions
 * for the HTTP client module.
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
     * Main plugin function that sends current text selection to an LLM API and processes the response
     *
     * This function:
     * 1. Gets the currently selected text in Notepad++
     * 2. Shows a loading dialog
     * 3. Prepares an API request with the selected text and configured parameters
     * 4. Sends the request to the configured LLM API
     * 5. Processes the response
     * 6. Updates the editor with the generated text
     */
    void askChatGPT();    /**
     * Display an error message with API error details
     *
     * @param errorResponse The error response from the API
     */
    void displayApiError(const std::string &errorResponse);
}

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
 * Callback function for streaming responses
 *
 * @param contents Pointer to the response data buffer
 * @param size Always 1
 * @param nmemb Number of bytes in the data buffer
 * @param userp User-provided pointer (window handle)
 * @return Number of bytes processed
 */
size_t OpenAIStreamCallback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * Shows an error message when the instructions file cannot be accessed
 *
 * @param errorMessage Error message to display
 * @param errorCaption Caption for the error dialog
 */
void instructionsFileError(const WCHAR *errorMessage, const WCHAR *errorCaption);
