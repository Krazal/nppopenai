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

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
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

	// Prepare icons to open Chat Settings
	int hToolbarBmp = IDB_PLUGINNPPOPENAI_TOOLBAR_CHAT;
	int hToolbarIcon = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT;
	int hToolbarIconDarkMode = IDI_PLUGINNPPOPENAI_TOOLBAR_CHAT_DM;
	/* // TODO: update toolbar icons on-the-fly (turning chat on/off or reaching chat limit)
	if (!_chatSettingsDlg.chatSetting_isChat || _chatSettingsDlg.chatSetting_chatLimit == 0)
	{
		hToolbarBmp = IDB_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT;
		hToolbarIcon = IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT;
		hToolbarIconDarkMode = IDI_PLUGINNPPOPENAI_TOOLBAR_NO_CHAT_DM;
	}
	// */

	// Send Chat Settings icons to Notepad++
	toolbarIconsWithDarkMode chatSettingsIcons;
	chatSettingsIcons.hToolbarBmp = ::LoadBitmap((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarBmp));
	chatSettingsIcons.hToolbarIcon = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIcon));
	chatSettingsIcons.hToolbarIconDarkMode = ::LoadIcon((HINSTANCE)_hModule, MAKEINTRESOURCE(hToolbarIconDarkMode));
	::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_FORDARKMODE, funcItem[7]._cmdID, (LPARAM)&chatSettingsIcons); // Open Chat Settings
	updateChatSettings();
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

