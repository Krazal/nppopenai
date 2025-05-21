#ifndef RESPONSE_PARSERS_H
#define RESPONSE_PARSERS_H

#include <string>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * This file defines response parser functions for different API endpoints
 * Each parser knows how to extract the generated text from a specific API response format
 */

namespace ResponseParsers
{
    // Parser function type definition
    using ParserFunction = std::function<std::string(const std::string &)>;

    /**
     * Parse response from standard OpenAI-compatible API
     * Format: {"choices": [{"message": {"content": "generated text"}}]}
     */
    std::string parseOpenAIResponse(const std::string &response);

    /**
     * Parse response from Ollama native API
     * Format: {"response": "generated text"}
     */
    std::string parseOllamaResponse(const std::string &response);

    /**
     * Parse response from Anthropic Claude API
     * Format: {"content": [{"type": "text", "text": "generated text"}]}
     */
    std::string parseClaudeResponse(const std::string &response);

    /**
     * Parse response from simple completion API
     * Format: {"text": "generated text"} or {"completion": "generated text"}
     */
    std::string parseSimpleResponse(const std::string &response); /**
                                                                   * Get the appropriate parser function for an endpoint
                                                                   * @param endpointType The endpoint type identifier from config
                                                                   * @return The parser function to use
                                                                   */
    ParserFunction getParserForEndpoint(const std::wstring &endpointType);

    /**
     * Process thinking sections in the response text
     * Removes <think>...</think> sections if show_reasoning is disabled
     * @param text The text to process
     * @return The processed text with thinking sections handled according to config
     */
    std::string processThinkingSections(const std::string &text);
}

#endif // RESPONSE_PARSERS_H
