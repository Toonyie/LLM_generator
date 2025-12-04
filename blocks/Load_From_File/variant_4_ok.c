#include <windows.h>
#include <limits.h> // For INT_MAX

int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LARGE_INTEGER fileSize;
    DWORD bytesRead = 0;
    
    LPCWSTR filePath = (LPCWSTR)file;

    hFile = CreateFileW(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return -1;
    }

    if (fileSize.QuadPart > INT_MAX) {
        CloseHandle(hFile);
        return -1; // File is too large for int return type or DWORD ReadFile argument
    }

    if (buffer == NULL) {
        CloseHandle(hFile);
        return (int)fileSize.QuadPart; // Return file size if buffer is NULL (query mode)
    }
    
    if (!ReadFile(
        hFile,
        buffer,
        (DWORD)fileSize.QuadPart,
        &bytesRead,
        NULL
    )) {
        CloseHandle(hFile);
        return -1;
    }

    if (bytesRead != (DWORD)fileSize.QuadPart) {
        CloseHandle(hFile);
        return -1; // Partial read
    }

    CloseHandle(hFile);
    return (int)bytesRead;
}