// Load (and create if not found) config file
void loadConfig(bool loadPluginSettings)
{
	wchar_t tbuffer2[256];
	FILE *instructionsFile;

	// Set up a default plugin config (if necessary)
	if (::GetPrivateProfileString(TEXT("API"), TEXT("model"), NULL, tbuffer2, 128, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; === PLEASE ENTER YOUR OPENAI SECRET API KEY BELOW =="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == For faster results, you may use `gpt-4o-mini` model ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == For more information about the [API] settings see the Playground: https://platform.openai.com/playground ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == You can create your secret API key here: https://platform.openai.com/account/api-keys ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Token and pricing info: https://openai.com/api/pricing/ ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Set `max_tokens=0` to skip this setting (infinite for `gpt-3.5-turbo` by default). The recommended value for `code-davinci-002` is 256, but you may increase to 4000 if you get truncated responses. ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("secret_key"), configAPIValue_secretKey.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("model"), configAPIValue_model.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("temperature"), configAPIValue_temperature.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("max_tokens"), configAPIValue_maxTokens.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("top_p"), configAPIValue_topP.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), configAPIValue_frequencyPenalty.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("API"), TEXT("presence_penalty"), configAPIValue_presencePenalty.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), TEXT("1"), iniFilePath);
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("total_tokens_used"), TEXT("0"), iniFilePath);
	}

	// Set up the default API URL (v0.2.1)
	if (::GetPrivateProfileString(TEXT("API"), TEXT("api_url"), NULL, tbuffer2, 256, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("API"), TEXT("api_url"), configAPIValue_baseURL.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == The endpoints, like '/v1/chat/completions' will be added to `api_url` automatically. The trailing slash is optional in `api_url`. You should use a query string for custom URL, e.g. 'http://localhost/openai_test.php?endpoint=' ="), TEXT(""), iniFilePath);
	}

	// Chat preparations + create file for instructions (aka. system message)
	if (::GetPrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), NULL, tbuffer2, 2, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), TEXT("0"), iniFilePath);
		if ((instructionsFile = _wfopen(instructionsFilePath, L"w, ccs=UNICODE")) != NULL)
		{
			fclose(instructionsFile);
		}
		else
		{
			instructionsFileError(TEXT("The instructions (system message) file could not be created:\n\n"), TEXT("NppOpenAI: unavailable instructions file"));
		}
	}

	// Set up the default chat settings (v0.4)
	if (::GetPrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), NULL, tbuffer2, 2, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), TEXT("10"), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Use `instructions` to set a system message for the OpenAI API e.g. 'Translate the given message into English.' or 'Create a PHP function based on the received text.' Optional, leave empty to skip. ="), TEXT(""), iniFilePath);
	}

	// Set up proxy settings (v0.4.2)
	if (::GetPrivateProfileString(TEXT("API"), TEXT("proxy_url"), NULL, tbuffer2, 256, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("API"), TEXT("proxy_url"), configAPIValue_proxyURL.c_str(), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Enter a `proxy_url` to use proxy like 'http://127.0.0.1:80'. Optional, enter 0 (zero) to skip. ="), TEXT(""), iniFilePath);
	}

	// Get instructions (aka. system message) file
	if ((instructionsFile = _wfopen(instructionsFilePath, L"r, ccs=UNICODE")) != NULL)
	{
		wchar_t instructionsBuffer[9999];
		configAPIValue_instructions = TEXT("");
		while (fgetws(instructionsBuffer, 9999, instructionsFile))
		{
			configAPIValue_instructions += instructionsBuffer;
		}
		fclose(instructionsFile);
	}
	else
	{
		instructionsFileError(TEXT("The instructions (system message) file was not found:\n\n"), TEXT("NppOpenAI: missing instructions file"));
	}

	// Get API config/settings
	::GetPrivateProfileString(TEXT("API"), TEXT("secret_key"), NULL, tbuffer2, 256, iniFilePath); // sk-abc123...
	configAPIValue_secretKey = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("api_url"), NULL, tbuffer2, 256, iniFilePath); // https://...
	configAPIValue_baseURL = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("proxy_url"), NULL, tbuffer2, 256, iniFilePath); // https://...
	configAPIValue_proxyURL = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("model"), NULL, tbuffer2, 128, iniFilePath); // gpt-4o-mini, ...
	if (std::wstring(tbuffer2) == TEXT("gpt-4"))											 // Sorry, this was a bad config in v0.3.0.1 ^^'
	{
		::WritePrivateProfileString(TEXT("API"), TEXT("model"), configAPIValue_model.c_str(), iniFilePath);
		::GetPrivateProfileString(TEXT("API"), TEXT("model"), NULL, tbuffer2, 128, iniFilePath); // gpt-4o-mini, ...
	}
	configAPIValue_model = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("temperature"), NULL, tbuffer2, 16, iniFilePath); // 0-1
	configAPIValue_temperature = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("max_tokens"), NULL, tbuffer2, 5, iniFilePath); // 0-4000
	configAPIValue_maxTokens = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("top_p"), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_topP = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("frequency_penalty"), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_frequencyPenalty = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("presence_penalty"), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_presencePenalty = std::wstring(tbuffer2);

	// Get Plugin config/settings
	// Do NOT load "PLUGIN" section when clicking the Load Config menu item (may cause misconfiguration)
	if (loadPluginSettings)
	{
		isKeepQuestion = (::GetPrivateProfileInt(TEXT("PLUGIN"), TEXT("keep_question"), 1, iniFilePath) != 0);
		_chatSettingsDlg.chatSetting_isChat = (::GetPrivateProfileInt(TEXT("PLUGIN"), TEXT("is_chat"), 0, iniFilePath) != 0);
		int tmpChatLimit = ::GetPrivateProfileInt(TEXT("PLUGIN"), TEXT("chat_limit"), _chatSettingsDlg.chatSetting_chatLimit, iniFilePath);
		_chatSettingsDlg.chatSetting_chatLimit = (tmpChatLimit <= 0)
													 ? 1 // Chat limit: min. value
													 : ((tmpChatLimit > UD_MAXVAL)
															? UD_MAXVAL // Chat limit: max. value
															: tmpChatLimit);

		// Update chat menu item text (if already initialized)
		if (funcItem[7]._pFunc)
		{
			updateChatSettings();
		}
	}
}

