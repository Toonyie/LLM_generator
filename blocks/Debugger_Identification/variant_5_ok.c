#include <windows.h>

BOOL Debugger_Identification() {
    // 1. Use IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        return TRUE;
    }

    // 2. Use CheckRemoteDebuggerPresent()
    BOOL bRemoteDebuggerPresent = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &bRemoteDebuggerPresent) && bRemoteDebuggerPresent) {
        return TRUE;
    }

    return FALSE;
}