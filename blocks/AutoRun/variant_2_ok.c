#include <windows.h>
#include <stdio.h>

// Define UNICODE_STRING if it's not already defined (MinGW compatibility)
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    char path[MAX_PATH];
    DWORD path_len = MAX_PATH;

    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        return; // Failed to get executable path
    }

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueExA(hKey, "MyApplication", 0, REG_SZ, (const BYTE*)path, strlen(path) + 1) != ERROR_SUCCESS) {
           // Handle error setting value
        }
        RegCloseKey(hKey);
    } else {
        // Handle error opening key
    }
}