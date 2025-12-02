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