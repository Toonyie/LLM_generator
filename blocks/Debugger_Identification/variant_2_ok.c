#include <windows.h>

// Define necessary structs if not including winternl.h
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef LONG NTSTATUS;

// Define necessary constants if not including winternl.h
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL remoteDebuggerPresent = FALSE;
    HANDLE hProcess = GetCurrentProcess();

    // 1. IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. CheckRemoteDebuggerPresent()
    if (CheckRemoteDebuggerPresent(hProcess, &remoteDebuggerPresent)) {
        // CheckRemoteDebuggerPresent succeeded
        if (remoteDebuggerPresent) {
            isDebuggerPresent = TRUE;
        }
    } else {
        // CheckRemoteDebuggerPresent failed.  Handle the error as needed.
        // For example:
        // DWORD error = GetLastError();
        // fprintf(stderr, "CheckRemoteDebuggerPresent failed with error code: %d\n", error);

        // Optionally, decide if a failure here should be considered evidence of debugging.
        // In a highly sensitive environment, even the *attempt* to check for a remote
        // debugger might be suspect.  But for most cases, treating the failure as
        // simply inconclusive is fine.
    }

    // Do NOT use NtQueryInformationProcess or winternl.h.
    // All requested checks are implemented above.

    return isDebuggerPresent;
}