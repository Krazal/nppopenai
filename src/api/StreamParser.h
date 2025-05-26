#pragma once
#include <string>
#include <functional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * StreamParser - A module for handling streaming responses from different LLM APIs
 *
 * This namespace contains functions for parsing different streaming formats
 * from various LLM providers like OpenAI, Claude, and Ollama. It handles
 * the extraction of content from streaming chunks and detection of
 * stream completion markers.
 */
namespace StreamParser
{
    // Function to extract content from streaming chunks based on API type
    std::string extractContent(const std::string &chunk, const std::string &apiType);

    // Specific parsers for each API type
    std::string parseOpenAIChunk(const std::string &chunk);
    std::string parseOllamaChunk(const std::string &chunk);
    std::string parseClaudeChunk(const std::string &chunk);

    // Helper function to check if chunk is a completion marker
    bool isCompletionMarker(const std::string &chunk);
}
