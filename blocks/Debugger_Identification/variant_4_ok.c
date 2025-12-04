#include <windows.h>

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL remoteDebuggerPresent = FALSE;

    // 1. Use IsDebuggerPresent()
    isDebuggerPresent = IsDebuggerPresent();

    // 2. Use CheckRemoteDebuggerPresent()
    // Get a handle to the current process
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess != NULL) {
        CheckRemoteDebuggerPresent(hProcess, &remoteDebuggerPresent);
        // No need to close GetCurrentProcess() handle as it's a pseudo-handle
    }

    // Return TRUE if either method detects a debugger
    return isDebuggerPresent || remoteDebuggerPresent;
}