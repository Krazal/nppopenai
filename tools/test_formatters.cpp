#include "../src/RequestFormatters.h"
#include "../src/EncodingUtils.h"
#include <iostream>
#include <cassert>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
using namespace RequestFormatters;

// Helper function to check if formatted JSON contains expected values
void validateJsonContains(const std::string &jsonStr, const std::string &field, const std::string &expectedValue)
{
    auto j = json::parse(jsonStr);
    assert(j.contains(field) && j[field] == expectedValue);
    std::cout << "✓ " << field << " validation passed" << std::endl;
}

// Test formatting for OpenAI API
void testOpenAIFormatter()
{
    std::cout << "Testing OpenAI Formatter..." << std::endl;

    std::wstring model = L"gpt-4";
    std::wstring prompt = L"Hello, world";
    std::wstring system = L"You are a helpful assistant";
    float temperature = 0.7f;
    int maxTokens = 100;
    float topP = 0.8f;
    float freqP = 0.1f;
    float presP = 0.2f;

    std::string request = formatOpenAIRequest(model, prompt, system, temperature, maxTokens, topP, freqP, presP);
    std::cout << "OpenAI Request: " << request << std::endl;

    // Parse and validate
    auto j = json::parse(request);
    assert(j["model"] == "gpt-4");
    assert(j["temperature"] == 0.7f);
    assert(j["max_tokens"] == 100);
    assert(j.contains("messages"));
    assert(j["messages"][0]["role"] == "system");
    assert(j["messages"][1]["role"] == "user");
    assert(j["messages"][1]["content"] == "Hello, world");

    std::cout << "✓ OpenAI formatter test passed" << std::endl;
}

// Test formatting for Ollama API
void testOllamaFormatter()
{
    std::cout << "Testing Ollama Formatter..." << std::endl;

    std::wstring model = L"llama3";
    std::wstring prompt = L"Tell me a joke";
    std::wstring system = L"You are funny";
    float temperature = 0.5f;
    int maxTokens = 50;
    float topP = 1.0f;
    float freqP = 0.1f;
    float presP = 0.0f;

    std::string request = formatOllamaRequest(model, prompt, system, temperature, maxTokens, topP, freqP, presP);
    std::cout << "Ollama Request: " << request << std::endl;

    // Parse and validate
    auto j = json::parse(request);
    assert(j["model"] == "llama3");
    assert(j["prompt"] == "Tell me a joke");
    assert(j["system"] == "You are funny");
    assert(j["temperature"] == 0.5f);
    assert(j["num_predict"] == 50);
    assert(j.contains("repeat_penalty"));

    std::cout << "✓ Ollama formatter test passed" << std::endl;
}

// Test formatting for Claude API
void testClaudeFormatter()
{
    std::cout << "Testing Claude Formatter..." << std::endl;

    std::wstring model = L"claude-3-haiku-20240307";
    std::wstring prompt = L"Explain what a dog is";
    std::wstring system = L"You are a friendly and knowledgeable teacher specializing in biology and animals";
    float temperature = 0.3f;
    int maxTokens = 500;
    float topP = 0.9f;
    float freqP = 0.0f;
    float presP = 0.0f;

    std::string request = formatClaudeRequest(model, prompt, system, temperature, maxTokens, topP, freqP, presP);
    std::cout << "Claude Request: " << request << std::endl;

    // Parse and validate
    auto j = json::parse(request);
    assert(j["model"] == "claude-3-haiku-20240307");
    assert(j.contains("messages"));
    assert(j["messages"][0]["role"] == "user");
    assert(j["messages"][0]["content"] == "Explain quantum physics");
    assert(j["system"] == "You are a physics professor");
    assert(j["temperature"] == 0.3f);
    assert(j["max_tokens"] == 500);
    assert(j["top_p"] == 0.9f);

    std::cout << "✓ Claude formatter test passed" << std::endl;
}

// Test the formatter selector function
void testFormatterSelector()
{
    std::cout << "Testing Formatter Selector..." << std::endl;

    auto openaiFormatter = getFormatterForEndpoint(L"openai");
    auto ollamaFormatter = getFormatterForEndpoint(L"ollama");
    auto claudeFormatter = getFormatterForEndpoint(L"claude");
    auto defaultFormatter = getFormatterForEndpoint(L"unknown");

    std::wstring model = L"test-model";
    std::wstring prompt = L"test prompt";
    std::wstring system = L"test system";

    // Test that each formatter produces different output for the same input
    std::string openaiRequest = openaiFormatter(model, prompt, system, 1.0f, 0, 1.0f, 0.0f, 0.0f);
    std::string ollamaRequest = ollamaFormatter(model, prompt, system, 1.0f, 0, 1.0f, 0.0f, 0.0f);
    std::string claudeRequest = claudeFormatter(model, prompt, system, 1.0f, 0, 1.0f, 0.0f, 0.0f);
    std::string defaultRequest = defaultFormatter(model, prompt, system, 1.0f, 0, 1.0f, 0.0f, 0.0f);

    assert(openaiRequest != ollamaRequest);
    assert(openaiRequest != claudeRequest);
    assert(ollamaRequest != claudeRequest);
    assert(openaiRequest == defaultRequest); // Default should be openai

    std::cout << "✓ Formatter selector test passed" << std::endl;
}

int main()
{
    std::cout << "Running RequestFormatters tests...\n"
              << std::endl;

    try
    {
        testOpenAIFormatter();
        std::cout << std::endl;

        testOllamaFormatter();
        std::cout << std::endl;

        testClaudeFormatter();
        std::cout << std::endl;

        testFormatterSelector();
        std::cout << std::endl;

        std::cout << "All RequestFormatters tests passed!" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
