#include <windows.h>
#include <string.h>   // For strlen and strcmp
#include <tlhelp32.h> // For CreateToolhelp32Snapshot and related functions

// Function to inject a DLL into a specified process
void DLL_Injection(char *dll, char *process) {
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD pid = 0;
    HANDLE hProcess = NULL;
    LPVOID remoteAllocatedMemory = NULL;
    HANDLE hRemoteThread = NULL;

    // 1. Find the target process ID (PID)
    // Create a snapshot of all running processes
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        // Error: Could not create process snapshot.
        return;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process, and exit if unsuccessful
    if (!Process32First(hSnapshot, &pe32)) {
        // Error: Could not retrieve first process information.
        CloseHandle(hSnapshot);
        return;
    }

    // Iterate through all processes to find the target by name
    do {
        // Compare the executable file name (ANSI)
        if (strcmp(pe32.szExeFile, process) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    // Close the snapshot handle as it's no longer needed
    CloseHandle(hSnapshot);

    if (pid == 0) {
        // Error: Target process not found.
        return;
    }

    // 2. Open the target process
    // Request necessary permissions: PROCESS_CREATE_THREAD, PROCESS_VM_OPERATION, PROCESS_VM_WRITE
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
    if (hProcess == NULL) {
        // Error: Could not open target process. (e.g., insufficient permissions)
        return;
    }

    // 3. Allocate memory in the target process for the DLL path string
    size_t dllPathLen = strlen(dll) + 1; // +1 for the null terminator
    remoteAllocatedMemory = VirtualAllocEx(hProcess, NULL, dllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteAllocatedMemory == NULL) {
        // Error: Could not allocate memory in the remote process.
        CloseHandle(hProcess);
        return;
    }

    // 4. Write the DLL path string into the allocated memory
    if (!WriteProcessMemory(hProcess, remoteAllocatedMemory, dll, dllPathLen, NULL)) {
        // Error: Could not write the DLL path into remote process memory.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE); // Free allocated memory
        CloseHandle(hProcess);
        return;
    }

    // 5. Get the address of LoadLibraryA in kernel32.dll
    // LoadLibraryA is used because the DLL path is an ANSI string (char*).
    // GetModuleHandleA retrieves a handle to kernel32.dll which is loaded in every process.
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32 == NULL) {
        // Error: Could not get handle to kernel32.dll. (Highly unlikely unless system is critically broken)
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Get the address of the LoadLibraryA function within kernel32.dll.
    // This address is the same across all processes.
    LPTHREAD_START_ROUTINE pLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        // Error: Could not get address of LoadLibraryA function.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 6. Create a remote thread in the target process
    // The thread will execute LoadLibraryA with the remote DLL path as its argument.
    hRemoteThread = CreateRemoteThread(hProcess, // Handle to the target process
                                       NULL,     // Default security attributes
                                       0,        // Default stack size
                                       pLoadLibraryA, // Start address (LoadLibraryA)
                                       remoteAllocatedMemory, // Argument to LoadLibraryA (DLL path)
                                       0,        // Creation flags (run immediately)
                                       NULL);    // Don't need thread ID
    if (hRemoteThread == NULL) {
        // Error: Could not create remote thread.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 7. Wait for the remote thread to finish execution (optional but good practice)
    // This ensures LoadLibraryA has completed loading the DLL.
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Clean up resources
    CloseHandle(hRemoteThread);

    // Free the memory allocated in the remote process for the DLL path.
    // This is safe to do after LoadLibraryA has finished.
    VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);

    CloseHandle(hProcess);
}