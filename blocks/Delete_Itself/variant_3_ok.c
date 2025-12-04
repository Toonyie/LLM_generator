#include <windows.h>
#include <stdio.h> // Required for swprintf_s

// Function prototype: void Delete_Itself();
void Delete_Itself() {
    wchar_t szPath[MAX_PATH];
    wchar_t szRenamedPath[MAX_PATH];
    // Buffer for cmd.exe arguments. 
    // MAX_PATH * 2 for potential long path (renamed file) + 
    // approximately 64 characters for the command prefix and suffix.
    wchar_t szCmdArgs[MAX_PATH * 2 + 64]; 

    // 1. Get the full path to the current executable file.
    // GetModuleFileNameW returns the number of characters copied to the buffer,
    // or 0 if it fails.
    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        return; // Could not get module file name, exit.
    }

    // 2. Construct a temporary name for the executable to be deleted.
    // We append ".tmp" to the original file name.
    // wcscpy_s and wcscat_s return 0 on success.
    if (wcscpy_s(szRenamedPath, MAX_PATH, szPath) != 0) {
        return; // Buffer copy failed
    }
    if (wcscat_s(szRenamedPath, MAX_PATH, L".tmp") != 0) {
        return; // Buffer concatenation failed
    }

    // 3. Rename the current executable.
    // This allows the original path to be free. The renamed file (szRenamedPath)
    // is still locked by the current process, but renaming usually succeeds.
    if (!MoveFileW(szPath, szRenamedPath)) {
        // If renaming fails, self-deletion using this method is not possible.
        // This could happen due to permission issues or a very strict file lock.
        return;
    }

    // 4. Prepare the command line arguments for a cleaner process (cmd.exe).
    // This command will:
    //   - 'ping 127.0.0.1 -n 2 > nul': Wait for approximately 1 second.
    //     This delay is critical to ensure the current process exits before deletion.
    //   - '&& del \"%s\"': Delete the renamed temporary file.
    // We use swprintf_s for safe formatting, providing the buffer size.
    if (swprintf_s(szCmdArgs, sizeof(szCmdArgs)/sizeof(szCmdArgs[0]), 
                    L"/c ping 127.0.0.1 -n 2 > nul && del \"%s\"", 
                    szRenamedPath) < 0) {
        return; // Formatting failed or buffer too small
    }

    // 5. Setup STARTUPINFO and PROCESS_INFORMATION structures for CreateProcessW.
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // Hide the command prompt window that would otherwise pop up for cmd.exe.
    si.dwFlags |= STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; 
    
    ZeroMemory(&pi, sizeof(pi));

    // 6. Launch a new 'cmd.exe' process that will perform the deletion.
    // We provide the full path to cmd.exe for robustness, avoiding reliance on system PATH.
    // The szCmdArgs buffer is passed as lpCommandLine, which CreateProcessW might modify,
    // so it must be writable.
    if (!CreateProcessW(
        L"C:\\Windows\\System32\\cmd.exe", // Path to the executable (cmd.exe)
        szCmdArgs,                         // Command line arguments for cmd.exe (mutable buffer)
        NULL,                              // Process handle not inheritable
        NULL,                              // Thread handle not inheritable
        FALSE,                             // Set handle inheritance to FALSE
        CREATE_NO_WINDOW,                  // Don't create a console window for the cleaner
        NULL,                              // Use parent's environment block
        NULL,                              // Use parent's starting directory
        &si,                               // Pointer to STARTUPINFO structure
        &pi                                // Pointer to PROCESS_INFORMATION structure
    )) {
        // Failed to create the cleaner process. The renamed file might remain on disk.
        // No further action is possible from this function as it's void.
        return;
    }

    // 7. Close the process and thread handles immediately as we don't need to wait for the cleaner.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 8. Exit the current process. This is crucial for the cleaner process to be able to delete the file.
    // If this process does not exit, the renamed file will remain locked and undeletable by the cleaner.
    ExitProcess(0);
}