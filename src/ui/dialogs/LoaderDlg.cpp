// Loading dialog implementation
// Provides animated progress indication during API requests with
// spinner animation and status text updates for user feedback

#include "core/PluginDefinition.h"
#include "LoaderDlg.h"
#include "core/external_globals.h"

extern NppData nppData;

// Animation counters
static int dotCount = 0;
static int spinnerCount = 0;
// Spinner characters (rotating animation) - using simple ASCII characters that work in all fonts
static const TCHAR *spinnerChars[] = {TEXT("|"), TEXT("/"), TEXT("-"), TEXT("\\")};
static const int spinnerCharsCount = sizeof(spinnerChars) / sizeof(spinnerChars[0]);

INT_PTR CALLBACK LoaderDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
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
			// Force a redraw when shown
			::InvalidateRect(_hSelf, NULL, TRUE);
			::UpdateWindow(_hSelf);

			// Set initial spinner state
			HWND spinnerControl = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
			if (spinnerControl)
			{
				::SetWindowText(spinnerControl, spinnerChars[0]);
			} // Set initial static text using the model name if available
			// This will be updated by the caller using setModelName()
			// No direct access to global config variables here

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
			_isCancelled = true; // We indicate that the user has cancelled the operation
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
