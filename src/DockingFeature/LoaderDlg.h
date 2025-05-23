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
	// LoaderDlg() : StaticDialog(), isCancelled(false), _startTime(0), _elapsedSeconds(0) {};
	LoaderDlg() : StaticDialog() {};

	/**
	* Flag to indicate if the operation was cancelled
	*/
    bool isCancelled;

	/**
	 * Resets the timer to zero and restarts counting
	 *
	 * Call this whenever a new operation starts, even if the dialog is already visible
	 */
	void resetTimer();

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
	};
	/**
	 * Creates and displays the loading dialog
	 *
	 * Creates the dialog if it doesn't exist yet and ensures
	 * the progress animation is active.
	 *
	 * @param isRTL Whether to use right-to-left text layout
	 */
	void doDialog(bool isRTL = false);

	/**
	 * Toggles the visibility of the loader dialog
	 *
	 * Shows or hides the dialog window. If showing, ensures
	 * the window is properly displayed and brought to the front.
	 * 
	 * @param toShow Whether to show or hide the dialog
	 */
	virtual void display(bool toShow = true)
	{
		StaticDialog::display(toShow);
	};

protected:
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
