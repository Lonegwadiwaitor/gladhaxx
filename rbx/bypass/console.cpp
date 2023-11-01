#include "../../src/pch.hpp"

#include "console.hpp"

void rbx::bypass::console(LPCWSTR lpWindowName) {
    DWORD dwOldProtection;
    VirtualProtect(static_cast<LPVOID>(FreeConsole), 1, PAGE_READWRITE, &dwOldProtection);
    *reinterpret_cast<BYTE*>(FreeConsole) = 0xC3;
    VirtualProtect(static_cast<LPVOID>(FreeConsole), 1, dwOldProtection, &dwOldProtection);
    AllocConsole();
    FILE* safe_handle_stream;
    SetConsoleTitleW(lpWindowName);
    freopen_s(&safe_handle_stream, _STR("CONIN$"), _STR("r"), stdin);
    freopen_s(&safe_handle_stream, _STR("CONOUT$"), _STR("w"), stdout);
    freopen_s(&safe_handle_stream, _STR("CONOUT$"), _STR("w"), stderr);
}
