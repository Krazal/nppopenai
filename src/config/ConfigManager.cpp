/**
 * ConfigManager.cpp - Configuration handling for NppOpenAI plugin
 *
 * This file provides functions for loading, saving, and managing plugin configuration.
 * It handles settings stored in the NppOpenAI.ini file and system prompts stored in
 * the NppOpenAI_instructions file. It manages user preferences including the "Keep my question"
 * setting which determines whether original queries are preserved in responses.
 */

#include <windows.h>
#include <shlwapi.h>
#include "ConfigManager.h"
#include "core/external_globals.h" // for global variables and functions
#include "EncodingUtils.h"         // for UTF-8 conversions
#include "PromptManager.h"         // for parsing instructions file
#include <cstdio>
#include <vector>
#include <algorithm> // for std::transform

/**
 * Creates a default configuration file with recommended settings
 *
 * This function writes a default INI file with minimal documentation
 * and standard settings for the plugin's API integration options.
 */
void writeDefaultConfig()
{
    // Basic info header
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; NppOpenAI Configuration File"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; Supports OpenAI, Claude, and Ollama API connections"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; Enter your API key below (OpenAI: sk-xxx, Claude: sk-ant-xxx, Ollama: blank)"), TEXT(""), iniFilePath);

    // API configurations    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; === OpenAI configuration ==="), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; api_url = https://api.openai.com/v1/"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; response_type = openai"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_chat_completions = chat/completions  # New naming convention"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_realtime_sessions = realtime/sessions  # Future support"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_audio_speech = audio/speech  # Future support"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_images_generations = images/generations  # Future support"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; model = gpt-4o-mini"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; streaming = 1 (enabled) or 0 (disabled)"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; show_reasoning = 1 (show AI reasoning sections) or 0 (hide reasoning)"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; "), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; === Claude configuration ==="), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; api_url = https://api.anthropic.com/v1/"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; response_type = claude"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_chat_completions = messages  # New naming convention"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; model = claude-3-haiku-20240307"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; streaming = 1 (enabled) or 0 (disabled)"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; show_reasoning = 1 (show AI reasoning sections) or 0 (hide reasoning)"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; "), TEXT(""), iniFilePath);

    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; === Ollama configuration ==="), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; api_url = http://localhost:11434/"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; response_type = ollama"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; route_chat_completions = api/generate  # New naming convention"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; model = qwen3:1.7b"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; streaming = 1 (enabled) or 0 (disabled)"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; show_reasoning = 1 (show AI reasoning sections) or 0 (hide reasoning)"), TEXT(""), iniFilePath); // Set the default OpenAI API values with new route naming convention
    ::WritePrivateProfileString(TEXT("API"), TEXT("secret_key"), TEXT("ENTER_YOUR_API_KEY_HERE"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("api_url"), TEXT("https://api.openai.com/v1/"), iniFilePath); // New route naming convention (recommended)
    ::WritePrivateProfileString(TEXT("API"), TEXT("route_chat_completions"), TEXT("chat/completions"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("response_type"), TEXT("openai"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("model"), TEXT("gpt-4o-mini"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("temperature"), TEXT("0.7"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("max_tokens"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("top_p"), TEXT("0.8"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("presence_penalty"), TEXT("0"), iniFilePath); // Add streaming option (0=disabled, 1=enabled)
    ::WritePrivateProfileString(TEXT("API"), TEXT("streaming"), TEXT("1"), iniFilePath);

    // Show reasoning (thinking) sections (0=hidden, 1=shown)
    ::WritePrivateProfileString(TEXT("API"), TEXT("show_reasoning"), TEXT("0"), iniFilePath);

    // Create plugin section with default values
    ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("total_tokens_used"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), TEXT("1"), iniFilePath);
    ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), TEXT("10"), iniFilePath);
}

// Implementation of the loadConfig function declared in ConfigManager.h
// This is the actual implementation that will be called from PluginDefinition.cpp

namespace ConfigManagerImpl
{
    /**
     * Loads configuration from the INI file
     *
     * @param loadPluginSettings If true, also loads plugin-specific settings (keep_question, is_chat, etc.)
     *                         If false, only loads API-related configuration
     */
    void loadConfig(bool loadPluginSettings)
    { // Check if the config file exists; if not, create it with default values
        if (PathFileExists(iniFilePath) == FALSE)
        {
            // Use the comprehensive writeDefaultConfig function to create a well-documented config file
            writeDefaultConfig();
        }

        // Read values from the config file
        TCHAR buffer[1024]; // Read API Key
        ::GetPrivateProfileString(TEXT("API"), TEXT("secret_key"), configAPIValue_secretKey.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_secretKey = buffer;

        // Debug check for API key
        if (configAPIValue_secretKey == TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE") || configAPIValue_secretKey.empty())
        {
            ::MessageBox(nppData._nppHandle,
                         TEXT("API key not properly configured. Please edit the config file and set a valid API key."),
                         TEXT("NppOpenAI Configuration Error"),
                         MB_ICONWARNING);
        } // Read other API settings
        ::GetPrivateProfileString(TEXT("API"), TEXT("api_url"), configAPIValue_baseURL.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_baseURL = buffer; // Auto-correct common OpenAI-compatible server URLs that are missing /v1
        std::wstring baseUrlLower = configAPIValue_baseURL;
        std::transform(baseUrlLower.begin(), baseUrlLower.end(), baseUrlLower.begin(),
                       [](wchar_t c)
                       { return static_cast<wchar_t>(::towlower(c)); });
        // Check for common patterns that should include /v1 for OpenAI-compatible servers
        bool needsV1 = false;
        if ((baseUrlLower.find(L"localhost:1234") != std::wstring::npos || // LM Studio
             baseUrlLower.find(L"localhost:8000") != std::wstring::npos || // vLLM
             baseUrlLower.find(L"localhost:8080") != std::wstring::npos || // LocalAI
             baseUrlLower.find(L"litellm") != std::wstring::npos ||        // LiteLLM
             baseUrlLower.find(L"fastchat") != std::wstring::npos ||       // FastChat
             baseUrlLower.find(L"localai") != std::wstring::npos) &&       // LocalAI (additional pattern)
            baseUrlLower.find(L"/v1") == std::wstring::npos &&
            baseUrlLower.find(L"11434") == std::wstring::npos)
        { // Exclude Ollama default port
            needsV1 = true;
        }

        if (needsV1)
        {
            // Remove trailing slash if present
            if (!configAPIValue_baseURL.empty() && configAPIValue_baseURL.back() == L'/')
            {
                configAPIValue_baseURL.pop_back();
            }
            configAPIValue_baseURL += L"/v1/";

            // Optionally notify user about the auto-correction
            if (debugMode)
            {
                ::MessageBox(nppData._nppHandle,
                             (L"Auto-corrected API URL to include /v1: " + configAPIValue_baseURL).c_str(),
                             L"NppOpenAI Auto-Correction",
                             MB_ICONINFORMATION);
            }
        }
        else
        {
            // Ensure trailing slash for baseURL
            if (!configAPIValue_baseURL.empty() && configAPIValue_baseURL.back() != L'/')
            {
                configAPIValue_baseURL.push_back(L'/');
            }
        } // Try new route naming convention first, then fall back to legacy
        ::GetPrivateProfileString(TEXT("API"), TEXT("route_chat_completions"), L"", buffer, 1024, iniFilePath);
        if (wcslen(buffer) > 0)
        {
            configAPIValue_chatRoute = buffer;
        }
        else
        {
            // Fall back to legacy naming for backward compatibility
            ::GetPrivateProfileString(TEXT("API"), TEXT("chat_completions_route"), configAPIValue_chatRoute.c_str(), buffer, 1024, iniFilePath);
            configAPIValue_chatRoute = buffer;
        }

        ::GetPrivateProfileString(TEXT("API"), TEXT("response_type"), configAPIValue_responseType.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_responseType = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("proxy_url"), configAPIValue_proxyURL.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_proxyURL = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("model"), configAPIValue_model.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_model = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("temperature"), configAPIValue_temperature.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_temperature = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("max_tokens"), configAPIValue_maxTokens.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_maxTokens = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("top_p"), configAPIValue_topP.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_topP = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), configAPIValue_frequencyPenalty.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_frequencyPenalty = buffer;

        ::GetPrivateProfileString(TEXT("API"), TEXT("presence_penalty"), configAPIValue_presencePenalty.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_presencePenalty = buffer; // Load streaming option
        ::GetPrivateProfileString(TEXT("API"), TEXT("streaming"), configAPIValue_streaming.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_streaming = buffer;

        // Load show_reasoning option
        ::GetPrivateProfileString(TEXT("API"), TEXT("show_reasoning"), configAPIValue_showReasoning.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_showReasoning = buffer;

        // Read plugin settings if requested
        if (loadPluginSettings)
        {
            // Read chat settings
            TCHAR keepQuestionBuffer[2];
            ::GetPrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), isKeepQuestion ? TEXT("1") : TEXT("0"), keepQuestionBuffer, 2, iniFilePath);
            isKeepQuestion = (keepQuestionBuffer[0] == '1');

            // Read chat mode settings
            TCHAR isChatBuffer[2];
            ::GetPrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), TEXT("0"), isChatBuffer, 2, iniFilePath);
            _chatSettingsDlg.chatSetting_isChat = (isChatBuffer[0] == '1');

            // Read chat limit
            TCHAR chatLimitBuffer[6];
            ::GetPrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), TEXT("10"), chatLimitBuffer, 6, iniFilePath);
            _chatSettingsDlg.chatSetting_chatLimit = _wtoi(chatLimitBuffer);
        }

        // Read system instructions from file if it exists
        if (PathFileExists(instructionsFilePath))
        {
            // Parse any instructions/prompts from the instructions file
            std::vector<Prompt> prompts;
            parseInstructionsFile(instructionsFilePath, prompts);

            // If no named prompts, just read the whole file
            if (prompts.empty())
            {
                // Read the entire file content
                FILE *file = _wfopen(instructionsFilePath, L"r");
                if (file)
                {
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    rewind(file);

                    if (fileSize > 0)
                    {
                        char *fileContent = new char[fileSize + 1];
                        size_t readSize = fread(fileContent, 1, fileSize, file);
                        fileContent[readSize] = 0;

                        // Convert to wide string
                        std::wstring wideContent = multiByteToWideChar(fileContent);
                        configAPIValue_instructions = wideContent;

                        delete[] fileContent;
                    }
                    fclose(file);
                }
            }
        }
    }
}

/**
 * Exposes the loadConfig function for external calls
 *
 * This function acts as a wrapper for the internal implementation.
 * @param loadPluginSettings If true, also loads plugin-specific settings
 */
void loadConfig(bool loadPluginSettings)
{
    ConfigManagerImpl::loadConfig(loadPluginSettings);
}

/**
 * Opens the configuration file in Notepad++
 *
 * This function sends a message to Notepad++ to open the INI file.
 */
void openConfigFile()
{
    // Open INI
    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniFilePath);
}

/**
 * Opens the instructions file in Notepad++
 *
 * This function sends a message to Notepad++ to open the instructions file
 */
void openInstructionsFile()
{
    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)instructionsFilePath);
}
