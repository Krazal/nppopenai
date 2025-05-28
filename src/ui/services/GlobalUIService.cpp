/**
 * GlobalUIService.cpp - Global Implementation of IUIService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This implementation provides the IUIService interface while using
 * current global variables and Win32 APIs. This maintains full backward
 * compatibility during the UI separation transition.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#include "GlobalUIService.h"
#include "../../core/external_globals.h"
#include "../../utils/EncodingUtils.h"
#include <windows.h>

namespace UIServices
{
    void GlobalUIService::showAboutDialog(const std::string &aboutText)
    {
        // Use existing global state and Win32 API
        ::MessageBox(nppData._nppHandle, myMultiByteToWideChar(aboutText.c_str()), TEXT("About"), MB_OK);
    }

    void GlobalUIService::setKeepQuestionState(bool enabled)
    {
        // Update global state
        isKeepQuestion = enabled;

        // Update menu UI to reflect the new state
        ::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[6]._cmdID,
                        MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
    }

    bool GlobalUIService::getKeepQuestionState() const
    {
        // Return current global state
        return isKeepQuestion;
    }

    void GlobalUIService::toggleKeepQuestion()
    {
        // Toggle the global state and update UI
        setKeepQuestionState(!isKeepQuestion);
    }
}
