#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    LONG lResult;
    wchar_t szPath[MAX_PATH];
    DWORD dwSize = sizeof(szPath);

    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        return; // GetModuleFileName failed
    }

    lResult = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS) {
        lResult = RegSetValueExW(hKey, L"MyApplication", 0, REG_SZ, (const BYTE*)szPath, (wcslen(szPath) + 1) * sizeof(wchar_t));

        if (lResult != ERROR_SUCCESS) {
            // Handle error if setting the value fails
        }

        RegCloseKey(hKey);
    } else {
        // Handle error if opening the key fails
    }
}