#include "window.h"

int Window::Create()
{
	const SetupParam::Param& param = SetupParam::GetInstance().GetParam();

	// �E�B���h�E�p�����[�^�̐ݒ�.
	wc_.cbSize = sizeof(WNDCLASSEX);
	wc_.style = CS_HREDRAW | CS_VREDRAW;
	wc_.lpfnWndProc = WindowProc;
	wc_.cbClsExtra = 0;
	wc_.cbWndExtra = 0;
	wc_.hInstance = param.hInst_;
	wc_.lpszMenuName = NULL;
	wc_.lpszClassName = param.windowName_;
	wc_.hIcon = NULL;
	wc_.hCursor = NULL; 
	wc_.hIconSm = NULL;
	wc_.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	// �E�B���h�E�N���X�̓o�^.
	if (RegisterClassEx(&wc_) == 0)
	{
		ASSERT(false);
		return -1;
	}

	// �E�B���h�E�̐���.
	hWnd_ = CreateWindowEx(
		WS_EX_COMPOSITED,
		param.windowName_,
		param.windowName_,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		param.windowSize_.cx,
		param.windowSize_.cy,
		NULL,
		NULL,
		param.hInst_,
		NULL);

	{
		// �N���C�A���g�̈�̐ݒ�.
		const Int32 screen_width = GetSystemMetrics(SM_CXSCREEN);
		const Int32 screen_height = GetSystemMetrics(SM_CYSCREEN);
		RECT rect{};
		GetClientRect(hWnd_, &rect);
		
		// �E�B���h�E�T�C�Y�̔z�u.
		MoveWindow(
			hWnd_,
			(screen_width / 2) - ((param.windowSize_.cx + (param.windowSize_.cx - rect.right)) / 2),
			(screen_height / 2) - ((param.windowSize_.cy + (param.windowSize_.cy - rect.bottom)) / 2),
			param.windowSize_.cx + (param.windowSize_.cx - rect.right),
			param.windowSize_.cy + (param.windowSize_.cy - rect.bottom),
			TRUE );
	}

	// �E�B���h�E�̕\��.
	ShowWindow(hWnd_, SW_SHOW);
	UpdateWindow(hWnd_);

	return 0;
}

void Window::Destroy()
{
	// �E�B���h�E�̓o�^����.
	const SetupParam::Param& param = SetupParam::GetInstance().GetParam();
	UnregisterClass(param.windowName_, NULL);
	hWnd_ = NULL;
}

HWND Window::GetWnd() const 
{ 
	ASSERT(hWnd_ != NULL);  
	return hWnd_; 
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY: 
			PostQuitMessage(0); 
			return -1;
		default: break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}