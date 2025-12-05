#include <windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef LONG NTSTATUS;
typedef NTSTATUS(NTAPI *PNT_QUERY_INFORMATION_PROCESS)(
    HANDLE           ProcessHandle,
    DWORD            ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
    );

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