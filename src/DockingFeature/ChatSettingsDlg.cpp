//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "../PluginDefinition.h"
#include "ChatSettingsDlg.h"

extern NppData nppData;

// Update + show the chat settings dialog
void ChatSettingsDlg::doDialog(bool) { // bool isRTL -- for modeless dialogs only!
    /* // You don't want to use modeless dialog, aren't you?
    if (!isCreated())
        create(IDD_PLUGINNPPOPENAI_CHATSETTINGS, isRTL);
    */

    // Use modal dialog instead. cU -- For `updateDialog()` see: `WM_INITDIALOG`
    ::DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_PLUGINNPPOPENAI_CHATSETTINGS), nppData._nppHandle, StaticDlgProc, reinterpret_cast<LPARAM>(this)); // Don't dare to use the `_hParent` variable! XDDD
};


// Required for `run_dlgProc()`
INT_PTR CALLBACK ChatSettingsDlg::StaticDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    ChatSettingsDlg* pThis = nullptr;

    if (message == WM_INITDIALOG) {
        pThis = reinterpret_cast<ChatSettingsDlg*>(lParam);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->_hSelf = hWnd;
    }
    else {
        pThis = reinterpret_cast<ChatSettingsDlg*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->run_dlgProc(message, wParam, lParam);
    }

    return FALSE;
}


// The magic happens here...
INT_PTR CALLBACK ChatSettingsDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrush = GetSysColorBrush(COLOR_BTNFACE);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        enableDisable_isChatChecked = chatSetting_isChat;
        // goToCenter(); <-- Not really working (on multiple monitors)...
        updateDialog();

        /* May be useful once...
        auto enable_dlg_theme = reinterpret_cast<ETDTProc>(::SendMessage(_hParent, NPPM_GETENABLETHEMETEXTUREFUNC, 0, 0));
        if (enable_dlg_theme != nullptr)
            enable_dlg_theme(_hSelf, ETDT_ENABLETAB);
        */

        /*
        // Set links -- may be uesful...
        std::wstring urlIssue(TEXT("<a href=\"__URL__\">__URL__</a>"));
        std::wstring urlRepo = urlIssue;

        urlIssue = StringHelper::ReplaceAll(urlIssue, TEXT("__URL__"), URL_REPORT_ISSUE);
        urlRepo = StringHelper::ReplaceAll(urlRepo, TEXT("__URL__"), URL_SOURCE_CODE);

        SetWindowText(::GetDlgItem(_hSelf, IDC_WEB_ISSUE), urlIssue.c_str());
        SetWindowText(::GetDlgItem(_hSelf, IDC_WEB_SOURCE), urlRepo.c_str());
        */

        return true;
    }

    // Update chat limit related text colors (disabled/default)
    case WM_CTLCOLORSTATIC:
    {
        if (::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_STATIC) == (HWND)lParam
            || ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_INFO) == (HWND)lParam)
        {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            if (IsDlgButtonChecked(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_USECHAT_CHECK))
            {
                SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            }
            else
            {
                SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
            }
        }
        return (LRESULT)hBrush;
    }

    // Toggle (enable/disable) items on repaint
    case WM_NOTIFY:
    {
        if (reinterpret_cast<LPNMHDR>(lParam)->code == NM_CUSTOMDRAW && ((LPNMHDR)lParam)->idFrom == ID_PLUGINNPPOPENAI_CHATSETTINGS_USECHAT_CHECK)
        {
            enableDisableDlgItems();
        }
        return FALSE;
    }

    // Handle "OK" and "Cancel" commands
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {

        // Save settings + close dialog
        case IDOK:
        case ID_PLUGINNPPOPENAI_CHATSETTINGS_OK:

            // Update chat settings (properties)
            TCHAR chatLimitBuffer[6];
            Edit_GetText(::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_EDIT), chatLimitBuffer, 6);
            chatSetting_chatLimit = _ttoi(chatLimitBuffer);
            chatSetting_isChat = (IsDlgButtonChecked(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_USECHAT_CHECK) == BST_CHECKED);
            updateChatSettings(true);

            // Handle dialog (close)
            ::DeleteObject(hBrush);
            ::EndDialog(_hSelf, wParam);
            // ::SendMessage(_hParent, NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, (WPARAM)_hSelf); <-- Not required as we don't use modeless dialog
            _hSelf = nullptr;
            return true;

        // Close dialog (without save)
        case IDCANCEL:
        case ID_PLUGINNPPOPENAI_CHATSETTINGS_CANCEL:
            ::DeleteObject(hBrush);
            ::EndDialog(_hSelf, wParam);
            // ::SendMessage(_hParent, NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE, (WPARAM)_hSelf); <-- Not required as we don't use modeless dialog
            return true;
        }
    }
    }
    return false;
}


// Helper method: Update dialog items (state/color, value etc.)
void ChatSettingsDlg::updateDialog()
{
    // Get + update "Use Chat" checkbox
    HWND useChatCheck = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_USECHAT_CHECK);
    Button_SetCheck(useChatCheck, chatSetting_isChat);

    // Get text field + up-down element
    HWND limitEdit = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_EDIT);
    HWND limitUpDown = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_UPDOWN);

    // Update text field (edit) value and up-down element
    int chatLimit2use = (chatSetting_chatLimit <= 0)
        ? 1 // Chat limit: min. value
        : ((chatSetting_chatLimit > UD_MAXVAL)
            ? UD_MAXVAL // Chat limit: max. value
            : chatSetting_chatLimit);
    wchar_t chatLimitBuffer[6];
    wsprintfW(chatLimitBuffer, L"%d", chatLimit2use);
    Edit_SetText(limitEdit, chatLimitBuffer);
    ::SendMessage(limitUpDown, UDM_SETBASE, 10, 0); // 10: for decimal
    ::SendMessage(limitUpDown, UDM_SETRANGE, 0, MAKELPARAM(UD_MAXVAL, 1));

    // Pair text field (edit) and up-down element (set buddy)
    ::SendMessage(limitUpDown, UDM_SETBUDDY, reinterpret_cast<WPARAM>(limitEdit), static_cast<LPARAM>(NULL));

    // Display dialog + focus on "Use chat" checkbox
    display();

    // Update text item colors + toggle text field / up-down
    enableDisableDlgItems(true);

    /*
    // Set focus on "Use Chat" checkbox -- modify if necessary!
    SetFocus(useChatCheck);
    */
}


// Helper method: Toggle chat limit (text field) and up-down item + re-paint static texts (WM_CTLCOLORSTATIC) by "updating" them
void ChatSettingsDlg::enableDisableDlgItems(bool forceUpdate)
{
    bool isChatChecked = (IsDlgButtonChecked(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_USECHAT_CHECK) == BST_CHECKED);
    if (forceUpdate || enableDisable_isChatChecked != isChatChecked) // Recommended to avoid flickering
    {
        HWND limitEdit = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_EDIT);
        HWND limitUpDown = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_UPDOWN);
        Edit_Enable(limitEdit, isChatChecked);
        Edit_Enable(limitUpDown, isChatChecked);

        // Re-paint static text items
        wchar_t tmpText[256];
        ::GetDlgItemTextW(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_STATIC, (LPWSTR)tmpText, 256);
        ::SetDlgItemTextW(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_LIMIT_STATIC, tmpText);
        ::GetDlgItemTextW(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_INFO, (LPWSTR)tmpText, 256);
        ::SetDlgItemTextW(_hSelf, ID_PLUGINNPPOPENAI_CHATSETTINGS_INFO, tmpText);
    }
    enableDisable_isChatChecked = isChatChecked;
}
