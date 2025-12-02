#include <Windows.h>
#include <Wincrypt.h> // Required for CryptStringToBinaryA

// Link with Crypt32.lib
#pragma comment(lib, "Crypt32.lib")

/**
 * @brief Decodes a Base64 encoded string into a binary buffer.
 *
 * @param src      Pointer to the Base64 encoded source string.
 * @param srcLen   Length of the source string in bytes.
 * @param dst      Pointer to the destination buffer where decoded bytes will be stored.
 * @param dstLen   Capacity of the destination buffer in bytes.
 *
 * @return The number of decoded bytes written to the destination buffer on success,
 *         or -1 on error (e.g., invalid input, decoding failure, or destination buffer too small).
 */
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    // 1. Validate input parameters
    if (src == NULL || srcLen < 0 || dst == NULL || dstLen < 0) {
        // Invalid arguments provided
        return -1;
    }

    DWORD cbBinary = 0; // Will store the required/actual size of the decoded data

    // 2. First call to CryptStringToBinaryA to determine the required buffer size.
    //    We pass NULL for pbBinary to indicate that we only want to query the size.
    if (!CryptStringToBinaryA(
        src,            // pszString: Pointer to the string to be converted.
        srcLen,         // cchString: Number of characters in the string to be converted.
        CRYPT_STRING_BASE64, // dwFlags: Specifies Base64 decoding.
        NULL,           // pbBinary: NULL to get the required buffer size.
        &cbBinary,      // pcbBinary: Receives the required buffer size.
        NULL,           // pdwSkip: Not used, set to NULL.
        NULL            // pdwFlags: Not used, set to NULL.
    )) {
        // CryptStringToBinaryA failed. This could be due to:
        // - Invalid Base64 characters in src.
        // - Other internal API errors.
        return -1;
    }

    // 3. Check if the provided destination buffer has sufficient capacity.
    //    We cast cbBinary to int for comparison with dstLen (which is int).
    //    If the required size exceeds the provided capacity, it's an error.
    if ((int)cbBinary > dstLen) {
        // Destination buffer is too small to hold the decoded data.
        // As per the requirement, return -1 on error.
        return -1;
    }

    // 4. Second call to CryptStringToBinaryA to perform the actual decoding.
    //    This time, we pass the destination buffer (dst) to store the result.
    //    cbBinary will be updated with the actual number of bytes written.
    if (!CryptStringToBinaryA(
        src,            // pszString: Pointer to the string to be converted.
        srcLen,         // cchString: Number of characters in the string to be converted.
        CRYPT_STRING_BASE64, // dwFlags: Specifies Base64 decoding.
        dst,            // pbBinary: Pointer to the buffer to receive the decoded data.
        &cbBinary,      // pcbBinary: Receives the actual number of bytes written.
        NULL,           // pdwSkip: Not used, set to NULL.
        NULL            // pdwFlags: Not used, set to NULL.
    )) {
        // This should ideally not fail if the first call succeeded with the same input
        // and buffer size check passed, but it's good practice for defensive programming.
        return -1;
    }

    // 5. Return the number of decoded bytes.
    return (int)cbBinary;
}

// Example Usage (for testing purposes, compile with main function if needed)
#ifdef _DEBUG_BASE64_DECODE
#include <stdio.h>
#include <string.h>

int main() {
    const char *base64_encoded = "SGVsbG8sIFdvcmxkIQ=="; // "Hello, World!"
    unsigned char decoded_buffer[256];
    int decoded_len;

    printf("Encoded string: \"%s\"\n", base64_encoded);

    // Test case 1: Successful decoding
    decoded_len = Base64DecodeBuffer(base64_encoded, strlen(base64_encoded), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        printf("Decoded successfully. Length: %d\n", decoded_len);
        printf("Decoded data: \"%.*s\"\n", decoded_len, decoded_buffer); // Print as string for readable content
    } else {
        printf("Decoding failed for valid input.\n");
    }
    printf("\n");

    // Test case 2: Buffer too small
    const char *short_encoded = "SGVsbG8="; // "Hello" (5 bytes)
    unsigned char small_buffer[4]; // Capacity for only 4 bytes
    printf("Encoded string (short): \"%s\"\n", short_encoded);
    decoded_len = Base64DecodeBuffer(short_encoded, strlen(short_encoded), small_buffer, sizeof(small_buffer));
    if (decoded_len == -1) {
        printf("Decoding correctly failed: Buffer too small or other error.\n");
    } else {
        printf("Decoding unexpectedly succeeded when buffer was too small.\n");
    }
    printf("\n");

    // Test case 3: Invalid Base64 input
    const char *invalid_base64 = "SGVsbG8!@#=";
    printf("Invalid Base64 string: \"%s\"\n", invalid_base64);
    decoded_len = Base64DecodeBuffer(invalid_base64, strlen(invalid_base64), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Decoding correctly failed for invalid Base64 input.\n");
    } else {
        printf("Decoding unexpectedly succeeded for invalid Base64 input.\n");
    }
    printf("\n");

    // Test case 4: Empty string
    const char *empty_encoded = "";
    unsigned char empty_decoded[1];
    printf("Empty encoded string: \"%s\"\n", empty_encoded);
    decoded_len = Base64DecodeBuffer(empty_encoded, strlen(empty_encoded), empty_decoded, sizeof(empty_decoded));
    if (decoded_len != -1) {
        printf("Decoded successfully. Length: %d\n", decoded_len);
        // Should print empty string
        printf("Decoded data: \"%.*s\"\n", decoded_len, empty_decoded);
    } else {
        printf("Decoding failed for empty input.\n");
    }
    printf("\n");

    // Test case 5: Null src pointer
    printf("Testing with NULL src pointer:\n");
    decoded_len = Base64DecodeBuffer(NULL, 10, decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Decoding correctly failed for NULL src.\n");
    } else {
        printf("Decoding unexpectedly succeeded for NULL src.\n");
    }
    printf("\n");
    
    // Test case 6: Null dst pointer
    printf("Testing with NULL dst pointer:\n");
    decoded_len = Base64DecodeBuffer(base64_encoded, strlen(base64_encoded), NULL, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Decoding correctly failed for NULL dst.\n");
    } else {
        printf("Decoding unexpectedly succeeded for NULL dst.\n");
    }
    printf("\n");

    return 0;
}
#endif // _DEBUG_BASE64_DECODE