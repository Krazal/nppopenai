// filepath: src/UIHelpers.h
/**
 * UIHelpers.h - User interface utilities for NppOpenAI
 *
 * This file declares functions for managing the plugin's user interface elements,
 * including toolbar icons, menu items, and dialog boxes. It helps separate UI
 * concerns from the core functionality of the plugin.
 */

#pragma once

#include <windows.h>

/**
 * UI helper functions for NppOpenAI plugin
 */
namespace UIHelpers
{
    /**
     * Updates toolbar icons based on current plugin settings
     *
     * Adds or refreshes the plugin's icons in the Notepad++ toolbar,
     * showing the appropriate icon based on current chat mode.
     */
    void updateToolbarIcons();

    /**
     * Updates chat menu text and optionally saves settings to INI file
     *
     * Changes the menu label to reflect current chat status and limits.
     *
     * @param isWriteToFile If true, also writes current chat settings to the INI file
     */
    void updateChatSettings(bool isWriteToFile = false);

    /**
     * Toggles the "Keep my question" menu item state
     *
     * Switches between keeping or removing the original question in API responses.
     */
    void keepQuestionToggler();

    /**
     * Opens the Chat Settings dialog
     *
     * Shows a dialog where users can enable/disable chat mode and set message history limits.
     */
    void openChatSettingsDlg();

    /**
     * Displays the About dialog with version information
     *
     * Shows plugin version, author information, and library versions.
     */
    void openAboutDlg();
}
