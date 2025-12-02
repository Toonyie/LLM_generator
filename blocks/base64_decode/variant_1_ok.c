#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinary

// Disable specific warning for CryptStringToBinaryA, if it appears
// #pragma comment(lib, "crypt32.lib") // Link with crypt32.lib if not using linker options

/**
 * @brief Decodes a Base64 encoded string into a byte buffer.
 *
 * This function decodes a Base64 encoded string from 'src' into the 'dst' buffer
 * using Windows Cryptography API.
 *
 * @param src The Base64 encoded source string.
 * @param srcLen The length of the source string in bytes.
 * @param dst The destination buffer for the decoded bytes.
 * @param dstLen The maximum capacity of the destination buffer in bytes.
 *               This must be large enough to hold the decoded data.
 * @return The number of decoded bytes written to 'dst' on success, or -1 on error.
 *         Errors can include invalid input, insufficient buffer size, or malformed
 *         Base64 string.
 */
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    if (src == NULL || srcLen < 0 || dst == NULL || dstLen < 0) {
        // Invalid input parameters
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (srcLen == 0) {
        // Empty source string, return 0 decoded bytes
        return 0;
    }

    DWORD cbDecoded = (DWORD)dstLen; // Initialize with the maximum buffer size

    // CryptStringToBinaryA decodes a string that has been encoded by CryptBinaryToStringA.
    // CRYPT_STRING_BASE64 flag indicates Base64 decoding.
    // It returns TRUE on success, FALSE otherwise.
    BOOL success = CryptStringToBinaryA(
        src,              // Input string
        (DWORD)srcLen,    // Length of the input string
        CRYPT_STRING_BASE64, // Flag for Base64 decoding
        dst,              // Output buffer for decoded data
        &cbDecoded,       // On input, max size of output buffer. On output, actual size used.
        NULL,             // Not used for Base64
        NULL              // Not used for Base64
    );

    if (!success) {
        // An error occurred during decoding.
        // GetLastError() could provide more specific details (e.g., ERROR_MORE_DATA if dstLen is too small,
        // or other errors for malformed base64).
        return -1;
    }

    return (int)cbDecoded; // Return the actual number of bytes decoded
}

/*
// --- Example Usage (Optional, for testing purposes) ---
#include <stdio.h>
#include <string.h>

void PrintHex(const char *label, const unsigned char *data, int len) {
    printf("%s (len %d): ", label, len);
    for (int i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1: Standard Base64
    const char *b64_src1 = "SGVsbG8sIFdvcmxkIQ=="; // "Hello, World!"
    unsigned char dst1[32];
    int decoded_len1 = Base64DecodeBuffer(b64_src1, strlen(b64_src1), dst1, sizeof(dst1));
    if (decoded_len1 != -1) {
        printf("Decoded '%s': ", b64_src1);
        for (int i = 0; i < decoded_len1; ++i) {
            printf("%c", dst1[i]);
        }
        printf(" (len %d)\n", decoded_len1);
        PrintHex("Hex", dst1, decoded_len1);
    } else {
        printf("Error decoding '%s'. GetLastError(): %lu\n", b64_src1, GetLastError());
    }
    printf("\n");

    // Test Case 2: Shorter data (padding)
    const char *b64_src2 = "Zm9vYg=="; // "foob"
    unsigned char dst2[16];
    int decoded_len2 = Base64DecodeBuffer(b64_src2, strlen(b64_src2), dst2, sizeof(dst2));
    if (decoded_len2 != -1) {
        printf("Decoded '%s': ", b64_src2);
        for (int i = 0; i < decoded_len2; ++i) {
            printf("%c", dst2[i]);
        }
        printf(" (len %d)\n", decoded_len2);
        PrintHex("Hex", dst2, decoded_len2);
    } else {
        printf("Error decoding '%s'. GetLastError(): %lu\n", b64_src2, GetLastError());
    }
    printf("\n");

    // Test Case 3: Binary data (no padding)
    const char *b64_src3 = "AwIEBQ=="; // { 0x03, 0x04, 0x05 }
    unsigned char dst3[16];
    int decoded_len3 = Base64DecodeBuffer(b64_src3, strlen(b64_src3), dst3, sizeof(dst3));
    if (decoded_len3 != -1) {
        printf("Decoded '%s': ", b64_src3);
        PrintHex("Hex", dst3, decoded_len3);
    } else {
        printf("Error decoding '%s'. GetLastError(): %lu\n", b64_src3, GetLastError());
    }
    printf("\n");

    // Test Case 4: Destination buffer too small
    const char *b64_src4 = "SGVsbG8sIFdvcmxkIQ=="; // "Hello, World!" (13 bytes)
    unsigned char dst4[10]; // Too small
    int decoded_len4 = Base64DecodeBuffer(b64_src4, strlen(b64_src4), dst4, sizeof(dst4));
    if (decoded_len4 != -1) {
        printf("Decoded '%s' successfully (unexpected for small buffer) len %d\n", b64_src4, decoded_len4);
    } else {
        printf("Error decoding '%s' (expected due to small buffer). GetLastError(): %lu\n", b64_src4, GetLastError());
    }
    printf("\n");

    // Test Case 5: Invalid Base64 character
    const char *b64_src5 = "SGVsbG8, IFdvcmxkIQ=="; // Comma is invalid
    unsigned char dst5[32];
    int decoded_len5 = Base64DecodeBuffer(b64_src5, strlen(b64_src5), dst5, sizeof(dst5));
    if (decoded_len5 != -1) {
        printf("Decoded '%s' successfully (unexpected for invalid char) len %d\n", b64_src5, decoded_len5);
    } else {
        printf("Error decoding '%s' (expected due to invalid char). GetLastError(): %lu\n", b64_src5, GetLastError());
    }
    printf("\n");

    // Test Case 6: Empty string
    const char *b64_src6 = "";
    unsigned char dst6[32];
    int decoded_len6 = Base64DecodeBuffer(b64_src6, strlen(b64_src6), dst6, sizeof(dst6));
    if (decoded_len6 != -1) {
        printf("Decoded empty string '%s': (len %d)\n", b64_src6, decoded_len6);
    } else {
        printf("Error decoding empty string '%s'. GetLastError(): %lu\n", b64_src6, GetLastError());
    }
    printf("\n");

    // Test Case 7: Null source
    unsigned char dst7[32];
    int decoded_len7 = Base64DecodeBuffer(NULL, 10, dst7, sizeof(dst7));
    if (decoded_len7 != -1) {
        printf("Decoded NULL source (unexpected) len %d\n", decoded_len7);
    } else {
        printf("Error decoding NULL source (expected). GetLastError(): %lu\n", GetLastError());
    }
    printf("\n");

    // Test Case 8: Null destination
    const char *b64_src8 = "SGVsbG8sIFdvcmxkIQ==";
    int decoded_len8 = Base64DecodeBuffer(b64_src8, strlen(b64_src8), NULL, 32);
    if (decoded_len8 != -1) {
        printf("Decoded NULL destination (unexpected) len %d\n", decoded_len8);
    } else {
        printf("Error decoding NULL destination (expected). GetLastError(): %lu\n", GetLastError());
    }
    printf("\n");

    return 0;
}
*/