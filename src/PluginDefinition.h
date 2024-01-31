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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H
#define CURL_STATICLIB

// Plugin version info
#define NPPOPENAI_VERSION "0.3.0.1"

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"
#include "DockingFeature/LoaderDlg.h"
#include <string>

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
const int nbFunc = 9;


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
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command 
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);


//
// Your plugin command functions
//
void loadConfig();
void openConfig();
void openInsturctions();
void keepQuestionToggler();
void askChatGPT();

/*** HELPER FUNCTIONS ***/
std::string to_utf8(std::wstring);
bool callOpenAI(std::string OpenAIURL, std::string JSONRequest, std::string& JSONResponse);
static size_t OpenAIcURLCallback(void *contents, size_t size, size_t nmemb, void *userp);
void replaceSelected(HWND curScintilla, std::string responseText);
void instructionsFileError(TCHAR* errorMessage, TCHAR* errorCaption);
void aboutDlg();


#endif //PLUGINDEFINITION_H
