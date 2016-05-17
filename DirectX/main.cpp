#include "window.hpp"
#include "types.hpp"
#include "additional.hpp"
#include "render.hpp"
#include "light.hpp"
#include "map.hpp"
#include "matrix.hpp"
#include "objects.hpp"
#include "mesh.hpp"
#include "circle.hpp"
#include "animation.hpp"
#include "man.hpp"

Window *window = nullptr;
Render *Rend = nullptr;
Light *light = nullptr;
Matrix *matrix = nullptr;
Map *gmap = nullptr;
Objects *objects = nullptr;

void render() {
	Rend->Clear();

	matrix->MoveCamWhile();
	matrix->SetMatrix();

	gmap->Draw();
	objects->draw();

	light->setLight();

	Rend->Show();
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static POINT start_rect = {-1.f, -1.f};
	static bool rect_start = false,
		mouse_move = false;

	switch (message) {
		case WM_ACTIVATEAPP:
			ClipCursor(&window->GetClientRect());
			break;

		case WM_SIZE: {
			RECT clientSize;
			GetClientRect(window->GetHWnd(), &clientSize);

			window->SetWindowSize(clientSize.right - clientSize.left, clientSize.bottom - clientSize.top);
		}
		break;

		case WM_LBUTTONDOWN: {
			GetCursorPos(&start_rect);
			ScreenToClient(window->GetHWnd(), &start_rect);

			rect_start = true;
			mouse_move = false;
		}
		break;

		case WM_LBUTTONUP: {
			POINT pt = {};
			GetCursorPos(&pt);
			ScreenToClient(window->GetHWnd(), &pt);

			rect_start = false;

			if (!mouse_move) {
				XMFLOAT3 pos = matrix->screenToWorldPoint(pt.x, pt.y, window, gmap->getMHeight());

				if (objects->pickObj(pos, GetKeyState(VK_SHIFT) < 0)) {
					objects->setHoverId(objects->push(new Man(pos, gmap)));
				}
			}
		}
		break;

		case WM_RBUTTONDOWN: {
			POINT ptCur;
			GetCursorPos(&ptCur);
			ScreenToClient(window->GetHWnd(), &ptCur);

			XMFLOAT3 pos = matrix->screenToWorldPoint(ptCur.x, ptCur.y, window, gmap->getMHeight());
			objects->gotoXY(pos);
		}
		break;

		#define ZSUV 0.5f
		case WM_MOUSEMOVE: {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(window->GetHWnd(), &pt);

			if (pt.y == 0)
				matrix->SetDZMoveCamWhile(ZSUV);
			else if (pt.y == window->GetWindowSize().height - 1)
				matrix->SetDZMoveCamWhile(-ZSUV);
			else
				matrix->SetDZMoveCamWhile(0.f);

			if (pt.x == 0)
				matrix->SetDXMoveCamWhile(-ZSUV);
			else if (pt.x == window->GetWindowSize().width - 1)
				matrix->SetDXMoveCamWhile(ZSUV);
			else
				matrix->SetDXMoveCamWhile(0.f);

			if (rect_start && !mouse_move) {
				mouse_move = sqrt(pow(pt.x - start_rect.x, 2.) + pow(pt.y - start_rect.y, 2.)) >= 20;
			}

			if (rect_start && mouse_move) {
				XMFLOAT3 pt_lb = matrix->screenToWorldPoint(start_rect.x, start_rect.y, window, gmap->getMHeight()),
					pt_lt = matrix->screenToWorldPoint(start_rect.x, pt.y, window, gmap->getMHeight()),
					pt_rt = matrix->screenToWorldPoint(pt.x, pt.y, window, gmap->getMHeight()),
					pt_rb = matrix->screenToWorldPoint(pt.x, start_rect.y, window, gmap->getMHeight());

				objects->pickObj(pt_lb, pt_lt, pt_rt, pt_rb, GetKeyState(VK_SHIFT) < 0);

				mouse_move = true;
			}
			else {
				XMFLOAT3 pos = matrix->screenToWorldPoint(pt.x, pt.y, window, gmap->getMHeight());

				objects->getHoverId(pos);
			}
		}
		break;

		case WM_MOUSEWHEEL: {
			static int wheelDelta = 0;
			wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
			for (; wheelDelta > WHEEL_DELTA; wheelDelta -= WHEEL_DELTA)
				matrix->UpdateCamFromPos(0.05f);
			for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA)
				matrix->UpdateCamFromPos(-0.05f);
		}
		break;

		case WM_KEYDOWN: {
			switch (wParam) {
			case VK_ESCAPE: PostQuitMessage(0);
				break;
			}
		}
		break;

		case WM_DESTROY: PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	RedirectIOToConsole();

	window = new Window(hInstance, nCmdShow);
	if (FAILED(window->InitWindow(WndProc, GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2, 200, 100))) {
		delete window;

		return 0;
	}
	else
		window->Show();

	Rend = new Render();
	if (FAILED(Rend->InitDevice(window))) {
		delete Rend;
		delete window;

		return 0;
	}

	light = new Light(Rend->GetID3D11Device(), Rend->GetID3D11DeviceContext());
	light->addLight(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(18, 10, -18, 1));
	light->addLight(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(110, 10, -50, 1));

	gmap = new Map(Rend->GetID3D11Device(), Rend->GetID3D11DeviceContext(), L"resource\\map\\map.bmp", L"resource\\map\\grass.png", Rend->CreateVSheider("MapVertexShader"), Rend->CreatePSheider("MapPixelShader"));
	matrix = new Matrix(Rend->GetID3D11Device(), Rend->GetID3D11DeviceContext(), 10.f, window);
	matrix->SetMez(gmap, window);

	objects = new Objects();

	render();
	SetTimer(window->GetHWnd(), IDC_TIMER_PAINT, 20, NULL);

	MSG msg = {};
	while (WM_QUIT != msg.message) {
		GetMessage(&msg, NULL, 0, 0);

		if (msg.message == WM_TIMER) {
			render();
			DispatchMessage(&msg);
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	

	delete window;
	return (int)msg.wParam;
}