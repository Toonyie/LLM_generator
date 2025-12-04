#include <windows.h>

// Define NTAPI types manually as requested
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

#ifndef BOOLEAN
typedef UCHAR BOOLEAN;
#endif

// STATUS_SUCCESS (0x00000000) for NTSTATUS
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

// Manually define UNICODE_STRING as specified in the prompt
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// Manually define OBJECT_ATTRIBUTES (often used with UNICODE_STRING in NTAPI)
typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

// FILE_INFORMATION_CLASS enum, specifically for FileDispositionInformation
typedef enum _FILE_INFORMATION_CLASS {
    // Omitting other enum values not used for brevity
    FileDispositionInformation = 13,
    // Omitting other enum values not used for brevity
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

// FILE_DISPOSITION_INFORMATION struct for marking a file for deletion
typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

// IO_STATUS_BLOCK struct for NTAPI calls
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    } DUMMYUNIONNAME;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// Function pointer typedef for NtSetInformationFile, dynamically loaded from ntdll.dll
typedef NTSTATUS (NTAPI *PFN_NtSetInformationFile)(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
);


void Delete_Itself() {
    WCHAR szPath[MAX_PATH];
    DWORD dwLen = GetModuleFileNameW(NULL, szPath, MAX_PATH);

    // If GetModuleFileNameW fails or the path is truncated, return.
    if (dwLen == 0 || dwLen >= MAX_PATH) {
        return;
    }

    // Load ntdll.dll to get the address of NtSetInformationFile
    HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
    if (hNtdll == NULL) {
        return;
    }

    // Get the address of NtSetInformationFile
    PFN_NtSetInformationFile pNtSetInformationFile = (PFN_NtSetInformationFile)GetProcAddress(hNtdll, "NtSetInformationFile");
    if (pNtSetInformationFile == NULL) {
        FreeLibrary(hNtdll);
        return;
    }

    // Open a handle to the current executable file with DELETE access
    // FILE_SHARE_DELETE is crucial to allow marking for deletion while the file is still in use
    HANDLE hFile = CreateFileW(
        szPath,
        DELETE,                                 // Request DELETE access
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // Allow other processes/handles
        NULL,                                   // Default security attributes
        OPEN_EXISTING,                          // File must already exist
        FILE_ATTRIBUTE_NORMAL,                  // Normal file attributes
        NULL                                    // No template file
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        FreeLibrary(hNtdll);
        return;
    }

    // Initialize FILE_DISPOSITION_INFORMATION to mark the file for deletion
    FILE_DISPOSITION_INFORMATION fdi = { .DeleteFile = TRUE };
    IO_STATUS_BLOCK ioStatusBlock;
    
    // Call NtSetInformationFile to mark the file for deletion upon closing of the last handle.
    // In this case, the last handle will be closed when the current process terminates.
    pNtSetInformationFile(
        hFile,
        &ioStatusBlock,
        &fdi,
        sizeof(fdi),
        FileDispositionInformation
    );

    // Close the file handle. The file is now marked for deletion by the OS.
    CloseHandle(hFile);
    FreeLibrary(hNtdll);

    // Terminate the current process. This releases the OS-internal handle to the executable file,
    // triggering its deletion by the operating system.
    ExitProcess(0);
}