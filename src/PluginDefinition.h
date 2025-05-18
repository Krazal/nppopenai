// this file is part of notepad++
// Copyright (C)2022 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
#define NPPOPENAI_VERSION "0.5.2"
#define NPPOPENAI_VERSION_MAJOR 0
#define NPPOPENAI_VERSION_MINOR 5
#define NPPOPENAI_VERSION_PATCH 2

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

//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
// Initialization of your plugin commands + toolbar icons
//
void commandMenuInit();
void updateToolbarIcons();

//
// Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);

//
// Your plugin command functions
//
void loadConfigWithoutPluginSettings();
void loadConfig(bool loadPluginSettings);
void askChatGPT();
void openConfig();
void openInsturctions();
void keepQuestionToggler();
void openChatSettingsDlg();
void updateChatSettings(bool isWriteToFile = false);
void openAboutDlg();

// Include refactored modules
#include "ConfigManager.h"
#include "PromptManager.h"
#include "EncodingUtils.h"
#include "DebugUtils.h"
#include "OpenAIClient.h"

#endif // PLUGINDEFINITION_H