// Call ChatGPT API
void askChatGPT()
{
	// Get current Scintilla
	long currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND curScintilla = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Get current selection
	size_t selstart = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
	size_t selend = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
	// sellength is no longer used

	// Check if everything is fine
	bool isSecretKey = configAPIValue_secretKey != TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE");
	bool isEditable = !(int)::SendMessage(curScintilla, SCI_GETREADONLY, 0, 0);

	// Retrieve selected text (or word) and store in UTF-8 string
	bool hasValidSelection = false;
	size_t count = selend > selstart ? (selend - selstart) : 0;
	if (count == 0)
	{
		// No selection: select word under caret
		int caretPos = (int)::SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
		int wordStart = (int)::SendMessage(curScintilla, SCI_WORDSTARTPOSITION, caretPos, TRUE);
		int wordEnd = (int)::SendMessage(curScintilla, SCI_WORDENDPOSITION, caretPos, TRUE);
		if (wordEnd > wordStart)
		{
			selstart = wordStart;
			selend = wordEnd;
			count = selend - selstart;
			::SendMessage(curScintilla, SCI_SETSEL, selstart, selend);
		}
	}
	if (count > 0)
	{
		// Use Scintilla target API to get full selection regardless of length
		int length = static_cast<int>(count);
		std::vector<char> buf(length + 1);
		buf[length] = '\0';
		// Define target range
		::SendMessage(curScintilla, SCI_SETTARGETSTART, static_cast<uptr_t>(selstart), 0);
		::SendMessage(curScintilla, SCI_SETTARGETEND, static_cast<uptr_t>(selend), 0);
		// Retrieve selection text into buffer
		::SendMessageA(curScintilla, SCI_GETTARGETTEXT, length + 1, reinterpret_cast<LPARAM>(buf.data()));
		lastSelection.assign(buf.data());
		hasValidSelection = true;
	}

	// Now proceed if we have valid text and other conditions are met
	if (isSecretKey && isEditable && hasValidSelection)
	{
		// Data to post via cURL
		json postData = {
			{"model", toUTF8(configAPIValue_model)},
			{"temperature", std::stod(configAPIValue_temperature)},
			{"top_p", std::stod(configAPIValue_topP)},
			{"frequency_penalty", std::stod(configAPIValue_frequencyPenalty)},
			{"presence_penalty", std::stod(configAPIValue_presencePenalty)}};

		// Add `max_tokens` setting
		if (std::stoi(configAPIValue_maxTokens) > 0)
		{
			postData[toUTF8(TEXT("max_tokens"))] = std::stoi(configAPIValue_maxTokens); // Int!
		}

		// Update postData + OpenAI URL
		bool isReady2CallOpenAI = true;
		std::string OpenAIURL = toUTF8(configAPIValue_baseURL).erase(toUTF8(configAPIValue_baseURL).find_last_not_of("/") + 1);
		std::string ProxyURL = toUTF8(configAPIValue_proxyURL).erase(toUTF8(configAPIValue_proxyURL).find_last_not_of("/") + 1);

		// Add system message (instructions)
		int msgCounter = 0;
		if (configAPIValue_instructions != TEXT(""))
		{
			postData["messages"][0] = {
				{"role", "system"},
				{"content", toUTF8(configAPIValue_instructions)}};
			msgCounter++;
		}

		// Add chat history
		if (_chatSettingsDlg.chatSetting_isChat && _chatSettingsDlg.chatSetting_chatLimit > 0 && chatHistory.size() > 0)
		{
			// Chat limit reached: should we start a new conversation or cancel OpenAI call?
			if (static_cast<int>(chatHistory.size()) == _chatSettingsDlg.chatSetting_chatLimit * 2)
			{
				char chatNewChatText[100];
				sprintf(chatNewChatText, "Chat limit reached: %d\n\nDo you want to start A WHOLE NEW conversation?", _chatSettingsDlg.chatSetting_chatLimit);
				const int result = ::MessageBox(nppData._nppHandle, myMultiByteToWideChar(chatNewChatText), TEXT("OpenAI: Chat limit!"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
				if (result == IDYES)
				{
					chatHistory = {};
				}
				else
				{
					isReady2CallOpenAI = false;
				}
			}

			// Collect chat history messages
			if (chatHistory.size() > 0)
			{
				for (size_t i = 0; i < chatHistory.size(); i++)
				{
					postData["messages"][msgCounter] = {
						{"role", (i % 2 == 0) ? "user" : "assistant"}, // The even (and zero) elements were our questions, the odd elements were the OpenAI's answers
						{"content", toUTF8(chatHistory[i])}			   // chatHistory DOESN'T contain the instructions (system message)!
					};
					msgCounter++;
				}
			}
		}
		else if ((!_chatSettingsDlg.chatSetting_isChat || _chatSettingsDlg.chatSetting_chatLimit == 0) && chatHistory.size() > 0) // Reset chat history here
		{
			chatHistory = {};
		}

		// Add the current question (selected text)
		// Use UTF-8 selection directly
		postData["messages"][msgCounter] = {{"role", "user"}, {"content", lastSelection}};
		OpenAIURL += "/v1/chat/completions";

		// Ready to call OpenAI
		if (isReady2CallOpenAI)
		{
			// Create/Show a loader dialog ("Please wait..."), disable main window
			_loaderDlg.doDialog();
			::EnableWindow(nppData._nppHandle, FALSE);

			// Prepare to start a new thread
			auto curlLambda = [](std::string OpenAIURL, std::string ProxyURL, json postData, HWND curScintilla)
			{
				std::string JSONRequest = postData.dump();

				// Try to call OpenAI and store the results in `JSONBuffer`
				std::string JSONBuffer;
				bool isSuccessCall = callOpenAI(OpenAIURL, ProxyURL, JSONRequest, JSONBuffer);

				// Hide loader dialog (`destroy()` doesn't necessary), enable main window
				_loaderDlg.display(false);
				::EnableWindow(nppData._nppHandle, TRUE);
				::SetForegroundWindow(nppData._nppHandle);

				// Return if something went wrong
				if (!isSuccessCall)
				{
					return;
				}

				// For debugging
				if (debugMode)
				{
					::MessageBox(nppData._nppHandle, myMultiByteToWideChar((char *)JSONBuffer.c_str()),
								 TEXT("Debug: API Response"), MB_OK);
				}

				// Parse response
				try
				{
					json JSONResponse = json::parse(JSONBuffer);

					// Handle JSON response
					if (JSONResponse.contains("choices") && JSONResponse["choices"].is_array() && JSONResponse.count("choices") > 0 && JSONResponse["choices"][0].contains("message") && JSONResponse["choices"][0]["message"].contains("content") && JSONResponse.contains("usage"))
					{
						// Get the appropriate response text
						std::string responseText;
						JSONResponse["choices"][0]["message"]["content"].get_to(responseText);

						// Replace selected text with response in the main Notepad++ window
						replaceSelected(curScintilla, responseText);

						// Update chat history (even if NO chat history has been set!)
						chatHistory.push_back(std::wstring(lastSelection.begin(), lastSelection.end()));
						chatHistory.push_back(std::wstring(responseText.begin(), responseText.end()));

						// Save total tokens spent/used so far
						unsigned int totalTokens = 0;
						JSONResponse["usage"]["total_tokens"].get_to(totalTokens);
						totalTokens += ::GetPrivateProfileInt(TEXT("PLUGIN"), TEXT("total_tokens_used"), 0, iniFilePath);
						std::wstring tmpTotalTokens = std::to_wstring(totalTokens);
						::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("total_tokens_used"), tmpTotalTokens.c_str(), iniFilePath);

						// Alert: incomplete response OR content filter
						if (JSONResponse["choices"][0]["finish_reason"] == "length")
						{
							::MessageBox(nppData._nppHandle, TEXT("The answer may be incomplete.\n\nAfter closing this message the configuration file will be opened. It's recommended to set `max_tokens=0` to use default (infinite) OpenAI setting OR increase e.g. to 3000.\n\nAfter saving the file don't forget to click Plugins » NppOpenAI » Load config."), TEXT("OpenAI: Incomplete answer"), MB_ICONINFORMATION);
							openConfig();
						}
						else if (JSONResponse["choices"][0]["finish_reason"] == "content_filter") // Categories: hate, hate/threatening, self-harm, sexual, sexual/minors, violence, and violence/graphic.
						{
							::MessageBox(nppData._nppHandle, TEXT("The answer has been omitted due to a flag from OpenAI content filters."), TEXT("OpenAI: Moderation"), MB_ICONWARNING);
						}
					}
					else if (JSONResponse.contains("error"))
					{
						std::string errorResponse;
						TCHAR errorResponseWide[512] = {
							0,
						};
						JSONResponse["error"]["message"].get_to(errorResponse);
						std::copy(errorResponse.begin(), errorResponse.end(), errorResponseWide);
						::MessageBox(nppData._nppHandle, errorResponseWide, TEXT("OpenAI: Error response"), MB_ICONEXCLAMATION);
					}
					else if (JSONResponse.contains("choices") && JSONResponse["choices"].is_array() && JSONResponse.count("choices") > 0 && !JSONResponse["choices"][0].contains("message"))
					{
						::MessageBox(nppData._nppHandle, TEXT("The 'choices' in the response does not contain a 'message' key. Is it possible that you are trying to use a legacy OpenAI API?"), TEXT("OpenAI: Missing message"), MB_ICONEXCLAMATION);
					}
					else
					{
						::MessageBox(nppData._nppHandle, TEXT("Missing/empty 'choices' and/or 'usage' from JSON response!"), TEXT("OpenAI: Invalid answer"), MB_ICONEXCLAMATION);
					}
				}
				catch (json::parse_error &ex)
				{
					std::string responseException = ex.what();
					std::string responseText = JSONBuffer.c_str();
					responseText += "\n\n" + responseException;
					replaceSelected(curScintilla, responseText);
					::MessageBox(nppData._nppHandle, TEXT("Invalid or non-JSON response!\n\nSee details in the main window"), TEXT("OpenAI: Invalid response"), MB_ICONERROR);
				}
			};

			std::thread curlThread(curlLambda, OpenAIURL, ProxyURL, postData, curScintilla);
			curlThread.detach();
		}
	}
	else if (!isSecretKey)
	{
		// ...existing code...
	}
	else if (!isEditable)
	{
		// ...existing code...
	}
	else if (selend <= selstart)
	{
		::MessageBox(nppData._nppHandle, TEXT("Please select text or place the cursor on a word first"),
					 TEXT("OpenAI: No text selected"), MB_ICONWARNING);
	}
	else
	{
		::MessageBox(nppData._nppHandle, TEXT("Unable to process your request. Please try again."),
					 TEXT("OpenAI: Unknown error"), MB_ICONERROR);
	}
}

// Open config file
void openConfig()
{
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniFilePath);
}

