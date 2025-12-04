#include <windows.h>

void AutoRun() {
    HKEY hKey;
    LPCWSTR subkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCWSTR valueName = L"MyApplicationAutoRun"; // Unique name for your application

    WCHAR currentExePath[MAX_PATH];
    DWORD pathLength = GetModuleFileNameW(NULL, currentExePath, MAX_PATH);

    if (pathLength == 0 || pathLength >= MAX_PATH) {
        // Failed to get executable path or path too long
        return;
    }

    LONG lResult = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        subkey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        NULL,
        &hKey,
        NULL
    );

    if (lResult != ERROR_SUCCESS) {
        // Failed to open or create registry key
        return;
    }

    lResult = RegSetValueExW(
        hKey,
        valueName,
        0,
        REG_SZ,
        (LPBYTE)currentExePath,
        (wcslen(currentExePath) + 1) * sizeof(WCHAR)
    );

    RegCloseKey(hKey);

    // In a real application, you might check 'lResult' here
    // to determine if RegSetValueExW was successful.
}