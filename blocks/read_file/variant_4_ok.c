#include <windows.h> // Required for Windows API functions

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * This function opens the specified file, reads up to `maxLen` bytes
 * from its beginning, and stores them in the provided `buffer`.
 * The file is then closed.
 *
 * @param path The null-terminated string specifying the path to the file.
 * @param buffer A pointer to the buffer where the data will be read into.
 *               The caller is responsible for ensuring this buffer is large
 *               enough to hold `maxLen` bytes.
 * @param maxLen The maximum number of bytes to read from the file. If `maxLen`
 *               is negative, 0 bytes will be read and an error will be returned.
 * @return The number of bytes actually read on success, or -1 on error.
 *         Errors can occur due to:
 *         - File not found or inaccessible.
 *         - Invalid `maxLen` (e.g., negative).
 *         - Issues during the read operation.
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    int result = -1; // Default to error

    // Validate maxLen input. A negative maxLen is not sensible for reading.
    if (maxLen < 0) {
        // Invalid input, return error immediately.
        SetLastError(ERROR_INVALID_PARAMETER); // Set a relevant Windows error code
        return -1;
    }

    // 1. Open the file for reading.
    hFile = CreateFileA(
        path,                // Path to the file (ANSI string)
        GENERIC_READ,        // Desired access: Read only
        FILE_SHARE_READ,     // Share mode: Allow other processes to read concurrently
        NULL,                // Security attributes: Default security
        OPEN_EXISTING,       // Creation disposition: File must exist
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes: Normal file
        NULL                 // Template file: No template
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open the file. GetLastError() can provide more details.
        return -1;
    }

    // 2. Read from the file into the buffer.
    // ReadFile expects a DWORD for nNumberOfBytesToRead.
    // An 'int' (like maxLen) fits safely into a 'DWORD'.
    if (maxLen > 0) { // Only attempt to read if maxLen is positive
        if (!ReadFile(
            hFile,           // Handle to the file
            buffer,          // Pointer to the buffer
            (DWORD)maxLen,   // Maximum number of bytes to read
            &bytesRead,      // Pointer to a DWORD to store the actual number of bytes read
            NULL             // Overlapped structure: NULL for synchronous I/O
        )) {
            // Failed to read from the file. GetLastError() can provide more details.
            // result is already -1.
        } else {
            // Read successful. Store the number of bytes read.
            result = (int)bytesRead;
        }
    } else { // If maxLen is 0, we intentionally read 0 bytes, which is a success.
        result = 0;
    }
    
    // 3. Close the file handle.
    CloseHandle(hFile);

    return result;
}

// Example Usage (for testing purposes, not part of the function itself):
/*
#include <stdio.h>
#include <string.h>

int main() {
    const char *testFilePath = "test_ReadFileToBuffer.txt";
    char buffer[256];
    int bytesRead;

    // --- Create a dummy test file ---
    HANDLE hFile = CreateFileA(testFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char *testContent = "Hello, this is a test file for ReadFileToBuffer functionality.";
        DWORD bytesWritten;
        if (WriteFile(hFile, testContent, strlen(testContent), &bytesWritten, NULL)) {
            printf("Created '%s' with %lu bytes.\n", testFilePath, bytesWritten);
        } else {
            fprintf(stderr, "Error writing to '%s': %lu\n", testFilePath, GetLastError());
        }
        CloseHandle(hFile);
    } else {
        fprintf(stderr, "Error creating '%s': %lu\n", testFilePath, GetLastError());
        return 1;
    }

    printf("\n--- Test Cases ---\n");

    // Test case 1: Read less than full content
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, buffer, 10);
    if (bytesRead != -1) {
        printf("1. Read %d bytes (maxLen=10): '%.*s'\n", bytesRead, bytesRead, buffer);
    } else {
        fprintf(stderr, "1. Error reading file (maxLen=10). Last Error: %lu\n", GetLastError());
    }

    // Test case 2: Read more than full content (should read full content)
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(testFilePath, buffer, sizeof(buffer));
    if (bytesRead != -1) {
        printf("2. Read %d bytes (maxLen=%zu): '%s'\n", bytesRead, sizeof(buffer), buffer);
    } else {
        fprintf(stderr, "2. Error reading file (maxLen=%zu). Last Error: %lu\n", sizeof(buffer), GetLastError());
    }

    // Test case 3: File not found
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer("non_existent_file.txt", buffer, 50);
    if (bytesRead == -1) {
        printf("3. Correctly failed to read 'non_existent_file.txt'. Last Error: %lu\n", GetLastError());
    } else {
        fprintf(stderr, "3. Unexpectedly succeeded reading 'non_existent_file.txt'.\n");
    }

    // Test case 4: Read 0 bytes
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(testFilePath, buffer, 0);
    if (bytesRead != -1) {
        printf("4. Read %d bytes (maxLen=0): '%.*s'\n", bytesRead, bytesRead, buffer); // Should be 0 bytes
    } else {
        fprintf(stderr, "4. Error reading file (maxLen=0). Last Error: %lu\n", GetLastError());
    }

    // Test case 5: Negative maxLen
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(testFilePath, buffer, -10);
    if (bytesRead == -1) {
        printf("5. Correctly failed with negative maxLen. Last Error: %lu\n", GetLastError());
    } else {
        fprintf(stderr, "5. Unexpectedly succeeded with negative maxLen.\n");
    }

    // --- Cleanup: Delete the test file ---
    printf("\n--- Cleanup ---\n");
    if (DeleteFileA(testFilePath)) {
        printf("Cleaned up '%s'.\n", testFilePath);
    } else {
        fprintf(stderr, "Error deleting '%s': %lu\n", testFilePath, GetLastError());
    }

    return 0;
}
*/