// Open insturctions file
void openInsturctions()
{
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)instructionsFilePath);
	::MessageBox(nppData._nppHandle, TEXT("You can give instructions (system message) to OpenAI here, e.g.: Translate the received text into English.\n\n\
Leave the file empty to skip the instructions.\n\n\
After saving this file, don't forget to click Plugins » NppOpenAI » Load Config to apply the changes."),
				 TEXT("NppOpenAI: Instructions"), MB_ICONINFORMATION);
}

// Toggle "Keep my question" menu item
void keepQuestionToggler()
{
	isKeepQuestion = !isKeepQuestion;
	::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[6]._cmdID, MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
}

// Open Chat Settings dialog
void openChatSettingsDlg()
{
	_chatSettingsDlg.doDialog();
}

// Update
void updateChatSettings(bool isWriteToFile)
{
	HMENU chatMenu = ::GetMenu(nppData._nppHandle);
	MENUITEMINFOW menuItemInfo{};
	menuItemInfo.cbSize = sizeof(MENUITEMINFOW);
	menuItemInfo.fMask = MIIM_TYPE | MIIM_DATA;
	// if (!isChat || !chatLimit)
	if (!_chatSettingsDlg.chatSetting_isChat || !_chatSettingsDlg.chatSetting_chatLimit)
	{
		menuItemInfo.dwTypeData = TEXT("&Chat: off");
	}
	else
	{
		wchar_t chatLimitNewBuffer[32];
		wsprintfW(chatLimitNewBuffer, L"&Chat limit: %d", _chatSettingsDlg.chatSetting_chatLimit);
		// wsprintfW(chatLimitNewBuffer, L"&Chat limit: %d", chatLimit);
		menuItemInfo.dwTypeData = chatLimitNewBuffer;
	}
	SetMenuItemInfoW(chatMenu, funcItem[7]._cmdID, MF_STRING, &menuItemInfo); // Earlier: MF_BYCOMMAND | MF_STRING

	// Update Profile (INI) settings
	if (isWriteToFile)
	{
		wchar_t chatLimitBuffer[6];
		wsprintfW(chatLimitBuffer, L"%d", _chatSettingsDlg.chatSetting_chatLimit);
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("is_chat"), _chatSettingsDlg.chatSetting_isChat ? TEXT("1") : TEXT("0"), iniFilePath);
		::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("chat_limit"), chatLimitBuffer, iniFilePath); // Convert int (9+) to LPCWSTR
	}
}

