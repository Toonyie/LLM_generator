#include <windows.h> // Required for Windows API functions
#include <stdio.h>   // Not strictly needed for the function, but useful for testing/debugging.

/**
 * @brief Reads up to maxLen bytes from a file into a buffer.
 *
 * @param path The null-terminated string specifying the path to the file.
 * @param buffer A pointer to the buffer where the data will be read into.
 *               The buffer must be large enough to hold maxLen bytes.
 * @param maxLen The maximum number of bytes to read from the file.
 *
 * @return The number of bytes successfully read, or -1 on error.
 *         Returns 0 if the file is empty, maxLen is 0, or if no bytes could be read.
 */
int ReadFileToBuffer(const char *path, void *buffer, int maxLen) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesRead = 0;

    // --- Input validation ---

    // 1. Check for a NULL file path
    if (path == NULL) {
        // Invalid parameter: file path cannot be NULL
        return -1;
    }

    // 2. Check for negative maxLen (invalid for byte count)
    if (maxLen < 0) {
        // Invalid parameter: maxLen cannot be negative
        return -1;
    }

    // 3. Handle the case where maxLen is 0.
    // If 0 bytes are requested, nothing is read, and it's not an error.
    // The buffer does not need to be valid in this case.
    if (maxLen == 0) {
        return 0; // Successfully read 0 bytes
    }

    // 4. If maxLen > 0, the buffer must be valid.
    if (buffer == NULL) {
        // Invalid parameter: buffer cannot be NULL if bytes are to be read
        return -1;
    }

    // --- File Operations ---

    // 1. Open the file
    // Using CreateFileA for char* path (ANSI version)
    hFile = CreateFileA(
        path,                   // File path (ANSI string)
        GENERIC_READ,           // Desired access: read-only
        FILE_SHARE_READ,        // Share mode: allow other processes to read concurrently
        NULL,                   // Security attributes: default
        OPEN_EXISTING,          // Creation disposition: file must already exist
        FILE_ATTRIBUTE_NORMAL,  // Flags and attributes: normal file, no special handling
        NULL                    // Template file: none
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open the file.
        // GetLastError() could be used here to retrieve specific error information (e.g., file not found).
        return -1;
    }

    // 2. Read from the file into the buffer
    // ReadFile expects a DWORD for the number of bytes to read.
    // We cast maxLen (int) to DWORD. This is safe as maxLen is positive and typically
    // within the range of DWORD_MAX (UINT_MAX) and INT_MAX.
    if (!ReadFile(
        hFile,                  // Handle to the file
        buffer,                 // Pointer to the buffer to receive the data
        (DWORD)maxLen,          // Maximum number of bytes to read
        &bytesRead,             // Pointer to a DWORD to receive the actual number of bytes read
        NULL                    // OVERLAPPED structure: Not used for synchronous I/O
    )) {
        // Failed to read from the file.
        // GetLastError() could provide more details about the error.
        CloseHandle(hFile); // Ensure the file handle is closed on error
        return -1;
    }

    // 3. Close the file handle
    // It's crucial to close the handle to release system resources.
    CloseHandle(hFile);

    // --- Return Value ---

    // Return the actual number of bytes read.
    // Cast bytesRead (DWORD) to int. This is safe because:
    // a) bytesRead will not exceed maxLen (which is int).
    // b) bytesRead is always non-negative.
    return (int)bytesRead;
}

// --- Example Usage (main function for testing) ---
#ifdef TEST_READFILETOBUFFER

#include <string.h> // For strlen

// A simple helper function to create a test file
void CreateTestFile(const char *filename, const char *content) {
    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_WRITE,
        0, // No sharing
        NULL,
        CREATE_ALWAYS, // Create a new file, overwrite if it exists
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error creating test file %s: %lu\n", filename, GetLastError());
        return;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, content, (DWORD)strlen(content), &bytesWritten, NULL)) {
        fprintf(stderr, "Error writing to test file %s: %lu\n", filename, GetLastError());
    } else if (bytesWritten != strlen(content)) {
        fprintf(stderr, "Warning: Mismatch in bytes written to %s\n", filename);
    }

    CloseHandle(hFile);
}

