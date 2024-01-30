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
#include "DockingFeature/LoaderDlg.h"
#include "menuCmdID.h"

// For file + cURL + JSON ops
#include <wchar.h>
#include <shlwapi.h>
#include <curl/curl.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert
#include <nlohmann/json.hpp>
#include <regex>

// For "async" cURL calls
#include <thread>

// For cURL JSON requests/responses
using json = nlohmann::json;

// Loader window ("Please wait for OpenAI's response…")
HANDLE _hModule;
LoaderDlg _loaderDlg;

// Config file related vars/constants
TCHAR iniFilePath[MAX_PATH];
TCHAR instructionsFilePath[MAX_PATH]; // Aka. file for OpenAI system message

// The plugin data that Notepad++ needs
FuncItem funcItem[nbFunc];

// The data of Notepad++ that you can use in your plugin commands
NppData nppData;

// Config file related vars
std::wstring configAPIValue_secretKey        = TEXT("ENTER_YOUR_OPENAI_API_KEY_HERE"); // Modify below on update!
std::wstring configAPIValue_baseURL          = TEXT("https://api.openai.com/"); // Trailing '/' will be erased (if any)
std::wstring configAPIValue_model            = TEXT("gpt-4"); // Recommended default model. NOTE: You can use use "gpt-3.5-turbo", "text-davinci-003" or even "code-davinci-002". Additional models are not tested yet.
std::wstring configAPIValue_instructions     = TEXT(""); // System message ("instuctions") for the OpenAI API e.g. "Translate the given text into English." or "Create a PHP function based on the received text.". Leave empty to skip.
std::wstring configAPIValue_temperature      = TEXT("0.7");
std::wstring configAPIValue_maxTokens        = TEXT("0"); // 0: Skip `max_tokens` API setting. Recommended max. value:  <4.000
std::wstring configAPIValue_topP             = TEXT("0.8");
std::wstring configAPIValue_frequencyPenalty = TEXT("0");
std::wstring configAPIValue_presencePenalty  = TEXT("0");
bool isKeepQuestion                          = true;

// Collect selected text by Scintilla here
TCHAR selectedText[9999];


