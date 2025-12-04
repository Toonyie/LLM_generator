#include <windows.h>
#include <tlhelp32.h>
#include <string.h> // For strlen and _stricmp

void DLL_Injection(char *dll, char *process) {
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD dwProcessId = 0;
    HANDLE hProcess = NULL;
    LPVOID lpRemoteDllPath = NULL;
    HANDLE hRemoteThread = NULL;
    HMODULE hKernel32 = NULL;
    FARPROC pLoadLibraryA = NULL;
    SIZE_T dwDllPathLen = 0;

    // 1. Get Process ID (PID)
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return;
    }

    do {
        // Compare process name using case-insensitive string comparison
        if (_stricmp(pe32.szExeFile, process) == 0) {
            dwProcessId = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot); // Always close the snapshot handle

    if (dwProcessId == 0) {
        // Process not found
        return;
    }

    // 2. Open Process with necessary permissions
    // PROCESS_CREATE_THREAD: Required to create a new thread in the process.
    // PROCESS_QUERY_INFORMATION: Required to retrieve certain information about the process.
    // PROCESS_VM_OPERATION: Required to perform operations on the address space of the process (e.g., VirtualAllocEx).
    // PROCESS_VM_WRITE: Required to write to memory in the process (e.g., WriteProcessMemory).
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
                           FALSE, dwProcessId);
    if (hProcess == NULL) {
        return;
    }

    // 3. Allocate memory in the target process for the DLL path
    dwDllPathLen = (strlen(dll) + 1) * sizeof(char); // +1 for null terminator
    lpRemoteDllPath = VirtualAllocEx(hProcess, NULL, dwDllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (lpRemoteDllPath == NULL) {
        CloseHandle(hProcess);
        return;
    }

    // 4. Write DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, lpRemoteDllPath, dll, dwDllPathLen, NULL)) {
        VirtualFreeEx(hProcess, lpRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 5. Get the address of LoadLibraryA from kernel32.dll
    // GetModuleHandleA does not require a corresponding CloseHandle.
    hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32 == NULL) {
        VirtualFreeEx(hProcess, lpRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        VirtualFreeEx(hProcess, lpRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 6. Create a remote thread in the target process to call LoadLibraryA
    hRemoteThread = CreateRemoteThread(hProcess,
                                       NULL,           // Default security attributes
                                       0,              // Default stack size
                                       (LPTHREAD_START_ROUTINE)pLoadLibraryA, // Start routine is LoadLibraryA
                                       lpRemoteDllPath, // Argument is the remote address of the DLL path
                                       0,              // Creation flags (run immediately)
                                       NULL);          // No thread ID needed
    if (hRemoteThread == NULL) {
        VirtualFreeEx(hProcess, lpRemoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 7. Wait for the remote thread to complete (optional, but ensures DLL is loaded)
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Clean up resources
    CloseHandle(hRemoteThread); // Close the handle to the remote thread
    VirtualFreeEx(hProcess, lpRemoteDllPath, 0, MEM_RELEASE); // Free memory in target process
    CloseHandle(hProcess); // Close the handle to the target process
}