#include <windows.h>
#include <wincrypt.h> // Required for CryptStringToBinaryA

int Decode_Base64(void *encoded, int size, void *decoded) {
    DWORD dwDecodedSize = 0;

    // First call to CryptStringToBinaryA to determine the required output buffer size.
    // We pass NULL for the output buffer (pbBinary) and let pcbBinary receive the size.
    // If 'size' is 0, the function expects 'encoded' to be a null-terminated string.
    // Otherwise, 'size' specifies the number of characters in 'encoded'.
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,      // Pointer to the Base64 encoded string
        (DWORD)size,          // Length of the encoded string
        CRYPT_STRING_BASE64,  // Flag indicating Base64 decoding
        NULL,                 // Output buffer (NULL to get size)
        &dwDecodedSize,       // Pointer to receive the required buffer size
        NULL,                 // dwSkip parameter (not used for this operation)
        NULL                  // pdwFlagsPtr parameter (not used for this operation)
    )) {
        // An error occurred (e.g., invalid Base64 input, or other CryptoAPI issue).
        // Return 0 to indicate no bytes were decoded.
        return 0;
    }

    // Second call to CryptStringToBinaryA to perform the actual decoding.
    // We pass the provided 'decoded' buffer and the obtained size.
    // dwDecodedSize will be updated with the actual number of bytes written.
    if (!CryptStringToBinaryA(
        (LPCSTR)encoded,
        (DWORD)size,
        CRYPT_STRING_BASE64,
        (PBYTE)decoded,       // Output buffer where decoded data will be written
        &dwDecodedSize,       // Pointer to the size of the output buffer (updated with actual decoded size)
        NULL,
        NULL
    )) {
        // An error occurred during the decoding process.
        // Return 0 to indicate no bytes were decoded.
        return 0;
    }

    // Return the actual number of bytes successfully decoded.
    return (int)dwDecodedSize;
}