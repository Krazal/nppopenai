/**
 * GlobalConfigService.h - Global Implementation of IConfigurationService for NppOpenAI
 *
 * SEPARATION PLAN: Phase 2 - Implementation Wrappers
 * This class implements IConfigurationService using the current global
 * iniFilePath variable and Win32 INI file APIs. It serves as a bridge
 * during the transition period, allowing UI components to use the service
 * interface while maintaining backward compatibility.
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#pragma once

#include "../interfaces/IConfigurationService.h"

namespace UIServices
{
    /**
     * Global implementation of IConfigurationService
     *
     * This implementation wraps the current global iniFilePath access
     * and Win32 INI file APIs to provide the IConfigurationService interface
     * while maintaining compatibility with existing code during transition.
     */
    class GlobalConfigService : public IConfigurationService
    {
    public:
        GlobalConfigService() = default;
        virtual ~GlobalConfigService() = default;

        // IConfigurationService implementation
        void saveChatSettings(bool isChat, int chatLimit) override;
        std::wstring getConfigPath() const override;
        void writeString(const std::wstring &section,
                         const std::wstring &key,
                         const std::wstring &value) override;
        std::wstring readString(const std::wstring &section,
                                const std::wstring &key,
                                const std::wstring &defaultValue = L"") override;
    };
}
