// This file is part of NppOpenAI, a Notepad++ plugin that integrates OpenAI's API
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

#include "PluginDefinition.h"
#include "DockingFeature/LoaderDlg.h"
#include "DockingFeature/ChatSettingsDlg.h"
#include "menuCmdID.h"
#include "ConfigManager.h" // Configuration management functions
#include "PromptManager.h" // System prompts management
#include "EncodingUtils.h" // UTF-8 / wide-char conversion utilities
#include "DebugUtils.h"	   // Debug logging functions
#include "OpenAIClient.h"  // API client wrapper for OpenAI integration
#include "UIHelpers.h"	   // UI-related functions for menus and dialogs

// Libraries for file operations, cURL, and JSON handling
#include <wchar.h>
#include <shlwapi.h>
#include <curl/curl.h>
#include <codecvt> // Unicode conversion utilities
#include <locale>  // For wstring_convert
#include <nlohmann/json.hpp>
#include <regex>
#include <iomanip> // For std::setfill and std::setw in debug functions

// For asynchronous API calls
#include <thread>
#include <chrono>  // For timing API calls
#include <fstream> // For file stream operations
#include <cstdio>  // For FILE* operations in parseInstructionsFile

#include <commctrl.h> // For TaskDialogIndirect and common controls
#pragma comment(lib, "comctl32.lib")

// Define UpDown control max value constant
#define UD_MAXVAL 0x7fff // 32767 (more than enough for max tokens)

// Use nlohmann/json for JSON parsing and generation
using json = nlohmann::json;

// Dialog windows for loading animation and settings
HANDLE _hModule;
LoaderDlg _loaderDlg;
ChatSettingsDlg _chatSettingsDlg;

// Config file paths
TCHAR iniFilePath[MAX_PATH];		  // Path to main config INI file
TCHAR instructionsFilePath[MAX_PATH]; // Path to system prompt instructions file

// Plugin command array for Notepad++ integration
FuncItem funcItem[nbFunc];

// Notepad++ data structure with handles to main window and Scintilla editor
NppData nppData;

// Debug mode flag for detailed logging
bool debugMode = false;

// Configuration variables for OpenAI API
std::wstring configAPIValue_secretKey = TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE"); // API key
std::wstring configAPIValue_baseURL = TEXT("https://api.openai.com/");			// API base URL (trailing '/' gets removed)
std::wstring configAPIValue_proxyURL = TEXT("0");								// Proxy URL (0 = no proxy)
std::wstring configAPIValue_model = TEXT("gpt-4o-mini");						// Default LLM model
std::wstring configAPIValue_instructions = TEXT("");							// System message for API requests
std::wstring configAPIValue_temperature = TEXT("0.7");							// Randomness parameter
std::wstring configAPIValue_maxTokens = TEXT("0");								// 0 = no limit, otherwise max token count
std::wstring configAPIValue_topP = TEXT("0.8");									// Nucleus sampling parameter
std::wstring configAPIValue_frequencyPenalty = TEXT("0");						// Repetition penalty
std::wstring configAPIValue_presencePenalty = TEXT("0");						// Topic repetition penalty
bool isKeepQuestion = true;														// Keep original question in response
std::vector<std::wstring> chatHistory = {};										// Chat history for context
bool isLoadConfigAlertShown = false;											// Show alert only once for loading config

// Buffer for selected text in Scintilla editor (UTF-8)
static char selectedTextA[9999];
static std::string lastSelection;

// Prompt management
static std::vector<Prompt> g_prompts;  // Parsed system prompts from instructions file
static int g_lastUsedPromptIndex = -1; // Last used prompt index for persistence

// Initialize plugin data and UI components
void pluginInit(HANDLE hModule)
{
	// Initialize common controls for TaskDialog
	INITCOMMONCONTROLSEX icex{sizeof(icex), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES};
	InitCommonControlsEx(&icex);

	_hModule = hModule;
	_loaderDlg.init((HINSTANCE)_hModule, nppData._nppHandle);

	// Init Chat Settings modal dialog
	_chatSettingsDlg.init((HINSTANCE)_hModule, nppData._nppHandle);
	_chatSettingsDlg.chatSetting_isChat = false;
	_chatSettingsDlg.chatSetting_chatLimit = 10;
}

// Clean up resources and save settings on plugin unload
void pluginCleanUp()
{
	wchar_t chatLimitBuffer[6];
	wsprintfW(chatLimitBuffer, L"%d", _chatSettingsDlg.chatSetting_chatLimit);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), isKeepQuestion ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), _chatSettingsDlg.chatSetting_isChat ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), chatLimitBuffer, iniFilePath); // Convert int to LPCWSTR
}

