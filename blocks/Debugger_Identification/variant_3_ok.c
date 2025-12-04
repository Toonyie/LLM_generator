#include <windows.h>

BOOL Debugger_Identification() {
    // 1. Check using IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        return TRUE;
    }

    // 2. Check using CheckRemoteDebuggerPresent()
    BOOL bRemoteDebuggerPresent = FALSE;
    HANDLE hCurrentProcess = GetCurrentProcess();

    // Check if CheckRemoteDebuggerPresent succeeds and indicates a debugger
    if (CheckRemoteDebuggerPresent(hCurrentProcess, &bRemoteDebuggerPresent)) {
        if (bRemoteDebuggerPresent) {
            return TRUE;
        }
    }
    // Note: If CheckRemoteDebuggerPresent fails, it returns FALSE,
    // and bRemoteDebuggerPresent might not be reliably updated or might indicate no debugger.
    // In either case, if it fails or returns FALSE for bRemoteDebuggerPresent,
    // we proceed assuming no remote debugger was detected by this method.

    // No debugger identified by either method
    return FALSE;
}