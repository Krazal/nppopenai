/**
 * GlobalMenuService.cpp - Global Implementation of IMenuService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This implementation provides the IMenuService interface while using
 * current global variables and Win32 menu APIs. This maintains full
 * backward compatibility during the UI separation transition.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#include "GlobalMenuService.h"
#include "../../core/external_globals.h"
#include "../../core/PluginDefinition.h"
#include <windows.h>

namespace UIServices
{
    void GlobalMenuService::updateChatMenuText(const std::wstring &text)
    {
        // Use existing global nppData and funcItem access
        HMENU chatMenu = ::GetMenu(nppData._nppHandle);
        MENUITEMINFOW menuItemInfo{};
        menuItemInfo.cbSize = sizeof(MENUITEMINFOW);
        menuItemInfo.fMask = MIIM_TYPE | MIIM_DATA;
        menuItemInfo.dwTypeData = const_cast<LPWSTR>(text.c_str());
        SetMenuItemInfoW(chatMenu, funcItem[7]._cmdID, MF_STRING, &menuItemInfo);
    }

    void GlobalMenuService::setMenuItemChecked(int commandId, bool checked)
    {
        // Use existing global nppData access
        ::CheckMenuItem(::GetMenu(nppData._nppHandle), commandId,
                        MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
    }

    void GlobalMenuService::updateToolbarIcons()
    {
        // Use existing global variables for toolbar icon management
        // This is a complex operation that involves resource loading and Notepad++ messaging
        // For now, maintain the existing implementation pattern

        // Prepare icons to open Chat Settings
        int hToolbarBmp = IDB_PLUGINNPPOPENAI_TOOLBAR_CHAT;
        int hToolbarIcon = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT;
        int hToolbarIconDarkMode = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT_DM;

        // Send Chat Settings icons to Notepad++
        toolbarIconsWithDarkMode chatSettingsIcons;
        chatSettingsIcons.hToolbarBmp = ::LoadBitmap((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarBmp));
        chatSettingsIcons.hToolbarIcon = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIcon));
        chatSettingsIcons.hToolbarIconDarkMode = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIconDarkMode));
        ::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_FORDARKMODE, funcItem[7]._cmdID, (LPARAM)&chatSettingsIcons);
    }

    HMENU GlobalMenuService::getMainMenu() const
    {
        // Return main menu handle from global nppData
        return ::GetMenu(nppData._nppHandle);
    }

    int GlobalMenuService::getCommandId(int functionIndex) const
    {
        // Return command ID from global funcItem array
        return funcItem[functionIndex]._cmdID;
    }
}