// Initialize plugin menus and config paths
void commandMenuInit()
{
	TCHAR configDirPath[MAX_PATH];

	// Get path to the plugin config directory
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDirPath);

	// Create config directory if it doesn't exist
	if (PathFileExistsW(configDirPath) == FALSE)
	{
		::CreateDirectory(configDirPath, NULL);
	}

	// Set paths for config and instructions files
	PathCombine(iniFilePath, configDirPath, TEXT("NppOpenAI.ini"));
	PathCombine(instructionsFilePath, configDirPath, TEXT("NppOpenAI_instructions"));

	// Load configuration from INI file
	loadConfig(true);

	//--------------------------------------------//
	//-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
	//--------------------------------------------//
	// with function :
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );

	// Define keyboard shortcut Ctrl+Shift+O for askChatGPT command
	ShortcutKey *askChatGPTKey = new ShortcutKey;
	askChatGPTKey->_isAlt = false;
	askChatGPTKey->_isCtrl = true;
	askChatGPTKey->_isShift = true;
	askChatGPTKey->_key = 0x4f; // 'O'

	// Add all menu items for the plugin
	setCommand(0, TEXT("Ask &OpenAI"), askChatGPT, askChatGPTKey, false);
	setCommand(1, TEXT("---"), NULL, NULL, false); // Separator
	setCommand(2, TEXT("&Edit Config"), openConfig, NULL, false);
	setCommand(3, TEXT("Edit &Instructions"), openInsturctions, NULL, false);
	setCommand(4, TEXT("&Load Config"), loadConfigWithoutPluginSettings, NULL, false);
	setCommand(5, TEXT("---"), NULL, NULL, false); // Separator
	setCommand(6, TEXT("&Keep my question"), keepQuestionToggler, NULL, isKeepQuestion);
	setCommand(7, TEXT("NppOpenAI &Chat Settings"), openChatSettingsDlg, NULL, false); // Text will be updated by updateToolbarIcons
	setCommand(8, TEXT("---"), NULL, NULL, false);									   // Separator
	setCommand(9, TEXT("&About"), openAboutDlg, NULL, false);
	setCommand(10, TEXT("&Toggle Debug Mode"), toggleDebugMode, NULL, debugMode);
}

// Add and update toolbar icons in Notepad++
void updateToolbarIcons()
{
	UIHelpers::updateToolbarIcons();
}

// Clean up shortcut keys and dialog resources
void commandMenuCleanUp()
{
	// Free shortcut key memory
	delete funcItem[0]._pShKey;

	// Destroy dialog resources
	_loaderDlg.destroy();
	_chatSettingsDlg.destroy();
}

// Load instructions and config files when the files are saved
void loadConfigAndInstructionsOnSave(uptr_t fileIDFrom)
{
	TCHAR fileName[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, fileIDFrom, reinterpret_cast<LPARAM>(fileName));

	// Compare the file name with the expected paths for instructions and config files
	// If they match, reload the configuration
	if (_wcsicmp(instructionsFilePath, fileName) == 0 || _wcsicmp(iniFilePath, fileName) == 0)
	{
		loadConfig(false);
	}
}

// Helper function to set up a menu command
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

// Wrapper function to reload configuration without modifying plugin settings
void loadConfigWithoutPluginSettings()
{
	if (!isLoadConfigAlertShown)
	{
		// Show alert only once for loading config
		::MessageBox(nppData._nppHandle,
			TEXT("When saving configuration and instruction files, the settings are loaded automatically."),
			TEXT("NppOpenAI: No manual loading required"),
			MB_ICONINFORMATION);
		isLoadConfigAlertShown = true;
	}
	loadConfig(false);
}

// Open the plugin configuration INI file
void openConfig()
{
	openConfigFile();
}

// Open the system instructions/prompts file
void openInsturctions()
{
	openInstructionsFile();
}

// Main function to send the current selection to OpenAI's API
void askChatGPT()
{
	// Call the implementation in the OpenAIClient namespace
	OpenAIClientImpl::askChatGPT();
}

// Toggle the "Keep my question" menu item state
void keepQuestionToggler()
{
	UIHelpers::keepQuestionToggler();
}

// Open the chat settings dialog
void openChatSettingsDlg()
{
	// Direct call to ChatSettingsDlg to avoid UIHelpers namespace issue
	_chatSettingsDlg.doDialog();
}

// Update chat settings menu text and save to INI if needed
void updateChatSettings(bool isWriteToFile)
{
	UIHelpers::updateChatSettings(isWriteToFile);
}

// Show the About dialog with version information
void openAboutDlg()
{
	UIHelpers::openAboutDlg();
}

/*** HELPER FUNCTIONS ***/

// All helper functions are now in EncodingUtils.cpp, DebugUtils.cpp, OpenAIClient.cpp, and UIHelpers.cpp