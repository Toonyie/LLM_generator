// Code in C for Windows
// Decodes a Base64 encoded string into a byte buffer using Windows Crypt32 API.

#include <windows.h> // Required for Windows API types and functions like DWORD, BOOL, GetLastError, SetLastError
#include <wincrypt.h> // Required for CryptStringToBinaryA

/**
 * @brief Decodes a Base64 encoded string into a byte buffer.
 *
 * This function takes a Base64 encoded string, decodes it, and places the resulting
 * binary data into a provided output buffer. It leverages the CryptStringToBinaryA
 * function from the Windows Cryptography API (Crypt32.lib).
 *
 * @param src The pointer to the Base64 encoded input string. This string does
 *            not need to be NULL-terminated if srcLen accurately reflects its length.
 * @param srcLen The length of the input string in bytes. This value must be non-negative.
 * @param dst The pointer to the output buffer where the decoded binary data will be stored.
 * @param dstLen The size of the output buffer in bytes. This value must be non-negative.
 *               If this buffer is too small to hold the entire decoded output, the function
 *               will fail, and GetLastError() will typically return ERROR_MORE_DATA (234).
 * @return The number of decoded bytes written to 'dst' on success, or -1 if an error occurred.
 *         On error, GetLastError() can be called to retrieve more specific error information.
 *         For invalid input parameters (NULL src/dst, negative srcLen/dstLen), GetLastError()
 *         will be set to ERROR_INVALID_PARAMETER.
 */
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    // 1. Input validation: Check for NULL pointers and negative lengths.
    if (src == NULL || srcLen < 0 || dst == NULL || dstLen < 0) {
        SetLastError(ERROR_INVALID_PARAMETER); // Indicate invalid arguments
        return -1; // Return -1 on error as per function prototype
    }

    // 2. Handle empty source string: An empty string decodes to zero bytes.
    if (srcLen == 0) {
        return 0; // No error, 0 bytes decoded.
    }

    // CryptStringToBinaryA expects DWORD for buffer lengths.
    // cbBinarySize will be initialized with the maximum capacity of 'dst'
    // and will be updated by CryptStringToBinaryA with the actual number of bytes written.
    DWORD cbBinarySize = (DWORD)dstLen;

    // 3. Call CryptStringToBinaryA to perform the Base64 decoding.
    // CRYPT_STRING_BASE64 flag specifies that the input string is Base64 encoded.
    // pdwSkip and pdwFlags are set to NULL as they are not needed for simple decoding.
    BOOL success = CryptStringToBinaryA(
        src,              // Pointer to the input Base64 string
        (DWORD)srcLen,    // Length of the input string
        CRYPT_STRING_BASE64, // Flag indicating Base64 encoding
        dst,              // Pointer to the output buffer for decoded bytes
        &cbBinarySize,    // On input: max size of 'dst'. On output: actual bytes written.
        NULL,             // pdwSkip - not used for simple decoding
        NULL              // pdwFlags - not used for simple decoding
    );

    // 4. Error checking: If CryptStringToBinaryA returns FALSE, an error occurred.
    if (!success) {
        // GetLastError() can be used by the caller to get more specific details
        // (e.g., ERROR_MORE_DATA if 'dst' buffer was too small, or a cryptographic
        // error code if the input string was malformed).
        return -1; // Return -1 on error.
    }

    // 5. Success: Return the number of bytes successfully decoded.
    // On success, cbBinarySize holds the count of bytes written to 'dst'.
    return (int)cbBinarySize;
}

