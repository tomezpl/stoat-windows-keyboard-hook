#include <iostream>
#include <thread>

#ifndef UNICODE
#define UNICODE
#endif

#include <cassert>
#include <windows.h>
#include <hidusage.h>
#include <atlstr.h>

HHOOK g_KbHook{};
bool g_IsHookActive{false};

typedef bool (__cdecl *SetupUdpSocketFn)(uint16_t port);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            if(g_IsHookActive) {
                g_IsHookActive = false;
                UnhookWindowsHookEx(g_KbHook);
            }
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"Stoat Keyboard Helper";
    WNDCLASS windowClass{};

    CStringA cmdLine(pCmdLine);
    std::cout << cmdLine << std::endl;

    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClass(&windowClass);

    HWND hWnd = CreateWindowEx(0, WINDOW_CLASS_NAME, L"Stoat Keyboard Helper", 0, 0, 0, 100, 100, HWND_MESSAGE, 0, windowClass.hInstance, 0);

    assert(hWnd);

    RAWINPUTDEVICE keyboard{};
    keyboard.usUsagePage = HID_USAGE_PAGE_GENERIC;
    keyboard.usUsage = HID_USAGE_GENERIC_KEYBOARD;
    keyboard.dwFlags = RIDEV_NOLEGACY;
    keyboard.hwndTarget = 0;

    assert(RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard)));

    static HINSTANCE hookDll = LoadLibrary(L"./kbhook_hook.dll");
    auto keyboardProc = (HOOKPROC)GetProcAddress(hookDll, "KeyboardProc");
    assert(keyboardProc);

    auto setupUdpSocket = (SetupUdpSocketFn)GetProcAddress(hookDll, "setupUdpSocket");
    assert(setupUdpSocket);

    assert(setupUdpSocket(42067));

    g_KbHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, hookDll, 0);
    if(!g_KbHook) {
        DWORD err = GetLastError();
        std::cout << std::hex << err << std::dec << std::endl;
        IErrorInfo* errInfo = nullptr;
        assert(!GetErrorInfo(err, &errInfo));
        BSTR errStr = nullptr;
        errInfo->GetDescription(&errStr);
        assert(errStr);
        OutputDebugString(errStr);
    }
    assert(g_KbHook);
    g_IsHookActive = true;

    ShowWindow(hWnd, nCmdShow);

    MSG currentMessage{};
    while (GetMessage(&currentMessage, hWnd, 0, 0) > 0) {
        TranslateMessage(&currentMessage);
        DispatchMessage(&currentMessage);
    }

    return 0;
}