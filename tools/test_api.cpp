#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdlib>

using json = nlohmann::json;

// Callback function for cURL to write response data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

// Pretty print the JSON response
void prettyPrintJson(const std::string &jsonStr)
{
    try
    {
        json j = json::parse(jsonStr);
        std::cout << "Response JSON:" << std::endl;
        std::cout << j.dump(2) << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to parse JSON: " << e.what() << std::endl;
        std::cout << "Raw response:" << std::endl;
        std::cout << jsonStr << std::endl;
    }
}

// Extract text based on the response type
std::string extractResponseText(const std::string &jsonStr, const std::string &responseType)
{
    try
    {
        json j = json::parse(jsonStr);

        if (responseType == "openai")
        {
            if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty())
            {
                return j["choices"][0]["message"]["content"].get<std::string>();
            }
        }
        else if (responseType == "ollama")
        {
            if (j.contains("response"))
            {
                return j["response"].get<std::string>();
            }
        }
        else if (responseType == "claude")
        {
            if (j.contains("content") && j["content"].is_array())
            {
                std::string result;
                for (const auto &part : j["content"])
                {
                    if (part.contains("type") && part["type"] == "text" && part.contains("text"))
                    {
                        result += part["text"].get<std::string>();
                    }
                }
                return result;
            }
        }
        else if (responseType == "simple")
        {
            if (j.contains("text"))
            {
                return j["text"].get<std::string>();
            }
            else if (j.contains("completion"))
            {
                return j["completion"].get<std::string>();
            }
            else if (j.contains("output"))
            {
                return j["output"].get<std::string>();
            }
        }

        return "Could not find expected fields in response";
    }
    catch (const std::exception &e)
    {
        return std::string("Error parsing response: ") + e.what();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "Usage: test_api <base_url> <endpoint_path> <response_type> [api_key]" << std::endl;
        std::cout << "Example: test_api http://localhost:11434/ api/generate ollama" << std::endl;
        return 1;
    }

    std::string baseUrl = argv[1];
    std::string endpointPath = argv[2];
    std::string responseType = argv[3];
    std::string apiKey = (argc > 4) ? argv[4] : "";

    // Ensure base URL ends with a slash
    if (!baseUrl.empty() && baseUrl.back() != '/')
    {
        baseUrl += '/';
    }

    // Construct full URL
    std::string url = baseUrl + endpointPath;
    std::cout << "Testing API URL: " << url << std::endl;
    std::cout << "Using response format: " << responseType << std::endl;

    // Initialize cURL
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Error: Failed to initialize cURL" << std::endl;
        return 1;
    } // Prepare request with system prompt option
    std::string systemPrompt = "You are a helpful assistant.";
    std::string userPrompt = "Hello, how are you?";
    float temperature = 0.7f;
    int maxTokens = 100;

    std::string request;
    if (responseType == "ollama")
    {
        json reqJson = {
            {"model", "llama3"},
            {"prompt", userPrompt},
            {"system", systemPrompt},
            {"temperature", temperature},
            {"num_predict", maxTokens}};
        request = reqJson.dump();
    }
    else if (responseType == "claude")
    {
        json reqJson = {
            {"model", "claude-3-haiku-20240307"},
            {"max_tokens", maxTokens},
            {"temperature", temperature},
            {"messages", json::array({{{"role", "user"}, {"content", userPrompt}}})},
            {"system", systemPrompt}};
        request = reqJson.dump();
    }
    else
    {
        json reqJson = {
            {"model", "gpt-3.5-turbo"},
            {"messages", json::array({{{"role", "system"}, {"content", systemPrompt}},
                                      {{"role", "user"}, {"content", userPrompt}}})},
            {"temperature", temperature},
            {"max_tokens", maxTokens}};
        request = reqJson.dump();
    }

    std::cout << "Request payload:" << std::endl;
    std::cout << json::parse(request).dump(2) << std::endl;

    // Set up cURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    // Headers - customized for each API type
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (responseType == "claude" && !apiKey.empty())
    {
        // Claude API uses x-api-key header and requires specific version header
        std::string authHeader = "x-api-key: " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
        std::cout << "Using Claude API authentication" << std::endl;
    }
    else if (!apiKey.empty())
    {
        // OpenAI and most others use Bearer token
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        std::cout << "Using Bearer token authentication" << std::endl;
    }
    else
    {
        std::cout << "No authentication provided" << std::endl;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Response handling
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request
    std::cout << "\nSending request..." << std::endl;
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
    }
    else
    {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        std::cout << "HTTP Response Code: " << httpCode << std::endl
                  << std::endl;

        if (httpCode >= 200 && httpCode < 300)
        {
            // Print the full JSON response
            prettyPrintJson(response);

            // Extract the actual text based on response type
            std::cout << "\nExtracted response text:" << std::endl;
            std::cout << "--------------------" << std::endl;
            std::cout << extractResponseText(response, responseType) << std::endl;
            std::cout << "--------------------" << std::endl;
        }
        else
        {
            std::cout << "Error Response:" << std::endl;
            std::cout << response << std::endl;
        }
    }

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
