#include "windows.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR str, int nWinMode) {

	MSG msg;
	WNDCLASS wcl;
	HWND hWnd;

	LPCSTR szClassName = "ClassName";
	LPCSTR szTitle = "Resourses Managment";

	// create class of window
	wcl.hInstance = hInstance;
	wcl.lpszClassName = szClassName;
	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.lpfnWndProc = WndProc;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcl.lpszMenuName = nullptr;

	// register the class of window
	RegisterClass(&wcl);

	// create window
	hWnd = CreateWindow(szClassName, szTitle, WS_OVERLAPPEDWINDOW |	WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		100, 50, 700, 120, HWND_DESKTOP, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, nWinMode);
	UpdateWindow(hWnd);
	
	// message processing
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static wchar_t info[100];

	switch (message) {
	case WM_CREATE:
		DWORD WINAPI ThreadFunc(void *);
		HANDLE hThread;	// descriptor (handle) of thread
		DWORD IDThread; // identificator of thread

		// Create thread:
		hThread = CreateThread(NULL, 0, ThreadFunc, (void*)info, 0, &IDThread);

		// Wait
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);

		break;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc, 10, 10, reinterpret_cast<LPCSTR>(info), 100);
		EndPaint(hWnd, &ps);
	}
	 break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


/// <summary>
///	Function: DWORD WINAPI ThreadFunc(void *info)
/// This function load function from dll and call it.
/// void *info - buffer for output string
/// <\summary>
DWORD WINAPI ThreadFunc(void *info) {
	// load function from dll
	typedef int(*ImportFunction)(void *);
	ImportFunction DLLInfo;
	HINSTANCE hinstLib = LoadLibrary(TEXT("lib.dll"));
	DLLInfo = (ImportFunction)GetProcAddress(hinstLib, "getProseccorInfo");

	// call function from dll:
	DLLInfo(info);

	// free library
	FreeLibrary(hinstLib);

	return 0;
}