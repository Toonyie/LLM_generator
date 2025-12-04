#include <windows.h> // Required for Windows API functions and types

// Function prototype: int Load_From_File(void *file, void *buffer);
// This function loads the content of a file into a buffer.
// Parameters:
//   file: A void pointer representing the file handle (e.g., obtained from CreateFile).
//   buffer: A void pointer to the buffer where the file content will be stored.
//           The caller is responsible for ensuring the buffer is large enough.
// Returns:
//   The number of bytes successfully loaded into the buffer if successful.
//   -1 if an error occurred (e.g., invalid handle, file too large, read error).
//   0 if the file is empty.
int Load_From_File(void *file, void *buffer) {
    HANDLE hFile = (HANDLE)file; // Cast the void* to a Windows HANDLE
    LARGE_INTEGER fileSize;      // Used to store the 64-bit file size
    DWORD bytesRead = 0;         // Stores the number of bytes actually read by ReadFile

    // Validate the file handle. INVALID_HANDLE_VALUE is the standard error indicator
    // for Windows handles returned by functions like CreateFile.
    if (hFile == INVALID_HANDLE_VALUE) {
        // Optionally, you could set a specific Windows error code here:
        // SetLastError(ERROR_INVALID_HANDLE);
        return -1; // Indicate an invalid handle error
    }

    // Get the file size. GetFileSizeEx supports files larger than 4GB.
    if (!GetFileSizeEx(hFile, &fileSize)) {
        // If GetFileSizeEx fails, return an error. GetLastError() can provide details.
        return -1;
    }

    // The function is specified to return 'int'.
    // A standard 'int' is typically a signed 32-bit integer,
    // with a maximum positive value of 2,147,483,647 bytes (approx 2 GB).
    // If the file size exceeds this, we cannot correctly represent it with 'int'.
    // ReadFile's third parameter (nNumberOfBytesToRead) is also a DWORD (unsigned 32-bit).
    // If fileSize.QuadPart exceeds INT_MAX, we must signal an error or a limitation.
    if (fileSize.QuadPart > (LONGLONG)0x7FFFFFFF) { // 0x7FFFFFFF is INT_MAX
        // The file is too large to fit its full size into a signed int.
        // Returning -1 indicates this limitation or error.
        SetLastError(ERROR_FILE_TOO_LARGE); // Set a relevant Windows error code
        return -1;
    }

    // If the file is empty (size is 0), return 0 immediately.
    if (fileSize.QuadPart == 0) {
        return 0;
    }

    // Cast the file size to DWORD for the nNumberOfBytesToRead parameter of ReadFile.
    // This cast is safe because we already checked that fileSize.QuadPart fits
    // within INT_MAX, which is also within the range of DWORD.
    DWORD bytesToRead = (DWORD)fileSize.QuadPart;

    // Read the entire file content into the provided buffer.
    // 'buffer' is passed directly as LPVOID (void*).
    // '&bytesRead' receives the actual number of bytes read.
    // The last parameter (lpOverlapped) is NULL for synchronous I/O.
    if (!ReadFile(hFile, buffer, bytesToRead, &bytesRead, NULL)) {
        // If ReadFile fails, return an error. GetLastError() can provide details.
        return -1;
    }

    // Return the number of bytes actually read.
    // If ReadFile succeeded and was not reading asynchronously or from a special device,
    // bytesRead should typically equal bytesToRead (the file's content size).
    return (int)bytesRead;
}