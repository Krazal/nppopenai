//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"

// For file + cURL + JSON ops
#include <wchar.h>
#include <shlwapi.h>
#include <curl/curl.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

// Config file related constants
TCHAR iniFilePath[MAX_PATH];
const TCHAR configFileName[]                     = TEXT("NppOpenAI.ini");
const TCHAR sectionInfo[]                        = TEXT("INFO");
const TCHAR sectionAPI[]                         = TEXT("API");
const TCHAR sectionPlugin[]                      = TEXT("PLUGIN");
const std::wstring configAPIKey_secretKey        = TEXT("secret_key");
const std::wstring configAPIKey_model            = TEXT("model");
const std::wstring configAPIKey_temperature      = TEXT("temperature");
const std::wstring configAPIKey_maxTokens        = TEXT("max_tokens");
const std::wstring configAPIKey_topP             = TEXT("top_p");
const std::wstring configAPIKey_frequencyPenalty = TEXT("frequency_penalty");
const std::wstring configAPIKey_presencePenalty  = TEXT("presence_penalty");
const std::wstring configPluginKey_keepQuestion  = TEXT("keep_question");
const std::wstring configPluginKey_totalTokens   = TEXT("total_tokens_used");

// The plugin data that Notepad++ needs
FuncItem funcItem[nbFunc];

// The data of Notepad++ that you can use in your plugin commands
NppData nppData;

// Config file related vars
std::wstring configAPIValue_secretKey        = TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE"); // Modify below on update!
std::wstring configAPIValue_model            = TEXT("gpt-3.5-turbo"); // Recommended default model. NOTE: You can use use "text-davinci-003" or even "code-davinci-002". Additional models are not tested yet.
std::wstring configAPIValue_temperature      = TEXT("0.7");
std::wstring configAPIValue_maxTokens        = TEXT("256");
std::wstring configAPIValue_topP             = TEXT("0.8");
std::wstring configAPIValue_frequencyPenalty = TEXT("0");
std::wstring configAPIValue_presencePenalty  = TEXT("0");
std::wstring configPluginValue_keepQuestion  = TEXT("1");
bool isKeepQuestion                          = true;

// Collect selected text here by Scintilla
TCHAR selectedText[9999];

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	::WritePrivateProfileString(sectionPlugin, configPluginKey_keepQuestion.c_str(), isKeepQuestion ? TEXT("1") : TEXT("0"), iniFilePath);
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

	// Get path of plugin config
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// If config path doesn't exist, create it
	if (PathFileExists(iniFilePath) == FALSE)
	{
		::CreateDirectory(iniFilePath, NULL);
	}

	// Make plugin config file full file path name
	PathAppend(iniFilePath, configFileName);

	// Load config file content
	loadConfig();

	// get the parameter value from plugin config
	isKeepQuestion = (::GetPrivateProfileInt(sectionPlugin, configPluginKey_keepQuestion.c_str(), 0, iniFilePath) != 0);


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
	setCommand(0, TEXT("Ask OpenAI"), askChatGPT, askChatGPTKey, false);
	setCommand(1, TEXT("---"), NULL, NULL, false);
    setCommand(2, TEXT("Edit Config"), openConfig, NULL, false);
    setCommand(3, TEXT("Load Config"), loadConfig, NULL, false);
    setCommand(4, TEXT("Keep my question"), keepQuestionToggler, NULL, isKeepQuestion);
	setCommand(5, TEXT("---"), NULL, NULL, false);
    setCommand(6, TEXT("About"), aboutDlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
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

// Open config file
void openConfig()
{
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniFilePath);
}

