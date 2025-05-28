/**
 * IMenuService.h - Menu Service Interface for NppOpenAI
 *
 * SEPARATION PLAN: Phase 1 - Interface Abstractions
 * This interface abstracts menu and toolbar operations to decouple UI
 * components from direct Win32 menu API calls and global menu handles.
 * This enables:
 * - Framework-independent menu management
 * - Easy testing with mock menu implementations
 * - Separation of UI logic from platform-specific menu APIs
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include <string>
#include <windows.h>

namespace UIServices
{
    /**
     * Interface for menu and toolbar operations
     *
     * This interface abstracts menu management to enable UI components
     * to update menus and toolbars without knowing about Win32 APIs
     * or specific menu handles.
     */
    class IMenuService
    {
    public:
        virtual ~IMenuService() = default;

        /**
         * Update the text of a chat-related menu item
         * @param text New text to display in the menu
         */
        virtual void updateChatMenuText(const std::wstring &text) = 0;

        /**
         * Set the checked state of a menu item
         * @param commandId Command ID of the menu item
         * @param checked True to check the item, false to uncheck
         */
        virtual void setMenuItemChecked(int commandId, bool checked) = 0;

        /**
         * Update toolbar icons based on current plugin state
         * Refreshes the toolbar to show appropriate icons for current settings
         */
        virtual void updateToolbarIcons() = 0;

        /**
         * Get the handle to the main application menu
         * @return HMENU handle (platform-specific, may be abstracted in future)
         */
        virtual HMENU getMainMenu() const = 0;

        /**
         * Get a command ID for a specific function index
         * @param functionIndex Index of the function in the plugin's function array
         * @return Command ID for the function
         */
        virtual int getCommandId(int functionIndex) const = 0;
    };
}
