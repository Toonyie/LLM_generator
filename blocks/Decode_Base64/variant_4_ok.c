#include <windows.h>
#include <wincrypt.h>

int Decode_Base64(void *encoded, int size, void *decoded) {
    DWORD dwDecodedSize = 0;
    LPCSTR pszEncoded = (LPCSTR)encoded;
    DWORD cchEncoded = (DWORD)size;

    // First call to CryptStringToBinaryA to determine the required output buffer size.
    // We pass NULL for the output buffer (pbBinary) and the address of dwDecodedSize.
    // The dwFlags parameter is set to CRYPT_STRING_BASE64 to specify Base64 decoding.
    if (!CryptStringToBinaryA(
        pszEncoded,
        cchEncoded,
        CRYPT_STRING_BASE64,
        NULL,                // Output buffer is NULL to query size
        &dwDecodedSize,      // Receives the required size in bytes
        NULL,                // Not used for this flag
        NULL                 // Not used for this flag
    )) {
        // If the first call fails, it indicates an issue with the encoded string
        // or an internal error. Return 0 to signify no bytes were decoded.
        return 0;
    }

    // Second call to CryptStringToBinaryA to perform the actual decoding.
    // We pass the caller-provided 'decoded' buffer and the previously obtained size.
    // It's assumed that the 'decoded' buffer is large enough to hold dwDecodedSize bytes.
    // If the provided 'decoded' buffer is too small, this call will fail.
    if (!CryptStringToBinaryA(
        pszEncoded,
        cchEncoded,
        CRYPT_STRING_BASE64,
        (LPBYTE)decoded,     // Pointer to the buffer where decoded data will be written
        &dwDecodedSize,      // On input: size of the 'decoded' buffer. On output: actual bytes written.
        NULL,                // Not used
        NULL                 // Not used
    )) {
        // If the second call fails, it could mean the 'decoded' buffer was too small
        // or other errors occurred during decoding. Return 0.
        return 0;
    }

    // Return the number of bytes successfully decoded into the 'decoded' buffer.
    return (int)dwDecodedSize;
}