// Load (and create if not found) config file
void loadConfig()
{
	// Get the parameter values from plugin config
	wchar_t tbuffer2[128];
	if (!::GetPrivateProfileString(sectionAPI, configAPIKey_model.c_str(), NULL, tbuffer2, 32, iniFilePath))
	{
		::WritePrivateProfileString(sectionInfo, TEXT("; === PLEASE ENTER YOUR OPENAI SECRET API KEY BELOW =="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(sectionInfo, TEXT("; == For faster results, you may use \"code-davinci-002\" model (may be less accurate) ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(sectionInfo, TEXT("; == For more information about the [API] settings see the Playground: https://platform.openai.com/playground ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(sectionInfo, TEXT("; == You can create your secret API key here: https://platform.openai.com/account/api-keys ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(sectionInfo, TEXT("; == Token and pricing info: https://openai.com/api/pricing/ ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_secretKey.c_str(), configAPIValue_secretKey.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_model.c_str(), configAPIValue_model.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_temperature.c_str(), configAPIValue_temperature.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_maxTokens.c_str(), configAPIValue_maxTokens.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_topP.c_str(), configAPIValue_topP.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_frequencyPenalty.c_str(), configAPIValue_frequencyPenalty.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionAPI, configAPIKey_presencePenalty.c_str(), configAPIValue_presencePenalty.c_str(), iniFilePath);
		::WritePrivateProfileString(sectionPlugin, configPluginKey_keepQuestion.c_str(), TEXT("1"), iniFilePath);
		::WritePrivateProfileString(sectionPlugin, configPluginKey_totalTokens.c_str(), TEXT("0"), iniFilePath);
	}
	::GetPrivateProfileString(sectionAPI, configAPIKey_secretKey.c_str(), NULL, tbuffer2, 128, iniFilePath); // sk-abc123...
	configAPIValue_secretKey = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_model.c_str(), NULL, tbuffer2, 32, iniFilePath); // text-davinci-003, code-davinci-002, ...
	configAPIValue_model = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_temperature.c_str(), NULL, tbuffer2, 16, iniFilePath); // 0-1
	configAPIValue_temperature = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_maxTokens.c_str(), NULL, tbuffer2, 5, iniFilePath); // 0-4000
	configAPIValue_maxTokens = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_topP.c_str(), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_topP = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_frequencyPenalty.c_str(), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_frequencyPenalty = std::wstring(tbuffer2);

	::GetPrivateProfileString(sectionAPI, configAPIKey_presencePenalty.c_str(), NULL, tbuffer2, 5, iniFilePath);
	configAPIValue_presencePenalty = std::wstring(tbuffer2);
}

// Toggle "Keep my question" menu item
void keepQuestionToggler()
{
	isKeepQuestion = !isKeepQuestion;
	::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[4]._cmdID, MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
}

// Call ChatGPT API
void askChatGPT()
{

	// Get current Scintilla
	long currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND curScintilla = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// Get current selection
	size_t selstart  = ::SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
	size_t selend    = ::SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);
	size_t sellength = selend - selstart;

	// Check if everything is fine
	bool isSecretKey = configAPIValue_secretKey != TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE");
	bool isEditable   = !(int)::SendMessage(curScintilla, SCI_GETREADONLY, 0, 0);
	if ( isSecretKey
		&& isEditable
		&& selend > selstart
		&& sellength < 9999
		&& ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTWORD, 9999, (LPARAM)selectedText)
	)
	{

		// Prepare cURL
		CURL *curl;
		CURLcode res;
		curl_global_init(CURL_GLOBAL_ALL); // In windows, this will init the winsock stuff

		// Get a cURL handle
		curl = curl_easy_init();
		if (curl)
		{
			json postData = {
				{to_utf8(configAPIKey_model),            to_utf8(configAPIValue_model)},
				{to_utf8(configAPIKey_temperature),      std::stod(configAPIValue_temperature)},
				{to_utf8(configAPIKey_maxTokens),        std::stoi(configAPIValue_maxTokens)}, // Int!
				{to_utf8(configAPIKey_topP),             std::stod(configAPIValue_topP)},
				{to_utf8(configAPIKey_frequencyPenalty), std::stod(configAPIValue_frequencyPenalty)},
				{to_utf8(configAPIKey_presencePenalty),  std::stod(configAPIValue_presencePenalty)}
			};

			// Update postData + OpenAI URL
			std::string OpenAIURL;
			if (to_utf8(configAPIValue_model).rfind("gpt-3.5-turbo", 0) == 0) // gpt-3.5-turbo (recommended), gpt-3.5-turbo-0301 (snapshot of `gpt-3.5-turbo` from March 1st 2023)
			{
				/* You may set the behavior of the assistant. This is NOT a real chat yet, as we don't have conversation history!
				postData["messages"][0] = { // 
					{"role",    "system"},
					{"content", "You are a helpful assistant."},
				};
				// */
				postData["messages"][0] = { // Use 1, if you set the behavior of the assistant
					{"role",    "user"},
					{"content", to_utf8(selectedText)}
				};
				OpenAIURL = "https://api.openai.com/v1/chat/completions";
			}
			else
			{
				postData["prompt"] = to_utf8(selectedText);
				OpenAIURL = "https://api.openai.com/v1/completions";
			}

			std::string JSONRequest = postData.dump();

			/* // TEST
			::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)JSONRequest.c_str());
			return;
			// */

			// Get the CA bundle file for cURL
			TCHAR CACertFilePath[MAX_PATH];
			const TCHAR CACertFileName[] = TEXT("NppOpenAI\\cacert.pem");
			::SendMessage(nppData._nppHandle, NPPM_GETPLUGINHOMEPATH, MAX_PATH, (LPARAM)CACertFilePath); // TODO: optimize path length (https://npp-user-manual.org/docs/plugin-communication/#nppm-getpluginhomepath)
			PathAppend(CACertFilePath, CACertFileName); // E.g. "C:\Program Files (x86)\Notepad++\plugins\NppOpenAI\cacert.pem"

			// Set cURL opts
			struct curl_slist *headerList  = NULL;
			std::string curlBuffer;
			std::wstring tmpBearer = TEXT("Authorization: Bearer ") + configAPIValue_secretKey;
			headerList = curl_slist_append(headerList, to_utf8(tmpBearer).c_str());
			headerList = curl_slist_append(headerList, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_URL, OpenAIURL.c_str()); // https://api.openai.com/v1/completions
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L); // Corp. proxies etc.
			curl_easy_setopt(curl, CURLOPT_CAINFO, to_utf8(CACertFilePath).c_str());
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JSONRequest.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlBuffer);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIcURLCallback); // Send all data to this function

			// Perform the request, res will get the return code
			res = curl_easy_perform(curl);
			bool isCurlOK = (res == CURLE_OK);

			// Handle response + check for errors
			if (!isCurlOK)
			{
				TCHAR curl_error_wide[512] = { 0, };
				char curl_error[512];
				sprintf(curl_error, "An error occurred when accessing the OpenAI server:\n%s", curl_easy_strerror(res));
				MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, curl_error, strlen(curl_error), curl_error_wide, 512);
				::MessageBox(nppData._nppHandle, curl_error_wide, TEXT("OpenAI: Connection Error"), MB_ICONEXCLAMATION);
			}

			// Cleanup (including headers)
			curl_easy_cleanup(curl);
			curl_slist_free_all(headerList);
			if (!isCurlOK)
			{
				return;
			}
		}
	}
	else if (!isSecretKey)
	{
		openConfig();
		::MessageBox(nppData._nppHandle, TEXT("1. Please enter your OpenAI private key\n\
2. Save the NppOpenAI.ini file\n\
3. Load NppOpenAI Config from Plugins » NppOpenAI » Load Config"), TEXT("OpenAI: Missing private key"), MB_ICONINFORMATION);
	}
	else if (!isEditable)
	{
		::MessageBox(nppData._nppHandle, TEXT("This file is not editable"), TEXT("OpenAI: Invalid file"), MB_ICONERROR);
	}
	else if (selend <= selstart)
	{
		::MessageBox(nppData._nppHandle, TEXT("Please select a text first"), TEXT("OpenAI: Missing question"), MB_ICONWARNING);
	}
	else if (sellength >= 9999)
	{
		::MessageBox(nppData._nppHandle, TEXT("The selected text is too long"), TEXT("OpenAI: Invalid question"), MB_ICONWARNING);
	}
	else
	{
		::MessageBox(nppData._nppHandle, TEXT("Please try to select a question first"), TEXT("OpenAI: Unknown error"), MB_ICONERROR);
	}
}

