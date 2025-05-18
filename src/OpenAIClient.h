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
    // High-level API: send current selection to OpenAI and replace with response
    void askChatGPT();
}

// Low-level call via cURL
bool callOpenAI(const std::string &url, const std::string &proxy, const std::string &request, std::string &response);

// Callback for cURL writes
size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp);

// Replace the current selection in the given Scintilla handle
void replaceSelected(HWND curScintilla, const std::string &responseText);

// Error helper when instructions file can't be read
void instructionsFileError(const WCHAR *errorMessage, const WCHAR *errorCaption);
