#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void DLL_Injection(char *dll, char *process) {
    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;
    LPVOID pRemoteBuf = NULL;
    DWORD dwPID = 0;
    LPVOID pLoadLibraryW = NULL;

    // Get process ID
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed (%d)\n", GetLastError());
        return;
    }

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Process32First failed (%d)\n", GetLastError());
        CloseHandle(hSnapshot);
        return;
    }

    do {
        if (strcmp(pe32.szExeFile, process) == 0) {
            dwPID = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (dwPID == 0) {
        printf("Process %s not found\n", process);
        return;
    }

    // Open process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (hProcess == NULL) {
        printf("OpenProcess failed (%d)\n", GetLastError());
        return;
    }

    // Allocate memory in the remote process for the DLL path
    pRemoteBuf = VirtualAllocEx(hProcess, NULL, strlen(dll) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteBuf == NULL) {
        printf("VirtualAllocEx failed (%d)\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteBuf, dll, strlen(dll) + 1, NULL)) {
        printf("WriteProcessMemory failed (%d)\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Get the address of LoadLibraryW
    pLoadLibraryW = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (pLoadLibraryW == NULL) {
        printf("GetProcAddress failed (%d)\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread that calls LoadLibraryW with the DLL path
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteBuf, 0, NULL);
    if (hThread == NULL) {
        printf("CreateRemoteThread failed (%d)\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    printf("DLL injected successfully!\n");
}