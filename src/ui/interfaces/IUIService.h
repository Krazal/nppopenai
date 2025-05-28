/**
 * IUIService.h - UI Service Interface for NppOpenAI
 *
 * SEPARATION PLAN: Phase 1 - Interface Abstractions
 * This interface abstracts general UI operations to decouple UI components
 * from direct global variable access. This enables:
 * - Easy unit testing with mock implementations
 * - Framework independence (Qt, WPF, etc.)
 * - Clean separation of UI logic from platform specifics
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include <string>

namespace UIServices
{
    /**
     * Interface for general UI operations
     *
     * This interface abstracts common UI operations like showing dialogs
     * and managing UI state to enable framework-independent UI code.
     */
    class IUIService
    {
    public:
        virtual ~IUIService() = default;

        /**
         * Display the About dialog with version information
         * @param aboutText The formatted about text to display
         */
        virtual void showAboutDialog(const std::string &aboutText) = 0;

        /**
         * Set the state of the "Keep Question" feature
         * @param enabled True to enable keeping questions in responses
         */
        virtual void setKeepQuestionState(bool enabled) = 0;

        /**
         * Get the current state of the "Keep Question" feature
         * @return True if questions are kept in responses
         */
        virtual bool getKeepQuestionState() const = 0;

        /**
         * Toggle the "Keep Question" feature state
         * Updates both internal state and UI representation
         */
        virtual void toggleKeepQuestion() = 0;
    };
}