// Open About dialog
void openAboutDlg()
{
	char about[255];
	sprintf(about, "\
OpenAI (aka. ChatGPT) plugin for Notepad++ v%s by Richard Stockinger\n\n\
This plugin uses libcurl v%s with OpenSSL and nlohmann/json v%d.%d.%d\
",
			NPPOPENAI_VERSION, LIBCURL_VERSION, NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
	::MessageBox(nppData._nppHandle, myMultiByteToWideChar(about), TEXT("About"), MB_OK);
}

/*** HELPER FUNCTIONS ***/

// Call OpenAI via cURL
bool callOpenAI(std::string OpenAIURL, std::string ProxyURL, std::string JSONRequest, std::string &JSONResponse)
{

	// Prepare cURL
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL); // In windows, this will init the winsock stuff

	// Get a cURL handle
	curl = curl_easy_init();
	if (!curl)
	{
		return false;
	}

	// Get the CA bundle file for cURL
	TCHAR CACertFilePath[MAX_PATH];
	const TCHAR CACertFileName[] = TEXT("NppOpenAI\\cacert.pem");
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, MAX_PATH, (LPARAM)CACertFilePath); // TODO: optimize path length (https://npp-user-manual.org/docs/plugin-communication/#nppm-getpluginhomepath)
	PathAppend(CACertFilePath, CACertFileName);													 // E.g. "C:\Program Files (x86)\Notepad++\plugins\NppOpenAI\cacert.pem"

	// Prepare cURL SetOpts
	struct curl_slist *headerList = NULL;
	std::wstring tmpBearer = TEXT("Authorization: Bearer ") + configAPIValue_secretKey;
	headerList = curl_slist_append(headerList, toUTF8(tmpBearer).c_str());
	headerList = curl_slist_append(headerList, "Content-Type: application/json");
	char userAgent[255];
	sprintf(userAgent, "NppOpenAI/%s", NPPOPENAI_VERSION); // E.g. "NppOpenAI/0.2"

	// cURL SetOpts
	curl_easy_setopt(curl, CURLOPT_URL, OpenAIURL.c_str()); // E.g. "https://api.openai.com/v1/completions"
	if (ProxyURL != "" && ProxyURL != "0")
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, ProxyURL.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L); // Corp. proxies etc.
	curl_easy_setopt(curl, CURLOPT_CAINFO, toUTF8(CACertFilePath).c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JSONRequest.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &JSONResponse);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIcURLCallback); // Send all data to this function

	// Perform the request, res will get the return code
	res = curl_easy_perform(curl);
	bool isCurlOK = (res == CURLE_OK);

	// Handle response + check for errors
	if (!isCurlOK)
	{
		char curl_error[512];
		sprintf(curl_error, "An error occurred while accessing the OpenAI server:\n%s", curl_easy_strerror(res));
		::MessageBox(nppData._nppHandle, myMultiByteToWideChar(curl_error), TEXT("OpenAI: Connection Error"), MB_ICONERROR);
	}

	// Cleanup (including headers)
	curl_easy_cleanup(curl);
	curl_slist_free_all(headerList);
	return isCurlOK;
}

