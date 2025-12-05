#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    LONG lResult;
    wchar_t szPath[MAX_PATH];
    DWORD dwSize = MAX_PATH * sizeof(wchar_t);

    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        return; // GetModuleFileName failed
    }

    lResult = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
    if (lResult != ERROR_SUCCESS) {
        return; // RegOpenKeyEx failed
    }

    lResult = RegSetValueExW(hKey, L"MyApplication", 0, REG_SZ, (const BYTE*)szPath, dwSize);
    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return; // RegSetValueEx failed
    }

    RegCloseKey(hKey);
}