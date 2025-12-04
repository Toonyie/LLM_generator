#include <windows.h>
#include <string.h> // For strlen

void AutoRun() {
    char szPath[MAX_PATH];
    HKEY hKey;
    const char* szSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const char* szValueName = "MyApplicationAutoRun"; // A unique name for your application's entry

    // Get the full path to the current executable
    if (GetModuleFileNameA(NULL, szPath, MAX_PATH) == 0) {
        // Failed to get module file name
        return;
    }

    // Open or create the "Run" key under HKEY_CURRENT_USER
    // KEY_SET_VALUE access is required to write to the key
    if (RegCreateKeyExA(HKEY_CURRENT_USER,
                        szSubKey,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,
                        NULL,
                        &hKey,
                        NULL) != ERROR_SUCCESS) {
        // Failed to open or create the registry key
        return;
    }

    // Set the registry value to the executable's path
    // REG_SZ indicates a null-terminated string
    // strlen(szPath) + 1 includes the null terminator in the length
    if (RegSetValueExA(hKey,
                       szValueName,
                       0,
                       REG_SZ,
                       (const BYTE*)szPath,
                       (DWORD)strlen(szPath) + 1) != ERROR_SUCCESS) {
        // Failed to set the registry value
    }

    // Close the registry key handle
    RegCloseKey(hKey);
}