// Handle cURL callback
static size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

// Replace selected text with the given string (OpenAI response)
void replaceSelected(HWND curScintilla, std::string responseText)
{
	// Update response text
	responseText.erase(0, responseText.find_first_not_of("\n"));

	// Update line endings
	std::string selectedAndResponseEOLs = "\n\n";
	switch ((int)::SendMessage(curScintilla, SCI_GETEOLMODE, 0, 0))
	{
	case SC_EOL_CRLF: // 0
		responseText = std::regex_replace(responseText, std::regex("\n"), "\r\n");
		selectedAndResponseEOLs = "\r\n\r\n";
		break;
	case SC_EOL_CR: // 1
		responseText = std::regex_replace(responseText, std::regex("\n"), "\r");
		selectedAndResponseEOLs = "\r\r";
		break;
	}

	if (isKeepQuestion)
	{
		// Prepend original question from UTF-8 buffer
		responseText = lastSelection + selectedAndResponseEOLs + responseText;
	}

	// Replace selection with OpenAI response in manageable chunks to avoid crash
	const int chunkSize = 4096;
	for (size_t pos = 0, total = responseText.size(); pos < total; pos += chunkSize)
	{
		size_t len = (total - pos > chunkSize) ? chunkSize : (total - pos);
		// send each chunk
		::SendMessageA(curScintilla, SCI_REPLACESEL, 0,
					   (LPARAM)responseText.substr(pos, len).c_str());
	}
}

