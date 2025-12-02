#include <stdio.h>
#include <windows.h>

#include <windows.h> // Common Windows API header, often included in Windows projects.
                     // Not strictly necessary for this specific function, but good practice.
#include <intrin.h>  // Required for the __cpuid intrinsic function on MSVC.
#include <string.h>  // Required for memcpy.

/**
 * @brief Retrieves the CPU vendor string (e.g., "GenuineIntel", "AuthenticAMD").
 *
 * This function uses the CPUID instruction (via the __cpuid intrinsic) to get
 * the processor's vendor identification string. This instruction is specific
 * to x86/x64 architectures.
 *
 * @param out Pointer to a character buffer where the vendor string will be stored.
 * @param outLen The size of the output buffer 'out', including space for the null terminator.
 *               Must be at least 13 to store the full 12-character vendor string + null terminator.
 * @return 0 on success.
 * @return -1 on failure (e.g., out is NULL, outLen is too small to fit the full string, or invalid outLen).
 */
int GetCPUVendor(char *out, int outLen) {
    // Validate input parameters
    if (out == NULL || outLen <= 0) {
        return -1; // Invalid output buffer or length.
    }

    // The CPU vendor string is always 12 characters long (e.g., "GenuineIntel").
    // We need 12 characters + 1 for the null terminator.
    const int VENDOR_STRING_LENGTH = 12; 
    const int REQUIRED_BUFFER_SIZE = VENDOR_STRING_LENGTH + 1;

    // Check if the provided buffer is large enough to hold the full vendor string
    // including the null terminator.
    // If it's too small, we will copy what fits, but return -1 to signal partial data.
    int return_code = 0;
    if (outLen < REQUIRED_BUFFER_SIZE) {
        return_code = -1; 
    }

    // cpuInfo[0] = EAX, cpuInfo[1] = EBX, cpuInfo[2] = ECX, cpuInfo[3] = EDX
    int cpuInfo[4];

    // Call CPUID with EAX = 0. This function returns:
    //   - The maximum supported CPUID leaf in EAX (cpuInfo[0]).
    //   - The Vendor ID string in EBX, EDX, ECX registers.
    // The correct order for assembling the string is EBX (first 4 chars),
    // then EDX (next 4 chars), then ECX (last 4 chars).
    __cpuid(cpuInfo, 0);

    // Create a temporary buffer to assemble the 12-character vendor string.
    char vendorString[REQUIRED_BUFFER_SIZE]; 

    // Copy the parts of the vendor string from the cpuInfo array into the temporary buffer.
    // cpuInfo[1] holds EBX (first 4 characters)
    memcpy(&vendorString[0], &cpuInfo[1], 4);
    // cpuInfo[3] holds EDX (next 4 characters)
    memcpy(&vendorString[4], &cpuInfo[3], 4);
    // cpuInfo[2] holds ECX (last 4 characters)
    memcpy(&vendorString[8], &cpuInfo[2], 4);
    
    // Null-terminate the temporary vendor string.
    vendorString[VENDOR_STRING_LENGTH] = '\0'; 

    // Determine how many characters to copy to the user's output buffer.
    // We copy at most (outLen - 1) characters to ensure there's space for the null terminator.
    // We also copy at most VENDOR_STRING_LENGTH (12) characters, as that's the full string length.
    int charsToCopy = (outLen - 1 < VENDOR_STRING_LENGTH) ? (outLen - 1) : VENDOR_STRING_LENGTH;
    
    // Copy the (possibly truncated) vendor string to the output buffer.
    if (charsToCopy > 0) {
        memcpy(out, vendorString, charsToCopy);
    }
    
    // Always null-terminate the output buffer at the correct position.
    out[charsToCopy] = '\0';

    return return_code; // Return 0 on success, -1 if buffer was too small.
}

#include <windows.h> // Required for general Windows API functions and data types
#include <tlhelp32.h>  // Required for CreateToolhelp32Snapshot, Process32First, Process32Next
#include <stdio.h>     // Required for printf

/**
 * @brief Lists all running processes and prints their executable names to the console.
 *
 * This function uses the Windows Tool Help Library (tlhelp32.h) to take a snapshot
 * of all running processes and then iterates through them, printing the executable
 * file name for each.
 */
void ListProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot was created successfully
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not create process snapshot. GetLastError: %lu\n", GetLastError());
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32); // Must set dwSize before calling Process32First/Next

    // Get the first process in the snapshot
    if (!Process32First(hSnapshot, &pe32)) {
        fprintf(stderr, "Error: Could not retrieve first process. GetLastError: %lu\n", GetLastError());
        CloseHandle(hSnapshot); // Always close the handle
        return;
    }

    printf("--- Currently Running Processes ---\n");
    // Loop through all processes
    do {
        // Print the executable file name
        // pe32.szExeFile contains the name (e.g., "notepad.exe")
        printf("%s\n", pe32.szExeFile);
    } while (Process32Next(hSnapshot, &pe32)); // Move to the next process

    printf("-----------------------------------\n");

    // Close the snapshot handle to release system resources
    CloseHandle(hSnapshot);
}

/*
// Example of how to use the function (optional main for testing)
int main() {
    ListProcesses();
    
    printf("\nPress Enter to exit...");
    getchar(); // Wait for user input before closing console
    
    return 0;
}
*/

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

#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinaryA

// Link with Crypt32.lib
#pragma comment(lib, "Crypt32.lib")

