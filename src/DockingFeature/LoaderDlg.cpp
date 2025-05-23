// this file is part of notepad++
// Copyright (C)2022 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "../PluginDefinition.h"
#include "LoaderDlg.h"
#include "../external_globals.h"

extern NppData nppData;

// Animation counters
static int dotCount = 0;
static int spinnerCount = 0;
// Spinner characters (rotating animation) - using simple ASCII characters that work in all fonts
static const TCHAR *spinnerChars[] = {TEXT("|"), TEXT("/"), TEXT("-"), TEXT("\\")};
static const int spinnerCharsCount = sizeof(spinnerChars) / sizeof(spinnerChars[0]);



void LoaderDlg::doDialog(bool) { // bool isRTL -- for modeless dialogs only!

	// TODO:
	// `::DialogBoxParam` (modal) blocks the main thread until the dialog is closed,
	// `::CreateDialogParam` (modeless) doesn't handle keyboard events (ESC, ENTER, etc.) properly
	::CreateDialogParam(_hInst, MAKEINTRESOURCE(IDD_PLUGINNPPOPENAI_LOADING), nppData._nppHandle, StaticDlgProc, reinterpret_cast<LPARAM>(this));
	if (_hSelf)
	{
		::ShowWindow(_hSelf, SW_SHOW);
		::UpdateWindow(_hSelf);
	}
}

// Required for `run_dlgProc()`
INT_PTR CALLBACK LoaderDlg::StaticDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LoaderDlg* pThis = nullptr;

	if (message == WM_INITDIALOG) {
		pThis = reinterpret_cast<LoaderDlg*>(lParam);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		pThis->_hSelf = hWnd;
	}
	else {
		pThis = reinterpret_cast<LoaderDlg*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->run_dlgProc(message, wParam, lParam);
	}

	return FALSE;
}

