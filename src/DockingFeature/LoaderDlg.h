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

#ifndef PLUGINNPPOPENAI_LOADER_DLG_H
#define PLUGINNPPOPENAI_LOADER_DLG_H

// Marquee for the progress bar -- see more: `LoaderDlg::create()`
#define PBS_MARQUEE  0x08
#define PBM_SETMARQUEE WM_USER + 10 

#include "DockingDlgInterface.h"
#include "loaderResource.h"

class LoaderDlg : public StaticDialog
{
public:
	LoaderDlg() = default;

	// Create a loader dialog with a center aligned text + progress bar
	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true) {
		StaticDialog::create(dialogID, isRTL, msgDestParent);

		// Set progress bar to 'indeterminate' (marquee)
		HWND progressBar = ::GetDlgItem(_hSelf, ID_PLUGINNPPOPENAI_LOADING_PROGRESS); // See more: `LoaderDlg.rc`
		::SendMessage(progressBar, PBM_SETMARQUEE, TRUE, 20); // <-- 20ms seems good (1: too fast; 50: too slow)
	};

	// Create + show a loader dialog
	void doDialog(bool isRTL = false) {
		if (!isCreated())
			create(IDD_PLUGINNPPOPENAI, isRTL);
		display();
	};

	// Toggle loader dialog visibility
	virtual void display(bool toShow = true) const {
		StaticDialog::display(toShow);
	};


protected:
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};


#endif // PLUGINNPPOPENAI_LOADER_DLG_H
