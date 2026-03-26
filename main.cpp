#include <iostream>
#include <string>
#include <thread>

#ifndef UNICODE
#define UNICODE
#endif

#include <cassert>
#include <windows.h>
#include <hidusage.h>

// int main() {
//     constexpr unsigned short interval = 3000;
//     constexpr unsigned short polls = 4;
//
//     uint8_t secondsPassed = 0;
//     for(unsigned short i = 0; i < polls; i++) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(interval));
//         std::string_view text = std::to_string(++secondsPassed).append("s passed");
//         MessageBox(0, text.data(), "HELLO", MB_OK);
//     }
//
//     return 0;
// }

HHOOK g_KbHook{};
bool g_IsHookActive{false};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY:
            if(g_IsHookActive) {
                g_IsHookActive = false;
                UnhookWindowsHookEx(g_KbHook);
            }
            PostQuitMessage(0);
            return 0;
        case WM_INPUT:
            std::cout << "key" << std::endl;
            break;
        case WM_PAINT:
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    constexpr wchar_t WINDOW_CLASS_NAME[] = L"Stoat Keyboard Helper";
    WNDCLASS windowClass{};

    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClass(&windowClass);

    HWND hWnd = CreateWindowEx(0, WINDOW_CLASS_NAME, L"Stoat Keyboard Helper", WS_OVERLAPPED, 0, 0, 100, 100, HWND_MESSAGE, 0, windowClass.hInstance, 0);

    assert(hWnd);

    RAWINPUTDEVICE keyboard{};
    keyboard.usUsagePage = HID_USAGE_PAGE_GENERIC;
    keyboard.usUsage = HID_USAGE_GENERIC_KEYBOARD;
    keyboard.dwFlags = RIDEV_NOLEGACY;
    keyboard.hwndTarget = 0;

    assert(RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard)));

    static HINSTANCE hookDll = LoadLibrary(L"./kbhook_hook.dll");
    HOOKPROC keyboardProc = (HOOKPROC)GetProcAddress(hookDll, "KeyboardProc");

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