/**
 * ResponseParsers.cpp - Implements response parsing for different API formats
 *
 * This file contains parser functions for extracting generated text from
 * different API response formats (OpenAI, Ollama native, etc.). Each parser
 * knows how to extract the content from a specific JSON response format.
 *
 * Support for different API formats:
 * - OpenAI format: {"choices":[{"message":{"content":"response text"}}]}
 * - Ollama format: {"response":"response text"}
 * - Simple format: {"text":"response text"} or {"completion":"response text"}
 * - Anthropic Claude format: {"content":[{"type":"text","text":"response text"}]}
 *
 * This design allows users to connect to various language model backends
 * without requiring an OpenAI-compatible adapter or proxy.
 */

#include "ResponseParsers.h"
#include "EncodingUtils.h"    // for multiByteToWideChar
#include "external_globals.h" // for configAPIValue_showReasoning
#include <string>
#include <stdexcept>

namespace ResponseParsers
{
    std::string parseOpenAIResponse(const std::string &response)
    {
        std::string replyText;
        try
        {
            auto respJson = json::parse(response);
            if (respJson.contains("choices") && respJson["choices"].is_array() && !respJson["choices"].empty())
            {
                replyText = respJson["choices"][0]["message"]["content"].get<std::string>();
                // Process any thinking sections in the response
                replyText = processThinkingSections(replyText);
            }
            else
            {
                replyText = "[Error: No valid 'choices' array or 'content' field found in OpenAI format response]";
            }
        }
        catch (const std::exception &e)
        {
            replyText = "[Failed to parse OpenAI response: ";
            replyText += e.what();
            replyText += ". Please check endpoint configuration and response_type setting.]";
        }
        return replyText;
    }
    std::string parseOllamaResponse(const std::string &response)
    {
        std::string replyText;
        try
        {
            // Check if the response is valid JSON
            if (response.empty())
            {
                return "[Error: Empty Ollama response]";
            }

            // For streamed responses, Ollama sends multiple JSON objects
            // We need to handle both streaming and non-streaming cases
            if (response.find("\n") != std::string::npos)
            {
                // This is likely a streamed response with multiple JSON objects
                // In streaming mode, we usually don't need to parse the response here
                // as streaming is handled by OpenAIStreamCallback
                // But return just the last chunk for compatibility
                size_t lastJsonStart = response.rfind("{");
                if (lastJsonStart != std::string::npos)
                {
                    std::string lastJsonObject = response.substr(lastJsonStart);
                    auto respJson = json::parse(lastJsonObject);
                    if (respJson.contains("response"))
                    {
                        replyText = respJson["response"].get<std::string>();
                        // Process any thinking sections in the response
                        replyText = processThinkingSections(replyText);
                    }
                }
                else
                {
                    replyText = "[Error: Invalid streamed response format from Ollama]";
                }
            }
            else
            { // Standard non-streamed response
                auto respJson = json::parse(response);
                if (respJson.contains("response"))
                {
                    replyText = respJson["response"].get<std::string>();
                    // Process any thinking sections in the response
                    replyText = processThinkingSections(replyText);
                }
                else if (respJson.contains("error"))
                {
                    replyText = "[Error from Ollama: ";
                    replyText += respJson["error"].get<std::string>();
                    replyText += "]";
                }
                else
                {
                    // Try to dump the entire response for debugging
                    replyText = "[Error: No 'response' field found in Ollama format. Raw JSON: ";
                    replyText += response.substr(0, 200); // First 200 chars to avoid too much text
                    if (response.length() > 200)
                        replyText += "...";
                    replyText += "]";
                }
            }
        }
        catch (const std::exception &e)
        {
            replyText = "[Failed to parse Ollama response: ";
            replyText += e.what();
            replyText += ". Please verify Ollama is running and response_type=ollama is set correctly.]";
        }
        return replyText;
    }