// Error message when the instructions (aka. assinstant) file is unavailable
void instructionsFileError(TCHAR *errorMessage, TCHAR *errorCaption)
{
	const size_t errorMessageSize = 256 + MAX_PATH;
	_tcscat_s(errorMessage, errorMessageSize, instructionsFilePath);
	::MessageBox(nppData._nppHandle, errorMessage, errorCaption, MB_ICONERROR);
}

// Convert std::wstring to std::string
std::string toUTF8(std::wstring wide_string)
{
	try
	{
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
		return utf8_conv.to_bytes(wide_string);
	}
	catch (...) // Catch all exceptions without using a variable name to avoid the compiler warning
	{
		// Fallback conversion in case of error
		std::string result;
		for (size_t i = 0; i < wide_string.length(); i++)
		{
			wchar_t c = wide_string[i];
			if (c <= 0x7F)
			{
				result += static_cast<char>(c);
			}
			else if (c <= 0x7FF)
			{
				result += static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
				result += static_cast<char>(0x80 | (c & 0x3F));
			}
			else if (c <= 0xFFFF)
			{
				result += static_cast<char>(0xE0 | ((c >> 12) & 0x0F));
				result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
				result += static_cast<char>(0x80 | (c & 0x3F));
			}
			else
			{
				// This is a surrogate pair or invalid character, replace with '?'
				result += '?';
			}
		}

		if (debugMode)
		{
			::MessageBox(nppData._nppHandle,
						 TEXT("Warning: Used fallback UTF-8 conversion!"),
						 TEXT("Encoding Warning"), MB_OK | MB_ICONWARNING);
		}

		return result;
	}
}

