#pragma once
#include "types.hpp"

class Window {
	HINSTANCE g_hInst;
	HWND g_hWnd;
	INT nCmdShw;

	WindowSize ws;
	RECT ClientRect;

public:
	Window(HINSTANCE hInstance, INT nCmdShow) {
		g_hInst = hInstance;
		nCmdShw = nCmdShow;
	}
	HRESULT InitWindow(WNDPROC WndProc, LONG w, LONG h, LONG x, LONG y, LPCWSTR name = L"Game Test") {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = g_hInst;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = name;
		wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON1);

		if (!RegisterClassEx(&wcex))
			return E_FAIL;

		// Создание окна
		RECT rc = { x, y, x + w, y + h };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
		g_hWnd = CreateWindow(wcex.lpszClassName, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, g_hInst, NULL);
		if (!g_hWnd)
			return E_FAIL;

		ws.width = w;
		ws.height = h;

		return NOERROR;
	}
	void Show() {
		ShowWindow(g_hWnd, nCmdShw);
	}

	const WindowSize& GetWindowSize() {
		return ws;
	}
	void SetWindowSize(UINT width, UINT height) {
		ws.width = width;
		ws.height = height;
	}

	const RECT& GetClientRect() {
		POINT pt = { 0, 0 };
		ClientToScreen(g_hWnd, &pt);

		ClientRect.top = ClientRect.bottom = pt.y;
		ClientRect.left = ClientRect.right = pt.x;

		ClientRect.right += ws.width;
		ClientRect.bottom += ws.height;

		return ClientRect;
	}

	HWND GetHWnd() {
		return g_hWnd;
	}
};