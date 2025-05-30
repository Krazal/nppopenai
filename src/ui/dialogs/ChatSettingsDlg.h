// Chat settings dialog header
// Provides UI for configuring OpenAI chat behavior, streaming options,
// and conversation management settings within the NppOpenAI plugin

/**
 * ChatSettingsDlg.h - Chat settings dialog class
 *
 * SEPARATION PLAN: Well-Separated Component ✅
 * This dialog is already well-separated with minimal coupling:
 * - Only accesses nppData for dialog parent window (necessary for modal dialogs)
 * - Self-contained state management with internal variables
 * - No direct access to configuration globals
 * - Clean dialog interface suitable for different UI frameworks
 *
 * FRAMEWORK REPLACEMENT: This dialog can be easily replaced with Qt/WPF equivalents
 * by implementing the same interface pattern with framework-specific modal dialogs.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 *
 * This file defines the dialog for configuring chat functionality in the NppOpenAI plugin.
 * It allows users to enable/disable chat mode and set the message history limit.
 *
 * Copyright (C)2022 Don HO <don.h@free.fr>
 */

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

/**
 * Dialog class for configuring chat functionality
 *
 * This dialog allows users to enable or disable chat mode, which maintains
 * conversation context between API calls, and set the history limit to
 * control token usage.
 */
class ChatSettingsDlg : public StaticDialog
{
public:
	ChatSettingsDlg() : StaticDialog() {};

	/**
	 * Shows the chat settings dialog
	 *
	 * @param isRTL Whether to use right-to-left text layout
	 */
	void doDialog(bool isRTL = false);

	/**
	 * Whether chat mode is enabled
	 * When enabled, conversation history is maintained between API calls
	 */
	bool chatSetting_isChat;

	/**
	 * Maximum number of messages to keep in chat history
	 * Higher values provide more context but use more tokens
	 */
	int chatSetting_chatLimit;

	/**
	 * Creates the dialog
	 *
	 * @param dialogID Resource ID of the dialog template
	 * @param isRTL Whether to use right-to-left text layout
	 * @param msgDestParent Whether messages should be sent to parent
	 */
	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true)
	{
		StaticDialog::create(dialogID, isRTL, msgDestParent);
	};

	/**
	 * Controls dialog visibility
	 *
	 * @param toShow Whether to show or hide the dialog
	 */
	virtual void display(bool toShow = true) const
	{
		StaticDialog::display(toShow);
	};

protected:
	/**
	 * Whether the chat checkbox is checked
	 * Used to enable or disable chat functionality
	 */
	bool enableDisable_isChatChecked;

	/**
	 * Static dialog procedure
	 *
	 * @param hWnd Handle to the dialog window
	 * @param message Message identifier
	 * @param wParam Additional message information
	 * @param lParam Additional message information
	 * @return Result of message processing
	 */
	static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/**
	 * Runs the dialog procedure
	 *
	 * @param message Message identifier
	 * @param wParam Additional message information
	 * @param lParam Additional message information
	 * @return Result of message processing
	 */
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

	/**
	 * Updates the dialog controls
	 * Ensures the controls reflect the current settings
	 */
	void updateDialog();

	/**
	 * Enables or disables dialog items
	 *
	 * @param forceUpdate Whether to force an update of the controls
	 */
	void enableDisableDlgItems(bool forceUpdate = false);
};

#endif // PLUGINNPPOPENAI_CHATSETTINGS_DLG_H
