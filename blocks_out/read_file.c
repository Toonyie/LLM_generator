```c
#include <windows.h> // Required for Windows API functions like CreateFileA, ReadFile, CloseHandle
#include <stdio.h>   // Not strictly required for the function itself, but useful for NULL and potentially debugging.

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * @param path The path to the file to read from (ANSI string).
 * @param buffer A pointer to the buffer where the data will be stored.
 * @param maxLen The maximum number of bytes to read into the buffer.
 * @return The number of bytes successfully read, or -1 if an error occurred.
 *         Returns 0 if the file exists but is empty, or if maxLen is 0.
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    int retVal = -1; // Default return value is -1 (error)

    // 1. Input validation
    if (path == NULL || buffer == NULL || maxLen < 0) {
        // Invalid arguments, cannot proceed.
        // If maxLen is 0, CreateFileA and ReadFile can still succeed, returning 0 bytes read.
        // So, maxLen < 0 is an error. maxLen == 0 is a valid case.
        if (maxLen == 0) {
            return 0; // Request to read 0 bytes is not an error, just reads 0.
        }
        return -1;
    }

    // 2. Open the file
    // CreateFileA is used for ANSI strings (const char* path).
    hFile = CreateFileA(
        path,                   // File to open
        GENERIC_READ,           // Open for reading
        FILE_SHARE_READ,        // Share for reading (allows other processes to read simultaneously)
        NULL,                   // Default security attributes
        OPEN_EXISTING,          // File must exist
        FILE_ATTRIBUTE_NORMAL,  // Normal file attributes
        NULL                    // No template file
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open the file. GetLastError() could provide more details.
        // For this function's contract, we simply return -1.
        goto cleanup; // Jump to cleanup to ensure no resources are left open (though none were opened yet if this fails)
    }

    // 3. Read from the file
    // ReadFile expects a DWORD for nNumberOfBytesToRead. maxLen is int.
    // Casting maxLen to DWORD is safe as long as maxLen is not negative and fits within DWORD_MAX.
    // Since maxLen is int and checked to be >= 0, this cast is fine.
    if (!ReadFile(
        hFile,                  // Handle to the file
        buffer,                 // Pointer to the buffer that receives the data
        (DWORD)maxLen,          // Maximum number of bytes to read
        &bytesRead,             // Pointer to a DWORD to receive the number of bytes read
        NULL                    // No OVERLAPPED structure (perform synchronous I/O)
    )) {
        // Failed to read from the file. GetLastError() could provide more details.
        // For this function's contract, we simply return -1.
        goto cleanup;
    }

    // If we reach here, the read operation was successful.
    // The number of bytes read is stored in 'bytesRead'.
    retVal = (int)bytesRead;

cleanup:
    // 4. Close the file handle if it was successfully opened.
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    return retVal;
}

// Example Usage (for testing the function)
#ifdef _MSC_VER // Only compile main for MSVC or similar compilers
#include <stdlib.h> // For EXIT_SUCCESS, EXIT_FAILURE

int main() {
    const char *testFilePath = "test_file.txt";
    char readBuffer[256]; // A buffer to hold read data
    int bytesRead;

    // --- Test Case 1: File does not exist ---
    printf("--- Test Case 1: File does not exist ---\n");
    bytesRead = ReadFileToBuffer("non_existent_file.txt", readBuffer, sizeof(readBuffer));
    if (bytesRead == -1) {
        printf("Successfully handled non-existent file (returned -1).\n");
    } else {
        printf("Error: Unexpected return for non-existent file: %d\n", bytesRead);
    }
    printf("\n");

    // --- Create a test file ---
    printf("--- Creating test_file.txt ---\n");
    HANDLE hFile = CreateFileA(
        testFilePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS, // Create a new file or overwrite existing
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error creating test file: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }
    const char *testContent = "Hello, Windows API!";
    DWORD bytesWritten;
    if (!WriteFile(hFile, testContent, (DWORD)strlen(testContent), &bytesWritten, NULL)) {
        fprintf(stderr, "Error writing to test file: %lu\n", GetLastError());
        CloseHandle(hFile);
        return EXIT_FAILURE;
    }
    CloseHandle(hFile);
    printf("Created '%s' with content: '%s' (%lu bytes)\n\n", testFilePath, testContent, bytesWritten);

    // --- Test Case 2: Read entire file (buffer larger than content) ---
    printf("--- Test Case 2: Read entire file (buffer larger than content) ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, sizeof(readBuffer));
    if (bytesRead != -1) {
        printf("Read %d bytes: '%.*s'\n", bytesRead, bytesRead, readBuffer);
    } else {
        printf("Error reading file: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test Case 3: Read partial file (buffer smaller than content) ---
    printf("--- Test Case 3: Read partial file (buffer smaller than content) ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, 5); // Read only 5 bytes
    if (bytesRead != -1) {
        printf("Read %d bytes: '%.*s'\n", bytesRead, bytesRead, readBuffer);
    } else {
        printf("Error reading file: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test Case 4: Read 0 bytes ---
    printf("--- Test Case 4: Read 0 bytes ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, 0); // Read 0 bytes
    if (bytesRead == 0) {
        printf("Successfully read 0 bytes.\n");
    } else {
        printf("Error: Unexpected return for reading 0 bytes: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test Case 5: Invalid buffer (NULL) ---
    printf("--- Test Case 5: Invalid buffer (NULL) ---\n");
    bytesRead = ReadFileToBuffer(testFilePath, NULL, sizeof(readBuffer));
    if (bytesRead == -1) {
        printf("Successfully handled NULL buffer (returned -1).\n");
    } else {
        printf("Error: Unexpected return for NULL buffer: %d\n", bytesRead);
    }
    printf("\n");

    // --- Clean up test file ---
    printf("--- Cleaning up test_file.txt ---\n");
    if (DeleteFileA(testFilePath)) {
        printf("Successfully deleted '%s'.\n", testFilePath);
    } else {
        fprintf(stderr, "Error deleting test file: %lu\n", GetLastError());
    }

    return EXIT_SUCCESS;
}
#endif // _MSC_VER
```