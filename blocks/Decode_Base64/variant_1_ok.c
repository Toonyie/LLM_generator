#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinary

// Function prototype: int Decode_Base64(void *encoded, int size, void *decoded);
// Decode buffer content from base64 into another buffer
// Return buffer size (decoded length), or 0 on failure.

int Decode_Base64(void *encoded, int size, void *decoded) {
    if (encoded == NULL || decoded == NULL || size <= 0) {
        // Invalid input parameters
        return 0;
    }

    LPCSTR pszEncoded = (LPCSTR)encoded;
    DWORD dwEncodedSize = (DWORD)size;

    DWORD dwDecodedSize = 0;

    // First call to determine the required buffer size for the decoded data.
    // CRYPT_STRING_BASE64_ANY is used for robustness, as it ignores headers,
    // footers, and non-base64 characters like newlines.
    if (!CryptStringToBinaryA(
            pszEncoded,
            dwEncodedSize,
            CRYPT_STRING_BASE64_ANY, // Flags for flexible Base64 decoding
            NULL,                    // Output buffer for decoded bytes (NULL to get required size)
            &dwDecodedSize,          // Pointer to receive the required size
            NULL,                    // Not interested in pdwSkip
            NULL                     // Not interested in pdwFlags
        )) {
        // Failed to get the required size.
        // GetLastError() could be used here for more detailed error information if needed.
        return 0;
    }

    // Now, dwDecodedSize holds the required size for the decoded buffer.
    // Perform the actual decoding into the 'decoded' buffer.
    // The 'decoded' buffer must be pre-allocated by the caller and be at least
    // 'dwDecodedSize' bytes in capacity.
    if (!CryptStringToBinaryA(
            pszEncoded,
            dwEncodedSize,
            CRYPT_STRING_BASE64_ANY, // Flags for flexible Base64 decoding
            (BYTE *)decoded,         // Output buffer for decoded bytes
            &dwDecodedSize,          // Input: size of 'decoded' buffer (or required size), Output: actual decoded size
            NULL,                    // Not interested in pdwSkip
            NULL                     // Not interested in pdwFlags
        )) {
        // Failed to decode the string.
        return 0;
    }

    // Successfully decoded, return the actual size of the decoded data.
    return (int)dwDecodedSize;
}