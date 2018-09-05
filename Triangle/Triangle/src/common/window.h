#pragma once

#include "common.h"

class Window : public Singleton<Window>
{
public:
	Window() {}
	~Window() {}
	int Create();
	void Destroy();
	HWND GetWnd() const;

private:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	WNDCLASSEX	wc_ = {};
	HWND		hWnd_ = {};
};