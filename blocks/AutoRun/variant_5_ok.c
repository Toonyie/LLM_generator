#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    LONG result;
    wchar_t exePath[MAX_PATH];
    DWORD pathLength = MAX_PATH;

    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        return; // Failed to get executable path
    }

    result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);

    if (result == ERROR_SUCCESS) {
        result = RegSetValueExW(hKey, L"MyApplication", 0, REG_SZ, (const BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t));

        if (result != ERROR_SUCCESS) {
            // Handle error, e.g., log it
            printf("Failed to set registry value. Error code: %ld\n", result);
        }

        RegCloseKey(hKey);
    } else {
        // Handle error opening key
        printf("Failed to open registry key. Error code: %ld\n", result);
    }
}