#include <windows.h> // Required for Windows API functions and types

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * This function opens the specified file, reads up to `maxLen` bytes into
 * the provided `buffer`, and then closes the file.
 *
 * @param path The null-terminated string path to the file to read.
 *             Expected to be an ANSI (char*) path.
 * @param buffer A pointer to the buffer where the read data will be stored.
 *               The caller is responsible for ensuring this buffer is large
 *               enough to hold `maxLen` bytes.
 * @param maxLen The maximum number of bytes to read from the file.
 *               If the file is smaller than `maxLen`, the function reads
 *               up to the end of the file.
 * @return The number of bytes actually read on success, or -1 on error.
 *         Errors include issues like file not found, invalid path,
 *         read permission problems, or invalid input parameters.
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile = INVALID_HANDLE_VALUE; // File handle
    DWORD bytesRead = 0;                 // Number of bytes actually read by ReadFile

    // Basic parameter validation
    if (path == NULL || buffer == NULL) {
        // One of the required pointers is NULL.
        // GetLastError() could be set to ERROR_INVALID_PARAMETER for more detail,
        // but the function simply returns -1 as per requirements.
        return -1;
    }

    if (maxLen < 0) {
        // maxLen cannot be negative.
        return -1;
    }

    if (maxLen == 0) {
        // If maxLen is 0, nothing to read, so return 0 bytes read.
        return 0;
    }

    // 1. Open the file
    // CreateFileA is used for ANSI (char*) paths. Use CreateFileW for Unicode (wchar_t*).
    hFile = CreateFileA(
        path,                 // Path to the file
        GENERIC_READ,         // Desired access: Read-only
        FILE_SHARE_READ,      // Share mode: Allow other processes to read this file
        NULL,                 // Security attributes: Default
        OPEN_EXISTING,        // Creation disposition: File must exist
        FILE_ATTRIBUTE_NORMAL,// Flags and attributes: Normal file, no special attributes
        NULL                  // Template file: No template
    );

    // Check if the file was opened successfully
    if (hFile == INVALID_HANDLE_VALUE) {
        // File opening failed. GetLastError() can provide specific error codes
        // (e.g., ERROR_FILE_NOT_FOUND, ERROR_ACCESS_DENIED).
        return -1;
    }

    // 2. Read from the file into the buffer
    // ReadFile returns TRUE on success, FALSE on failure.
    // Even if TRUE, bytesRead might be less than maxLen if EOF is reached.
    if (!ReadFile(
            hFile,                // Handle to the file
            buffer,               // Pointer to the buffer to receive the data
            (DWORD)maxLen,        // Number of bytes to read (cast int to DWORD)
            &bytesRead,           // Pointer to a DWORD to store the actual number of bytes read
            NULL                  // Overlapped structure: NULL for synchronous I/O
        )) {
        // Reading failed. GetLastError() can provide specific error codes.
        // Close the file handle before returning.
        CloseHandle(hFile);
        return -1;
    }

    // 3. Close the file handle to release resources
    CloseHandle(hFile);

    // 4. Return the number of bytes successfully read
    // bytesRead is DWORD, but maxLen is int, so it's safe to cast to int.
    return (int)bytesRead;
}

/*
// Example Usage (for testing purposes)
#include <stdio.h>
#include <string.h> // For memset

int main() {
    const char *testFilePath = "test_file.txt";
    char readBuffer[256];
    int bytesRead;

    // --- Test 1: File does not exist ---
    printf("--- Test 1: File does not exist ---\n");
    bytesRead = ReadFileToBuffer("non_existent_file.txt", readBuffer, sizeof(readBuffer));
    if (bytesRead == -1) {
        printf("Successfully handled non-existent file. Error: %lu\n", GetLastError());
    } else {
        printf("Failed test 1. Unexpected bytes read: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test 2: Create a test file ---
    printf("--- Test 2: Create a test file ---\n");
    FILE *fp = fopen(testFilePath, "w");
    if (fp) {
        fputs("Hello, this is a test file for ReadFileToBuffer function.", fp);
        fclose(fp);
        printf("Created '%s'\n", testFilePath);
    } else {
        printf("Failed to create test file.\n");
        return 1;
    }
    printf("\n");

    // --- Test 3: Read entire file (maxLen >= file size) ---
    printf("--- Test 3: Read entire file ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, sizeof(readBuffer));
    if (bytesRead != -1) {
        printf("Bytes read: %d\n", bytesRead);
        printf("Content: '%.*s'\n", bytesRead, readBuffer);
    } else {
        printf("Error reading file: %lu\n", GetLastError());
    }
    printf("\n");

    // --- Test 4: Read partial file (maxLen < file size) ---
    printf("--- Test 4: Read partial file ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    int partialReadLen = 10;
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, partialReadLen);
    if (bytesRead != -1) {
        printf("Bytes read: %d\n", bytesRead);
        printf("Content: '%.*s'\n", bytesRead, readBuffer);
    } else {
        printf("Error reading file: %lu\n", GetLastError());
    }
    printf("\n");

    // --- Test 5: Read with maxLen = 0 ---
    printf("--- Test 5: Read with maxLen = 0 ---\n");
    memset(readBuffer, 0, sizeof(readBuffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, 0);
    if (bytesRead != -1) {
        printf("Bytes read: %d (expected 0)\n", bytesRead);
    } else {
        printf("Error reading with maxLen 0: %lu\n", GetLastError());
    }
    printf("\n");

    // --- Test 6: Invalid parameters (NULL path) ---
    printf("--- Test 6: Invalid parameters (NULL path) ---\n");
    bytesRead = ReadFileToBuffer(NULL, readBuffer, sizeof(readBuffer));
    if (bytesRead == -1) {
        printf("Successfully handled NULL path.\n");
    } else {
        printf("Failed test 6. Unexpected bytes read: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test 7: Invalid parameters (NULL buffer) ---
    printf("--- Test 7: Invalid parameters (NULL buffer) ---\n");
    bytesRead = ReadFileToBuffer(testFilePath, NULL, sizeof(readBuffer));
    if (bytesRead == -1) {
        printf("Successfully handled NULL buffer.\n");
    } else {
        printf("Failed test 7. Unexpected bytes read: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test 8: Invalid parameters (negative maxLen) ---
    printf("--- Test 8: Invalid parameters (negative maxLen) ---\n");
    bytesRead = ReadFileToBuffer(testFilePath, readBuffer, -5);
    if (bytesRead == -1) {
        printf("Successfully handled negative maxLen.\n");
    } else {
        printf("Failed test 8. Unexpected bytes read: %d\n", bytesRead);
    }
    printf("\n");

    // --- Cleanup: Delete the test file ---
    printf("--- Cleanup ---\n");
    if (DeleteFileA(testFilePath)) {
        printf("Deleted '%s'\n", testFilePath);
    } else {
        printf("Failed to delete test file. Error: %lu\n", GetLastError());
    }

    return 0;
}
*/