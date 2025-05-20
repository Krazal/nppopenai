/**
 * PluginDefinition.h - Main header for NppOpenAI plugin
 *
 * This file defines the core structures and functions for the NppOpenAI plugin,
 * which allows Notepad++ users to interact with OpenAI's API directly from the editor.
 * It includes definitions for plugin commands, toolbar icons, and function declarations
 * for the plugin's core functionality.
 *
 * Copyright (C)2022 Don HO <don.h@free.fr>
 */

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H
#define CURL_STATICLIB

#include <windows.h>

#ifdef RC_INVOKED
// For Resource Compiler, provide minimal includes
#else
// Regular includes for C++ compilation
#include <nlohmann/json.hpp>
#endif

//
// All definitions of plugin interface
//
#include "PluginInterface.h"
#include "DockingFeature/LoaderDlg.h"
#include <string>

// Plugin version info
#define NPPOPENAI_VERSION "0.5.0"
#define NPPOPENAI_VERSION_MAJOR 0
#define NPPOPENAI_VERSION_MINOR 5
#define NPPOPENAI_VERSION_PATCH 0

// Plugin toolbar icons
#define IDB_PLUGINNPPOPENAI_TOOLBAR_CHAT 101
#define IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT 102
#define IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT_DM 103
// #define IDB_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT 104
// #define IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT 105
// #define IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT_DM 106

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("NppOpenAI");

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 10;

// Config vars: API
extern std::wstring configAPIValue_secretKey;
extern std::wstring configAPIValue_baseURL;
extern std::wstring configAPIValue_proxyURL;

// Debug mode flag
extern bool debugMode;

/**
 * Initialization of plugin data
 * Called during plugin loading
 *
 * @param hModule Handle to the plugin DLL module
 */
void pluginInit(HANDLE hModule);

/**
 * Cleanup function called during plugin unloading
 * Releases resources and saves settings
 */
void pluginCleanUp();

/**
 * Initializes plugin commands and toolbar icons
 */
void commandMenuInit();
void updateToolbarIcons();

/**
 * Cleans up plugin commands allocation
 */
void commandMenuCleanUp();

/**
 * Sets up a plugin command
 *
 * @param index Zero-based index in the command array
 * @param cmdName Name of the command to show in plugin menu
 * @param pFunc Function pointer to execute when command is triggered
 * @param sk Optional keyboard shortcut
 * @param check0nInit Whether menu item should be checked initially
 * @return true if command setup succeeded, false otherwise
 */
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);

/**
 * Plugin command functions
 */
// Reloads configuration without changing plugin settings
void loadConfigWithoutPluginSettings();
// Loads or reloads complete configuration
void loadConfig(bool loadPluginSettings);
// Sends selected text to OpenAI API and replaces with response
void askChatGPT();
// Opens the configuration INI file
void openConfig();
// Opens the system instructions file
void openInsturctions();
// Toggles whether to keep user's question in response
void keepQuestionToggler();
// Opens dialog to configure chat settings
void openChatSettingsDlg();
// Updates chat settings menu text and optionally saves to INI
void updateChatSettings(bool isWriteToFile = false);
// Shows the About dialog
void openAboutDlg();

// Include refactored modules
#include "ConfigManager.h"
#include "PromptManager.h"
#include "EncodingUtils.h"
#include "DebugUtils.h"
#include "OpenAIClient.h"

#endif // PLUGINDEFINITION_H
