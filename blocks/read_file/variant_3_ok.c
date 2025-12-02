#include <windows.h> // Required for Windows API functions
#include <stdio.h>   // For NULL, might be useful for testing with printf

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * This function opens the specified file, reads data from it into the provided
 * buffer, and then closes the file. It reads at most maxLen bytes.
 *
 * @param path The path to the file to read. Must be a null-terminated string.
 * @param buffer A pointer to the buffer where the file data will be stored.
 *               The buffer must be large enough to hold at least maxLen bytes.
 * @param maxLen The maximum number of bytes to read from the file.
 *               If maxLen is 0, the function returns 0 bytes read without
 *               attempting to open the file.
 *               If maxLen is negative, the function returns -1 (error).
 *
 * @return The number of bytes successfully read into the buffer, or -1 if an
 *         error occurred (e.g., file not found, permission denied, invalid
 *         parameters, read error).
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;
    BOOL bResult = FALSE;

    // --- Input Validation ---
    if (path == NULL || buffer == NULL) {
        // Invalid arguments provided
        // You might want to set GetLastError() here for more specific error info,
        // but for this function, -1 suffices.
        // SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (maxLen < 0) {
        // Cannot read a negative number of bytes
        // SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (maxLen == 0) {
        // No bytes to read, return 0 immediately.
        return 0;
    }

    // --- Open the file ---
    // CreateFileA is used for ANSI strings (char* path)
    hFile = CreateFileA(
        path,                  // File path
        GENERIC_READ,          // Desired access: read only
        FILE_SHARE_READ,       // Share mode: allow other processes to read
        NULL,                  // Security attributes: default
        OPEN_EXISTING,         // Creation disposition: file must exist
        FILE_ATTRIBUTE_NORMAL, // Attributes: normal file
        NULL                   // Template file: none
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open file (e.g., file not found, access denied)
        // GetLastError() can provide specific error details
        return -1;
    }

    // --- Read from the file ---
    bResult = ReadFile(
        hFile,           // Handle to the file
        buffer,          // Pointer to the buffer
        (DWORD)maxLen,   // Number of bytes to read (cast to DWORD)
        &bytesRead,      // Pointer to a DWORD to receive the number of bytes read
        NULL             // OVERLAPPED structure (NULL for synchronous read)
    );

    // --- Close the file handle ---
    // It's crucial to close the handle regardless of whether ReadFile succeeded or failed.
    CloseHandle(hFile);

    if (!bResult) {
        // ReadFile failed (e.g., I/O error during read)
        // GetLastError() can provide specific error details
        return -1;
    }

    // --- Return bytes read ---
    return (int)bytesRead;
}

