// Base window class for UI components
// Provides fundamental window management functionality for plugin dialogs
// and UI elements within the Notepad++ environment

#pragma once
#include <windows.h>

class Window
{
public:
	//! \name Constructors & Destructor
	//@{
	Window() = default;
	Window(const Window &) = delete;
	virtual ~Window() = default;
	//@}

	virtual void init(HINSTANCE hInst, HWND parent)
	{
		_hInst = hInst;
		_hParent = parent;
	}

	virtual void destroy() = 0;

	virtual void display(bool toShow = true) const
	{
		::ShowWindow(_hSelf, toShow ? SW_SHOW : SW_HIDE);
	}

	virtual void reSizeTo(RECT &rc) // should NEVER be const !!!
	{
		::MoveWindow(_hSelf, rc.left, rc.top, rc.right, rc.bottom, TRUE);
		redraw();
	}

	virtual void reSizeToWH(RECT &rc) // should NEVER be const !!!
	{
		::MoveWindow(_hSelf, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		redraw();
	}

	virtual void redraw(bool forceUpdate = false) const
	{
		::InvalidateRect(_hSelf, nullptr, TRUE);
		if (forceUpdate)
			::UpdateWindow(_hSelf);
	}

	virtual void getClientRect(RECT &rc) const
	{
		::GetClientRect(_hSelf, &rc);
	}

	virtual void getWindowRect(RECT &rc) const
	{
		::GetWindowRect(_hSelf, &rc);
	}

	virtual int getWidth() const
	{
		RECT rc;
		::GetClientRect(_hSelf, &rc);
		return (rc.right - rc.left);
	}

	virtual int getHeight() const
	{
		RECT rc;
		::GetClientRect(_hSelf, &rc);
		if (::IsWindowVisible(_hSelf) == TRUE)
			return (rc.bottom - rc.top);
		return 0;
	}

	virtual bool isVisible() const
	{
		return (::IsWindowVisible(_hSelf) ? true : false);
	}

	HWND getHSelf() const
	{
		return _hSelf;
	}

	HWND getHParent() const
	{
		return _hParent;
	}

	void getFocus() const
	{
		::SetFocus(_hSelf);
	}

	HINSTANCE getHinst() const
	{
		// assert(_hInst != 0);
		return _hInst;
	}

	Window &operator=(const Window &) = delete;

protected:
	HINSTANCE _hInst = NULL;
	HWND _hParent = NULL;
	HWND _hSelf = NULL;
};