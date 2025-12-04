#include <windows.h>

BOOL Debugger_Identification() {
    // 1. Use IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        return TRUE;
    }

    // 2. Use CheckRemoteDebuggerPresent()
    BOOL remoteDebuggerPresent = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebuggerPresent)) {
        if (remoteDebuggerPresent) {
            return TRUE;
        }
    }
    // If CheckRemoteDebuggerPresent fails, we conservatively assume no remote debugger was detected via this method.

    return FALSE;
}