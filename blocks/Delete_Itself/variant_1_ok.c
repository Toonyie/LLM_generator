#include <windows.h>

#ifndef UNICODE
typedef unsigned short WCHAR;
#define UNICODE
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void Delete_Itself() {
    WCHAR filename[MAX_PATH + 1];
    DWORD length = GetModuleFileNameW(NULL, filename, MAX_PATH);
    if (length == 0 || length > MAX_PATH) {
        return; // Failed to get the filename
    }

    // Create a batch file to delete the executable
    WCHAR batchFilename[MAX_PATH + 1];
    WCHAR tempPath[MAX_PATH + 1];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        return;
    }

    if (GetTempFileNameW(tempPath, L"del", 0, batchFilename) == 0) {
        return;
    }

    HANDLE hFile = CreateFileW(batchFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    WCHAR batchContent[MAX_PATH * 2 + 100];
    swprintf_s(batchContent, sizeof(batchContent) / sizeof(WCHAR),
               L"@echo off\r\n"
               L":repeat\r\n"
               L"del \"%s\"\r\n"
               L"if exist \"%s\" (\r\n"
               L"   timeout /t 1 /nobreak > nul\r\n"
               L"   goto repeat\r\n"
               L")\r\n"
               L"del \"%%~f0\"\r\n"
               L"exit",
               filename, filename);
    DWORD bytesWritten;
    WriteFile(hFile, batchContent, (DWORD)(wcslen(batchContent) * sizeof(WCHAR)), &bytesWritten, NULL);
    CloseHandle(hFile);

    // Execute the batch file in a separate process
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    WCHAR cmdLine[MAX_PATH + 10];
    swprintf_s(cmdLine, sizeof(cmdLine) / sizeof(WCHAR), L"cmd.exe /c start \"\" /min %s", batchFilename);

    if (CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    ExitProcess(0); // Terminate the current process
}