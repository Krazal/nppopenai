#include "StreamParser.h"
#include <sstream>
#include <fstream>
#include "core/external_globals.h"

/**
 * Extract content from a streaming chunk based on API type
 *
 * @param chunk The raw chunk received from the API
 * @param apiType The type of API (openai, claude, ollama, etc.)
 * @return The extracted content, or empty string if no content was found
 */
std::string StreamParser::extractContent(const std::string &chunk, const std::string &apiType)
{
    if (chunk.empty())
    {
        return "";
    }

    // Add debugging output to understand parsing
    if (debugMode)
    {
        static int parseCount = 0;
        parseCount++;
        std::string filename = "C:\\temp\\parse_" + std::to_string(parseCount) + ".txt";
        std::ofstream parseFile(filename);
        if (parseFile.is_open())
        {
            parseFile << "Parse attempt #" << parseCount << std::endl;
            parseFile << "API Type: " << apiType << std::endl;
            parseFile << "Chunk size: " << chunk.size() << std::endl;
            parseFile << "Raw chunk:" << std::endl;
            parseFile << chunk << std::endl;
            parseFile << "===== ATTEMPTING JSON PARSE =====" << std::endl;
        }
    }

    // First try parsing as JSON to detect if it's a valid JSON string regardless of API type
    try
    {
        json j = json::parse(chunk);

        // Try OpenAI format (most common)
        if (j.contains("choices") && j["choices"].size() > 0)
        {
            if (j["choices"][0].contains("delta") && j["choices"][0]["delta"].contains("content"))
            {
                std::string extracted = j["choices"][0]["delta"]["content"].get<std::string>();
                if (debugMode)
                {
                    std::ofstream parseFile("C:\\temp\\parse_success.txt", std::ios::app);
                    if (parseFile.is_open())
                    {
                        parseFile << "SUCCESS: OpenAI format, extracted: [" << extracted << "]" << std::endl;
                        parseFile.close();
                    }
                }
                return extracted;
            }
        }

        // Try Ollama format
        if (j.contains("response"))
        {
            std::string extracted = j["response"].get<std::string>();
            if (debugMode)
            {
                std::ofstream parseFile("C:\\temp\\parse_success.txt", std::ios::app);
                if (parseFile.is_open())
                {
                    parseFile << "SUCCESS: Ollama format, extracted: [" << extracted << "]" << std::endl;
                    parseFile.close();
                }
            }
            return extracted;
        }

        // Try Claude format
        if (j.contains("type") && j["type"] == "content_block_delta" &&
            j.contains("delta") && j["delta"].contains("text"))
        {
            std::string extracted = j["delta"]["text"].get<std::string>();
            if (debugMode)
            {
                std::ofstream parseFile("C:\\temp\\parse_success.txt", std::ios::app);
                if (parseFile.is_open())
                {
                    parseFile << "SUCCESS: Claude format, extracted: [" << extracted << "]" << std::endl;
                    parseFile.close();
                }
            }
            return extracted;
        }

        if (debugMode)
        {
            std::ofstream parseFile("C:\\temp\\parse_fail.txt", std::ios::app);
            if (parseFile.is_open())
            {
                parseFile << "JSON parsed but no content found in known formats" << std::endl;
                parseFile << "JSON keys: ";
                for (auto it = j.begin(); it != j.end(); ++it)
                {
                    parseFile << it.key() << " ";
                }
                parseFile << std::endl;
                parseFile.close();
            }
        }
    }
    catch (...)
    {
        if (debugMode)
        {
            std::ofstream parseFile("C:\\temp\\parse_fail.txt", std::ios::app);
            if (parseFile.is_open())
            {
                parseFile << "JSON parse failed, falling back to type-specific parsing" << std::endl;
                parseFile.close();
            }
        }
        // Not valid JSON, continue with type-specific parsing
    } // Type-specific parsing as fallback
    if (apiType == "openai")
    {
        std::string result = parseOpenAIChunk(chunk);
        if (debugMode)
        {
            std::ofstream parseFile("C:\\temp\\parse_fallback.txt", std::ios::app);
            if (parseFile.is_open())
            {
                parseFile << "OpenAI fallback parsing result: [" << result << "]" << std::endl;
                parseFile.close();
            }
        }
        return result;
    }
    else if (apiType == "ollama")
    {
        std::string result = parseOllamaChunk(chunk);
        if (debugMode)
        {
            std::ofstream parseFile("C:\\temp\\parse_fallback.txt", std::ios::app);
            if (parseFile.is_open())
            {
                parseFile << "Ollama fallback parsing result: [" << result << "]" << std::endl;
                parseFile.close();
            }
        }
        return result;
    }
    else if (apiType == "claude")
    {
        std::string result = parseClaudeChunk(chunk);
        if (debugMode)
        {
            std::ofstream parseFile("C:\\temp\\parse_fallback.txt", std::ios::app);
            if (parseFile.is_open())
            {
                parseFile << "Claude fallback parsing result: [" << result << "]" << std::endl;
                parseFile.close();
            }
        }
        return result;
    }
    else if (apiType == "simple" || apiType.empty())
    {
        // For simple/unknown types, return the raw chunk if it's reasonable
        if (chunk.size() < 100 && !isCompletionMarker(chunk))
        {
            if (debugMode)
            {
                std::ofstream parseFile("C:\\temp\\parse_fallback.txt", std::ios::app);
                if (parseFile.is_open())
                {
                    parseFile << "Simple/empty API type, returning raw chunk: [" << chunk << "]" << std::endl;
                    parseFile.close();
                }
            }
            return chunk;
        }
    }

    if (debugMode)
    {
        std::ofstream parseFile("C:\\temp\\parse_fallback.txt", std::ios::app);
        if (parseFile.is_open())
        {
            parseFile << "All parsing failed, returning empty string" << std::endl;
            parseFile.close();
        }
    }

    return "";
}

