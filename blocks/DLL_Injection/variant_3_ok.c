#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#ifndef _UNICODE
typedef wchar_t WCHAR;
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void DLL_Injection(char *dll, char *process) {
    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;
    LPVOID pRemoteBuffer = NULL;
    DWORD processId = 0;
    HMODULE hKernel32 = NULL;
    LPVOID pLoadLibraryW = NULL;

    // Convert process name to wide string for CreateToolhelp32Snapshot
    WCHAR wProcessName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, process, -1, wProcessName, MAX_PATH);

    // Get process ID by name
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot failed: %lu\n", GetLastError());
        return;
    }

    if (!Process32FirstW(hSnapshot, &pe32)) {
        fprintf(stderr, "Process32FirstW failed: %lu\n", GetLastError());
        CloseHandle(hSnapshot);
        return;
    }

    do {
        if (wcscmp(pe32.szExeFile, wProcessName) == 0) {
            processId = pe32.th32ProcessID;
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (processId == 0) {
        fprintf(stderr, "Process not found.\n");
        return;
    }

    // Open the process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        fprintf(stderr, "OpenProcess failed: %lu\n", GetLastError());
        return;
    }

    // Allocate memory in the remote process
    pRemoteBuffer = VirtualAllocEx(hProcess, NULL, strlen(dll) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteBuffer == NULL) {
        fprintf(stderr, "VirtualAllocEx failed: %lu\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(hProcess, pRemoteBuffer, dll, strlen(dll) + 1, NULL)) {
        fprintf(stderr, "WriteProcessMemory failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Get the address of LoadLibraryW
    hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32 == NULL) {
        fprintf(stderr, "GetModuleHandleW failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryW == NULL) {
        fprintf(stderr, "GetProcAddress failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread to call LoadLibraryW with the DLL path
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteBuffer, 0, NULL);
    if (hThread == NULL) {
        fprintf(stderr, "CreateRemoteThread failed: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, pRemoteBuffer, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}