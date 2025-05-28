/**
 * GlobalConfigService.cpp - Global Implementation of IConfigurationService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This implementation provides the IConfigurationService interface while using
 * current global iniFilePath variable and Win32 INI APIs. This maintains full
 * backward compatibility during the UI separation transition.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#include "GlobalConfigService.h"
#include "../../core/external_globals.h"
#include <windows.h>
#include <sstream>

namespace UIServices
{
    void GlobalConfigService::saveChatSettings(bool isChat, int chatLimit)
    {
        // Use existing global iniFilePath with Win32 APIs
        wchar_t chatLimitBuffer[16];
        wsprintfW(chatLimitBuffer, L"%d", chatLimit);

        ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"),
                                    isChat ? TEXT("1") : TEXT("0"), iniFilePath);
        ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"),
                                    chatLimitBuffer, iniFilePath);
    }

    std::wstring GlobalConfigService::getConfigPath() const
    {
        // Return current global iniFilePath
        return std::wstring(iniFilePath);
    }

    void GlobalConfigService::writeString(const std::wstring &section,
                                          const std::wstring &key,
                                          const std::wstring &value)
    {
        // Use existing global iniFilePath
        ::WritePrivateProfileString(section.c_str(), key.c_str(), value.c_str(), iniFilePath);
    }
    std::wstring GlobalConfigService::readString(const std::wstring &section,
                                                 const std::wstring &key,
                                                 const std::wstring &defaultValue)
    {
        // Use existing global iniFilePath
        wchar_t buffer[MAX_PATH];
        ::GetPrivateProfileString(section.c_str(), key.c_str(),
                                  defaultValue.c_str(), buffer,
                                  MAX_PATH, iniFilePath);
        return std::wstring(buffer);
    }
}
