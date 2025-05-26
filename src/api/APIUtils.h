#pragma once
#include <string>

/**
 * APIUtils - A module for handling API-related utilities
 *
 * This namespace contains functions for building API URLs, fetching system
 * prompts, and preparing API requests for different LLM providers.
 */
namespace APIUtils
{
    // Build API URL with proper endpoints
    std::string buildApiUrl(const std::string &baseUrl, const std::string &chatRoute);

    // Get system prompt from configuration or selected file
    std::wstring getSystemPrompt();

    // Create API request with all necessary parameters
    std::string prepareApiRequest(
        const std::string &selectedText,
        const std::wstring &systemPrompt,
        const std::wstring &model,
        const std::wstring &responseType,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        bool streaming);
}
