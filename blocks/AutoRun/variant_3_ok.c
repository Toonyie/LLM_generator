#include <windows.h> // Required for Windows API functions and types
#include <wchar.h>   // Required for wcslen for wide character string manipulation

// Function prototype as requested: void AutoRun();
void AutoRun() {
    HKEY hKey = NULL;
    WCHAR szPath[MAX_PATH];
    DWORD dwPathLen;

    // Get the full path to the current executable.
    // GetModuleFileNameW returns the number of characters copied to the buffer,
    // not including the null terminator. If it fails, it returns 0.
    // If the buffer is too small, it returns dwSize (MAX_PATH in this case).
    dwPathLen = GetModuleFileNameW(NULL, szPath, MAX_PATH);

    if (dwPathLen == 0 || dwPathLen >= MAX_PATH) {
        // Error getting path or path too long to fit in the buffer.
        // For a void function, we simply exit without performing the autorun registration.
        return;
    }

    // Open the "Run" key in the current user's registry hive.
    // This key is used for programs that should run automatically when the user logs on.
    // KEY_SET_VALUE access is required to write a new value.
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0,
                      KEY_SET_VALUE,
                      &hKey) == ERROR_SUCCESS) {

        // Set the value in the "Run" key.
        // L"MyApplicationAutoRun" is the name of the registry value that will be created.
        // You can choose any unique name for your application.
        // REG_SZ indicates that the data type is a null-terminated string.
        // The size is calculated to include the null terminator: (dwPathLen + 1) * sizeof(WCHAR).
        RegSetValueExW(hKey,
                       L"MyApplicationAutoRun", // Name of the value (e.g., "My Application")
                       0,                        // Reserved, must be zero
                       REG_SZ,                   // Data type is a null-terminated string
                       (const BYTE*)szPath,      // Pointer to the string data (the executable path)
                       (dwPathLen + 1) * sizeof(WCHAR)); // Size of the data in bytes

        // Close the registry key after the operation.
        RegCloseKey(hKey);
    }
    // If RegOpenKeyExW fails, hKey will be NULL, and the function will silently complete
    // without attempting to set the registry value.
}