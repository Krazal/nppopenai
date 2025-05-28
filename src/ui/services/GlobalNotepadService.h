/**
 * GlobalNotepadService.h - Global Implementation of INotepadService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This class implements INotepadService using the current global NppData
 * structure and direct Notepad++ API access. It serves as a bridge during
 * the transition period, allowing UI components to use the service interface
 * while maintaining backward compatibility.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include "../interfaces/INotepadService.h"

namespace UIServices
{
    /**
     * Global implementation of INotepadService
     *
     * This implementation wraps the current global NppData access
     * and Notepad++ APIs to provide the INotepadService interface
     * while maintaining compatibility with existing code during transition.
     */
    class GlobalNotepadService : public INotepadService
    {
    public:
        GlobalNotepadService() = default;
        virtual ~GlobalNotepadService() = default;

        // INotepadService implementation
        HWND getNotepadHandle() const override;
        HMENU getMainMenu() const override;
        LRESULT sendMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;
        HANDLE getModuleHandle() const override;
        HWND getCurrentScintillaHandle() const override;
    };
}
