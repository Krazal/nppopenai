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

#include "PluginDefinition.h"
#include "DockingFeature/LoaderDlg.h"
#include "DockingFeature/ChatSettingsDlg.h"
#include "menuCmdID.h"
#include "ConfigManager.h" // newly added for config delegation
#include "PromptManager.h" // manage named prompts
#include "EncodingUtils.h" // UTF-8 / wide-char helpers
#include "DebugUtils.h"	   // debug logging functions
#include "OpenAIClient.h"  // askChatGPT, cURL wrapper
#include "UIHelpers.h"	   // UI-related functions

// For file + cURL + JSON ops
#include <wchar.h>
#include <shlwapi.h>
#include <curl/curl.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert
#include <nlohmann/json.hpp>
#include <regex>
#include <iomanip> // For std::setfill and std::setw in debug functions

// For "async" cURL calls
#include <thread>
#include <fstream> // For file stream operations
#include <cstdio>  // For FILE* operations in parseInstructionsFile

#include <commctrl.h> // For TaskDialogIndirect
#pragma comment(lib, "comctl32.lib")

// Instead of `#include <commctrl.h>` we define the required constants only!
#define UD_MAXVAL 0x7fff // 32767 (more than enough)

// For cURL JSON requests/responses
using json = nlohmann::json;

// Loader window ("Please wait for OpenAI's response…")
HANDLE _hModule;
LoaderDlg _loaderDlg;
ChatSettingsDlg _chatSettingsDlg;

// Config file related vars/constants
TCHAR iniFilePath[MAX_PATH];
TCHAR instructionsFilePath[MAX_PATH]; // Aka. file for OpenAI system message

// The plugin data that Notepad++ needs
FuncItem funcItem[nbFunc];

// The data of Notepad++ that you can use in your plugin commands
NppData nppData;

// For debug purposes
bool debugMode = false;

// Config file related vars
std::wstring configAPIValue_secretKey = TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE"); // Modify below on update!
std::wstring configAPIValue_baseURL = TEXT("https://api.openai.com/");			// Trailing '/' will be erased (if any)
std::wstring configAPIValue_proxyURL = TEXT("0");								// 0: don't use proxy. Trailing '/' will be erased (if any)
std::wstring configAPIValue_model = TEXT("gpt-4o-mini");						// Recommended default model. NOTE: You can use use "gpt-3.5-turbo", "text-davinci-003" or even "code-davinci-002". Additional models are not tested yet.
std::wstring configAPIValue_instructions = TEXT("");							// System message ("instuctions") for the OpenAI API e.g. "Translate the given text into English." or "Create a PHP function based on the received text.". Leave empty to skip.
std::wstring configAPIValue_temperature = TEXT("0.7");
std::wstring configAPIValue_maxTokens = TEXT("0"); // 0: Skip `max_tokens` API setting. Recommended max. value:  <4.000
std::wstring configAPIValue_topP = TEXT("0.8");
std::wstring configAPIValue_frequencyPenalty = TEXT("0");
std::wstring configAPIValue_presencePenalty = TEXT("0");
bool isKeepQuestion = true;
std::vector<std::wstring> chatHistory = {};

// Collect selected text by Scintilla here (UTF-8)
static char selectedTextA[9999];
static std::string lastSelection;

// Parsed prompts and last-used index
static std::vector<Prompt> g_prompts;
static int g_lastUsedPromptIndex = -1;

//
// Initialize your plugin data here
// It will be called while plugin loading
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

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	wchar_t chatLimitBuffer[6];
	wsprintfW(chatLimitBuffer, L"%d", _chatSettingsDlg.chatSetting_chatLimit);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), isKeepQuestion ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), _chatSettingsDlg.chatSetting_isChat ? TEXT("1") : TEXT("0"), iniFilePath);
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), chatLimitBuffer, iniFilePath); // Convert int (9+) to LPCWSTR
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	TCHAR configDirPath[MAX_PATH];

	// Get path to the plugin config + instructions (aka. OpenAI system message) file
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configDirPath);

	// If config path doesn't exist, create it
	if (PathFileExistsW(configDirPath) == FALSE) // Modified from `PathFileExists()`
	{
		::CreateDirectory(configDirPath, NULL);
	}

	// Prepare config + instructions (aka. system message) file
	PathCombine(iniFilePath, configDirPath, TEXT("NppOpenAI.ini"));
	PathCombine(instructionsFilePath, configDirPath, TEXT("NppOpenAI_instructions"));

	// Load config file content
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

	// Shortcuts for ChatGPT plugin
	ShortcutKey *askChatGPTKey = new ShortcutKey;
	askChatGPTKey->_isAlt = false;
	askChatGPTKey->_isCtrl = true;
	askChatGPTKey->_isShift = true;
	askChatGPTKey->_key = 0x4f; // 'O'

	// Plugin menu items
	setCommand(0, TEXT("Ask &OpenAI"), askChatGPT, askChatGPTKey, false);
	setCommand(1, TEXT("---"), NULL, NULL, false);
	setCommand(2, TEXT("&Edit Config"), openConfig, NULL, false);
	setCommand(3, TEXT("Edit &Instructions"), openInsturctions, NULL, false);
	setCommand(4, TEXT("&Load Config"), loadConfigWithoutPluginSettings, NULL, false);
	setCommand(5, TEXT("---"), NULL, NULL, false);
	setCommand(6, TEXT("&Keep my question"), keepQuestionToggler, NULL, isKeepQuestion);
	setCommand(7, TEXT("NppOpenAI &Chat Settings"), openChatSettingsDlg, NULL, false); // Text will be updated by `updateToolbarIcons()` » `updateChatSettings()`
	setCommand(8, TEXT("---"), NULL, NULL, false);
	setCommand(9, TEXT("&About"), openAboutDlg, NULL, false);
	setCommand(10, TEXT("&Toggle Debug Mode"), toggleDebugMode, NULL, debugMode);
}

// Add/update toolbar icons
void updateToolbarIcons()
{
	UIHelpers::updateToolbarIcons();
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
	_loaderDlg.destroy();
	_chatSettingsDlg.destroy();
}

//
// This function help you to initialize your plugin commands
//
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

// Wrapper function to match the PFUNCPLUGINCMD signature
void loadConfigWithoutPluginSettings()
{
	loadConfig(false);
}

// Load (and create if not found) config file is in ConfigManager.cpp
// No need to redefine it here, as it's already declared in ConfigManager.h

// Open config file
void openConfig()
{
	openConfigFile();
}

// Open insturctions file
void openInsturctions()
{
	openInstructionsFile();
}

// askChatGPT has been moved to OpenAIClient.cpp and is now called by this function
void askChatGPT()
{
	// Call the implementation in the OpenAIClient namespace
	OpenAIClientImpl::askChatGPT();
}

// Toggle "Keep my question" menu item
void keepQuestionToggler()
{
	UIHelpers::keepQuestionToggler();
}

// Open Chat Settings dialog
void openChatSettingsDlg()
{
	// Direct call to ChatSettingsDlg to avoid UIHelpers namespace issue
	_chatSettingsDlg.doDialog();
}

// Update chat settings menu and INI if needed
void updateChatSettings(bool isWriteToFile)
{
	UIHelpers::updateChatSettings(isWriteToFile);
}

// Open About dialog
void openAboutDlg()
{
	UIHelpers::openAboutDlg();
}

/*** HELPER FUNCTIONS ***/

// ...all helper functions are now in EncodingUtils.cpp, DebugUtils.cpp, OpenAIClient.cpp, and UIHelpers.cpp