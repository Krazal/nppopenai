#include "APIUtils.h"
#include "EncodingUtils.h"
#include "config/PromptManager.h"
#include "core/external_globals.h"
#include "RequestFormatters.h"

/**
 * Build a complete API URL with the proper endpoint
 *
 * @param baseUrl The base URL of the API
 * @param chatRoute The specific endpoint for chat completions
 * @return The complete URL for the API request
 */
std::string APIUtils::buildApiUrl(const std::string &baseUrl, const std::string &chatRoute)
{
    std::string url = baseUrl;

    // Ensure base URL ends with a trailing slash
    if (!url.empty() && url.back() != '/')
    {
        url += '/';
    }

    // Check if URL already includes the chat completions endpoint
    bool hasCompletionsEndpoint = url.find(chatRoute) != std::string::npos;

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

    return url;
}

/**
 * Get the system prompt from configuration or prompt file
 *
 * @return The system prompt as a wide string
 */
std::wstring APIUtils::getSystemPrompt()
{
    // Parse prompts from instructions file
    std::vector<Prompt> prompts;
    parseInstructionsFile(instructionsFilePath, prompts);

    // If no prompts in file, use default from configuration
    if (prompts.empty())
        return configAPIValue_instructions;

    // If only one prompt, use it
    if (prompts.size() == 1)
        return prompts[0].content;

    // If multiple prompts, let user choose one
    // NOTE: This function should be called from askChatGPT with proper prompt selection logic
    return L"MULTIPLE_PROMPTS_AVAILABLE";
}

/**
 * Prepare an API request with all necessary parameters
 *
 * @param selectedText The user's selected text (query)
 * @param systemPrompt The system prompt to use
 * @param model The model to use
 * @param responseType The type of API to use (openai, claude, ollama)
 * @param temperature The temperature parameter for the API
 * @param maxTokens The maximum number of tokens for the API
 * @param topP The top-p parameter for the API
 * @param frequencyPenalty The frequency penalty parameter for the API
 * @param presencePenalty The presence penalty parameter for the API
 * @param streaming Whether to enable streaming mode
 * @return The formatted request JSON as a string
 */
std::string APIUtils::prepareApiRequest(
    const std::string &selectedText,
    const std::wstring &systemPrompt,
    const std::wstring &model,
    const std::wstring &responseType,
    float temperature,
    int maxTokens,
    float topP,
    float frequencyPenalty,
    float presencePenalty,
    bool streaming)
{
    // Get the appropriate request formatter based on the configured response type
    auto formatter = RequestFormatters::getFormatterForEndpoint(responseType);

    // Format request using the selected formatter
    std::string request = formatter(
        model,
        multiByteToWideChar(selectedText.c_str()),
        systemPrompt,
        temperature,
        maxTokens,
        topP,
        frequencyPenalty,
        presencePenalty);

    // Add streaming parameter if needed
    if (streaming)
    {
        auto pos = request.rfind('}');
        if (pos != std::string::npos)
        {
            request.insert(pos, ",\"stream\":true");
        }
    }
    else if (responseType == L"ollama")
    {
        // Ensure stream is explicitly set to false for non-streaming Ollama requests
        auto pos = request.rfind('}');
        if (pos != std::string::npos)
        {
            request.insert(pos, ",\"stream\":false");
        }
    }

    return request;
}
