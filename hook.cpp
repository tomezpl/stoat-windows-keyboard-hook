#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <string>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) LRESULT CALLBACK KeyboardProc(const int code, const WPARAM wParam, const LPARAM lParam) {
    std::cout << "key: " << std::to_string(wParam) << std::endl;

    return CallNextHookEx(nullptr, code, wParam, lParam);
}

#ifdef __cplusplus
}
#endif