#include <windows.h>
#include <tlhelp32.h> // Required for PROCESSENTRY32, CreateToolhelp32Snapshot, Process32First/Next
#include <string.h>   // Required for strlen, _stricmp

// Helper function to get the Process ID (PID) from the process name
// Declared static to indicate internal linkage, if this was part of a larger compilation unit.
static DWORD GetProcessIdByName(const char *processName) {
    PROCESSENTRY32 pe32;
    // Initialize structure size before use
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Take a snapshot of all running processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        // Failed to create snapshot
        return 0;
    }

    // Retrieve information about the first process and then iterate
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Compare process names (case-insensitive for robust matching)
            // _stricmp is a Microsoft-specific function (available in MinGW)
            // It's like strcasecmp for POSIX systems.
            if (_stricmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnapshot); // Close the snapshot handle
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot); // Close the snapshot handle
    return 0; // Process not found
}

void DLL_Injection(char *dll, char *process) {
    // 1. Get the Process ID (PID) of the target process
    DWORD pid = GetProcessIdByName(process);
    if (pid == 0) {
        // Target process not found or error occurred
        return;
    }

    // 2. Open the target process with necessary permissions
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD |     // Required for CreateRemoteThread
        PROCESS_QUERY_INFORMATION | // Required by some debuggers/tools, good for general access
        PROCESS_VM_OPERATION |      // Required for VirtualAllocEx, VirtualFreeEx
        PROCESS_VM_WRITE |          // Required for WriteProcessMemory
        PROCESS_VM_READ,            // Good for general memory operations, though not strictly needed here
        FALSE,                      // Do not inherit handles
        pid                         // Target Process ID
    );

    if (hProcess == NULL) {
        // Failed to open target process
        return;
    }

    // 3. Determine the length of the DLL path string, including null terminator
    size_t dllPathLen = strlen(dll) + 1; // +1 for the null-terminator byte

    // 4. Allocate memory in the target process for the DLL path string
    LPVOID remoteDllPath = VirtualAllocEx(
        hProcess,           // Handle to the target process
        NULL,               // Let the system determine the allocation address
        dllPathLen,         // Size of memory to allocate (bytes)
        MEM_COMMIT | MEM_RESERVE, // Commit and reserve memory
        PAGE_READWRITE      // Memory protection
    );

    if (remoteDllPath == NULL) {
        // Failed to allocate memory in target process
        CloseHandle(hProcess);
        return;
    }

    // 5. Write the DLL path string into the allocated memory in the target process
    if (!WriteProcessMemory(
            hProcess,       // Handle to the target process
            remoteDllPath,  // Base address of the memory in the target process
            dll,            // Pointer to the buffer containing the data to write
            dllPathLen,     // Number of bytes to write
            NULL            // Optional: receives the number of bytes written
        )) {
        // Failed to write memory
        VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE); // Free allocated memory
        CloseHandle(hProcess);
        return;
    }

    // 6. Get the address of the LoadLibraryA function from kernel32.dll
    // This address is typically the same across all processes for LoadLibraryA/W
    LPTHREAD_START_ROUTINE loadLibraryA_Addr = (LPTHREAD_START_ROUTINE)GetProcAddress(
        GetModuleHandleA("kernel32.dll"), // Handle to kernel32.dll in the current process
        "LoadLibraryA"                    // Name of the function to get
    );

    if (loadLibraryA_Addr == NULL) {
        // Failed to get LoadLibraryA address
        VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 7. Create a remote thread in the target process that executes LoadLibraryA
    // The argument to LoadLibraryA will be the address of the DLL path string
    HANDLE hRemoteThread = CreateRemoteThread(
        hProcess,           // Handle to the target process
        NULL,               // Default security attributes
        0,                  // Default stack size
        loadLibraryA_Addr,  // Starting address of the thread (LoadLibraryA)
        remoteDllPath,      // Argument to the thread function (DLL path address)
        0,                  // Creation flags (run immediately)
        NULL                // Optional: receives the thread ID
    );

    if (hRemoteThread == NULL) {
        // Failed to create remote thread
        VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 8. Wait for the remote thread to finish its execution (i.e., LoadLibraryA to return)
    WaitForSingleObject(hRemoteThread, INFINITE);

    // 9. Clean up resources
    CloseHandle(hRemoteThread);
    // Note: VirtualFreeEx is typically NOT called here for DLL injection
    // because the DLL is expected to remain loaded in the target process.
    // The small amount of memory holding the DLL path string is usually left.
    CloseHandle(hProcess);
}