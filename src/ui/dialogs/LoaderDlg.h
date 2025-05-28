/**
 * LoaderDlg.h - Loading animation dialog
 *
 * SEPARATION PLAN: Fully Separated Component ✅ COMPLETED
 * This dialog has been successfully refactored with interface abstraction:
 * - Implements ILoadingDialog interface for framework independence
 * - Uses dependency injection for model name (no direct global access)
 * - Self-contained state management with timer and animation logic
 * - Clean separation suitable for replacement with Qt/WPF/etc.
 *
 * REFACTORING COMPLETED:
 * ✅ Interface abstraction (ILoadingDialog)
 * ✅ Dependency injection (setModelName)
 * ✅ Removed global variable coupling
 * ✅ Framework-independent design
 *
 * FRAMEWORK REPLACEMENT: This dialog demonstrates the target architecture
 * for UI separation. It can be easily replaced with different UI frameworks
 * by implementing ILoadingDialog with framework-specific components.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
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
#include "../interfaces/ILoadingDialog.h"
#include <commctrl.h>

/**
 * Dialog for displaying a loading animation
 *
 * This dialog shows a marquee progress bar to indicate that a request
 * to the AI API is in progress. It provides visual feedback to the user
 * during potentially lengthy API calls.
 */
class LoaderDlg : public StaticDialog, public ILoadingDialog
{
public:
	LoaderDlg() : _startTime(0), _elapsedSeconds(0) {}

	/**
	 * Sets the model name to display in the dialog
	 * @param modelName The AI model name to show
	 */
	void setModelName(const std::wstring &modelName) override
	{
		_modelName = modelName;
		if (isCreated() && ::IsWindowVisible(_hSelf))
		{
			updateModelText();
		}
	}

	/**
	 * Shows the loading dialog
	 * @param isRTL Whether to use right-to-left text layout
	 */
	void show(bool isRTL = false) override
	{
		doDialog(isRTL);
	}

	/**
	 * Hides the loading dialog
	 */
	void hide() override
	{
		display(false);
	}
	/**
	 * Checks if the dialog is currently visible
	 * @return true if visible, false otherwise
	 */
	bool isVisible() const override
	{
		return isCreated() && ::IsWindowVisible(_hSelf);
	}

	/**
	 * Resets the timer to zero and restarts counting
	 *
	 * Call this whenever a new operation starts, even if the dialog is already visible
	 */
	void resetTimer() override
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

		// Update model name text (only if model name was set)
		updateModelText();

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
	std::wstring _modelName;   // AI model name to display

	/**
	 * Updates the model name text in the dialog
	 */
	void updateModelText()
	{
		if (isCreated() && !_modelName.empty())
		{
			TCHAR modelText[128];
			swprintf(modelText, 128, TEXT("%s AI model will respond"), _modelName.c_str());
			::SetDlgItemText(_hSelf, ID_PLUGINNPPOPENAI_LOADING_STATIC, modelText);
		}
	}
};

#endif // PLUGINNPPOPENAI_LOADER_DLG_H
