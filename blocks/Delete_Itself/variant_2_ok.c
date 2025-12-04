#include <windows.h>
#include <stdio.h> // For _snwprintf_s
#include <wchar.h> // For wcsrchr, wcscpy_s, wcslen

void Delete_Itself() {
    WCHAR szPath[MAX_PATH];
    WCHAR szTempDir[MAX_PATH];
    WCHAR szTempBatchPath[MAX_PATH];
    WCHAR szBatchContent[MAX_PATH * 2]; // Buffer for the batch file content
    HANDLE hFile;
    DWORD dwBytesWritten;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // 1. Get the full path to the current executable file.
    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        // Failed to get executable path. Cannot proceed with self-deletion.
        return;
    }

    // 2. Get the path to the system's temporary directory.
    if (GetTempPathW(MAX_PATH, szTempDir) == 0) {
        // Failed to get temporary directory.
        return;
    }

    // 3. Create a unique temporary file name for our batch script.
    // The name will be like "C:\TEMP\delxxxx.tmp".
    if (GetTempFileNameW(szTempDir, L"del", 0, szTempBatchPath) == 0) {
        // Failed to create temporary file name.
        return;
    }

    // Change the extension from .tmp to .bat so ShellExecute or CreateProcess
    // recognizes it as a batch file.
    WCHAR* dot = wcsrchr(szTempBatchPath, L'.');
    if (dot) {
        wcscpy_s(dot, MAX_PATH - (dot - szTempBatchPath), L".bat");
    } else {
        // Fallback if no dot found (should not happen with GetTempFileNameW)
        wcscat_s(szTempBatchPath, MAX_PATH, L".bat");
    }

    // 4. Create the batch file.
    hFile = CreateFileW(szTempBatchPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to create the batch file.
        return;
    }

    // The batch file content:
    // @echo off
    // timeout /t 3 /nobreak > nul  -- Wait for 3 seconds to allow the parent process to exit
    // del "C:\path\to\original\executable.exe"
    // del "C:\path\to\this\batchfile.bat"
    _snwprintf_s(szBatchContent, MAX_PATH * 2, _TRUNCATE,
                 L"@echo off\r\n"
                 L"timeout /t 3 /nobreak > nul\r\n" // Wait for parent process to exit
                 L"del \"%s\"\r\n"                  // Delete the original executable
                 L"del \"%s\"\r\n",                 // Delete the batch file itself
                 szPath, szTempBatchPath);

    // Write the batch file content.
    // The string length is multiplied by sizeof(WCHAR) because WriteFile expects byte count.
    if (!WriteFile(hFile, szBatchContent, wcslen(szBatchContent) * sizeof(WCHAR), &dwBytesWritten, NULL)) {
        CloseHandle(hFile);
        // Failed to write to batch file. Try to clean up.
        DeleteFileW(szTempBatchPath);
        return;
    }

    CloseHandle(hFile); // Close the handle to the batch file

    // 5. Execute the batch file.
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Construct the command line for cmd.exe to execute the batch file.
    // cmd.exe /c executes the command and then terminates.
    WCHAR szCmdLine[MAX_PATH + 100]; // Buffer for cmd.exe /c ...
    _snwprintf_s(szCmdLine, MAX_PATH + 100, _TRUNCATE, L"cmd.exe /c \"%s\"", szTempBatchPath);

    if (!CreateProcessW(NULL,               // No module name (use command line)
                       szCmdLine,          // Command line
                       NULL,               // Process handle not inheritable
                       NULL,               // Thread handle not inheritable
                       FALSE,              // Set handle inheritance to FALSE
                       CREATE_NO_WINDOW,   // Do not create a console window
                       NULL,               // Use parent's environment block
                       NULL,               // Use parent's starting directory
                       &si,                // Pointer to STARTUPINFO structure
                       &pi)                // Pointer to PROCESS_INFORMATION structure
    ) {
        // If CreateProcess fails, the batch file wasn't launched.
        // Try deleting the batch file as it's no longer needed (and failed to launch).
        DeleteFileW(szTempBatchPath);
        return;
    }

    // Close the handles for the newly created process and its primary thread.
    // We don't need to wait for it, as it will outlive us to delete our executable.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 6. Exit the current process. This is crucial as it releases the lock on
    // the executable file, allowing the batch script to delete it.
    ExitProcess(0); // The function will not return from this point.
}