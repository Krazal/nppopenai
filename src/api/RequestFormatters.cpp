/**
 * RequestFormatters.cpp - Implements request formatting for different API formats
 *
 * This file contains formatter functions for creating request payloads for
 * different API endpoints (OpenAI, Ollama native, etc.). Each formatter
 * knows how to format the request for a specific API.
 *
 * Support for different API formats:
 * - OpenAI format: {"model":"...", "messages":[{"role":"system","content":"..."},{"role":"user","content":"..."}]}
 * - Ollama format: {"model":"...", "prompt":"...", "system":"...", "temperature":...}
 * - Claude format: {"model":"...", "messages":[{"role":"user","content":"..."}], "system":"..."}
 *
 * This design allows users to connect to various language model backends
 * without requiring an OpenAI-compatible adapter or proxy.
 */

#include "RequestFormatters.h"
#include "utils/EncodingUtils.h" // for toUTF8
#include <string>
#include <stdexcept>
#include <sstream>

namespace RequestFormatters
{
    std::string formatOpenAIRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty)
    {
        json requestJson;

        // Convert wstring to UTF-8 string
        std::string modelStr = toUTF8(model);
        std::string promptStr = toUTF8(prompt);
        std::string systemPromptStr = toUTF8(systemPrompt);

        // Basic request structure
        requestJson["model"] = modelStr;

        // Build messages array
        json messagesArray = json::array();

        // Add system message if not empty
        if (!systemPromptStr.empty())
        {
            messagesArray.push_back({{"role", "system"},
                                     {"content", systemPromptStr}});
        }

        // Add user message
        messagesArray.push_back({{"role", "user"},
                                 {"content", promptStr}});

        requestJson["messages"] = messagesArray;

        // Add parameters if they have non-default values
        if (temperature != 1.0f)
        {
            requestJson["temperature"] = temperature;
        }

        if (maxTokens > 0)
        {
            requestJson["max_tokens"] = maxTokens;
        }

        if (topP != 1.0f)
        {
            requestJson["top_p"] = topP;
        }

        if (frequencyPenalty != 0.0f)
        {
            requestJson["frequency_penalty"] = frequencyPenalty;
        }

        if (presencePenalty != 0.0f)
        {
            requestJson["presence_penalty"] = presencePenalty;
        }

        return requestJson.dump();
    }
    std::string formatOllamaRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty)
    {
        // Mark unused parameters to avoid compiler warnings
        (void)presencePenalty; // Ollama doesn't use presence_penalty

        json requestJson;

        // Convert wstring to UTF-8 string
        std::string modelStr = toUTF8(model);
        std::string promptStr = toUTF8(prompt);
        std::string systemPromptStr = toUTF8(systemPrompt);

        // Ollama uses different parameter names
        requestJson["model"] = modelStr;
        requestJson["prompt"] = promptStr;

        // Add system prompt if not empty
        if (!systemPromptStr.empty())
        {
            requestJson["system"] = systemPromptStr;
        }

        // Ollama parameters (use only those that are supported)
        if (temperature != 1.0f)
        {
            requestJson["temperature"] = temperature;
        }

        // Ollama uses num_predict instead of max_tokens
        if (maxTokens > 0)
        {
            requestJson["num_predict"] = maxTokens;
        }

        // Ollama supports top_p
        if (topP != 1.0f)
        {
            requestJson["top_p"] = topP;
        }

        // Ollama doesn't support frequency_penalty and presence_penalty
        // but has optional frequency_penalty called repeat_penalty
        if (frequencyPenalty != 0.0f)
        {
            requestJson["repeat_penalty"] = 1.0f + frequencyPenalty; // Approximate conversion
        }

        return requestJson.dump();
    }
    std::string formatClaudeRequest(
        const std::wstring &model,
        const std::wstring &prompt,
        const std::wstring &systemPrompt,
        float temperature,
        int maxTokens,
        float topP,
        float frequencyPenalty,
        float presencePenalty)
    {
        // Mark unused parameters to avoid compiler warnings
        (void)frequencyPenalty; // Claude doesn't use frequency_penalty
        (void)presencePenalty;  // Claude doesn't use presence_penalty

        json requestJson;

        // Convert wstring to UTF-8 string
        std::string modelStr = toUTF8(model);
        std::string promptStr = toUTF8(prompt);
        std::string systemPromptStr = toUTF8(systemPrompt);

        // Claude API structure
        requestJson["model"] = modelStr;

        // Build messages array (Claude has slightly different format)
        json messagesArray = json::array();

        // Add user message
        messagesArray.push_back({{"role", "user"},
                                 {"content", promptStr}});

        requestJson["messages"] = messagesArray;

        // Claude uses 'system' field at the top level for system prompt
        if (!systemPromptStr.empty())
        {
            requestJson["system"] = systemPromptStr;
        }

        // Claude supports temperature
        if (temperature != 1.0f)
        {
            requestJson["temperature"] = temperature;
        }

        // Claude uses max_tokens
        if (maxTokens > 0)
        {
            requestJson["max_tokens"] = maxTokens;
        }

        // Claude supports top_p
        if (topP != 1.0f)
        {
            requestJson["top_p"] = topP;
        }

        // Claude doesn't support frequency_penalty and presence_penalty

        return requestJson.dump();
    }

    FormatterFunction getFormatterForEndpoint(const std::wstring &endpointType)
    {
        // Convert wstring to string for comparison
        std::string type = toUTF8(endpointType);

        if (type == "openai" || type == "") // Default to OpenAI format
        {
            return formatOpenAIRequest;
        }
        else if (type == "ollama")
        {
            return formatOllamaRequest;
        }
        else if (type == "claude")
        {
            return formatClaudeRequest;
        }
        else
        {
            // Default to OpenAI for unknown types
            return formatOpenAIRequest;
        }
    }
}
