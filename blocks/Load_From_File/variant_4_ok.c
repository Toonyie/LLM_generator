#include <windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = (HANDLE)file;
    DWORD fileSize;
    DWORD bytesRead;

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1; // Indicate an error (invalid file handle)
    }

    fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        return -1; // Indicate an error (failed to get file size)
    }

    if (buffer == NULL) {
        return fileSize; // return required buffer size.
    }

    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        return -1; // Indicate an error (failed to read from file)
    }

    if (bytesRead != fileSize)
    {
        return -1;
    }

    return (int)fileSize; // Return the size of the data read (file size)
}