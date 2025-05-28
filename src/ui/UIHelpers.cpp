/**
 * UIHelpers.cpp - User interface helper functions for NppOpenAI
 *
 * SEPARATION PLAN: Phase 3 - Dependency Injection Implementation
 * This file contains utility functions for managing UI elements of the plugin,
 * including menu items, toolbar icons, dialog boxes, and general user interaction.
 * It handles user preference toggling (including the "Keep my question" setting that
 * controls whether the original query is preserved in AI responses) and visual state management.
 *
 * REFACTORING STATUS: Prepared for service injection with backward compatibility
 * - Supports both service-based and legacy global-based operations
 * - Services can be injected for framework-independent operation
 * - Falls back to global variables when services not initialized
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#include <windows.h>
#include "UIHelpers.h"
#include "core/external_globals.h"
#include "PluginDefinition.h" // For toolbar icon definitions
#include <curl/curl.h>        // For LIBCURL_VERSION
#include <nlohmann/json.hpp>  // For JSON version constants
#include "EncodingUtils.h"    // For myMultiByteToWideChar
#include "interfaces/IUIService.h"
#include "interfaces/IConfigurationService.h"
#include "interfaces/IMenuService.h"
#include "interfaces/INotepadService.h"
#include <memory>

// SEPARATION PLAN: Service injection support
namespace
{
    // Private service instances for dependency injection
    std::shared_ptr<UIServices::IUIService> g_uiService = nullptr;
    std::shared_ptr<UIServices::IConfigurationService> g_configService = nullptr;
    std::shared_ptr<UIServices::IMenuService> g_menuService = nullptr;
    std::shared_ptr<UIServices::INotepadService> g_notepadService = nullptr;
}

/**
 * Toggles the "Keep my question" menu item state
 *
 * When enabled, the original user question is kept in the response.
 * When disabled, only the AI response is shown without the original question.
 *
 * SEPARATION PLAN: Updated to use service injection when available,
 * falls back to legacy global access for backward compatibility.
 */
void UIHelpers::keepQuestionToggler()
{
    if (areServicesInitialized())
    {
        // Use service-based approach
        g_uiService->toggleKeepQuestion();
    }
    else
    {
        // Legacy global-based approach (backward compatibility)
        isKeepQuestion = !isKeepQuestion;
        ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[6]._cmdID, MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
    }
}

/**
 * Opens the Chat Settings dialog
 *
 * Shows a dialog where users can toggle chat mode and set message history limits
 */
void UIHelpers::openChatSettingsDlg()
{
    _chatSettingsDlg.doDialog();
}

/**
 * Updates the Chat Settings menu item text and optionally saves to INI file
 *
 * SEPARATION PLAN: Updated to use service injection when available,
 * falls back to legacy global access for backward compatibility.
 *
 * @param isWriteToFile If true, writes the current settings to the INI file
 */
void UIHelpers::updateChatSettings(bool isWriteToFile)
{
    // Prepare menu text based on chat settings
    std::wstring menuText;
    if (!_chatSettingsDlg.chatSetting_isChat || !_chatSettingsDlg.chatSetting_chatLimit)
    {
        menuText = L"&Chat: off";
    }
    else
    {
        wchar_t chatLimitNewBuffer[32];
        wsprintfW(chatLimitNewBuffer, L"&Chat limit: %d", _chatSettingsDlg.chatSetting_chatLimit);
        menuText = chatLimitNewBuffer;
    }

    if (areServicesInitialized())
    {
        // Use service-based approach
        g_menuService->updateChatMenuText(menuText);

        if (isWriteToFile)
        {
            g_configService->saveChatSettings(_chatSettingsDlg.chatSetting_isChat,
                                              _chatSettingsDlg.chatSetting_chatLimit);
        }
    }
    else
    {
        // Legacy global-based approach (backward compatibility)
        HMENU chatMenu = ::GetMenu(nppData._nppHandle);
        MENUITEMINFOW menuItemInfo{};
        menuItemInfo.cbSize = sizeof(MENUITEMINFOW);
        menuItemInfo.fMask = MIIM_TYPE | MIIM_DATA;
        menuItemInfo.dwTypeData = const_cast<LPWSTR>(menuText.c_str());
        SetMenuItemInfoW(chatMenu, funcItem[7]._cmdID, MF_STRING, &menuItemInfo);

        if (isWriteToFile)
        {
            wchar_t chatLimitBuffer[6];
            wsprintfW(chatLimitBuffer, L"%d", _chatSettingsDlg.chatSetting_chatLimit);
            ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), _chatSettingsDlg.chatSetting_isChat ? TEXT("1") : TEXT("0"), iniFilePath);
            ::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), chatLimitBuffer, iniFilePath);
        }
    }
}

/**
 * Adds or updates toolbar icons
 *
 * SEPARATION PLAN: Updated to use service injection when available,
 * falls back to legacy global access for backward compatibility.
 *
 * Prepares and sends toolbar icons to Notepad++ for the Chat Settings feature.
 * Icons are updated based on the current chat mode and settings.
 */
void UIHelpers::updateToolbarIcons()
{
    if (areServicesInitialized())
    {
        // Use service-based approach
        g_menuService->updateToolbarIcons();
        // Also update the chat settings menu
        updateChatSettings();
    }
    else
    {
        // Legacy global-based approach (backward compatibility)
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
}

/**
 * Opens the About dialog
 *
 * SEPARATION PLAN: Updated to use service injection when available,
 * falls back to legacy global access for backward compatibility.
 *
 * Displays information about the plugin, including version details and dependencies.
 */
void UIHelpers::openAboutDlg()
{
    char about[255];
    sprintf(about, "\
OpenAI (aka. ChatGPT) plugin for Notepad++ v%s by Richard Stockinger\n\n\
This plugin uses libcurl v%s with OpenSSL and nlohmann/json v%d.%d.%d\n\n\
Thank you to the contributors for their support!\n\
- Andrea Tomassi\n\
- chcg\n\
- Gitoffthelawn\
",
            NPPOPENAI_VERSION, LIBCURL_VERSION, NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);

    if (areServicesInitialized())
    {
        // Use service-based approach
        g_uiService->showAboutDialog(std::string(about));
    }
    else
    {
        // Legacy global-based approach (backward compatibility)
        ::MessageBox(nppData._nppHandle, myMultiByteToWideChar(about), TEXT("About"), MB_OK);
    }
}

// SEPARATION PLAN: Service injection implementation
void UIHelpers::initializeServices(
    std::shared_ptr<UIServices::IUIService> uiService,
    std::shared_ptr<UIServices::IConfigurationService> configService,
    std::shared_ptr<UIServices::IMenuService> menuService,
    std::shared_ptr<UIServices::INotepadService> notepadService)
{
    g_uiService = uiService;
    g_configService = configService;
    g_menuService = menuService;
    g_notepadService = notepadService;
}

bool UIHelpers::areServicesInitialized()
{
    return g_uiService != nullptr && g_configService != nullptr &&
           g_menuService != nullptr && g_notepadService != nullptr;
}