/**
 * Check if a chunk is a completion marker
 *
 * @param chunk The chunk to check
 * @return true if the chunk is a completion marker, false otherwise
 */
bool StreamParser::isCompletionMarker(const std::string &chunk)
{
    return (chunk == "data: [DONE]" ||
            chunk == "data: [DONE]\n" ||
            chunk == "data: [DONE]\r\n" ||
            chunk.find("data: [DONE]") == 0);
}

/**
 * Parse a streaming chunk from OpenAI
 *
 * @param chunk The chunk to parse
 * @return The extracted content
 */
std::string StreamParser::parseOpenAIChunk(const std::string &chunk)
{
    std::string content;
    std::istringstream stream(chunk);
    std::string line;

    while (std::getline(stream, line))
    {
        // Skip empty lines
        if (line.empty() || line == "\r")
        {
            continue;
        }

        // Process data: lines
        if (line.find("data:") == 0)
        {
            // Extract the JSON part after "data: "
            std::string jsonStr = line.substr(5);

            // Remove any trailing \r if present
            if (!jsonStr.empty() && jsonStr.back() == '\r')
            {
                jsonStr.pop_back();
            }

            // Trim leading/trailing whitespace
            size_t start = jsonStr.find_first_not_of(" \t\r\n");
            if (start == std::string::npos)
            {
                continue; // Empty after trimming
            }
            size_t end = jsonStr.find_last_not_of(" \t\r\n");
            jsonStr = jsonStr.substr(start, end - start + 1);

            if (jsonStr == "[DONE]" || jsonStr.find("[DONE]") != std::string::npos)
            {
                // End of stream marker for OpenAI
                continue;
            }

            try
            {
                auto json = json::parse(jsonStr);
                if (json.contains("choices") && json["choices"].size() > 0 &&
                    json["choices"][0].contains("delta") &&
                    json["choices"][0]["delta"].contains("content"))
                {
                    std::string deltaContent = json["choices"][0]["delta"]["content"].get<std::string>();
                    content += deltaContent;
                }
            }
            catch (...)
            {
                // Ignore parse errors for OpenAI stream chunks
            }
        }
    }

    return content;
}

/**
 * Parse a streaming chunk from Ollama
 *
 * @param chunk The chunk to parse
 * @return The extracted content
 */
std::string StreamParser::parseOllamaChunk(const std::string &chunk)
{
    try
    {
        auto json = json::parse(chunk);

        // Extract response content if available
        if (json.contains("response"))
        {
            return json["response"].get<std::string>();
        }

        // Check for stream completion
        if (json.contains("done") && json["done"].is_boolean() && json["done"].get<bool>())
        {
            // This is just a completion marker, return empty string
            return "";
        }
    }
    catch (...)
    {
        // Ignore parse errors for Ollama stream chunks
    }

    return "";
}

/**
 * Parse a streaming chunk from Claude
 *
 * @param chunk The chunk to parse
 * @return The extracted content
 */
std::string StreamParser::parseClaudeChunk(const std::string &chunk)
{
    try
    {
        auto json = json::parse(chunk);
        if (json.contains("type") && json["type"] == "content_block_delta" &&
            json.contains("delta") && json["delta"].contains("text"))
        {
            return json["delta"]["text"].get<std::string>();
        }
    }
    catch (...)
    {
        // Ignore parse errors for Claude stream chunks
    }

    return "";
}
