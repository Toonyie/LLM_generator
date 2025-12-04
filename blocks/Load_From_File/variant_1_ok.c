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