/**
 * @brief Decodes a Base64 string into a binary buffer.
 *
 * @param src The null-terminated Base64 source string.
 * @param srcLen The length of the source string in bytes.
 * @param dst The destination buffer to store the decoded binary data.
 * @param dstLen The maximum capacity of the destination buffer in bytes.
 *
 * @return The number of decoded bytes written to 'dst' on success,
 *         or -1 on error (e.g., invalid input, insufficient buffer).
 */
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    // 1. Validate input parameters
    if (src == NULL || srcLen < 0) {
        // Invalid source string or length
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }
    if (dst == NULL || dstLen < 0) {
        // Invalid destination buffer or length
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    DWORD cbBinary = 0; // Will hold the required size for the decoded data

    // 2. First call to CryptStringToBinaryA to determine the required buffer size.
    //    We pass NULL for the output buffer (pbBinary) and 0 for its size,
    //    and the function will return the necessary size in cbBinary.
    if (!CryptStringToBinaryA(
            src,                // Pointer to the string to be converted
            (DWORD)srcLen,      // Length of the string to be converted
            CRYPT_STRING_BASE64, // Flag for Base64 decoding
            NULL,               // Output buffer (set to NULL to get size)
            &cbBinary,          // Pointer to a DWORD that receives the required buffer size
            NULL,               // Not used for this flag
            NULL                // Not used for this flag
        )) {
        // Decoding failed, likely due to an invalid Base64 string format.
        // GetLastError() will provide specific error codes (e.g., CRYPT_E_BAD_DECODE).
        return -1;
    }

    // 3. Check if the provided destination buffer is large enough
    if ((int)cbBinary > dstLen) {
        // The destination buffer is too small to hold the decoded data.
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return -1;
    }

    // 4. Second call to CryptStringToBinaryA to perform the actual decoding.
    //    We now pass the actual destination buffer and its capacity.
    //    On success, cbBinary will be updated with the actual number of bytes written.
    if (!CryptStringToBinaryA(
            src,                // Pointer to the string to be converted
            (DWORD)srcLen,      // Length of the string to be converted
            CRYPT_STRING_BASE64, // Flag for Base64 decoding
            dst,                // Output buffer
            &cbBinary,          // Pointer to a DWORD that receives the actual decoded bytes written
            NULL,               // Not used for this flag
            NULL                // Not used for this flag
        )) {
        // This should generally not fail if the first call succeeded and the buffer is sufficient.
        // If it does, it indicates an unexpected system error.
        return -1;
    }

    // 5. Return the number of bytes successfully decoded.
    return (int)cbBinary;
}

/*
// Example Usage (for testing purposes):
#include <stdio.h>
#include <string.h>

int main() {
    const char *base64_src = "SGVsbG8sIFdvcmxkIQ=="; // Base64 for "Hello, World!"
    unsigned char decoded_buffer[100];
    int decoded_len;

    printf("Original Base64: \"%s\"\n", base64_src);

    // Test case 1: Successful decoding with sufficient buffer
    decoded_len = Base64DecodeBuffer(base64_src, strlen(base64_src), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        printf("Test 1 (Success):\n");
        printf("  Decoded length: %d bytes\n", decoded_len);
        // Print as string, ensure it's null-terminated for printf if you want to print it as a C string
        // In real-world scenarios, binary data should be handled byte-by-byte or in hex.
        if ((unsigned int)decoded_len < sizeof(decoded_buffer)) {
             decoded_buffer[decoded_len] = '\0';
             printf("  Decoded data: \"%s\"\n", (char*)decoded_buffer);
        } else {
             printf("  Decoded data (first few bytes): ");
             for (int i = 0; i < decoded_len && i < 20; ++i) {
                 printf("%02X ", decoded_buffer[i]);
             }
             printf("...\n");
        }
    } else {
        printf("Test 1 (Failure): Decoding failed. Last error: %lu\n", GetLastError());
    }

    printf("\n");

    // Test case 2: Insufficient buffer
    unsigned char small_buffer[5]; // "Hello, World!" is 13 bytes
    printf("Attempting to decode into a small buffer (size 5):\n");
    decoded_len = Base64DecodeBuffer(base64_src, strlen(base64_src), small_buffer, sizeof(small_buffer));
    if (decoded_len == -1) {
        printf("Test 2 (Success): Correctly returned -1. Last error: %lu\n", GetLastError());
    } else {
        printf("Test 2 (Failure): Unexpectedly decoded %d bytes into small buffer.\n", decoded_len);
    }

    printf("\n");

    // Test case 3: Invalid Base64 string
    const char *invalid_base64 = "Invalid-Base64!";
    printf("Attempting to decode an invalid Base64 string: \"%s\"\n", invalid_base64);
    decoded_len = Base64DecodeBuffer(invalid_base64, strlen(invalid_base64), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Test 3 (Success): Correctly returned -1. Last error: %lu\n", GetLastError());
    } else {
        printf("Test 3 (Failure): Unexpectedly decoded %d bytes from invalid string.\n", decoded_len);
    }

    printf("\n");

    // Test case 4: NULL source
    printf("Attempting to decode with NULL source:\n");
    decoded_len = Base64DecodeBuffer(NULL, 10, decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Test 4 (Success): Correctly returned -1. Last error: %lu\n", GetLastError());
    } else {
        printf("Test 4 (Failure): Unexpectedly decoded %d bytes with NULL source.\n", decoded_len);
    }

    printf("\n");

    // Test case 5: NULL destination
    printf("Attempting to decode with NULL destination:\n");
    decoded_len = Base64DecodeBuffer(base64_src, strlen(base64_src), NULL, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Test 5 (Success): Correctly returned -1. Last error: %lu\n", GetLastError());
    } else {
        printf("Test 5 (Failure): Unexpectedly decoded %d bytes with NULL destination.\n", decoded_len);
    }

    return 0;
}
*/

