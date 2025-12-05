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