#include "Win32Acrylic.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_CREATE:
		{
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(TRUE);
			break;
		}

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);
	}

	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	HWND hWnd = nullptr;
	MSG msg = {};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	wc.hbrBackground = nullptr;
	wc.lpszClassName = _T("Win32Acrylic");
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	// call RoInitialize to make WinRT libraries initialized
	RoInitializeWrapper wrapper{RO_INIT_SINGLETHREADED};

	if (FAILED(wrapper))
	{
		return 0;
	}

	if (!RegisterClassEx(&wc))
	{
		MessageBox(nullptr, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hWnd = CreateWindowEx(
			WS_EX_NOREDIRECTIONBITMAP,
			_T("Win32Acrylic"), _T("Win32Acrylic"),
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			640,
			480,
			nullptr, nullptr, hInstance, nullptr
		);

	if (hWnd == nullptr)
	{
		MessageBox(nullptr, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	{
		Win32Acrylic acrylic(hWnd);

		while (GetMessage(&msg, nullptr, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Uninitialize();

	return (int)msg.wParam;
}