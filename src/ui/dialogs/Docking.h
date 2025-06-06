// Docking system definitions and structures
// Defines constants and data structures for the Notepad++ docking system
// (included for framework completeness, not actively used in NppOpenAI)

#pragma once

#include <windows.h>

// ATTENTION : It's a part of interface header, so don't include the others header here

// styles for containers
#define CAPTION_TOP TRUE
#define CAPTION_BOTTOM FALSE

//   defines for docking manager
#define CONT_LEFT 0
#define CONT_RIGHT 1
#define CONT_TOP 2
#define CONT_BOTTOM 3
#define DOCKCONT_MAX 4

// mask params for plugins of internal dialogs
#define DWS_ICONTAB 0x00000001 // Icon for tabs are available
#define DWS_ICONBAR 0x00000002 // Icon for icon bar are available (currently not supported)
#define DWS_ADDINFO 0x00000004 // Additional information are in use
#define DWS_PARAMSALL (DWS_ICONTAB | DWS_ICONBAR | DWS_ADDINFO)

// default docking values for first call of plugin
#define DWS_DF_CONT_LEFT (CONT_LEFT << 28)	   // default docking on left
#define DWS_DF_CONT_RIGHT (CONT_RIGHT << 28)   // default docking on right
#define DWS_DF_CONT_TOP (CONT_TOP << 28)	   // default docking on top
#define DWS_DF_CONT_BOTTOM (CONT_BOTTOM << 28) // default docking on bottom
#define DWS_DF_FLOATING 0x80000000			   // default state is floating

typedef struct
{
	HWND hClient;		  // client Window Handle
	const TCHAR *pszName; // name of plugin (shown in window)
	int dlgID;			  // a funcItem provides the function pointer to start a dialog. Please parse here these ID

	// user modifications
	UINT uMask;				 // mask params: look to above defines
	HICON hIconTab;			 // icon for tabs
	const TCHAR *pszAddInfo; // for plugin to display additional informations

	// internal data, do not use !!!
	RECT rcFloat;				// floating position
	int iPrevCont;				// stores the privious container (toggling between float and dock)
	const TCHAR *pszModuleName; // it's the plugin file name. It's used to identify the plugin
} tTbData;

typedef struct
{
	HWND hWnd;					 // the docking manager wnd
	RECT rcRegion[DOCKCONT_MAX]; // position of docked dialogs
} tDockMgr;

#define HIT_TEST_THICKNESS 20
#define SPLITTER_WIDTH 4
