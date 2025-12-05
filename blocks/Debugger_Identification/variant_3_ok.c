#include <windows.h>

// Define necessary structures if NTAPI functions are used
// (Not using NTAPI in this implementation as requested)

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL isRemoteDebuggerPresent = FALSE;

    // 1. Check with IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. Check with CheckRemoteDebuggerPresent()
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess != NULL) {
        CheckRemoteDebuggerPresent(hProcess, &isRemoteDebuggerPresent);
        CloseHandle(hProcess);
    }

    return isDebuggerPresent || isRemoteDebuggerPresent;
}