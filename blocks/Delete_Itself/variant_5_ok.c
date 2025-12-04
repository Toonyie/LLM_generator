#include <windows.h>

void Delete_Itself() {
    WCHAR filePath[MAX_PATH];
    DWORD pathLength = GetModuleFileNameW(NULL, filePath, MAX_PATH);

    if (pathLength > 0 && pathLength < MAX_PATH) {
        // Schedule the file for deletion upon next system reboot.
        // The first parameter is the existing file path.
        // The second parameter is NULL to indicate deletion, not renaming.
        // MOVEFILE_DELAY_UNTIL_REBOOT flag ensures deletion occurs after a restart.
        MoveFileExW(filePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
}