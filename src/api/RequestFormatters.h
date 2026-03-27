/* RequestFormatters.h - Implements request formatting for different API formats */

#ifndef REQUEST_FORMATTERS_H
#define REQUEST_FORMATTERS_H

#include <string>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace RequestFormatters
{
    // Formatter function type definition
    using FormatterFunction = std::function<std::string(
        const std::wstring& model,
        const std::wstring& prompt,
        const std::wstring& systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        const std::wstring& keepAlive)>; // keepAlive passed as string (e.g. "3600", "10m", "24h")

    /**
     * Format request for standard OpenAI-compatible API
     */
    std::string formatOpenAIRequest(
        const std::wstring& model,
        const std::wstring& prompt,
        const std::wstring& systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        const std::wstring& keepAlive);

    /**
     * Format request for Ollama native API
     */
    std::string formatOllamaRequest(
        const std::wstring& model,
        const std::wstring& prompt,
        const std::wstring& systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        const std::wstring& keepAlive);

    /**
     * Format request for Anthropic Claude API
     */
    std::string formatClaudeRequest(
        const std::wstring& model,
        const std::wstring& prompt,
        const std::wstring& systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        const std::wstring& keepAlive);

    /**
     * Format request for simple completion API
     */
    std::string formatSimpleRequest(
        const std::wstring& model,
        const std::wstring& prompt,
        const std::wstring& systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty,
        const std::wstring& keepAlive);

    /**
     * Get the appropriate formatter function for an endpoint
     */
    FormatterFunction getFormatterForEndpoint(const std::wstring& endpointType);
}

#endif // REQUEST_FORMATTERS_H