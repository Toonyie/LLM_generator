#include <windows.h> // For HANDLE, GetFileSizeEx, ReadFile, etc.
#include <limits.h>  // For INT_MAX

// Function prototype: int Load_From_File(void *file, void *buffer);
//
// This function loads the entire content of a file into a provided buffer.
// It assumes the 'file' parameter is an open Windows HANDLE to a file.
// It assumes the 'buffer' parameter is a pointer to a pre-allocated memory
// block that is large enough to hold the entire file content.
//
// Parameters:
//   file:   A void pointer expected to be an open HANDLE to the file.
//   buffer: A void pointer to the destination memory buffer where the file
//           content will be written. The caller is responsible for ensuring
//           this buffer is sufficiently large.
//
// Returns:
//   The number of bytes successfully loaded into the buffer (file size).
//   -1 if a general error occurs (e.g., invalid handle, API call failure).
//   -2 if the file size exceeds INT_MAX, meaning the full size cannot be
//      represented by the 'int' return type (typically 2GB on 32/64-bit systems).

int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = (HANDLE)file;
    LPVOID lpBuffer = (LPVOID)buffer;
    LARGE_INTEGER fileSize;
    DWORD bytesRead = 0; // Actual bytes read by ReadFile
    BOOL bResult;

    // 1. Validate the file handle
    if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
        SetLastError(ERROR_INVALID_HANDLE); // Set an appropriate error code
        return -1; // Error: Invalid file handle
    }

    // 2. Get the file size
    bResult = GetFileSizeEx(hFile, &fileSize);
    if (!bResult) {
        // GetFileSizeEx failed. GetLastError() can provide more details.
        return -1; // Error: Could not get file size
    }

    // 3. Check if file size exceeds what can be returned by 'int'
    // The 'int' return type typically limits the maximum size to INT_MAX (around 2GB).
    // If the file is larger, we cannot return its full size correctly.
    // Also, ReadFile's nNumberOfBytesToRead parameter is a DWORD, which means it
    // can handle up to 4GB in a single call. If fileSize.QuadPart exceeds INT_MAX
    // but is within DWORD_MAX, it can be read but not fully reported by 'int'.
    if (fileSize.QuadPart > INT_MAX) {
        SetLastError(ERROR_FILE_TOO_LARGE); // Set an appropriate error code
        return -2; // Error: File too large for function's return type/design
    }
    
    // At this point, fileSize.QuadPart is guaranteed to fit within INT_MAX and
    // therefore also within DWORD, allowing safe casting for ReadFile.
    DWORD nBytesToRead = (DWORD)fileSize.QuadPart;

    // 4. Read the file content into the buffer
    // IMPORTANT: This function relies entirely on the caller providing a buffer
    // that is large enough to hold 'nBytesToRead' bytes. No internal buffer
    // overflow check is performed here.
    bResult = ReadFile(
        hFile,
        lpBuffer,
        nBytesToRead, // Number of bytes to read
        &bytesRead,   // Pointer to store the actual number of bytes read
        NULL          // No OVERLAPPED structure for synchronous I/O
    );

    if (!bResult) {
        // ReadFile failed. GetLastError() can provide more details.
        return -1; // Error: Could not read file
    }

    // 5. Verify that all requested bytes were actually read.
    // In synchronous file reading, if ReadFile succeeds, bytesRead should
    // typically equal nBytesToRead, unless end-of-file was reached prematurely
    // or other unusual circumstances.
    if (bytesRead != nBytesToRead) {
        // This might indicate an incomplete read or a logical error.
        // For this function's purpose, we return the actual number of bytes read.
        SetLastError(ERROR_PARTIAL_COPY); // Indicate a partial read might have occurred
    }

    // Return the number of bytes successfully read.
    // Since we've already checked fileSize.QuadPart against INT_MAX,
    // bytesRead (which is <= fileSize.QuadPart) will also fit into an 'int'.
    return (int)bytesRead;
}