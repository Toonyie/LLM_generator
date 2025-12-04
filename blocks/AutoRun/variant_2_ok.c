#include <windows.h>
#include <wchar.h>

void AutoRun() {
    wchar_t szPath[MAX_PATH];
    HKEY hKey;
    LONG lResult;

    // Get the full path to the current executable.
    // GetModuleFileNameW(NULL, ...) retrieves the path for the current process.
    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        // Failed to get the module file name.
        // In a real application, consider logging this error.
        return;
    }

    // Open or create the "Run" key for the current user.
    // HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
    lResult = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,                  // Reserved
        NULL,               // Class (ignored for existing key)
        REG_OPTION_NON_VOLATILE, // Key is stored permanently
        KEY_WRITE,          // We need write access to set a value
        NULL,               // Security attributes
        &hKey,              // Pointer to a variable that receives the opened/created key handle
        NULL                // Optional: pointer to a variable that indicates if the key was created or opened
    );

    if (lResult != ERROR_SUCCESS) {
        // Failed to open or create the registry key.
        // In a real application, consider logging this error.
        return;
    }

    // Set a new value within the "Run" key.
    // The value name "MyApplicationAutoRun" should be unique to your application.
    // REG_SZ indicates a null-terminated string.
    // The size in bytes includes the null terminator (wcslen + 1) * sizeof(wchar_t).
    lResult = RegSetValueExW(
        hKey,
        L"MyApplicationAutoRun", // Name of the value (e.g., your app's name)
        0,                       // Reserved
        REG_SZ,                  // Data type (string)
        (const BYTE*)szPath,     // Pointer to the path string
        (wcslen(szPath) + 1) * sizeof(wchar_t) // Size of the data in bytes
    );

    if (lResult != ERROR_SUCCESS) {
        // Failed to set the registry value.
        // In a real application, consider logging this error.
    }

    // Close the registry key handle.
    RegCloseKey(hKey);
}