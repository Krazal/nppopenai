/**
 * LoaderDlg.h - Loading animation dialog
 *
 * This file defines the dialog that displays a loading animation while
 * waiting for a response from the OpenAI API. It provides visual feedback
 * to the user that a request is in progress.
 *
 * Copyright (C)2022 Don HO <don.h@free.fr>
 */

#ifndef PLUGINNPPOPENAI_LOADER_DLG_H
#define PLUGINNPPOPENAI_LOADER_DLG_H

#include "DockingDlgInterface.h"
#include "loaderResource.h"
#include <commctrl.h>

// Add extern declaration for configAPIValue_model
extern std::wstring configAPIValue_model;

/**
 * Dialog for displaying a loading animation
 *
 * This dialog shows a marquee progress bar to indicate that a request
 * to the OpenAI API is in progress. It provides visual feedback to the user
 * during potentially lengthy API calls.
 */
class LoaderDlg : public StaticDialog
{
public:
	LoaderDlg() : _startTime(0), _elapsedSeconds(0) {}

	/**
	 * Resets the timer to zero and restarts counting
	 *
	 * Call this whenever a new operation starts, even if the dialog is already visible
	 */
	void resetTimer()
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

	/**
	 * Creates the loading dialog with an animated progress bar
	 *
	 * @param dialogID Resource ID of the dialog template
	 * @param isRTL Whether to use right-to-left text layout
	 * @param msgDestParent Whether messages should be sent to parent
	 */
	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true)
	{
		StaticDialog::create(dialogID, isRTL, msgDestParent);

		// Set progress bar to 'indeterminate' (marquee) animation style
		HWND progressBar = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS); // See more: `LoaderDlg.rc`
		::SendMessage(progressBar, PBM_SETMARQUEE, TRUE, 20);						  // 20ms refresh rate (1: too fast; 50: too slow)
	};
	/**
	 * Creates and displays the loading dialog
	 *
	 * Creates the dialog if it doesn't exist yet and ensures
	 * the progress animation is active.
	 *
	 * @param isRTL Whether to use right-to-left text layout
	 */
	void doDialog(bool isRTL = false)
	{
		if (!isCreated())
		{
			create(IDD_PLUGINNPPOPENAI_LOADING, isRTL);

			// Ensure the dialog is given focus and brought to the front
			HWND progressBar = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS);
			if (progressBar)
				::SendMessage(progressBar, PBM_SETMARQUEE, TRUE, 20);
		}

		// Update model name text to show current model from config
		TCHAR modelText[128];
		swprintf(modelText, 128, TEXT("«%s» AI model will respond"), configAPIValue_model.c_str());
		::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_STATIC, modelText);

		// Show and update window
		display();
		::SetWindowPos(_hSelf, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::UpdateWindow(_hSelf);
	};

	/**
	 * Toggles the visibility of the loader dialog
	 *
	 * Shows or hides the dialog window. If showing, ensures
	 * the window is properly displayed and brought to the front.
	 *	 * @param toShow Whether to show or hide the dialog
	 */
	virtual void display(bool toShow = true)
	{
		// Show or hide the window
		::ShowWindow(_hSelf, toShow ? SW_SHOW : SW_HIDE);

		// If showing the window, ensure it's properly displayed
		if (toShow)
		{
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
		}
		else
		{
			// Kill all timers when hiding the dialog
			::KillTimer(_hSelf, 1); // Animation timer
			::KillTimer(_hSelf, 2); // Elapsed time timer
		}
	};

protected:
	/**
	 * Dialog procedure for handling messages
	 *
	 * Processes messages sent to the dialog window.
	 *
	 * @param message The message identifier
	 * @param wParam Additional message information
	 * @param lParam Additional message information
	 * @return Result of message processing
	 */
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
	ULONGLONG _startTime;	   // Timestamp when dialog is first shown
	ULONGLONG _elapsedSeconds; // Elapsed time in seconds for display
};

#endif // PLUGINNPPOPENAI_LOADER_DLG_H
