/**
 * IConfigurationService.h - Configuration Service Interface for NppOpenAI
 *
 * SEPARATION PLAN: Phase 1 - Interface Abstractions
 * This interface abstracts configuration file operations to decouple UI
 * components from direct INI file access and global paths. This enables:
 * - Framework-independent configuration management
 * - Easy testing with mock configuration sources
 * - Separation of UI logic from file I/O operations
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include <string>

namespace UIServices
{
    /**
     * Interface for configuration management operations
     *
     * This interface abstracts configuration persistence to enable
     * UI components to save/load settings without knowing about
     * file paths or INI file specifics.
     */
    class IConfigurationService
    {
    public:
        virtual ~IConfigurationService() = default;

        /**
         * Save chat-related settings to persistent storage
         * @param isChat True if chat mode is enabled
         * @param chatLimit Maximum number of messages in chat history
         */
        virtual void saveChatSettings(bool isChat, int chatLimit) = 0;

        /**
         * Get the path to the configuration file
         * @return Wide string path to the configuration file
         */
        virtual std::wstring getConfigPath() const = 0;

        /**
         * Write a string value to the configuration
         * @param section Configuration section name
         * @param key Configuration key name
         * @param value String value to write
         */
        virtual void writeString(const std::wstring &section,
                                 const std::wstring &key,
                                 const std::wstring &value) = 0;

        /**
         * Read a string value from the configuration
         * @param section Configuration section name
         * @param key Configuration key name
         * @param defaultValue Default value if key not found
         * @return The configuration value or default
         */
        virtual std::wstring readString(const std::wstring &section,
                                        const std::wstring &key,
                                        const std::wstring &defaultValue = L"") = 0;
    };
}
