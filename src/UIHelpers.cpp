// filepath: src/UIHelpers.cpp
#include <windows.h>
#include "UIHelpers.h"
#include "external_globals.h"
#include "PluginDefinition.h" // For toolbar icon definitions
#include <curl/curl.h>        // For LIBCURL_VERSION
#include <nlohmann/json.hpp>  // For JSON version constants
#include "EncodingUtils.h"    // For myMultiByteToWideChar

// Toggle "Keep my question" menu item
void UIHelpers::keepQuestionToggler()
{
    isKeepQuestion = !isKeepQuestion;
    ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[6]._cmdID, MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
}

// Open Chat Settings dialog
void UIHelpers::openChatSettingsDlg()
{
    _chatSettingsDlg.doDialog();
}

// Update chat settings menu label and write to INI if needed
void UIHelpers::updateChatSettings(bool isWriteToFile)
{
    HMENU chatMenu = ::GetMenu(nppData._nppHandle);
    MENUITEMINFOW menuItemInfo{};
    menuItemInfo.cbSize = sizeof(MENUITEMINFOW);
    menuItemInfo.fMask = MIIM_TYPE | MIIM_DATA;
    if (!_chatSettingsDlg.chatSetting_isChat || !_chatSettingsDlg.chatSetting_chatLimit)
    {
        menuItemInfo.dwTypeData = TEXT("&Chat: off");
    }
    else
    {
        wchar_t chatLimitNewBuffer[32];
        wsprintfW(chatLimitNewBuffer, L"&Chat limit: %d", _chatSettingsDlg.chatSetting_chatLimit);
        menuItemInfo.dwTypeData = chatLimitNewBuffer;
    }
    SetMenuItemInfoW(chatMenu, funcItem[7]._cmdID, MF_STRING, &menuItemInfo);

    if (isWriteToFile)
    {
        wchar_t chatLimitBuffer[6];
        wsprintfW(chatLimitBuffer, L"%d", _chatSettingsDlg.chatSetting_chatLimit);
        ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), _chatSettingsDlg.chatSetting_isChat ? TEXT("1") : TEXT("0"), iniFilePath);
        ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), chatLimitBuffer, iniFilePath);
    }
}

// Add/update toolbar icons
void UIHelpers::updateToolbarIcons()
{
    // Prepare icons to open Chat Settings
    int hToolbarBmp = IDB_PLUGINNPPOPENAI_TOOLBAR_CHAT;
    int hToolbarIcon = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT;
    int hToolbarIconDarkMode = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT_DM;
    /* // TODO: update toolbar icons on-the-fly (turning chat on/off or reaching chat limit)
    if (!_chatSettingsDlg.chatSetting_isChat || _chatSettingsDlg.chatSetting_chatLimit == 0)
    {
        hToolbarBmp = IDB_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT;
        hToolbarIcon = IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT;
        hToolbarIconDarkMode = IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT_DM;
    }
    // */

    // Send Chat Settings icons to Notepad++
    toolbarIconsWithDarkMode chatSettingsIcons;
    chatSettingsIcons.hToolbarBmp = ::LoadBitmap((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarBmp));
    chatSettingsIcons.hToolbarIcon = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIcon));
    chatSettingsIcons.hToolbarIconDarkMode = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIconDarkMode));
    ::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_FORDARKMODE, funcItem[7]._cmdID, (LPARAM)&chatSettingsIcons);

    // Update chat settings menu label
    UIHelpers::updateChatSettings();
}

// About dialog
void UIHelpers::openAboutDlg()
{
    char about[255];
    sprintf(about, "\
OpenAI (aka. ChatGPT) plugin for Notepad++ v%s by Richard Stockinger\n\n\
This plugin uses libcurl v%s with OpenSSL and nlohmann/json v%d.%d.%d\
",
            NPPOPENAI_VERSION, LIBCURL_VERSION, NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
    ::MessageBox(nppData._nppHandle, myMultiByteToWideChar(about), TEXT("About"), MB_OK);
}
