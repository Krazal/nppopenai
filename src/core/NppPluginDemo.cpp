// filepath: c:\Users\andre\VSCode Workspace\nppopenai\src\NppPluginDemo.cpp
/**
 * NppPluginDemo.cpp - Entry point for NppOpenAI plugin
 *
 * This file contains the plugin's entry point and exports the standard
 * Notepad++ plugin interface functions that are required for integration
 * with the editor. These functions handle plugin lifecycle events such as
 * initialization, command setup, and cleanup.
 *
 * Copyright (C)2022 Don HO <don.h@free.fr>
 */

#include "PluginDefinition.h"
#include "editor/EditorInterface.h"
#include "Scintilla.h"
#include "core/external_globals.h"
#include "utils/EncodingUtils.h"
#include <fstream>

// Define streaming message used in OpenAIClient.cpp
#define WM_OPENAI_STREAM_CHUNK (WM_APP + 100)

extern FuncItem funcItem[nbFunc];
extern NppData nppData;

/**
 * DLL entry point
 *
 * This function is called by Windows when the DLL is loaded or unloaded.
 * It handles plugin initialization and cleanup.
 *
 * @param hModule Handle to the DLL module
 * @param reasonForCall Reason why this function is being called (attach/detach)
 * @param lpReserved Reserved, not used
 * @return TRUE if successful, FALSE otherwise
 */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/)
{
	try
	{
		switch (reasonForCall)
		{
		case DLL_PROCESS_ATTACH:
			// Initialize plugin with given module handle
			pluginInit(hModule);
			break;

		case DLL_PROCESS_DETACH:
			// Clean up resources when plugin is unloaded
			pluginCleanUp();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
		}
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Sets Notepad++ data for the plugin
 *
 * This is the first function Notepad++ calls when loading the plugin.
 * It provides handles to the main window and Scintilla editor instances.
 *
 * @param notpadPlusData Structure containing Notepad++ window handles
 */
extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

/**
 * Gets the name of the plugin
 *
 * This function returns the name of the plugin as defined in PluginDefinition.h.
 *
 * @return Plugin name as a TCHAR string
 */
extern "C" __declspec(dllexport) const TCHAR *getName()
{
	return NPP_PLUGIN_NAME;
}

/**
 * Gets the array of plugin commands
 *
 * This function provides Notepad++ with the list of commands that the plugin
 * supports. Each command is represented as a FuncItem structure.
 *
 * @param nbF Pointer to an integer to store the number of commands
 * @return Array of FuncItem structures
 */
extern "C" __declspec(dllexport) FuncItem *getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

/**
 * Handles notifications from Notepad++
 *
 * This function is called by Notepad++ to notify the plugin of various events
 * such as toolbar modification or shutdown.
 *
 * @param notifyCode Notification code from Notepad++
 */
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code)
	{
	case NPPN_TBMODIFICATION:
	{
		// Update toolbar icons when toolbar is modified
		updateToolbarIcons();
	}
	break;

	case NPPN_FILESAVED:
	{
		loadConfigAndInstructionsOnSave(notifyCode->nmhdr.idFrom);
	}
	break;

	case NPPN_SHUTDOWN:
	{
		// Clean up resources when Notepad++ is shutting down
		commandMenuCleanUp();
	}
	break;

	default:
		return;
	}
}

/**
 * Processes custom messages sent to the plugin
 *
 * This function handles custom Windows messages sent to the plugin including
 * streaming response chunks from OpenAI.
 *
 * @param Message The message identifier
 * @param wParam Additional message information
 * @param lParam Additional message information
 * @return TRUE if the message was processed, FALSE otherwise
 */
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	// Suppress unused parameter warning
	(void)wParam;
	// Handle streaming chunks from OpenAI API
	if (Message == WM_OPENAI_STREAM_CHUNK)
	{
		try
		{
			// Get the chunk data from lParam
			std::string *pChunk = reinterpret_cast<std::string *>(lParam);
			if (pChunk && !pChunk->empty())
			{ // When debugging is enabled, show what we're receiving
				if (debugMode)
				{
					static int receivedCount = 0;
					receivedCount++;
					std::wstring status = L"Stream chunk #" + std::to_wstring(receivedCount) + L" received: [" +
										  multiByteToWideChar(pChunk->substr(0, 10).c_str()) + L"...]";
					::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)status.c_str());

					// Also log to file
					std::ofstream msgFile("C:\\temp\\messages_received.txt", std::ios::app);
					if (msgFile.is_open())
					{
						msgFile << "Message #" << receivedCount << ": [" << *pChunk << "]" << std::endl;
						msgFile.close();
					}
				}

				// Use the stored Scintilla handle if available, otherwise get the current one
				HWND curScintilla = s_streamTargetScintilla;
				if (!curScintilla)
				{
					curScintilla = EditorInterface::getCurrentScintilla();
				}

				if (curScintilla)
				{
					// Insert the chunk text at the current cursor position
					::SendMessageA(curScintilla, SCI_REPLACESEL, 0,
								   reinterpret_cast<LPARAM>(pChunk->c_str()));

					if (debugMode)
					{
						std::ofstream msgFile("C:\\temp\\messages_received.txt", std::ios::app);
						if (msgFile.is_open())
						{
							msgFile << "  -> Inserted into editor successfully" << std::endl;
							msgFile.close();
						}
					}
				}
				else
				{
					if (debugMode)
					{
						std::ofstream msgFile("C:\\temp\\messages_received.txt", std::ios::app);
						if (msgFile.is_open())
						{
							msgFile << "  -> ERROR: No Scintilla handle available!" << std::endl;
							msgFile.close();
						}
					}
				}
			}

			// Free the memory allocated for the chunk
			delete pChunk;
			return TRUE;
		}
		catch (...)
		{
			// Log error or handle exception
			return FALSE;
		}
	}

	return TRUE;
}

#ifdef UNICODE
/**
 * Checks if the plugin supports Unicode
 *
 * This function is called by Notepad++ to determine if the plugin supports
 * Unicode. It always returns TRUE for this plugin.
 *
 * @return TRUE if the plugin supports Unicode
 */
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
#endif // UNICODE
