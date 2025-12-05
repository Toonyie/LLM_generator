#include <windows.h>
#include <stdio.h>

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
    if (GetModuleFileNameW(NULL, filename, MAX_PATH) == 0) {
        return;
    }

    WCHAR cmd[MAX_PATH * 2 + 50];
    swprintf(cmd, sizeof(cmd) / sizeof(WCHAR), L"cmd /c del /f /q \"%s\" > nul 2>&1 & del /f /q \"%s\"", filename, filename);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    ExitProcess(0);
}