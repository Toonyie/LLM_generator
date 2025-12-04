#include <windows.h>
#include <limits.h>

int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = (HANDLE)file;
    LARGE_INTEGER fileSize;
    DWORD bytesRead = 0;
    BOOL success;

    if (hFile == INVALID_HANDLE_VALUE || hFile == NULL || buffer == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (!GetFileSizeEx(hFile, &fileSize)) {
        return -1;
    }

    // Check if the file size exceeds the maximum value representable by 'int'.
    // If it does, the return type 'int' cannot accurately represent the size.
    // In such cases, this function returns -1 to indicate a size limitation error,
    // as per the constraints of the 'int' return type.
    if (fileSize.QuadPart > INT_MAX) {
        SetLastError(ERROR_FILE_TOO_LARGE); // Indicate file size exceeds int capacity
        return -1;
    }

    // The number of bytes to read should now fit within a DWORD, and also within INT_MAX.
    DWORD bytesToRead = (DWORD)fileSize.QuadPart;

    success = ReadFile(
        hFile,           // handle to file
        buffer,          // data buffer
        bytesToRead,     // number of bytes to read
        &bytesRead,      // number of bytes read
        NULL             // no overlapped structure
    );

    if (!success) {
        return -1;
    }

    // This check is mostly defensive; it should not trigger if fileSize.QuadPart <= INT_MAX check passed.
    if (bytesRead > INT_MAX) {
        SetLastError(ERROR_INTERNAL_ERROR); // Unexpected state
        return -1;
    }

    return (int)bytesRead;
}