// Handle cURL response (callback)
static size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	auto& memoryBuffer = *static_cast<std::string*>(userp);
	memoryBuffer.append(static_cast<char*>(contents), realsize);

	// Get current Scintilla
	long currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND curScintilla = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	/* // TEST
	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)memoryBuffer.c_str());
	// */

	// Parse response
	json JSONResponse = json::parse(memoryBuffer);

	// Handle JSON response
	if (JSONResponse.contains("choices") && JSONResponse.contains("usage"))
	{

		// Get the appropriate response text
		std::string responseText;
		if (to_utf8(configAPIValue_model).rfind("gpt-3.5-turbo", 0) == 0)
		{
			JSONResponse["choices"][0]["message"]["content"].get_to(responseText);
		}
		else
		{
			JSONResponse["choices"][0]["text"].get_to(responseText);
		}

		// Update response text
		responseText.erase(0, responseText.find_first_not_of("\n"));
		if (isKeepQuestion)
		{
			responseText = to_utf8(selectedText) + "\n\n" + responseText;
		}

		// Update line endings
		switch ((int)::SendMessage(curScintilla, SCI_GETEOLMODE, 0, 0))
		{
			case SC_EOL_CRLF: // 0
				responseText = std::regex_replace(responseText, std::regex("\n"), "\r\n");
				break;
			case SC_EOL_CR: // 1
				responseText = std::regex_replace(responseText, std::regex("\n"), "\r");
				break;
		}
		char* tmpResponseText = &responseText[0];

		// Replace selection with OpenAI response (including original question -- optional)
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)tmpResponseText);

		// Save total tokens spent/used so far
		unsigned int totalTokens = 0;
		JSONResponse["usage"]["total_tokens"].get_to(totalTokens);
		totalTokens += ::GetPrivateProfileInt(sectionPlugin, configPluginKey_totalTokens.c_str(), 0, iniFilePath);
		std::wstring tmpTotalTokens = std::to_wstring(totalTokens);
		::WritePrivateProfileString(sectionPlugin, configPluginKey_totalTokens.c_str(), tmpTotalTokens.c_str(), iniFilePath);
	}
	else if (JSONResponse.contains("error"))
	{
		std::string errorResponse;
		TCHAR errorResponseWide[512] = { 0, };
		JSONResponse["error"]["message"].get_to(errorResponse);
		std::copy(errorResponse.begin(), errorResponse.end(), errorResponseWide);
		::MessageBox(nppData._nppHandle, errorResponseWide, TEXT("OpenAI: Error response"), MB_ICONEXCLAMATION);
	}
	else
	{
		::MessageBox(nppData._nppHandle, TEXT("Missing 'choices' and/or 'usage' from JSON response!"), TEXT("OpenAI: Invalid answer"), MB_ICONEXCLAMATION);
	}
	return realsize;
}

// About
void aboutDlg()
{
	char about[255];
	TCHAR about_wide[255] = { 0, };
	sprintf(about, "\
OpenAI (aka. ChatGPT) plugin for Notepad++ v0.1.5 by Richard Stockinger\n\n\
This plugin uses libcurl v%s with OpenSSL and nlohmann/json v%d.%d.%d\
", LIBCURL_VERSION, NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, about, strlen(about), about_wide, 255);

	// Show about
	::MessageBox(nppData._nppHandle, about_wide, TEXT("About"), MB_OK);
}


// HELPER FUNCTIONS //

// Convert std::wstring to std::string
std::string to_utf8(std::wstring wide_string)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	return utf8_conv.to_bytes(wide_string);
}