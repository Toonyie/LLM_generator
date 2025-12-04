#include <windows.h>

BOOL Debugger_Identification() {
    // 1. Use IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        return TRUE;
    }

    // 2. Use CheckRemoteDebuggerPresent()
    BOOL remoteDebuggerPresent = FALSE;
    // GetCurrentProcess() returns a pseudo-handle for the current process.
    // This handle does not need to be closed.
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebuggerPresent)) {
        if (remoteDebuggerPresent) {
            return TRUE;
        }
    }
    // If CheckRemoteDebuggerPresent fails or indicates no remote debugger,
    // we proceed to return FALSE unless a debugger was already found.

    // If neither method detected a debugger, return FALSE
    return FALSE;
}