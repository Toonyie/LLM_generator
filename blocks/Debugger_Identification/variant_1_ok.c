#include <windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL remoteDebuggerPresent = FALSE;
    HANDLE hProcess = GetCurrentProcess();

    // 1. IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. CheckRemoteDebuggerPresent()
    CheckRemoteDebuggerPresent(hProcess, &remoteDebuggerPresent);

    return isDebuggerPresent || remoteDebuggerPresent;
}