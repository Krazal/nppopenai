/**
 * GlobalUIService.h - Global Implementation of IUIService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This class implements IUIService using the current global variables
 * and direct API calls. It serves as a bridge during the transition period,
 * allowing UIHelpers to use the service interface while maintaining
 * backward compatibility with existing global state management.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include "../interfaces/IUIService.h"

namespace UIServices
{
    /**
     * Global implementation of IUIService
     *
     * This implementation wraps the current global variable access
     * and Win32 API calls to provide the IUIService interface while
     * maintaining compatibility with existing code during transition.
     */
    class GlobalUIService : public IUIService
    {
    public:
        GlobalUIService() = default;
        virtual ~GlobalUIService() = default;

        // IUIService implementation
        void showAboutDialog(const std::string &aboutText) override;
        void setKeepQuestionState(bool enabled) override;
        bool getKeepQuestionState() const override;
        void toggleKeepQuestion() override;
    };
}
