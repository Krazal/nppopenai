// Base dialog class for modal dialogs
// Provides common functionality for plugin dialogs including window management,
// positioning, and message handling within the Notepad++ environment
#pragma once
#include "../../npp/Notepad_plus_msgs.h"
#include "Window.h"

typedef HRESULT(WINAPI *ETDTProc)(HWND, DWORD);

enum class PosAlign
{
	left,
	right,
	top,
	bottom
};

struct DLGTEMPLATEEX
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
	// The structure has more fields but are variable length
};

class StaticDialog : public Window
{
public:
	virtual ~StaticDialog();

	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true);

	virtual bool isCreated() const
	{
		return (_hSelf != NULL);
	}

	void goToCenter();

	void display(bool toShow = true, bool enhancedPositioningCheckWhenShowing = false) const;

	RECT getViewablePositionRect(RECT testRc) const;

	POINT getTopPoint(HWND hwnd, bool isLeft = true) const;

	bool isCheckedOrNot(int checkControlID) const
	{
		return (BST_CHECKED == ::SendMessage(::GetDlgItem(_hSelf, checkControlID), BM_GETCHECK, 0, 0));
	}

	void setChecked(int checkControlID, bool checkOrNot = true) const
	{
		::SendDlgItemMessage(_hSelf, checkControlID, BM_SETCHECK, checkOrNot ? BST_CHECKED : BST_UNCHECKED, 0);
	}

	virtual void destroy() override;

protected:
	RECT _rc;
	static INT_PTR CALLBACK dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) = 0;

	void alignWith(HWND handle, HWND handle2Align, PosAlign pos, POINT &point);
	HGLOBAL makeRTLResource(int dialogID, DLGTEMPLATE **ppMyDlgTemplate);
};
