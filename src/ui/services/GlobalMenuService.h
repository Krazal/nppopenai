/**
 * GlobalMenuService.h - Global Implementation of IMenuService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This class implements IMenuService using the current global variables
 * (funcItem[], nppData, _hModule) and Win32 menu APIs. It serves as a bridge
 * during the transition period, allowing UI components to use the service
 * interface while maintaining backward compatibility.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include "../interfaces/IMenuService.h"

namespace UIServices
{
    /**
     * Global implementation of IMenuService
     *
     * This implementation wraps the current global menu access
     * and Win32 menu APIs to provide the IMenuService interface
     * while maintaining compatibility with existing code during transition.
     */
    class GlobalMenuService : public IMenuService
    {
    public:
        GlobalMenuService() = default;
        virtual ~GlobalMenuService() = default;

        // IMenuService implementation
        void updateChatMenuText(const std::wstring &text) override;
        void setMenuItemChecked(int commandId, bool checked) override;
        void updateToolbarIcons() override;
        HMENU getMainMenu() const override;
        int getCommandId(int functionIndex) const override;
    };
}
