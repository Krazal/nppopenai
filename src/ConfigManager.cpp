#include <windows.h>
#include <shlwapi.h>
#include "ConfigManager.h"
#include "external_globals.h" // for global variables and functions
#include "EncodingUtils.h"    // for UTF-8 conversions
#include "PromptManager.h"    // for parsing instructions file
#include <cstdio>
#include <vector>

void writeDefaultConfig()
{
    // Write default config - headers first
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; === PLEASE ENTER YOUR OPENAI SECRET API KEY BELOW ==="), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; This plugin requires a OpenAI registration and a secret API key."), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; You can get your API key at: https://platform.openai.com/account/api-keys"), TEXT(""), iniFilePath);
    ::WritePrivateProfileString(TEXT("INFO"), TEXT("; After obtaining, please enter it below (API_KEY=...) and then reload config!"), TEXT(""), iniFilePath); // Write default API section values
    ::WritePrivateProfileString(TEXT("API"), TEXT("secret_key"), TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("api_url"), TEXT("https://api.openai.com/"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("proxy_url"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("model"), TEXT("gpt-4o-mini"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("temperature"), TEXT("0.7"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("max_tokens"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("top_p"), TEXT("0.8"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), TEXT("0"), iniFilePath);
    ::WritePrivateProfileString(TEXT("API"), TEXT("presence_penalty"), TEXT("0"), iniFilePath);

    // Create empty plugin section
    ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("total_tokens_used"), TEXT("0"), iniFilePath);
}

// Implementation of the loadConfig function declared in ConfigManager.h
// This is the actual implementation that will be called from PluginDefinition.cpp

namespace ConfigManagerImpl
{
    void loadConfig(bool loadPluginSettings)
    {
        // Check if the config file exists; if not, create it with default values
        if (PathFileExists(iniFilePath) == FALSE)
        {
            // Write default config - headers first
            ::WritePrivateProfileString(TEXT("INFO"), TEXT("; === PLEASE ENTER YOUR OPENAI SECRET API KEY BELOW ==="), TEXT(""), iniFilePath);
            ::WritePrivateProfileString(TEXT("INFO"), TEXT("; This plugin requires a OpenAI registration and a secret API key."), TEXT(""), iniFilePath);
            ::WritePrivateProfileString(TEXT("INFO"), TEXT("; You can get your API key at: https://platform.openai.com/account/api-keys"), TEXT(""), iniFilePath);
            ::WritePrivateProfileString(TEXT("INFO"), TEXT("; After obtaining, please enter it below (API_KEY=...) and then reload config!"), TEXT(""), iniFilePath); // Write default API section values
            ::WritePrivateProfileString(TEXT("API"), TEXT("secret_key"), configAPIValue_secretKey.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("api_url"), configAPIValue_baseURL.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("proxy_url"), configAPIValue_proxyURL.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("model"), configAPIValue_model.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("temperature"), configAPIValue_temperature.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("max_tokens"), configAPIValue_maxTokens.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("top_p"), configAPIValue_topP.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), configAPIValue_frequencyPenalty.c_str(), iniFilePath);
            ::WritePrivateProfileString(TEXT("API"), TEXT("presence_penalty"), configAPIValue_presencePenalty.c_str(), iniFilePath);

            // Create empty plugin section
            ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("total_tokens_used"), TEXT("0"), iniFilePath);
        } // Read values from the config file
        TCHAR buffer[1024]; // Read API Key
        ::GetPrivateProfileString(TEXT("API"), TEXT("secret_key"), configAPIValue_secretKey.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_secretKey = buffer;

        // Debug check for API key
        if (configAPIValue_secretKey == TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE") || configAPIValue_secretKey.empty())
        {
            ::MessageBox(nppData._nppHandle,
                         TEXT("API key not properly configured. Please edit the config file and set a valid API key."),
                         TEXT("NppOpenAI Configuration Error"),
                         MB_ICONERROR);
        }

        // Read other API settings
        ::GetPrivateProfileString(TEXT("API"), TEXT("api_url"), configAPIValue_baseURL.c_str(), buffer, 1024, iniFilePath);
        configAPIValue_baseURL = buffer;

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
        configAPIValue_presencePenalty = buffer;

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

// Expose as a global function for external calls
void loadConfig(bool loadPluginSettings)
{
    ConfigManagerImpl::loadConfig(loadPluginSettings);
}

void openConfigFile()
{
    // Open INI
    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniFilePath);
}

void openInstructionsFile()
{
    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)instructionsFilePath);
    ::MessageBox(nppData._nppHandle,
                 TEXT("After saving this file, click Plugins » NppOpenAI » Load Config to apply changes."),
                 TEXT("NppOpenAI: Instructions"),
                 MB_ICONINFORMATION);
}
