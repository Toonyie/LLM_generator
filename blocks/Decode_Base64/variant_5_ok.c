#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinary

int Decode_Base64(void *encoded, int size, void *decoded) {
    if (encoded == NULL || decoded == NULL || size < 0) {
        // Invalid input parameters
        return 0;
    }

    DWORD decoded_size = 0;

    // First call to CryptStringToBinaryA to determine the required output buffer size.
    // The 'size' parameter is passed as cchString, so the input buffer
    // 'encoded' does not need to be null-terminated.
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the encoded string buffer
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag to specify Base64 decoding
        NULL,                   // Output buffer is NULL to get the required size
        &decoded_size,          // Receives the required size of the output buffer
        NULL,                   // Not used for this operation
        NULL                    // Not used for this operation
    )) {
        // An error occurred (e.g., invalid Base64 string)
        return 0;
    }

    if (decoded_size == 0 && size == 0) {
        // Special case: an empty input string correctly decodes to an empty output.
        // In this specific scenario, decoded_size will be 0, and this is a valid result.
        return 0;
    }
    
    // Second call to CryptStringToBinaryA to perform the actual decoding
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the encoded string buffer
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag to specify Base64 decoding
        (BYTE *)decoded,        // Pointer to the caller-provided output buffer
        &decoded_size,          // Receives the actual size of the decoded data
        NULL,                   // Not used for this operation
        NULL                    // Not used for this operation
    )) {
        // An error occurred during decoding (e.g., buffer too small, or other error)
        // Note: This function assumes the 'decoded' buffer is large enough.
        // The caller is responsible for allocating 'decoded' with at least the size
        // obtained from the first call to CryptStringToBinaryA.
        return 0;
    }

    return (int)decoded_size;
}