/*
// Example Usage (Compile with /D_DEBUG and link crypt32.lib):
#ifdef _DEBUG
#include <stdio.h>  // For printf
#include <string.h> // For strlen

// Helper function to print bytes in hexadecimal format
void PrintBytes(const unsigned char* data, int len) {
    for (int i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    printf("--- Base64DecodeBuffer Test Cases ---\n\n");

    // Test Case 1: Standard Base64 decoding
    const char* base64_src1 = "SGVsbG8gV29ybGQh"; // Decodes to "Hello World!" (12 bytes)
    unsigned char decoded_dst1[20]; // Buffer large enough
    printf("Test 1: Decoding '%s'\n", base64_src1);
    int decoded_len1 = Base64DecodeBuffer(base64_src1, (int)strlen(base64_src1), decoded_dst1, sizeof(decoded_dst1));

    if (decoded_len1 != -1) {
        printf("  SUCCESS: Decoded %d bytes.\n", decoded_len1);
        printf("  Output string: '");
        for(int i = 0; i < decoded_len1; ++i) {
            printf("%c", decoded_dst1[i]);
        }
        printf("'\n");
        printf("  Raw bytes: ");
        PrintBytes(decoded_dst1, decoded_len1);
    } else {
        printf("  FAILED: Error decoding. GetLastError: %lu\n", GetLastError());
    }
    printf("\n");

    // Test Case 2: Output buffer too small
    const char* base64_src2 = "SGVsbG8gV29ybGQh"; // "Hello World!" (12 bytes)
    unsigned char decoded_dst2[10]; // Buffer too small (needs 12 bytes)
    printf("Test 2: Decoding '%s' with small buffer (size %zu)\n", base64_src2, sizeof(decoded_dst2));
    int decoded_len2 = Base64DecodeBuffer(base64_src2, (int)strlen(base64_src2), decoded_dst2, sizeof(decoded_dst2));

    if (decoded_len2 != -1) {
        printf("  UNEXPECTED SUCCESS (Should fail due to buffer size): Decoded %d bytes.\n", decoded_len2);
    } else {
        printf("  EXPECTED FAILURE: Error decoding. GetLastError: %lu (Expected ERROR_MORE_DATA (234))\n", GetLastError());
    }
    printf("\n");

    // Test Case 3: Empty input string
    const char* base64_src3 = "";
    unsigned char decoded_dst3[10];
    printf("Test 3: Decoding empty string '%s'\n", base64_src3);
    int decoded_len3 = Base64DecodeBuffer(base64_src3, (int)strlen(base64_src3), decoded_dst3, sizeof(decoded_dst3));

    if (decoded_len3 != -1) {
        printf("  SUCCESS: Decoded %d bytes.\n", decoded_len3);
    } else {
        printf("  FAILED: Error decoding. GetLastError: %lu\n", GetLastError());
    }
    printf("\n");

    // Test Case 4: Invalid Base64 input string (contains non-Base64 characters)
    const char* base64_src4 = "Invalid-Base64!"; 
    unsigned char decoded_dst4[20];
    printf("Test 4: Decoding invalid string '%s'\n", base64_src4);
    int decoded_len4 = Base64DecodeBuffer(base64_src4, (int)strlen(base64_src4), decoded_dst4, sizeof(decoded_dst4));

    if (decoded_len4 != -1) {
        printf("  UNEXPECTED SUCCESS (Should fail due to invalid format): Decoded %d bytes.\n", decoded_len4);
    } else {
        printf("  EXPECTED FAILURE: Error decoding. GetLastError: %lu\n", GetLastError());
    }
    printf("\n");

    // Test Case 5: NULL source pointer
    printf("Test 5: Decoding with NULL src pointer\n");
    int decoded_len5 = Base64DecodeBuffer(NULL, 10, decoded_dst1, sizeof(decoded_dst1));
    if (decoded_len5 != -1) {
        printf("  UNEXPECTED SUCCESS: Decoded with NULL src.\n");
    } else {
        printf("  EXPECTED FAILURE: Error decoding NULL src. GetLastError: %lu (Expected ERROR_INVALID_PARAMETER (87))\n", GetLastError());
    }
    printf("\n");

    // Test Case 6: Negative source length
    printf("Test 6: Decoding with negative srcLen (-5)\n");
    int decoded_len6 = Base64DecodeBuffer(base64_src1, -5, decoded_dst1, sizeof(decoded_dst1));
    if (decoded_len6 != -1) {
        printf("  UNEXPECTED SUCCESS: Decoded with negative srcLen.\n");
    } else {
        printf("  EXPECTED FAILURE: Error decoding with negative srcLen. GetLastError: %lu (Expected ERROR_INVALID_PARAMETER (87))\n", GetLastError());
    }
    printf("\n");

    // Test Case 7: Negative destination length
    printf("Test 7: Decoding with negative dstLen (-5)\n");
    int decoded_len7 = Base64DecodeBuffer(base64_src1, (int)strlen(base64_src1), decoded_dst1, -5);
    if (decoded_len7 != -1) {
        printf("  UNEXPECTED SUCCESS: Decoded with negative dstLen.\n");
    } else {
        printf("  EXPECTED FAILURE: Error decoding with negative dstLen. GetLastError: %lu (Expected ERROR_INVALID_PARAMETER (87))\n", GetLastError());
    }
    printf("\n");

    return 0;
}
#endif // _DEBUG
*/