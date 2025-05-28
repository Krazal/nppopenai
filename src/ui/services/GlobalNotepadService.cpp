/**
 * GlobalNotepadService.cpp - Global Implementation of INotepadService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This implementation provides the INotepadService interface while using
 * current global NppData structure and Notepad++ APIs. This maintains full
 * backward compatibility during the UI separation transition.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#include "GlobalNotepadService.h"
#include "../../core/external_globals.h"
#include <windows.h>

namespace UIServices
{
    HWND GlobalNotepadService::getNotepadHandle() const
    {
        // Return Notepad++ handle from global nppData
        return nppData._nppHandle;
    }

    HMENU GlobalNotepadService::getMainMenu() const
    {
        // Return main menu handle via global nppData
        return ::GetMenu(nppData._nppHandle);
    }

    LRESULT GlobalNotepadService::sendMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Send message to Notepad++ using global nppData handle
        return ::SendMessage(nppData._nppHandle, msg, wParam, lParam);
    }

    HANDLE GlobalNotepadService::getModuleHandle() const
    {
        // Return plugin module handle from global variable
        return _hModule;
    }

    HWND GlobalNotepadService::getCurrentScintillaHandle() const
    {
        // Get current Scintilla handle using Notepad++ API
        int currentEdit;
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
        return (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
    }
}
