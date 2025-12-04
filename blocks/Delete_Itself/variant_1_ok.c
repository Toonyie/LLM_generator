#include <windows.h>

// NTSTATUS types (simplified for this example)
typedef LONG NTSTATUS;
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

// Boolean type for NTAPI
typedef UCHAR BOOLEAN; 

// Define UNICODE_STRING
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// Define IO_STATUS_BLOCK
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// Define FILE_INFORMATION_CLASS enum (only relevant entry for this task)
typedef enum _FILE_INFORMATION_CLASS {
    // ... other entries omitted for brevity as they are not used ...
    FileDispositionInformation = 13, // This is the one we need
    // ... other entries omitted for brevity ...
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;


// Define FILE_DISPOSITION_INFORMATION
typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

// Define function pointer type for NtSetInformationFile
typedef NTSTATUS (NTAPI *PNtSetInformationFile)(
    HANDLE                 FileHandle,
    PIO_STATUS_BLOCK       IoStatusBlock,
    PVOID                  FileInformation,
    ULONG                  Length,
    FILE_INFORMATION_CLASS FileInformationClass
);

void Delete_Itself() {
    WCHAR szExePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, szExePath, MAX_PATH) == 0) {
        // Failed to get module file name.
        return;
    }

    HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
    if (hNtdll == NULL) {
        // Failed to load ntdll.dll.
        return;
    }

    PNtSetInformationFile pNtSetInformationFile = (PNtSetInformationFile)GetProcAddress(hNtdll, "NtSetInformationFile");
    if (pNtSetInformationFile == NULL) {
        // Failed to get NtSetInformationFile address.
        FreeLibrary(hNtdll);
        return;
    }

    // Attempt to open the executable file with DELETE access and FILE_SHARE_DELETE.
    // This is crucial. If the process or OS loader holds an exclusive lock
    // that prevents FILE_SHARE_DELETE, this CreateFileW call will fail.
    HANDLE hFile = CreateFileW(
        szExePath,
        DELETE,                                 // Desired access: DELETE permission
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // Share mode: Allow others (and self) to delete
        NULL,                                   // Security attributes
        OPEN_EXISTING,                          // Creation disposition: File must exist
        FILE_ATTRIBUTE_NORMAL,                  // Flags and attributes
        NULL                                    // Template file
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open the file handle.
        // This is often due to a sharing violation if the running executable
        // was not originally opened with FILE_SHARE_DELETE by the OS loader.
        FreeLibrary(hNtdll);
        return;
    }

    FILE_DISPOSITION_INFORMATION fdi;
    fdi.DeleteFile = TRUE; // Mark the file for deletion

    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status = pNtSetInformationFile(
        hFile,
        &ioStatusBlock,
        &fdi,
        sizeof(FILE_DISPOSITION_INFORMATION),
        FileDispositionInformation
    );

    CloseHandle(hFile);
    FreeLibrary(hNtdll);

    if (status != STATUS_SUCCESS) {
        // NtSetInformationFile failed.
        // The file was not successfully marked for deletion.
        return;
    }

    // If successful, the file is marked for deletion. It will be actually deleted
    // when the last handle to it is closed (i.e., when the current process exits).
    // The current process continues to run until it terminates.
}