//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit(HANDLE hModule)
{
	_hModule = hModule;
	_loaderDlg.init((HINSTANCE)_hModule, nppData._nppHandle);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	::WritePrivateProfileString(TEXT("PLUGIN"), TEXT("keep_question"), isKeepQuestion ? TEXT("1") : TEXT("0"), iniFilePath);
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
	loadConfig();


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
	setCommand(4, TEXT("&Load Config"), loadConfig, NULL, false);
	setCommand(5, TEXT("---"), NULL, NULL, false);
	setCommand(6, TEXT("&Keep my question"), keepQuestionToggler, NULL, isKeepQuestion);
	setCommand(7, TEXT("---"), NULL, NULL, false);
	setCommand(8, TEXT("&About"), aboutDlg, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[0]._pShKey;
	_loaderDlg.destroy();
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

// Load (and create if not found) config file
void loadConfig()
{
	wchar_t tbuffer2[256];
	FILE* instructionsFile;

	// Set up a default plugin config (if necessary)
	if (::GetPrivateProfileString(TEXT("API"), TEXT("model"), NULL, tbuffer2, 32, iniFilePath) == NULL)
	{
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; === PLEASE ENTER YOUR OPENAI SECRET API KEY BELOW =="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == For faster results, you may use `code-davinci-002` model (may be less accurate) ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == For more information about the [API] settings see the Playground: https://platform.openai.com/playground ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == You can create your secret API key here: https://platform.openai.com/account/api-keys ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Token and pricing info: https://openai.com/api/pricing/ ="), TEXT(""), iniFilePath);
		::WritePrivateProfileString(TEXT("INFO"), TEXT("; == Set `max_token=0` to skip this setting (infinite for `gpt-3.5-turbo` by default). The recommended value for `code-davinci-002` is 256, but you may increase to 4000 if you get truncated responses. ="), TEXT(""), iniFilePath);
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

	// Get instructions (aka. system message) file
	if ((instructionsFile = _wfopen(instructionsFilePath, L"r, ccs=UNICODE")) != NULL)
	{
		wchar_t instructionsBuffer[9999];
		configAPIValue_instructions = TEXT("");
		while (fgetws(instructionsBuffer, 9999, instructionsFile) != NULL)
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
	::GetPrivateProfileString(TEXT("API"), TEXT("secret_key"), NULL, tbuffer2, 128, iniFilePath); // sk-abc123...
	configAPIValue_secretKey = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("api_url"), NULL, tbuffer2, 128, iniFilePath); // sk-abc123...
	configAPIValue_baseURL = std::wstring(tbuffer2);

	::GetPrivateProfileString(TEXT("API"), TEXT("model"), NULL, tbuffer2, 32, iniFilePath); // text-davinci-003, ...
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
	isKeepQuestion = (::GetPrivateProfileInt(TEXT("PLUGIN"), TEXT("keep_question"), 1, iniFilePath) != 0);
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
	bool isEditable  = !(int)::SendMessage(curScintilla, SCI_GETREADONLY, 0, 0);
	if ( isSecretKey
		&& isEditable
		&& selend > selstart
		&& sellength < 9999
		&& ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTWORD, 9999, (LPARAM)selectedText)
		)
	{

		// Data to post via cURL
		json postData = {
			{to_utf8(TEXT("model")),             to_utf8(configAPIValue_model)},
			{to_utf8(TEXT("temperature")),       std::stod(configAPIValue_temperature)},
			{to_utf8(TEXT("top_p")),             std::stod(configAPIValue_topP)},
			{to_utf8(TEXT("frequency_penalty")), std::stod(configAPIValue_frequencyPenalty)},
			{to_utf8(TEXT("presence_penalty")),  std::stod(configAPIValue_presencePenalty)}
		};

		// Add `max_tokens` setting
		if (std::stoi(configAPIValue_maxTokens) > 0)
		{
			postData[to_utf8(TEXT("max_tokens"))] = std::stoi(configAPIValue_maxTokens); // Int!
		}

		// Update postData + OpenAI URL
		std::string OpenAIURL = to_utf8(configAPIValue_baseURL).erase(to_utf8(configAPIValue_baseURL).find_last_not_of("/") + 1);
		if (to_utf8(configAPIValue_model).rfind("gpt-", 0) == 0) // gpt-3.5-turbo (recommended), gpt-3.5-turbo-0301 (snapshot of `gpt-3.5-turbo` from March 1st 2023). Prepare for GPT-4!
		{
			int msgCounter = 0;
			if (configAPIValue_instructions != TEXT(""))
			{
				postData["messages"][msgCounter] = {
					{"role",    "system"},
					{"content", to_utf8(configAPIValue_instructions)}
				};
				msgCounter++;
			}
			postData["messages"][msgCounter] = { // Use 1, if you set the behavior of the assistant
				{"role",    "user"},
				{"content", to_utf8(selectedText)}
			};
			OpenAIURL += "/v1/chat/completions";
		}
		else
		{
			postData["prompt"] = to_utf8(selectedText);
			OpenAIURL += "/v1/completions";
		}

		/* // TEST: REQUEST
		::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)JSONRequest.c_str());
		return;
		// */

		// Create/Show a loader dialog ("Please wait..."), disable main window
		_loaderDlg.doDialog();
		::EnableWindow(nppData._nppHandle, FALSE);

		// Prepare to start a new thread
		auto curlLambda = [](std::string OpenAIURL, json postData, HWND curScintilla)
		{
			std::string JSONRequest = postData.dump();

			// Try to call OpenAI and store the results in `JSONBuffer`
			std::string JSONBuffer;
			if (!callOpenAI(OpenAIURL, JSONRequest, JSONBuffer))
			{
				return;
			}

			// Hide loader dialog (`destroy()` doesn't necessary), enable main window
			_loaderDlg.display(false);
			::EnableWindow(nppData._nppHandle, TRUE);
			::SetForegroundWindow(nppData._nppHandle);

			/* // TEST: RESPONSE
			::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)JSONBuffer.c_str());
			// */

			// Parse response
			try
			{
				json JSONResponse = json::parse(JSONBuffer);

				// Handle JSON response
				if (JSONResponse.contains("choices") && JSONResponse.contains("usage"))
				{

					// Get the appropriate response text
					std::string responseText;
					if (to_utf8(configAPIValue_model).rfind("gpt-", 0) == 0)
					{
						JSONResponse["choices"][0]["message"]["content"].get_to(responseText);
					}
					else
					{
						JSONResponse["choices"][0]["text"].get_to(responseText);
					}

					// Replace selected text with response in the main Notepad++ window
					replaceSelected(curScintilla, responseText);

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
					TCHAR errorResponseWide[512] = { 0, };
					JSONResponse["error"]["message"].get_to(errorResponse);
					std::copy(errorResponse.begin(), errorResponse.end(), errorResponseWide);
					::MessageBox(nppData._nppHandle, errorResponseWide, TEXT("OpenAI: Error response"), MB_ICONEXCLAMATION);
				}
				else
				{
					::MessageBox(nppData._nppHandle, TEXT("Missing 'choices' and/or 'usage' from JSON response!"), TEXT("OpenAI: Invalid answer"), MB_ICONEXCLAMATION);
				}
			}
			catch (json::parse_error& ex)
			{
				std::string responseException = ex.what();
				std::string responseText      = JSONBuffer.c_str();
				responseText += "\n\n" + responseException;
				replaceSelected(curScintilla, responseText);
				::MessageBox(nppData._nppHandle, TEXT("Invalid or non-JSON response!\n\nSee details in the main window"), TEXT("OpenAI: Invalid response"), MB_ICONERROR);
			}
		};

		// Start + detach thread
		std::thread curlThread(curlLambda, OpenAIURL, postData, curScintilla);
		curlThread.detach();
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
After saving this file, don't forget to click Plugins » NppOpenAI » Load Config to apply the changes."), TEXT("NppOpenAI: Instructions"), MB_ICONINFORMATION);
}

/*** HELPER FUNCTIONS ***/

// Toggle "Keep my question" menu item
void keepQuestionToggler()
{
	isKeepQuestion = !isKeepQuestion;
	::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[6]._cmdID, MF_BYCOMMAND | (isKeepQuestion ? MF_CHECKED : MF_UNCHECKED));
}

