#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <cassert>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

SOCKET g_Socket = INVALID_SOCKET;

void sendData(uint8_t vkCode, bool up) {
    assert(g_Socket != INVALID_SOCKET);

    static struct {
        uint8_t vkCode{0xFF};
        bool up{true};
    } buffer;

    buffer.up = up;
    buffer.vkCode = vkCode;

    int result = send(g_Socket, reinterpret_cast<const char *>(&buffer), sizeof(buffer), 0);

    if(result == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(g_Socket);
        g_Socket = INVALID_SOCKET;
        WSACleanup();
    }
}

__declspec(dllexport) bool __cdecl setupUdpSocket(const uint16_t port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result) {
        printf("WSAStartup failed: %d\n", result);
        return false;
    }

    struct addrinfo *addrInfoResult = nullptr, *ptr = nullptr, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    result = getaddrinfo("localhost", std::to_string(port).c_str(), &hints, &addrInfoResult);

    if(result) {
        printf("getaddrinfo failed: %d\n", result);
        WSACleanup();
        return false;
    }

    g_Socket = socket(addrInfoResult->ai_family, addrInfoResult->ai_socktype, addrInfoResult->ai_protocol);

    if(g_Socket == INVALID_SOCKET) {
        printf("Error creating socket: %ld\n", WSAGetLastError());
        freeaddrinfo(addrInfoResult);
        WSACleanup();
        return false;
    }

    result = connect(g_Socket, addrInfoResult->ai_addr, (int)addrInfoResult->ai_addrlen);
    if(result == SOCKET_ERROR) {
        closesocket(g_Socket);
        g_Socket = INVALID_SOCKET;
        freeaddrinfo(addrInfoResult);
        WSACleanup();
        return false;
    }


    return true;
}

__declspec(dllexport) LRESULT CALLBACK KeyboardProc(const int code, const WPARAM wParam, const LPARAM lParam) {
    constexpr auto MAX_VK = 0xFF;
    constexpr uint8_t BITS_PER_BYTE = 8;
    constexpr uint8_t VK_PAGE_BITS = sizeof(uint8_t) * BITS_PER_BYTE;
    constexpr uint8_t VK_PAGE_REMAINDER = MAX_VK % VK_PAGE_BITS;
    constexpr uint8_t VK_PAGES = ((MAX_VK - VK_PAGE_REMAINDER) / VK_PAGE_BITS) + (VK_PAGE_REMAINDER ? 1 : 0);
    static uint8_t KeyState[VK_PAGES] = {0};

    if(wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
        auto vkCode = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
        auto vkRemainder = vkCode % VK_PAGE_BITS;
        auto vkPageIndex = (vkCode - vkRemainder) / VK_PAGE_BITS;
        auto vkPage = KeyState[vkPageIndex];
        auto bit = 1 << vkRemainder;
        bool isAlreadyPressed = vkPage & bit;
        bool report = false;
        if(isAlreadyPressed && wParam == WM_KEYUP) {
            KeyState[vkPageIndex] = vkPage & ~bit;
            report = true;
        } else if(!isAlreadyPressed && wParam == WM_KEYDOWN) {
            KeyState[vkPageIndex] = vkPage | bit;
            report = true;
        }

        if(report) {
#ifdef _DEBUG
            std::cout << "key " << (wParam == WM_KEYUP ? "up" : "down") << ": " << std::to_string(vkCode) << std::endl;
#endif
            sendData(vkCode, wParam == WM_KEYUP);
        }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}

#ifdef __cplusplus
}
#endif