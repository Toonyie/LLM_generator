// Code in C
// Code for Windows
// Use Windows APIs (Crypt32)

#include <windows.h>   // Required for Windows API types and common functions
#include <wincrypt.h>  // Required specifically for CryptStringToBinaryA

/**
 * @brief Decodes a Base64-encoded string into a binary buffer.
 *
 * This function decodes a Base64-encoded string pointed to by `src` with length
 * `srcLen` and writes the resulting binary data into the buffer `dst`, which
 * has a maximum capacity of `dstLen` bytes.
 *
 * @param src Pointer to the Base64-encoded string. It does not need to be
 *            null-terminated if `srcLen` accurately specifies its length.
 * @param srcLen The length of the Base64-encoded string in bytes. If 0, an
 *               empty string is processed, resulting in 0 decoded bytes.
 * @param dst Pointer to the buffer where the decoded binary data will be stored.
 * @param dstLen The maximum capacity of the `dst` buffer in bytes.
 *
 * @return On success, returns the number of bytes written to the `dst` buffer.
 *         On error (e.g., invalid input parameters, malformed Base64 string,
 *         or `dst` buffer being too small), returns -1.
 */
int Base64DecodeBuffer(const char *src, int srcLen, unsigned char *dst, int dstLen) {
    // Input validation: Check for NULL pointers and negative lengths.
    if (src == NULL || dst == NULL) {
        // Source or destination buffer pointer is NULL.
        return -1;
    }
    if (srcLen < 0 || dstLen < 0) {
        // Source or destination length cannot be negative.
        return -1;
    }

    // decodedBytesCount will serve as input for CryptStringToBinaryA to specify
    // the size of the destination buffer, and as output to receive the number
    // of bytes actually written.
    DWORD decodedBytesCount = (DWORD)dstLen; 

    // Use CryptStringToBinaryA to perform the Base64 decoding.
    // CRYPT_STRING_BASE64_ANY is a versatile flag that attempts to decode
    // various Base64 formats, including standard Base64 and URL-safe Base64.
    BOOL success = CryptStringToBinaryA(
        src,                 // Pointer to the input Base64 string
        (DWORD)srcLen,       // Length of the input string
        CRYPT_STRING_BASE64_ANY, // Flag indicating Base64 decoding type
        dst,                 // Pointer to the output buffer for binary data
        &decodedBytesCount,  // Pointer to DWORD. In: size of dst. Out: bytes written.
        NULL,                // Optional: not interested in characters skipped (pass NULL)
        NULL                 // Optional: not interested in flags used (pass NULL)
    );

    if (success) {
        // Decoding was successful. Return the number of bytes written to the
        // destination buffer.
        return (int)decodedBytesCount;
    } else {
        // An error occurred during decoding.
        // Common reasons for failure include:
        // - The `dst` buffer being too small (GetLastError() would return ERROR_MORE_DATA).
        // - The `src` string containing invalid Base64 characters or being malformed.
        // As per the function contract, we return -1 for any error.
        // For debugging purposes, one could inspect GetLastError() here to get
        // more specific error information.
        // DWORD error = GetLastError();
        // (void)error; // Suppress unused variable warning if 'error' is not used
        return -1;
    }
}