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

#include "../PluginDefinition.h"
#include "LoaderDlg.h"

extern NppData nppData;


INT_PTR CALLBACK LoaderDlg::run_dlgProc(UINT message, WPARAM, LPARAM) // UINT message, WPARAM wParam, LPARAM lParam
{
	switch (message) 
	{
		case WM_INITDIALOG:
		{
			// goToCenter(); // <-- Using `DS_CENTER` style instead
			return TRUE;
		}

		default :
			return FALSE;
	}
}
