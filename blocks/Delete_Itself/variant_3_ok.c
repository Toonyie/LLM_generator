#include <windows.h>
#include <stdio.h>

// Define UNICODE_STRING if not available (MinGW-w64 compatibility)
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void Delete_Itself() {
    TCHAR szModuleName[MAX_PATH];
    TCHAR szCmd[2 * MAX_PATH];
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    GetModuleFileName(NULL, szModuleName, MAX_PATH);

    // Construct the command to delete the file using cmd.exe
    // We use cmd.exe because it can delete files that are currently in use
    // with the /c flag to execute a command and then terminate
    // and the /s flag to parse the string after /c as a single command.
    // We use ping to wait for a short period of time to ensure the file is released.
    // We use del /f /q to forcefully delete the file without prompting.
    // We also add an exit command to terminate the cmd.exe process.
    
    snprintf(szCmd, sizeof(szCmd), "cmd.exe /c ping 127.0.0.1 -n 2 > nul && del /f /q \"%s\" && exit", szModuleName);
    

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Hide the command prompt window

    // Create the process to delete the file
    if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        // Close handles to the process and thread
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // Terminate the current process
    ExitProcess(0);
}