#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void DLL_Injection(char *dll, char *process) {
    DWORD processID = 0;
    HANDLE hProcess = NULL;
    LPVOID LoadLibAddr = NULL;
    LPVOID RemoteStringAddr = NULL;
    HANDLE hThread = NULL;

    // Get process ID by process name
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (strcmp(entry.szExeFile, process) == 0) {
                processID = entry.th32ProcessID;
                break;
            }
        }
    }

    CloseHandle(snapshot);

    if (processID == 0) {
        fprintf(stderr, "Process not found!\n");
        return;
    }

    // Open the process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        fprintf(stderr, "OpenProcess failed: %d\n", GetLastError());
        return;
    }

    // Get the address of LoadLibraryA
    LoadLibAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (LoadLibAddr == NULL) {
        fprintf(stderr, "GetProcAddress(LoadLibraryA) failed: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Allocate memory in the remote process for the DLL path
    RemoteStringAddr = VirtualAllocEx(hProcess, NULL, strlen(dll) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (RemoteStringAddr == NULL) {
        fprintf(stderr, "VirtualAllocEx failed: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, RemoteStringAddr, dll, strlen(dll) + 1, NULL)) {
        fprintf(stderr, "WriteProcessMemory failed: %d\n", GetLastError());
        VirtualFreeEx(hProcess, RemoteStringAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread to call LoadLibraryA with the DLL path
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibAddr, RemoteStringAddr, 0, NULL);
    if (hThread == NULL) {
        fprintf(stderr, "CreateRemoteThread failed: %d\n", GetLastError());
        VirtualFreeEx(hProcess, RemoteStringAddr, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, RemoteStringAddr, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}