// Convert chars e.g. for Menu Items, MessageBoxes etc.
TCHAR *myMultiByteToWideChar(char *fromChar)
{
	int cchWideChar = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, fromChar, -1, NULL, 0);
	TCHAR *toWide = new TCHAR[cchWideChar]{
		0,
	};
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, fromChar, -1, toWide, cchWideChar);
	return toWide;
}

// Debug helper functions
void toggleDebugMode()
{
	debugMode = !debugMode;
	::MessageBox(nppData._nppHandle,
				 debugMode ? TEXT("Debug mode enabled") : TEXT("Debug mode disabled"),
				 TEXT("NppOpenAI Debug"),
				 MB_OK | MB_ICONINFORMATION);
}

void debugText(const wchar_t *text)
{
	if (!debugMode)
		return;

	std::wstring wideStr(text);
	std::string utf8Str = toUTF8(wideStr);

	char buffer[8192] = {0};
	sprintf(buffer, "Text length: %zu chars\nUTF-8 length: %zu bytes\nContent: %s",
			wideStr.length(), utf8Str.length(), utf8Str.c_str());

	::MessageBox(nppData._nppHandle, myMultiByteToWideChar(buffer), TEXT("Text Debug Info"), MB_OK | MB_ICONINFORMATION);
}

void debugTextBinary(const wchar_t *text)
{
	if (!debugMode)
		return;

	std::wstring wideStr(text);
	std::string utf8Str = toUTF8(wideStr);

	std::string hexOutput = hexDump(utf8Str.c_str(), utf8Str.length());

	char buffer[16384] = {0};
	sprintf(buffer, "Text length: %zu chars\nUTF-8 length: %zu bytes\nHex dump:\n%s",
			wideStr.length(), utf8Str.length(), hexOutput.c_str());

	::MessageBox(nppData._nppHandle, myMultiByteToWideChar(buffer), TEXT("Binary Text Debug"), MB_OK | MB_ICONINFORMATION);
}

void debugTextCharByChar(const wchar_t *text)
{
	if (!debugMode)
		return;

	std::wstring wideStr(text);
	std::string utf8Str = toUTF8(wideStr);

	std::string output;
	char buffer[128];

	// Process wide characters
	output += "Wide characters:\n";
	for (size_t i = 0; i < wideStr.length(); i++)
	{
		sprintf(buffer, "Pos %3zu: U+%04X\n", i, (unsigned int)wideStr[i]);
		output += buffer;
	}

	// Process UTF-8 bytes
	output += "\nUTF-8 bytes:\n";
	for (size_t i = 0; i < utf8Str.length(); i++)
	{
		sprintf(buffer, "Pos %3zu: 0x%02X\n", i, (unsigned char)utf8Str[i]);
		output += buffer;
	}

	::MessageBox(nppData._nppHandle, myMultiByteToWideChar((char *)output.c_str()),
				 TEXT("Character-by-Character Debug"), MB_OK | MB_ICONINFORMATION);
}

std::string hexDump(const char *data, size_t size)
{
	std::string result;
	char line[128];

	for (size_t i = 0; i < size; i += 16)
	{
		std::string hex, ascii;

		// Print offset
		sprintf(line, "%08zX: ", i);
		result += line;

		// Process 16 bytes per line
		for (size_t j = 0; j < 16; j++)
		{
			if (i + j < size)
			{
				unsigned char c = data[i + j];
				sprintf(line, "%02X ", c);
				hex += line;

				// ASCII representation (only printable chars)
				if (c >= 32 && c <= 126)
					ascii += c;
				else
					ascii += '.';
			}
			else
			{
				hex += "   "; // Padding for incomplete line
				ascii += " ";
			}
		}

		result += hex + " | " + ascii + "\n";
	}

	return result;
}