    std::string parseClaudeResponse(const std::string &response)
    {
        std::string replyText;
        try
        {
            auto respJson = json::parse(response);
            // Anthropic Claude format uses content array with type/text objects
            if (respJson.contains("content") && respJson["content"].is_array())
            {
                // Combine all text parts from the content array
                for (const auto &part : respJson["content"])
                {
                    if (part.contains("type") && part["type"] == "text" && part.contains("text"))
                    {
                        replyText += part["text"].get<std::string>();
                    }
                }
                if (replyText.empty())
                {
                    replyText = "[Error: No text content found in Claude response]";
                }
                else
                {
                    // Process any thinking sections in the response
                    replyText = processThinkingSections(replyText);
                }
            }
            else
            {
                replyText = "[Error: No valid 'content' array found in Claude format response]";
            }
        }
        catch (const std::exception &e)
        {
            replyText = "[Failed to parse Claude response: ";
            replyText += e.what();
            replyText += ". Please check if response_type=claude is the correct format for this endpoint.]";
        }
        return replyText;
    }

    std::string parseSimpleResponse(const std::string &response)
    {
        std::string replyText;
        try
        {
            auto respJson = json::parse(response);
            // Try multiple common response field names
            if (respJson.contains("text"))
            {
                replyText = respJson["text"].get<std::string>();
            }
            else if (respJson.contains("completion"))
            {
                replyText = respJson["completion"].get<std::string>();
            }
            else if (respJson.contains("output"))
            {
                replyText = respJson["output"].get<std::string>();
            }
            else if (respJson.contains("generated_text"))
            {
                replyText = respJson["generated_text"].get<std::string>();
            }
            else
            {
                replyText = "[Error: No recognized field found in simple format response. Expected 'text', 'completion', 'output', or 'generated_text'.]";
                return replyText; // Return early to skip processing non-existent text
            }

            // Process any thinking sections in the response
            replyText = processThinkingSections(replyText);
        }
        catch (const std::exception &e)
        {
            replyText = "[Failed to parse simple response: ";
            replyText += e.what();
            replyText += ". Please check if response_type=simple is the correct format for this endpoint.]";
        }
        return replyText;
    }
    ParserFunction getParserForEndpoint(const std::wstring &endpointType)
    {
        // Convert wstring to string for comparison
        std::string type = toUTF8(endpointType);

        if (type == "openai" || type == "") // Default to OpenAI format
        {
            return parseOpenAIResponse;
        }
        else if (type == "ollama")
        {
            return parseOllamaResponse;
        }
        else if (type == "simple")
        {
            return parseSimpleResponse;
        }
        else if (type == "claude")
        {
            return parseClaudeResponse;
        }
        else
        {
            // Default to OpenAI for unknown types
            return parseOpenAIResponse;
        }
    } /**
       * Process thinking sections in LLM responses
       *
       * Some LLMs provide their reasoning within <think>...</think> tags before giving the
       * final answer. This function handles these reasoning sections based on user preference.
       *
       * If show_reasoning=1 is set in the INI file, these sections are preserved.
       * If show_reasoning=0 (default), these sections are removed from the final output.
       *
       * For streaming responses, this is applied to each chunk as it arrives, which means that
       * thinking sections split across multiple chunks might only be partially removed.
       *
       * @param text The input text potentially containing thinking sections
       * @return The text with thinking sections either preserved or removed
       */
    std::string processThinkingSections(const std::string &text)
    {
        // Check if we should show reasoning sections
        bool showReasoning = (configAPIValue_showReasoning == L"1");

        if (showReasoning)
        {
            // Return the text as-is if we're showing reasoning
            return text;
        }

        std::string result = text;
        size_t startPos = 0;
        size_t thinkStart, thinkEnd;

        // Find all <think>...</think> sections and remove them
        while ((thinkStart = result.find("<think>", startPos)) != std::string::npos)
        {
            thinkEnd = result.find("</think>", thinkStart);
            if (thinkEnd != std::string::npos)
            {
                // Remove the section including the tags
                result.erase(thinkStart, (thinkEnd + 8) - thinkStart);
                // Continue searching from the current position
                startPos = thinkStart;
            }
            else
            {
                // If closing tag is not found, break to avoid infinite loop
                break;
            }
        }

        return result;
    }
}
