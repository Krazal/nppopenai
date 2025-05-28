/**
 * external_globals.h - Global variables and function declarations
 *
 * SEPARATION PLAN: Phase 4 - Future Service Declaration
 * This file defines global variables and functions that need to be accessible
 * across multiple source files. It provides forward declarations to avoid
 * circular dependencies and ensures consistent access to shared resources.
 *
 * REFACTORING STATUS: Being prepared for service-based architecture
 * - Service interfaces will be added for dependency injection
 * - Current global variables maintained for backward compatibility
 * - Future framework replacement will use service instances
 *
 * Part of the UI Separation Plan - see UI_SEPARATION_PLAN.md for details.
 */

#ifndef EXTERNAL_GLOBALS_H
#define EXTERNAL_GLOBALS_H

#include <windows.h>
#include "ui/dialogs/LoaderDlg.h"
#include "ui/dialogs/ChatSettingsDlg.h"
#include <string>
#include <memory>
#include "PluginInterface.h"

// SEPARATION PLAN: Forward declarations for service interfaces
namespace UIServices
{
    class IUIService;
    class IConfigurationService;
    class IMenuService;
    class INotepadService;
}

/**
 * Forward declarations for global functions
 * These functions are defined elsewhere but need to be accessible globally
 */
void openConfigFile();       // Opens the configuration INI file in the editor
void openInstructionsFile(); // Opens the system prompts file in the editor
void toggleDebugMode();      // Toggles debug mode on/off

/**
 * Namespace forward declarations
 * Functions from the UIHelpers namespace that are needed globally
 */
namespace UIHelpers
{
    void keepQuestionToggler();                  // Toggles the "keep question" option
    void updateToolbarIcons();                   // Updates toolbar icons based on current settings
    void updateChatSettings(bool isWriteToFile); // Updates chat settings UI and optionally saves to INI
    void openAboutDlg();                         // Shows the About dialog
}

/**
 * External global variables needed across multiple files
 * These are declared here but defined in PluginDefinition.cpp
 */
extern NppData nppData;                              // Notepad++ handles and data
extern HANDLE _hModule;                              // Plugin module handle
extern TCHAR iniFilePath[MAX_PATH];                  // Path to the configuration INI file
extern TCHAR instructionsFilePath[MAX_PATH];         // Path to the system prompts file
extern LoaderDlg _loaderDlg;                         // Loading animation dialog
extern ChatSettingsDlg _chatSettingsDlg;             // Chat settings dialog
extern FuncItem funcItem[];                          // Array of plugin commands
extern bool isKeepQuestion;                          // Flag for "keep question" option
extern bool debugMode;                               // Flag for debug mode
extern std::wstring configAPIValue_secretKey;        // API secret key (e.g., "sk-...")
extern std::wstring configAPIValue_baseURL;          // Base URL for API requests (e.g., "https://api.openai.com/v1/")
extern std::wstring configAPIValue_chatRoute;        // Chat completions route path (e.g., "chat/completions") - corresponds to route_chat_completions
extern std::wstring configAPIValue_responseType;     // Response format type (openai, ollama, claude, simple)
extern std::wstring configAPIValue_proxyURL;         // Proxy URL for API requests (e.g., "http://proxy:8080" or "0" for none)
extern std::wstring configAPIValue_model;            // Model name for API requests
extern std::wstring configAPIValue_instructions;     // Instructions for API requests
extern std::wstring configAPIValue_temperature;      // Temperature setting for API requests
extern std::wstring configAPIValue_maxTokens;        // Maximum tokens for API responses
extern std::wstring configAPIValue_topP;             // Top-p setting for API requests
extern std::wstring configAPIValue_frequencyPenalty; // Frequency penalty for API requests
extern std::wstring configAPIValue_presencePenalty;  // Presence penalty for API requests
extern std::wstring configAPIValue_streaming;        // Add streaming flag ("1" for enabled, "0" for disabled)
extern std::wstring configAPIValue_showReasoning;    // Show reasoning sections ("1" to show <think></think> sections, "0" to remove them)
extern HWND s_streamTargetScintilla;                 // Global handle to the Scintilla editor used for streaming responses

/**
 * SEPARATION PLAN: Service instances for dependency injection
 * These will be used to initialize UIHelpers with service-based operation
 * instead of direct global variable access
 */
extern std::shared_ptr<UIServices::IUIService> g_globalUIService;
extern std::shared_ptr<UIServices::IConfigurationService> g_globalConfigService;
extern std::shared_ptr<UIServices::IMenuService> g_globalMenuService;
extern std::shared_ptr<UIServices::INotepadService> g_globalNotepadService;

#endif // EXTERNAL_GLOBALS_H
