/**
 * ILoadingDialog.h - Interface for loading dialogs
 *
 * This interface defines the contract for loading dialog implementations,
 * making it easy to switch between different UI frameworks or implementations
 * with minimal impact on the rest of the codebase.
 *
 * Copyright (C)2022 Don HO <don.h@free.fr>
 */

#ifndef ILOADING_DIALOG_H
#define ILOADING_DIALOG_H

#include <string>

/**
 * Interface for loading animation dialogs
 *
 * This interface provides a clean abstraction for loading dialogs,
 * allowing easy switching between UI implementations (Win32, Qt, etc.)
 */
class ILoadingDialog
{
public:
    virtual ~ILoadingDialog() = default;

    /**
     * Sets the model name to display in the dialog
     * @param modelName The AI model name to show
     */
    virtual void setModelName(const std::wstring &modelName) = 0;

    /**
     * Shows the loading dialog
     * @param isRTL Whether to use right-to-left text layout
     */
    virtual void show(bool isRTL = false) = 0;

    /**
     * Hides the loading dialog
     */
    virtual void hide() = 0;

    /**
     * Checks if the dialog is currently visible
     * @return true if visible, false otherwise
     */
    virtual bool isVisible() const = 0;

    /**
     * Resets the timer to zero and restarts counting
     */
    virtual void resetDialog() = 0;

    /**
	 * Get the isCancelled state of the dialog
     * @return true if the dialog is cancelled, false otherwise
     */
    virtual bool isCancelled() const = 0;
};

#endif // ILOADING_DIALOG_H