/*
// --- Example Usage (for testing purposes) ---
#include <string.h> // For memset

int main() {
    const char *testFilePath = "test_file.txt";
    const char *largeTestFilePath = "large_test_file.bin";
    char buffer[256];
    int bytesRead;

    // 1. Create a dummy test file
    HANDLE hFile = CreateFileA(testFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char *content = "Hello, Windows API File Read!";
        DWORD bytesWritten;
        WriteFile(hFile, content, (DWORD)strlen(content), &bytesWritten, NULL);
        CloseHandle(hFile);
        printf("Created '%s' with %d bytes.\n", testFilePath, (int)bytesWritten);
    } else {
        fprintf(stderr, "Error creating test file: %lu\n", GetLastError());
        return 1;
    }

    // 2. Test reading a part of the file
    printf("\n--- Test 1: Reading part of the file ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, buffer, 10);
    if (bytesRead != -1) {
        printf("Read %d bytes: '%.*s'\n", bytesRead, bytesRead, buffer);
    } else {
        fprintf(stderr, "Error reading file: %lu\n", GetLastError());
    }

    // 3. Test reading the whole file
    printf("\n--- Test 2: Reading the whole file ---\n");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, buffer, sizeof(buffer) - 1); // -1 to leave space for null terminator if needed
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate for printing
        printf("Read %d bytes: '%s'\n", bytesRead, buffer);
    } else {
        fprintf(stderr, "Error reading file: %lu\n", GetLastError());
    }

    // 4. Test with a buffer smaller than content
    printf("\n--- Test 3: Reading with small buffer ---\n");
    char smallBuffer[5];
    memset(smallBuffer, 0, sizeof(smallBuffer));
    bytesRead = ReadFileToBuffer(testFilePath, smallBuffer, sizeof(smallBuffer) -1);
    if (bytesRead != -1) {
        smallBuffer[bytesRead] = '\0';
        printf("Read %d bytes: '%s'\n", bytesRead, smallBuffer);
    } else {
        fprintf(stderr, "Error reading file: %lu\n", GetLastError());
    }

    // 5. Test with a non-existent file
    printf("\n--- Test 4: Reading non-existent file ---\n");
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer("non_existent_file.txt", buffer, sizeof(buffer));
    if (bytesRead == -1) {
        fprintf(stderr, "Correctly failed to read non-existent file. GetLastError: %lu\n", GetLastError());
    } else {
        printf("Unexpectedly read %d bytes from non-existent file.\n", bytesRead);
    }

    // 6. Test with maxLen = 0
    printf("\n--- Test 5: Reading with maxLen = 0 ---\n");
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(testFilePath, buffer, 0);
    if (bytesRead == 0) {
        printf("Correctly read 0 bytes when maxLen is 0.\n");
    } else {
        fprintf(stderr, "Unexpectedly read %d bytes when maxLen is 0.\n", bytesRead);
    }

    // 7. Test with maxLen < 0
    printf("\n--- Test 6: Reading with maxLen < 0 ---\n");
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(testFilePath, buffer, -5);
    if (bytesRead == -1) {
        printf("Correctly failed when maxLen is negative.\n");
    } else {
        fprintf(stderr, "Unexpectedly read %d bytes when maxLen is negative.\n", bytesRead);
    }

    // 8. Test with NULL path
    printf("\n--- Test 7: Reading with NULL path ---\n");
    memset(buffer, 0, sizeof(buffer));
    bytesRead = ReadFileToBuffer(NULL, buffer, 10);
    if (bytesRead == -1) {
        printf("Correctly failed when path is NULL.\n");
    } else {
        fprintf(stderr, "Unexpectedly read %d bytes when path is NULL.\n", bytesRead);
    }

    // 9. Test with NULL buffer
    printf("\n--- Test 8: Reading with NULL buffer ---\n");
    bytesRead = ReadFileToBuffer(testFilePath, NULL, 10);
    if (bytesRead == -1) {
        printf("Correctly failed when buffer is NULL.\n");
    } else {
        fprintf(stderr, "Unexpectedly read %d bytes when buffer is NULL.\n", bytesRead);
    }


    // 10. Test reading a larger file than buffer, ensure it reads only up to maxLen
    // Create a larger dummy file
    hFile = CreateFileA(largeTestFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char largeContent[500];
        for(int i = 0; i < sizeof(largeContent); ++i) {
            largeContent[i] = 'A' + (i % 26);
        }
        DWORD bytesWritten;
        WriteFile(hFile, largeContent, sizeof(largeContent), &bytesWritten, NULL);
        CloseHandle(hFile);
        printf("\nCreated '%s' with %d bytes.\n", largeTestFilePath, (int)bytesWritten);
    } else {
        fprintf(stderr, "Error creating large test file: %lu\n", GetLastError());
        return 1;
    }

    printf("\n--- Test 9: Reading large file with small buffer ---\n");
    char bigBuffer[100];
    memset(bigBuffer, 0, sizeof(bigBuffer));
    bytesRead = ReadFileToBuffer(largeTestFilePath, bigBuffer, sizeof(bigBuffer) - 1);
    if (bytesRead != -1) {
        bigBuffer[bytesRead] = '\0';
        printf("Read %d bytes: '%.*s'\n", bytesRead, bytesRead, bigBuffer);
    } else {
        fprintf(stderr, "Error reading large file: %lu\n", GetLastError());
    }


    // Clean up test files
    DeleteFileA(testFilePath);
    DeleteFileA(largeTestFilePath);
    printf("\nCleaned up test files.\n");

    return 0;
}
*/