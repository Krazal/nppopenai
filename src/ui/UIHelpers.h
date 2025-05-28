// filepath: src/UIHelpers.h
/**
 * UIHelpers.h - User interface utilities for NppOpenAI
 *
 * SEPARATION PLAN: Phase 3 - Dependency Injection
 * This file declares functions for managing the plugin's user interface elements,
 * including toolbar icons, menu items, and dialog boxes. It is being refactored
 * to use service interfaces instead of direct global variable access to enable:
 * - Framework-independent UI management
 * - Easy unit testing with mock services
 * - Clean separation of UI logic from platform specifics
 *
 * Current status: PREPARED for service injection (backward compatible)
 * See UI_SEPARATION_PLAN.md for complete separation strategy.
 */

#pragma once

#include <windows.h>
#include <memory>

// Forward declarations for service interfaces
namespace UIServices
{
    class IUIService;
    class IConfigurationService;
    class IMenuService;
    class INotepadService;
}

/**
 * UI helper functions for NppOpenAI plugin
 *
 * SEPARATION PLAN NOTE: These functions are being prepared for service injection.
 * They will be updated to accept service parameters or use injected services
 * instead of directly accessing global variables.
 */
namespace UIHelpers
{
    /**
     * SEPARATION PLAN: Service Injection Interface
     * These functions enable dependency injection of services to replace
     * direct global variable access. During transition, both service-based
     * and legacy global-based operations are supported.
     */

    /**
     * Initialize UIHelpers with service dependencies
     * @param uiService Service for general UI operations
     * @param configService Service for configuration management
     * @param menuService Service for menu/toolbar operations
     * @param notepadService Service for Notepad++ interactions
     */
    void initializeServices(
        std::shared_ptr<UIServices::IUIService> uiService,
        std::shared_ptr<UIServices::IConfigurationService> configService,
        std::shared_ptr<UIServices::IMenuService> menuService,
        std::shared_ptr<UIServices::INotepadService> notepadService);

    /**
     * Check if services are initialized
     * @return True if services have been injected, false if using legacy globals
     */
    bool areServicesInitialized();

    /**
     * Updates toolbar icons based on current plugin settings
     *
     * Adds or refreshes the plugin's icons in the Notepad++ toolbar,
     * showing the appropriate icon based on current chat mode.
     */
    void updateToolbarIcons();

    /**
     * Updates chat menu text and optionally saves settings to INI file
     *
     * Changes the menu label to reflect current chat status and limits.
     *
     * @param isWriteToFile If true, also writes current chat settings to the INI file
     */
    void updateChatSettings(bool isWriteToFile = false);

    /**
     * Toggles the "Keep my question" menu item state
     *
     * Switches between keeping or removing the original question in API responses.
     */
    void keepQuestionToggler();

    /**
     * Opens the Chat Settings dialog
     *
     * Shows a dialog where users can enable/disable chat mode and set message history limits.
     */
    void openChatSettingsDlg();

    /**
     * Displays the About dialog with version information
     *
     * Shows plugin version, author information, and library versions.
     */
    void openAboutDlg();
}