int main() {
    char buffer[256]; // A buffer for reading data
    int bytesRead;
    const char *test_file = "test_data.txt";
    const char *empty_file = "empty.txt";
    const char *long_file = "long_data.txt";

    // --- Test Case 1: File does not exist ---
    printf("--- Test Case 1: Non-existent file ---\n");
    bytesRead = ReadFileToBuffer("non_existent_file.txt", buffer, sizeof(buffer));
    if (bytesRead == -1) {
        printf("PASS: ReadFileToBuffer correctly returned -1 for non-existent file.\n");
    } else {
        printf("FAIL: ReadFileToBuffer did not return -1 for non-existent file. Returned: %d\n", bytesRead);
    }
    printf("\n");

    // --- Test Case 2: Read from a standard file ---
    printf("--- Test Case 2: Standard file read ---\n");
    CreateTestFile(test_file, "Hello, Windows API!");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(test_file, buffer, sizeof(buffer));
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate for printing
        printf("PASS: Read %d bytes from '%s'. Content: \"%s\"\n", bytesRead, test_file, buffer);
        if (strcmp(buffer, "Hello, Windows API!") == 0) {
            printf("PASS: Content matches expected.\n");
        } else {
            printf("FAIL: Content mismatch.\n");
        }
    } else {
        printf("FAIL: ReadFileToBuffer returned -1 for '%s'.\n", test_file);
    }
    DeleteFileA(test_file); // Clean up
    printf("\n");

    // --- Test Case 3: Read exactly maxLen bytes (file content is longer) ---
    printf("--- Test Case 3: Read exactly maxLen bytes ---\n");
    CreateTestFile(long_file, "This is a very long string that should be truncated.");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    int read_len = 10;
    bytesRead = ReadFileToBuffer(long_file, buffer, read_len);
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate
        printf("PASS: Read %d bytes from '%s'. Content: \"%s\"\n", bytesRead, long_file, buffer);
        if (bytesRead == read_len && strcmp(buffer, "This is a ") == 0) {
            printf("PASS: Correct number of bytes read and content matches.\n");
        } else {
            printf("FAIL: Mismatch in bytes read or content.\n");
        }
    } else {
        printf("FAIL: ReadFileToBuffer returned -1 for '%s'.\n", long_file);
    }
    DeleteFileA(long_file); // Clean up
    printf("\n");

    // --- Test Case 4: Read less than maxLen (file content is shorter) ---
    printf("--- Test Case 4: Read less than maxLen ---\n");
    CreateTestFile(test_file, "Short.");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    read_len = 100; // Request more than file size
    bytesRead = ReadFileToBuffer(test_file, buffer, read_len);
    if (bytesRead != -1) {
        buffer[bytesRead] = '\0'; // Null-terminate
        printf("PASS: Read %d bytes from '%s'. Content: \"%s\"\n", bytesRead, test_file, buffer);
        if (bytesRead == strlen("Short.") && strcmp(buffer, "Short.") == 0) {
            printf("PASS: Correct number of bytes read and content matches.\n");
        } else {
            printf("FAIL: Mismatch in bytes read or content.\n");
        }
    } else {
        printf("FAIL: ReadFileToBuffer returned -1 for '%s'.\n", test_file);
    }
    DeleteFileA(test_file); // Clean up
    printf("\n");

    // --- Test Case 5: Empty file ---
    printf("--- Test Case 5: Empty file ---\n");
    CreateTestFile(empty_file, "");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(empty_file, buffer, sizeof(buffer));
    if (bytesRead == 0) {
        printf("PASS: ReadFileToBuffer correctly returned 0 for empty file.\n");
    } else if (bytesRead == -1) {
        printf("FAIL: ReadFileToBuffer returned -1 for empty file. Expected 0.\n");
    } else {
        printf("FAIL: ReadFileToBuffer returned %d for empty file. Expected 0.\n", bytesRead);
    }
    DeleteFileA(empty_file); // Clean up
    printf("\n");

    // --- Test Case 6: maxLen is 0 ---
    printf("--- Test Case 6: maxLen is 0 ---\n");
    CreateTestFile(test_file, "Some content");
    memset(buffer, 0, sizeof(buffer)); // Clear buffer
    bytesRead = ReadFileToBuffer(test_file, buffer, 0);
    if (bytesRead == 0) {
        printf("PASS: ReadFileToBuffer correctly returned 0 when maxLen is 0.\n");
    } else if (bytesRead == -1) {
        printf("FAIL: ReadFileToBuffer returned -1 when maxLen is 0. Expected 0.\n");
    } else {
        printf("FAIL: ReadFileToBuffer returned %d when maxLen is 0. Expected 0.\n", bytesRead);
    }
    DeleteFileA(test_file); // Clean up
    printf("\n");

    // --- Test Case 7: Invalid parameters (path=NULL, buffer=NULL, maxLen=-1) ---
    printf("--- Test Case 7: Invalid parameters ---\n");
    bytesRead = ReadFileToBuffer(NULL, buffer, 10);
    if (bytesRead == -1) {
        printf("PASS: ReadFileToBuffer correctly returned -1 for NULL path.\n");
    } else {
        printf("FAIL: ReadFileToBuffer did not return -1 for NULL path. Returned: %d\n", bytesRead);
    }

    bytesRead = ReadFileToBuffer(test_file, NULL, 10); // test_file exists from previous test case or create a dummy.
    CreateTestFile(test_file, "dummy"); // Ensure file exists for this test
    if (bytesRead == -1) {
        printf("PASS: ReadFileToBuffer correctly returned -1 for NULL buffer (with maxLen > 0).\n");
    } else {
        printf("FAIL: ReadFileToBuffer did not return -1 for NULL buffer. Returned: %d\n", bytesRead);
    }
    DeleteFileA(test_file); // Clean up

    bytesRead = ReadFileToBuffer(test_file, buffer, -5);
    if (bytesRead == -1) {
        printf("PASS: ReadFileToBuffer correctly returned -1 for negative maxLen.\n");
    } else {
        printf("FAIL: ReadFileToBuffer did not return -1 for negative maxLen. Returned: %d\n", bytesRead);
    }
    printf("\n");

    return 0;
}

#endif // TEST_READFILETOBUFFER