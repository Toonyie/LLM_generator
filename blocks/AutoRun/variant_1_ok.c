#include <windows.h>
#include <string.h> // Required for wcslen

void AutoRun() {
    WCHAR szPath[MAX_PATH];
    DWORD dwRet = GetModuleFileNameW(NULL, szPath, MAX_PATH);

    if (dwRet == 0 || dwRet >= MAX_PATH) {
        // Failed to get executable path or buffer too small
        return;
    }

    HKEY hKey;
    LONG lResult = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE, // Required access for setting a value
        &hKey
    );

    // If the key could not be opened for writing, try creating it with write access.
    // This handles cases where the key might not exist (though "Run" typically does).
    if (lResult != ERROR_SUCCESS) {
        lResult = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE, // Ensure write access for the key
            NULL,
            &hKey,
            NULL
        );
    }
    
    if (lResult == ERROR_SUCCESS) {
        // Define a unique name for your application's Autorun entry.
        // It's good practice to make this name specific to your application.
        LPCWSTR pszValueName = L"MyApplicationAutoRun";

        // Set the registry value.
        // The data is the full path to the executable.
        lResult = RegSetValueExW(
            hKey,
            pszValueName,
            0,
            REG_SZ,
            (const BYTE*)szPath, // Cast to const BYTE* as required by the API
            (wcslen(szPath) + 1) * sizeof(WCHAR) // Size in bytes, including the null terminator
        );

        RegCloseKey(hKey); // Always close the registry key handle
    }
    // No explicit error handling requested beyond the function scope for RegSetValueExW failure.
}