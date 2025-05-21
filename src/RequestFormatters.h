#ifndef REQUEST_FORMATTERS_H
#define REQUEST_FORMATTERS_H

#include <string>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * This file defines request formatter functions for different API endpoints
 * Each formatter knows how to create an API-specific request payload for a particular LLM backend
 */

namespace RequestFormatters
{
    // Formatter function type definition
    using FormatterFunction = std::function<std::string(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty)>;

    /**
     * Format request for standard OpenAI-compatible API
     * Format: {"model": "...", "messages": [{"role": "system", "content": "..."}, {"role": "user", "content": "..."}]}
     */
    std::string formatOpenAIRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty);

    /**
     * Format request for Ollama native API
     * Format: {"model": "...", "prompt": "...", "system": "...", "temperature": ...}
     */
    std::string formatOllamaRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty);

    /**
     * Format request for Anthropic Claude API
     * Format: {"model": "...", "messages": [{"role": "user", "content": "..."}, ...], "system": "..."}
     */
    std::string formatClaudeRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty);

    /**
     * Get the appropriate formatter function for an endpoint
     * @param endpointType The endpoint type identifier from config
     * @return The formatter function to use
     */
    FormatterFunction getFormatterForEndpoint(const std::wstring &endpointType);
}

#endif // REQUEST_FORMATTERS_H
