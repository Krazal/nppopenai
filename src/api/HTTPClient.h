#pragma once
#include <string>
#include <functional>

/**
 * HTTPClient - A module for handling HTTP requests to different LLM APIs
 *
 * This class encapsulates all HTTP communication functionality, handling
 * both regular and streaming API requests. It provides a clean interface
 * that abstracts away the details of cURL usage.
 */
class HTTPClient
{
public:
    // For standard requests
    static bool performRequest(
        const std::string &url,
        const std::string &request,
        std::string &response,
        const std::string &apiType,
        const std::string &secretKey,
        const std::string &proxy = "");

    // For streaming requests
    static bool performStreamingRequest(
        const std::string &url,
        const std::string &request,
        const std::string &apiType,
        const std::string &secretKey,
        void *targetWindow,
        unsigned int streamMessageType,
        const std::string &proxy = "");

private:
    static void setupCommonOptions(void *curl, const std::string &apiType, const std::string &secretKey);
    static void setupStreamingOptions(void *curl, const std::string &apiType);
};
