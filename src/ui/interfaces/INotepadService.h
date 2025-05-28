/**
 * INotepadService.h - Notepad++ Service Interface for NppOpenAI
 *
 * SEPARATION PLAN: Phase 1 - Interface Abstractions
 * This interface abstracts Notepad++ specific operations to decouple UI
 * components from direct NppData access and Notepad++ message handling.
 * This enables:
 * - Framework-independent host application interaction
 * - Easy testing with mock host implementations
 * - Separation of UI logic from Notepad++ specific APIs
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include <windows.h>

namespace UIServices
{
    /**
     * Interface for Notepad++ host application operations
     *
     * This interface abstracts Notepad++ interactions to enable UI components
     * to communicate with the host application without knowing about
     * NppData structures or specific message protocols.
     */
    class INotepadService
    {
    public:
        virtual ~INotepadService() = default;

        /**
         * Get the main Notepad++ window handle
         * @return HWND handle to the main Notepad++ window
         */
        virtual HWND getNotepadHandle() const = 0;

        /**
         * Get the main menu handle from Notepad++
         * @return HMENU handle to the main Notepad++ menu
         */
        virtual HMENU getMainMenu() const = 0;

        /**
         * Send a message to the Notepad++ application
         * @param msg Message identifier
         * @param wParam First message parameter
         * @param lParam Second message parameter
         * @return Result of the message processing
         */
        virtual LRESULT sendMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

        /**
         * Get the plugin module handle
         * @return HANDLE to the plugin module
         */
        virtual HANDLE getModuleHandle() const = 0;

        /**
         * Get the Scintilla handle for the current editor
         * @return HWND handle to the current Scintilla editor
         */
        virtual HWND getCurrentScintillaHandle() const = 0;
    };
}
