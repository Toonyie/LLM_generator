#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Define UNICODE_STRING structure
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void DLL_Injection(char *dll, char *process) {
    DWORD processID = 0;
    HANDLE hProcess = NULL;
    LPVOID remoteString = NULL;
    HANDLE hThread = NULL;

    // Get process ID by name
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, process) == 0) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (processID == 0) {
        return;
    }

    // Open the process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        return;
    }

    // Allocate memory in the remote process for the DLL path
    remoteString = VirtualAllocEx(hProcess, NULL, strlen(dll) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteString == NULL) {
        CloseHandle(hProcess);
        return;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, remoteString, dll, strlen(dll) + 1, NULL)) {
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Get the address of LoadLibraryA
    LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddress == NULL) {
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread to call LoadLibraryA with the DLL path
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, remoteString, 0, NULL);
    if (hThread == NULL) {
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}