// Convert std::wstring to std::string
std::string to_utf8(std::wstring wide_string)
{
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	return utf8_conv.to_bytes(wide_string);
}

// Call OpenAI via cURL
bool callOpenAI(std::string OpenAIURL, std::string JSONRequest, std::string& JSONResponse)
{

	// Prepare cURL
	CURL* curl;
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
	PathAppend(CACertFilePath, CACertFileName); // E.g. "C:\Program Files (x86)\Notepad++\plugins\NppOpenAI\cacert.pem"

	// Prepare cURL SetOpts
	struct curl_slist* headerList = NULL;
	std::wstring tmpBearer = TEXT("Authorization: Bearer ") + configAPIValue_secretKey;
	headerList = curl_slist_append(headerList, to_utf8(tmpBearer).c_str());
	headerList = curl_slist_append(headerList, "Content-Type: application/json");
	char userAgent[255];
	sprintf(userAgent, "NppOpenAI/%s", NPPOPENAI_VERSION); // E.g. "NppOpenAI/0.2"

	// cURL SetOpts
	curl_easy_setopt(curl, CURLOPT_URL, OpenAIURL.c_str()); // E.g. "https://api.openai.com/v1/completions"
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L); // Corp. proxies etc.
	curl_easy_setopt(curl, CURLOPT_CAINFO, to_utf8(CACertFilePath).c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JSONRequest.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &JSONResponse);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OpenAIcURLCallback); // Send all data to this function

	// Perform the request, res will get the return code
	res = curl_easy_perform(curl);
	bool isCurlOK = (res == CURLE_OK);

	// Handle response + check for errors
	if (!isCurlOK)
	{
		TCHAR curl_error_wide[512] = { 0, };
		char curl_error[512];
		sprintf(curl_error, "An error occurred while accessing the OpenAI server:\n%s", curl_easy_strerror(res));
		MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, curl_error, strlen(curl_error), curl_error_wide, 512);
		::MessageBox(nppData._nppHandle, curl_error_wide, TEXT("OpenAI: Connection Error"), MB_ICONEXCLAMATION);
	}

	// Cleanup (including headers)
	curl_easy_cleanup(curl);
	curl_slist_free_all(headerList);
	return isCurlOK;
}

// Handle cURL callback
static size_t OpenAIcURLCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
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
		responseText = to_utf8(selectedText) + selectedAndResponseEOLs + responseText;
	}
	char* tmpResponseText = &responseText[0];

	// Replace selection with OpenAI response (including original question -- optional)
	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)tmpResponseText);
}

// Error message when the instructions (aka. assinstant) file is unavailable
void instructionsFileError(TCHAR* errorMessage, TCHAR* errorCaption)
{
	const size_t errorMessageSize = 256 + MAX_PATH;
	_tcscat_s(errorMessage, errorMessageSize, instructionsFilePath);
	::MessageBox(nppData._nppHandle, errorMessage, errorCaption, MB_ICONERROR);
}

// About
void aboutDlg()
{
	char about[255];
	TCHAR about_wide[255] = { 0, };
	sprintf(about, "\
OpenAI (aka. ChatGPT) plugin for Notepad++ v%s by Richard Stockinger\n\n\
This plugin uses libcurl v%s with OpenSSL and nlohmann/json v%d.%d.%d\
", NPPOPENAI_VERSION, LIBCURL_VERSION, NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, about, strlen(about), about_wide, 255);

	// Show about
	::MessageBox(nppData._nppHandle, about_wide, TEXT("About"), MB_OK);
}