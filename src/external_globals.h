#ifndef EXTERNAL_GLOBALS_H
#define EXTERNAL_GLOBALS_H

#include <windows.h>
#include "DockingFeature/LoaderDlg.h"
#include "DockingFeature/ChatSettingsDlg.h"
#include <string>
#include "PluginInterface.h"

// Forward declarations for global functions
void openConfigFile();
void openInstructionsFile();
void toggleDebugMode();

// Namespace forward declarations
namespace UIHelpers
{
    void keepQuestionToggler();
    void updateToolbarIcons();
    void updateChatSettings(bool isWriteToFile);
    void openAboutDlg();
}

// External global variables needed across multiple files
extern NppData nppData;
extern HANDLE _hModule;
extern TCHAR iniFilePath[MAX_PATH];
extern TCHAR instructionsFilePath[MAX_PATH];
extern LoaderDlg _loaderDlg;
extern ChatSettingsDlg _chatSettingsDlg;
extern FuncItem funcItem[];
extern bool isKeepQuestion;
extern bool debugMode;
extern std::wstring configAPIValue_secretKey;
extern std::wstring configAPIValue_baseURL;
extern std::wstring configAPIValue_proxyURL;
extern std::wstring configAPIValue_model;
extern std::wstring configAPIValue_instructions;
extern std::wstring configAPIValue_temperature;
extern std::wstring configAPIValue_maxTokens;
extern std::wstring configAPIValue_topP;
extern std::wstring configAPIValue_frequencyPenalty;
extern std::wstring configAPIValue_presencePenalty;

#endif // EXTERNAL_GLOBALS_H