// Handle the dialog messages
INT_PTR CALLBACK LoaderDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		// Set progress bar to 'indeterminate' (marquee) animation style
		HWND progressBar = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS); // See more: `LoaderDlg.rc`
		::SendMessage(progressBar, PBM_SETMARQUEE, TRUE, 20);						  // 20ms refresh rate (1: too fast; 50: too slow)

		// Center the dialog (already using DS_CENTER style)
		HWND spinnerControl = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
		if (spinnerControl)
		{
			// Set the initial spinner character
			::SetWindowText(spinnerControl, spinnerChars[0]);

			// Update font for spinner to make it larger - use a common monospace font
			HFONT hFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
									 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
									 DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Courier New"));
			if (hFont)
				SendMessage(spinnerControl, WM_SETFONT, (WPARAM)hFont, TRUE);
		}

		// Reset counters and elapsed time
		dotCount = 0; // dots disabled
		spinnerCount = 0;
		_startTime = ::GetTickCount64();
		_elapsedSeconds = 0;

		// Start timers for animation and elapsed time counter
		::SetTimer(_hSelf, 1, 150, NULL);  // Timer ID 1 for dots/spinner animation (faster)
		::SetTimer(_hSelf, 2, 1000, NULL); // Timer ID 2 for elapsed time (every second)

		return TRUE;
	}
	case WM_SHOWWINDOW:
	{
		if (wParam == TRUE) // Window is being shown
		{

			// If showing the window, ensure it's properly displayed
			// Record the start time when first showing the dialog
			_startTime = ::GetTickCount64();
			_elapsedSeconds = 0;

			// Start timers for both animations and elapsed time
			::SetTimer(_hSelf, 1, 150, NULL);  // Timer ID 1 for spinner animation
			::SetTimer(_hSelf, 2, 1000, NULL); // Timer ID 2 is for elapsed time (every second)

			// Bring to top, update, and force repaint
			::SetForegroundWindow(_hSelf);
			::SetWindowPos(_hSelf, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			::InvalidateRect(_hSelf, NULL, TRUE);
			::UpdateWindow(_hSelf);

			// Reset the cancel flag
			isCancelled = false;

			// Force a redraw when shown
			::InvalidateRect(_hSelf, NULL, TRUE);
			::UpdateWindow(_hSelf);

			// Set initial spinner state
			HWND spinnerControl = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
			if (spinnerControl)
			{
				::SetWindowText(spinnerControl, spinnerChars[0]);
			}

			// Set initial static text to model name response
			TCHAR modelText[128];
			swprintf(modelText, 128, TEXT("“%s” AI model will respond"), configAPIValue_model.c_str());
			::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_STATIC, modelText);

			// Reset the elapsed time text
			TCHAR timeText[128];
			swprintf(timeText, 128, TEXT("Waiting for %llu seconds..."), static_cast<unsigned long long>(0));
			::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_ESTIMATE, timeText);

			// Reset dot counter, elapsed time, and start timers
			dotCount = 0;
			spinnerCount = 0;
			_startTime = ::GetTickCount64();
			_elapsedSeconds = 0;
			::SetTimer(_hSelf, 1, 150, NULL);  // Animation timer (faster for smoother animation)
			::SetTimer(_hSelf, 2, 1000, NULL); // Elapsed time counter
		}
		else
		{
			// Window is being hidden, kill the timers
			::KillTimer(_hSelf, 1);
			::KillTimer(_hSelf, 2);
		}
		return TRUE;
	}
	case WM_TIMER:
	{
		if (wParam == 1) // Our animation timer
		{
			// Update spinner animation (no dots)
			spinnerCount = (spinnerCount + 1) % spinnerCharsCount;
			HWND spinnerControl = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
			if (spinnerControl)
			{
				::SetWindowText(spinnerControl, spinnerChars[spinnerCount]);
				::InvalidateRect(spinnerControl, NULL, TRUE);
				::UpdateWindow(spinnerControl);
			}
		}
		else if (wParam == 2) // Elapsed time timer
		{
			// Calculate elapsed time in seconds
			ULONGLONG currentTime = ::GetTickCount64();
			_elapsedSeconds = (currentTime - _startTime) / 1000;

			// Update the elapsed time text
			TCHAR timeText[128];
			swprintf(timeText, 128, TEXT("Waiting for %llu seconds..."), _elapsedSeconds);
			::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_ESTIMATE, timeText);
		}
		return TRUE;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{

		// Stop loading/streaming AI answer and close the dialog
		case IDCANCEL:
		case ID_PLUGINNPPOPENAI_LOADING_CANCEL:
			isCancelled = true; // We indicate that the user has cancelled the operation
			::KillTimer(_hSelf, 1);
			::KillTimer(_hSelf, 2);
			::EndDialog(_hSelf, LOWORD(wParam));
			return TRUE;
		}
		return TRUE;
	}
	case WM_DESTROY:
	{

		// Make sure we kill all timers when the dialog is destroyed
		::KillTimer(_hSelf, 1); // Animation timer
		::KillTimer(_hSelf, 2); // Elapsed time timer

		// Clean up font resource
		HWND spinnerControl = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
		if (spinnerControl)
		{
			HFONT hFont = (HFONT)SendMessage(spinnerControl, WM_GETFONT, 0, 0);
			if (hFont && hFont != (HFONT)GetStockObject(DEFAULT_GUI_FONT))
				DeleteObject(hFont);
		}
		return TRUE;
	}

	default:
		return FALSE;
	}
}


void LoaderDlg::resetTimer()
{
	_startTime = ::GetTickCount64();
	_elapsedSeconds = 0;

	// Update the elapsed time text immediately
	if (isCreated() && ::IsWindowVisible(_hSelf))
	{
		TCHAR timeText[128];
		swprintf(timeText, 128, TEXT("Waiting for %llu seconds..."), static_cast<unsigned long long>(0));
		::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_ESTIMATE, timeText);
	}
}