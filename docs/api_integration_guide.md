# NppOpenAI API Integration Guide

This guide explains how to integrate additional LLM API formats into NppOpenAI. The plugin has been designed with a modular architecture that separates request formatting, response parsing, and authentication handling to make it easy to add support for new APIs.

## Architecture Overview

NppOpenAI uses three key components to handle different API formats:

1. **Request Formatters**: Convert the user's input into API-specific JSON requests
2. **Response Parsers**: Extract the generated text from API-specific JSON responses
3. **Authentication Handlers**: Add the appropriate authentication headers for each API

## Adding Support for a New API Format

To add support for a new LLM API, follow these steps:

### 1. Add a Response Parser

Add a new parser function to `ResponseParsers.cpp`:

```cpp
std::string parseNewAPIResponse(const std::string& response)
{
    std::string replyText;
    try
    {
        auto respJson = json::parse(response);
        // Extract text from your API's response format
        if (respJson.contains("your_field"))
        {
            replyText = respJson["your_field"].get<std::string>();
        }
        else
        {
            replyText = "[Error: No valid response in NewAPI format]";
        }
    }
    catch (const std::exception& e)
    {
        replyText = "[Failed to parse NewAPI response: ";
        replyText += e.what();
        replyText += "]";
    }
    return replyText;
}
```

Then add it to the parser selector function:

```cpp
ParserFunction getParserForEndpoint(const std::wstring& endpointType)
{
    // Convert wstring to string for comparison
    std::string type = toUTF8(endpointType);

    if (type == "newapi")
    {
        return parseNewAPIResponse;
    }
    else if (type == "openai" || type == "")  // Default to OpenAI format
    {
        return parseOpenAIResponse;
    }
    // ...existing parsers...
}
```

### 2. Add a Request Formatter

Add a new formatter function to `RequestFormatters.cpp`:

```cpp
std::string formatNewAPIRequest(
    const std::wstring& model,
    const std::wstring& prompt,
    const std::wstring& systemPrompt,
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

    // Format the request according to your API requirements
    requestJson["model"] = modelStr;
    requestJson["input"] = promptStr;

    if (!systemPromptStr.empty()) {
        requestJson["context"] = systemPromptStr;
    }

    // Add parameters that make sense for your API
    if (temperature != 1.0f) {
        requestJson["temperature"] = temperature;
    }

    if (maxTokens > 0) {
        requestJson["max_length"] = maxTokens;
    }

    return requestJson.dump();
}
```

Then add it to the formatter selector function:

```cpp
FormatterFunction getFormatterForEndpoint(const std::wstring& endpointType) {
    std::string type = toUTF8(endpointType);

    if (type == "newapi")
    {
        return formatNewAPIRequest;
    }
    else if (type == "openai" || type == "")
    {
        return formatOpenAIRequest;
    }
    // ...existing formatters...
}
```

### 3. Update Authentication Handling

Modify the authentication handling in `OpenAIClient.cpp`:

```cpp
// Headers vary by API
std::string apiType = toUTF8(configAPIValue_responseType);

// Content-Type is always JSON
headers = curl_slist_append(headers, "Content-Type: application/json");

// Add authentication headers based on API type
if (apiType == "newapi") {
    // Your API authentication style
    std::string authHeader = "Authorization: ApiKey " + toUTF8(configAPIValue_secretKey);
    headers = curl_slist_append(headers, authHeader.c_str());
    // Any additional headers your API requires
    headers = curl_slist_append(headers, "api-version: 2023-01-01");
}
else if (apiType == "claude") {
    // Claude API uses x-api-key header
    // ...existing code...
}
```

### 4. Update Configuration Documentation

Add your new API type to the INI documentation in `ConfigManager.cpp`:

```cpp
::WritePrivateProfileString(TEXT("INFO"), TEXT(";   - newapi: Your API format explanation"), TEXT(""), iniFilePath);
```

### 5. Add User Documentation

Create a setup guide for your API in the docs folder:

````markdown
# Using NppOpenAI with YourAPI

This guide explains how to use NppOpenAI with [YourAPI](https://your-api.com/).

## Configuration for YourAPI

Add the following to your `NppOpenAI.ini` file:

```ini
[API]
secret_key=YOUR_API_KEY
api_url=https://api.yourapi.com/v1/
chat_completions_route=completion
response_type=newapi
model=your-model-name
```
````

## Notes

1. You need an API key from YourAPI
2. The `response_type=newapi` setting enables compatible formatting and parsing
3. ...other important notes...

````

### 6. Update the README

Add your API to the examples in README.md:

```ini
# YourAPI Example
api_url=https://api.yourapi.com/v1/
chat_completions_route=completion
response_type=newapi
````

### 7. Update the Test Utility

Add support for your API in `test_api.cpp`:

```cpp
else if (responseType == "newapi") {
    json reqJson = {
        {"model", "your-model"},
        {"input", "Hello, how are you?"},
        {"temperature", 0.7}
    };
    request = reqJson.dump();
}
```

## Testing Your Integration

Use the `test_api.cpp` utility to test your integration:

```bash
test_api.exe https://api.yourapi.com/v1/ completion newapi YOUR_API_KEY
```

## Best Practices

1. Keep parsing and formatting code separate for clean modularity
2. Add proper error messages to help users troubleshoot
3. Document all specifics of your API's format
4. Test with both valid and invalid inputs
5. Update all relevant documentation files
