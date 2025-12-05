#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>

#ifndef NTAPI
#define NTAPI __stdcall
#endif

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void DLL_Injection(char *dll, char *process) {
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, process) == 0) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);

    if (processID == 0) {
        printf("Process not found.\n");
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        printf("OpenProcess failed: %d\n", GetLastError());
        return;
    }

    LPVOID pRemoteString = VirtualAllocEx(hProcess, NULL, strlen(dll) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteString == NULL) {
        printf("VirtualAllocEx failed: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    if (!WriteProcessMemory(hProcess, pRemoteString, dll, strlen(dll) + 1, NULL)) {
        printf("WriteProcessMemory failed: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (pLoadLibrary == NULL) {
        printf("GetProcAddress failed: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteString, 0, NULL);
    if (hThread == NULL) {
        printf("CreateRemoteThread failed: %d\n", GetLastError());
        VirtualFreeEx(hProcess, pRemoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteString, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    if (exitCode == 0) {
        printf("LoadLibrary failed in remote process.\n");
    }
}