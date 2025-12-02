#include <windows.h> // Required for Windows API functions like CreateFileA, ReadFile, CloseHandle, etc.

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * This function opens the specified file, reads a maximum of 'maxLen' bytes
 * into the provided 'buffer', and then closes the file. It uses synchronous I/O.
 *
 * @param path The null-terminated string path to the file to be read.
 *             Must not be NULL.
 * @param buffer A pointer to the buffer where the file content will be stored.
 *               Must not be NULL.
 * @param maxLen The maximum number of bytes to read from the file.
 *               If the file is smaller than maxLen, the entire file content
 *               up to the end of the file will be read.
 *               If maxLen is 0, the function will immediately return 0.
 *               Must not be negative.
 * @return The number of bytes successfully read, or -1 on error.
 *         Errors include: invalid path, file not found, permission issues,
 *         invalid buffer, negative maxLen, or I/O errors during read.
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile;           // Handle to the file
    DWORD bytesRead = 0;    // To store the number of bytes actually read

    // 1. Input validation
    if (path == NULL || buffer == NULL || maxLen < 0) {
        // Invalid input parameters
        // Example error code for internal use: SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    // If maxLen is 0, there's nothing to read, so return 0 bytes read.
    if (maxLen == 0) {
        return 0;
    }

    // 2. Open the file
    // CreateFileA is used for ANSI strings (char*). Use CreateFileW for Unicode (wchar_t*).
    hFile = CreateFileA(
        path,                  // Path to the file
        GENERIC_READ,          // Desired access: Read-only
        FILE_SHARE_READ,       // Share mode: Allow other processes to read the file
        NULL,                  // Security attributes: Default
        OPEN_EXISTING,         // Creation disposition: File must exist
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes: Normal file
        NULL                   // Template file: None
    );

    // Check if CreateFileA failed
    if (hFile == INVALID_HANDLE_VALUE) {
        // Could be file not found (ERROR_FILE_NOT_FOUND), access denied, invalid path, etc.
        // GetLastError() could provide more specific error details.
        return -1; // Indicate error
    }

    // 3. Read from the file into the buffer
    // ReadFile expects a DWORD for nNumberOfBytesToRead. We cast maxLen.
    // &bytesRead will receive the actual number of bytes read.
    // NULL for lpOverlapped indicates synchronous I/O.
    if (!ReadFile(
        hFile,                 // Handle to the file
        buffer,                // Pointer to the buffer to receive data
        (DWORD)maxLen,         // Maximum number of bytes to read
        &bytesRead,            // Pointer to a DWORD to receive the number of bytes read
        NULL                   // Not using overlapped I/O (synchronous read)
    )) {
        // ReadFile failed (e.g., I/O error during read operation).
        // GetLastError() could provide more specific error details.
        CloseHandle(hFile);    // Crucial: Close the file handle before returning
        return -1;             // Indicate error
    }

    // 4. Close the file handle
    CloseHandle(hFile);

    // 5. Return the number of bytes successfully read
    return (int)bytesRead;
}

/*
// Example Usage (for testing purposes, compile with a main function):

#include <stdio.h> // For printf

int main() {
    const char *filePath = "testfile.txt";
    char buffer[1024]; // A buffer to hold up to 1024 bytes
    int bytesRead;

    // Create a dummy file for testing
    HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error creating test file: %lu\n", GetLastError());
        return 1;
    }
    const char *testContent = "Hello, Windows API file reading!\nThis is a test file content.";
    DWORD bytesWritten;
    WriteFile(hFile, testContent, (DWORD)strlen(testContent), &bytesWritten, NULL);
    CloseHandle(hFile);
    printf("Created test file '%s' with %lu bytes.\n", filePath, bytesWritten);

    // Test Case 1: Read less than file size
    bytesRead = ReadFileToBuffer(filePath, buffer, 10);
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate for printing
        printf("Read %d bytes: '%s'\n", bytesRead, buffer); // Expected: "Hello, Win"
    } else {
        fprintf(stderr, "Error reading file (case 1): %lu\n", GetLastError());
    }

    // Test Case 2: Read more than file size
    bytesRead = ReadFileToBuffer(filePath, buffer, sizeof(buffer));
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate for printing
        printf("Read %d bytes: '%s'\n", bytesRead, buffer); // Expected: entire content
    } else {
        fprintf(stderr, "Error reading file (case 2): %lu\n", GetLastError());
    }

    // Test Case 3: Read exactly file size (or close to it)
    int contentLen = (int)strlen(testContent);
    bytesRead = ReadFileToBuffer(filePath, buffer, contentLen);
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate for printing
        printf("Read %d bytes: '%s'\n", bytesRead, buffer); // Expected: entire content
    } else {
        fprintf(stderr, "Error reading file (case 3): %lu\n", GetLastError());
    }


    // Test Case 4: File does not exist
    bytesRead = ReadFileToBuffer("nonexistent.txt", buffer, sizeof(buffer));
    if (bytesRead == -1) {
        printf("Correctly failed to read nonexistent file.\n");
    } else {
        fprintf(stderr, "Unexpected success for nonexistent file. Read %d bytes.\n", bytesRead);
    }

    // Test Case 5: Null buffer
    bytesRead = ReadFileToBuffer(filePath, NULL, sizeof(buffer));
    if (bytesRead == -1) {
        printf("Correctly failed with NULL buffer.\n");
    } else {
        fprintf(stderr, "Unexpected success with NULL buffer. Read %d bytes.\n", bytesRead);
    }

    // Test Case 6: Negative maxLen
    bytesRead = ReadFileToBuffer(filePath, buffer, -10);
    if (bytesRead == -1) {
        printf("Correctly failed with negative maxLen.\n");
    } else {
        fprintf(stderr, "Unexpected success with negative maxLen. Read %d bytes.\n", bytesRead);
    }

    // Test Case 7: maxLen = 0
    bytesRead = ReadFileToBuffer(filePath, buffer, 0);
    if (bytesRead == 0) {
        printf("Correctly read 0 bytes with maxLen = 0.\n");
    } else {
        fprintf(stderr, "Unexpected result for maxLen = 0. Read %d bytes.\n", bytesRead);
    }
    
    // Clean up dummy file
    if (!DeleteFileA(filePath)) {
        fprintf(stderr, "Error deleting test file: %lu\n", GetLastError());
    } else {
        printf("Cleaned up test file '%s'.\n", filePath);
    }

    return 0;
}
*/