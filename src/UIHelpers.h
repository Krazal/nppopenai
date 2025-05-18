// filepath: src/UIHelpers.h
#pragma once

#include <windows.h>

// UI helper functions for NppOpenAI plugin
namespace UIHelpers
{
    // Add/update toolbar icons
    void updateToolbarIcons();
    // Update chat menu text and optionally write settings to INI
    void updateChatSettings(bool isWriteToFile = false);
    // Toggle "Keep my question" menu item
    void keepQuestionToggler();
    // Open Chat Settings dialog
    void openChatSettingsDlg();
    // Display the About dialog
    void openAboutDlg();
}
