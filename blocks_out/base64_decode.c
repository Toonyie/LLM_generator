```c
#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinaryA
#include <stdio.h>    // For demonstration purposes (printf)
#include <string.h>   // For demonstration purposes (strlen)

// Function prototype as specified
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    if (src == NULL || dst == NULL) {
        // Invalid input pointers
        // SetLastError(ERROR_INVALID_PARAMETER); // Could set a specific Windows error
        return -1;
    }
    if (srcLen < 0 || dstLen < 0) {
        // Invalid lengths (lengths cannot be negative)
        // SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (srcLen == 0) {
        // Empty source string results in 0 decoded bytes
        return 0;
    }

    DWORD dwDecodedLen = 0;

    // Step 1: Call CryptStringToBinaryA with pbBinary = NULL to get the required buffer size
    // CRYPT_STRING_BASE64 flag specifies standard base64 decoding.
    if (!CryptStringToBinaryA(
        src,            // pszString: Pointer to the base64 encoded string
        srcLen,         // cchString: Length of the source string
        CRYPT_STRING_BASE64, // dwFlags: Specifies base64 decoding
        NULL,           // pbBinary: NULL to retrieve the required size
        &dwDecodedLen,  // pcbBinary: On success, receives the required buffer size
        NULL,           // pdwSkip: Not used for simple decoding
        NULL            // pdwFlags: Not used for simple decoding
    )) {
        // CryptStringToBinaryA failed (e.g., invalid base64 format)
        // GetLastError() could be used here to get more specific error information,
        // but the requirement is to return -1 on error.
        return -1;
    }

    // Step 2: Check if the provided destination buffer is large enough
    if ((int)dwDecodedLen > dstLen) {
        // The destination buffer is too small to hold the decoded data
        // SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return -1;
    }

    // Step 3: Call CryptStringToBinaryA again to perform the actual decoding
    if (!CryptStringToBinaryA(
        src,            // pszString: Pointer to the base64 encoded string
        srcLen,         // cchString: Length of the source string
        CRYPT_STRING_BASE64, // dwFlags: Specifies base64 decoding
        dst,            // pbBinary: Pointer to the destination buffer
        &dwDecodedLen,  // pcbBinary: On success, receives the number of decoded bytes
        NULL,           // pdwSkip: Not used
        NULL            // pdwFlags: Not used
    )) {
        // CryptStringToBinaryA failed during the actual decoding
        return -1;
    }

    // Return the number of decoded bytes
    return (int)dwDecodedLen;
}

// --- Demonstration / Test Code ---
int main() {
    const char *base64_encoded_str1 = "SGVsbG8gV29ybGQh"; // "Hello World!"
    const char *base64_encoded_str2 = "SW5mb3JtYXRpb24gdGVzdGluZy4="; // "Information testing."
    const char *base64_encoded_str3 = ""; // Empty string
    const char *base64_encoded_str4 = "MTIzNDU="; // "12345"
    const char *base64_encoded_str5 = "aW52YWxpZCBjaGFyISo="; // Contains an invalid char '*'

    unsigned char decoded_buffer[256]; // A sufficiently large buffer for tests
    int decoded_len;

    printf("--- Base64 Decode Tests ---\n\n");

    // Test Case 1: Standard string
    printf("Source: \"%s\"\n", base64_encoded_str1);
    decoded_len = Base64DecodeBuffer(base64_encoded_str1, strlen(base64_encoded_str1), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        decoded_buffer[decoded_len] = '\0'; // Null-terminate if it's a string for printing
        printf("Decoded (%d bytes): \"%s\"\n", decoded_len, decoded_buffer);
    } else {
        printf("Error decoding string 1 (Error Code: %lu)\n", GetLastError());
    }
    printf("Expected: \"Hello World!\"\n\n");

    // Test Case 2: Another standard string
    printf("Source: \"%s\"\n", base64_encoded_str2);
    decoded_len = Base64DecodeBuffer(base64_encoded_str2, strlen(base64_encoded_str2), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        decoded_buffer[decoded_len] = '\0';
        printf("Decoded (%d bytes): \"%s\"\n", decoded_len, decoded_buffer);
    } else {
        printf("Error decoding string 2 (Error Code: %lu)\n", GetLastError());
    }
    printf("Expected: \"Information testing.\"\n\n");

    // Test Case 3: Empty source string
    printf("Source: \"%s\"\n", base64_encoded_str3);
    decoded_len = Base64DecodeBuffer(base64_encoded_str3, strlen(base64_encoded_str3), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        decoded_buffer[decoded_len] = '\0';
        printf("Decoded (%d bytes): \"%s\"\n", decoded_len, decoded_buffer);
    } else {
        printf("Error decoding empty string (Error Code: %lu)\n", GetLastError());
    }
    printf("Expected: \"\" (0 bytes)\n\n");

    // Test Case 4: Numeric string
    printf("Source: \"%s\"\n", base64_encoded_str4);
    decoded_len = Base64DecodeBuffer(base64_encoded_str4, strlen(base64_encoded_str4), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len != -1) {
        decoded_buffer[decoded_len] = '\0';
        printf("Decoded (%d bytes): \"%s\"\n", decoded_len, decoded_buffer);
    } else {
        printf("Error decoding numeric string (Error Code: %lu)\n", GetLastError());
    }
    printf("Expected: \"12345\"\n\n");

    // Test Case 5: Insufficient buffer size
    printf("Source: \"%s\"\n", base64_encoded_str1);
    printf("Attempting to decode into a buffer of size 5 (too small)\n");
    unsigned char small_buffer[5];
    decoded_len = Base64DecodeBuffer(base64_encoded_str1, strlen(base64_encoded_str1), small_buffer, sizeof(small_buffer));
    if (decoded_len == -1) {
        printf("Error: Destination buffer too small (Correctly returned -1).\n");
    } else {
        printf("Decoded unexpectedly into small buffer (%d bytes)\n", decoded_len);
    }
    printf("Expected: Error (-1)\n\n");

    // Test Case 6: Invalid base64 character
    printf("Source: \"%s\"\n", base64_encoded_str5);
    decoded_len = Base64DecodeBuffer(base64_encoded_str5, strlen(base64_encoded_str5), decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Error: Invalid Base64 character (Correctly returned -1).\n");
    } else {
        printf("Decoded unexpectedly (%d bytes): \"%s\"\n", decoded_len, decoded_buffer);
    }
    printf("Expected: Error (-1)\n\n");

    // Test Case 7: NULL source pointer
    printf("Source: NULL\n");
    decoded_len = Base64DecodeBuffer(NULL, 10, decoded_buffer, sizeof(decoded_buffer));
    if (decoded_len == -1) {
        printf("Error: NULL source pointer (Correctly returned -1).\n");
    } else {
        printf("Decoded unexpectedly (%d bytes)\n", decoded_len);
    }
    printf("Expected: Error (-1)\n\n");

    // Test Case 8: NULL destination pointer
    printf("Destination: NULL\n");
    decoded_len = Base64DecodeBuffer(base64_encoded_str1, strlen(base64_encoded_str1), NULL, 10);
    if (decoded_len == -1) {
        printf("Error: NULL destination pointer (Correctly returned -1).\n");
    } else {
        printf("Decoded unexpectedly (%d bytes)\n", decoded_len);
    }
    printf("Expected: Error (-1)\n\n");

    return 0;
}
```