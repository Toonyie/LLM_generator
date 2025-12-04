#include <windows.h>
#include <tlhelp32.h>
#include <string.h> // For strlen and _stricmp

void DLL_Injection(char *dll, char *process) {
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD pid = 0;
    HANDLE hProcess = NULL;
    LPVOID remotePath = NULL;
    HMODULE hKernel32 = NULL;
    FARPROC pLoadLibraryA = NULL;
    HANDLE hThread = NULL;

    // 1. Find the target process ID (PID)
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return; // Failed to create snapshot
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return; // Failed to get first process
    }

    do {
        // Compare process name (case-insensitive)
        if (_stricmp(pe32.szExeFile, process) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot); // Always close snapshot handle

    if (pid == 0) {
        return; // Process not found
    }

    // 2. Open the target process with necessary permissions
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return; // Failed to open process
    }

    // 3. Allocate memory in the target process for the DLL path string
    size_t dllPathLen = strlen(dll);
    remotePath = VirtualAllocEx(hProcess, NULL, dllPathLen + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remotePath == NULL) {
        CloseHandle(hProcess);
        return; // Failed to allocate memory in remote process
    }

    // 4. Write the DLL path string into the allocated memory in the target process
    if (!WriteProcessMemory(hProcess, remotePath, dll, dllPathLen + 1, NULL)) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE); // Clean up allocated memory
        CloseHandle(hProcess);
        return; // Failed to write to remote process memory
    }

    // 5. Get the address of LoadLibraryA in kernel32.dll
    hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32 == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return; // Failed to get handle to kernel32.dll
    }

    pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return; // Failed to get address of LoadLibraryA
    }

    // 6. Create a remote thread in the target process to execute LoadLibraryA
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryA, remotePath, 0, NULL);
    if (hThread == NULL) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return; // Failed to create remote thread
    }

    // 7. Wait for the remote thread to finish (optional)
    WaitForSingleObject(hThread, INFINITE);

    // 8. Clean up
    CloseHandle(hThread);

    // Free the memory allocated for the DLL path string.
    // The DLL itself remains loaded in the target process.
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);

    CloseHandle(hProcess);
}