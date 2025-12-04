#include <windows.h>
#include <wincrypt.h>

int Decode_Base64(void *encoded, int size, void *decoded) {
    DWORD dwDecodedSize = 0;
    BOOL bResult;

    // Step 1: Determine the required buffer size for the decoded data.
    // Call CryptStringToBinaryA with a NULL output buffer.
    bResult = CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the base64 encoded string
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag for Base64 decoding
        NULL,                   // Output buffer (NULL to get size)
        &dwDecodedSize,         // Pointer to receive the required size
        NULL,                   // Reserved, must be NULL
        NULL                    // Reserved, must be NULL
    );

    if (!bResult) {
        // An error occurred (e.g., invalid base64 string format).
        // Return 0 to indicate failure.
        return 0;
    }

    // Step 2: Perform the actual decoding into the provided buffer.
    // Use the determined size (dwDecodedSize) which will be updated
    // with the actual number of bytes written.
    bResult = CryptStringToBinaryA(
        (LPCSTR)encoded,        // Pointer to the base64 encoded string
        (DWORD)size,            // Length of the encoded string
        CRYPT_STRING_BASE64,    // Flag for Base64 decoding
        (PBYTE)decoded,         // Output buffer for decoded data
        &dwDecodedSize,         // Pointer to the actual size of the decoded data
        NULL,                   // Reserved, must be NULL
        NULL                    // Reserved, must be NULL
    );

    if (!bResult) {
        // An error occurred during decoding.
        // Return 0 to indicate failure.
        return 0;
    }

    // Return the size of the successfully decoded data.
    return (int)dwDecodedSize;
}