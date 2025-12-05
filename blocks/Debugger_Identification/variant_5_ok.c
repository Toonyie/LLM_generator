#include <windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    ULONG Reserved2[2];
    ULONG UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;


BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL isRemoteDebuggerPresent = FALSE;
    HANDLE hProcess = GetCurrentProcess();

    // 1. IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. CheckRemoteDebuggerPresent()
    if (CheckRemoteDebuggerPresent(hProcess, &isRemoteDebuggerPresent)) {
        // CheckRemoteDebuggerPresent success
    } else {
        // CheckRemoteDebuggerPresent failed, handle error if needed
    }
    
    if (isDebuggerPresent || isRemoteDebuggerPresent)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}