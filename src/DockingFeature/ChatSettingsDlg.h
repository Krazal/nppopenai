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

#ifndef PLUGINNPPOPENAI_CHATSETTINGS_DLG_H
#define PLUGINNPPOPENAI_CHATSETTINGS_DLG_H

/*
// Instead of `#include <commctrl.h>` we can define the required constants only!
#define UDM_SETRANGE WM_USER + 101
#define UDM_SETBASE  WM_USER + 109
#define UDM_SETBUDDY WM_USER + 105
#define UD_MAXVAL    0x7fff // 32767 (more than enough)
// #define UD_MINVAL (-UD_MAXVAL) // Not required
*/

#include "DockingDlgInterface.h"
#include "chatSettingsResource.h"
#include <windowsx.h>
#include <commctrl.h>

class ChatSettingsDlg : public StaticDialog
{
public:
	ChatSettingsDlg() : StaticDialog() {};

	void doDialog(bool isRTL = false);
	bool chatSetting_isChat;
	int chatSetting_chatLimit;

	// Create a chat settings MODAL dialog to enable/disable OpenAI chat and set its limit
	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true) {
		StaticDialog::create(dialogID, isRTL, msgDestParent);
	};

	// Toggle loader dialog visibility
	virtual void display(bool toShow = true) const {
		StaticDialog::display(toShow);
	};


protected:
	bool enableDisable_isChatChecked;
	static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
	void updateDialog();
	void enableDisableDlgItems(bool forceUpdate = false);
};


#endif // PLUGINNPPOPENAI_CHATSETTINGS_DLG_H
