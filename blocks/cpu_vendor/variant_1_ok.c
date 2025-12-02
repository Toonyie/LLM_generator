#include <windows.h> // For general Windows context (though not strictly required for __cpuid itself)
#include <intrin.h>  // Required for the __cpuid intrinsic
#include <string.h>  // Required for memcpy and strcpy_s

/**
 * @brief Retrieves the CPU vendor string (e.g., "GenuineIntel", "AuthenticAMD").
 *
 * This function uses the CPUID instruction (via the __cpuid intrinsic) to query
 * the processor's vendor identification string. The string is 12 characters long.
 *
 * @param out A pointer to a character buffer where the vendor string will be stored.
 * @param outLen The size of the output buffer in bytes, including space for the
 *               null terminator. It must be at least 13 bytes to hold the full
 *               12-character string plus null terminator.
 * @return 0 on success, -1 on failure (e.g., invalid buffer, buffer too small).
 */
int GetCPUVendor(char *out, int outLen) {
    // 1. Validate input parameters
    if (out == NULL || outLen <= 0) {
        return -1; // Invalid output buffer or length
    }

    // The CPU vendor string is exactly 12 characters long.
    // We need 12 characters + 1 for the null terminator.
    const int VENDOR_STRING_LENGTH = 12;
    if (outLen < VENDOR_STRING_LENGTH + 1) {
        // Buffer too small to hold the entire vendor string plus null terminator.
        // Ensure the output buffer is null-terminated even on failure if it's accessible.
        out[0] = '\0';
        return -1;
    }

    int cpuInfo[4]; // Array to store the EAX, EBX, ECX, EDX register values

    // Call the CPUID instruction with EAX set to 0.
    // This returns the highest CPUID function supported (in EAX)
    // and the vendor identification string in EBX, EDX, and ECX.
    // The __cpuid intrinsic fills cpuInfo[0] with EAX, cpuInfo[1] with EBX,
    // cpuInfo[2] with ECX, and cpuInfo[3] with EDX.
    __cpuid(cpuInfo, 0);

    // The vendor string is distributed across EBX, EDX, and ECX registers
    // in that specific order: EBX (bytes 0-3), EDX (bytes 4-7), ECX (bytes 8-11).
    char vendorString[VENDOR_STRING_LENGTH + 1]; // Temporary buffer for the vendor string

    // Copy the bytes from the registers into the temporary vendor string buffer.
    memcpy(&vendorString[0], &cpuInfo[1], 4); // Copy EBX (cpuInfo[1])
    memcpy(&vendorString[4], &cpuInfo[3], 4); // Copy EDX (cpuInfo[3])
    memcpy(&vendorString[8], &cpuInfo[2], 4); // Copy ECX (cpuInfo[2])

    // Null-terminate the temporary vendor string to make it a valid C string.
    vendorString[VENDOR_STRING_LENGTH] = '\0';

    // Copy the resulting vendor string to the user-provided output buffer.
    // strcpy_s is a safer alternative to strcpy, as it prevents buffer overflows
    // by checking the destination buffer size. We've already validated `outLen`
    // to be sufficient for `vendorString`.
    strcpy_s(out, outLen, vendorString);

    return 0; // Success
}

/*
// Example Usage (for testing purposes, compile with MSVC):
#include <stdio.h>

int main() {
    char vendorBuffer[32]; // A buffer large enough for the vendor string + null terminator
    int result;

    printf("Attempting to retrieve CPU Vendor string...\n");

    // Test with a valid buffer
    result = GetCPUVendor(vendorBuffer, sizeof(vendorBuffer));
    if (result == 0) {
        printf("Successfully retrieved CPU Vendor: '%s'\n", vendorBuffer);
    } else {
        printf("Failed to retrieve CPU Vendor (Error code: %d)\n", result);
    }

    // Test with a buffer that is too small (e.g., 10 bytes, needs 13)
    char smallBuffer[10];
    printf("\nTesting with a small buffer (size %d):\n", sizeof(smallBuffer));
    result = GetCPUVendor(smallBuffer, sizeof(smallBuffer));
    if (result == 0) {
        printf("Unexpected success with small buffer: '%s'\n", smallBuffer);
    } else {
        printf("Correctly failed with small buffer (Error code: %d). Buffer content: '%s'\n", result, smallBuffer);
    }

    // Test with a NULL buffer
    printf("\nTesting with a NULL buffer:\n");
    result = GetCPUVendor(NULL, sizeof(vendorBuffer));
    if (result == 0) {
        printf("Unexpected success with NULL buffer.\n");
    } else {
        printf("Correctly failed with NULL buffer (Error code: %d).\n", result);
    }

    // Test with zero length
    char zeroLenBuffer[1];
    printf("\nTesting with zero length (size %d):\n", 0);
    result = GetCPUVendor(zeroLenBuffer, 0);
    if (result == 0) {
        printf("Unexpected success with zero length.\n");
    } else {
        printf("Correctly failed with zero length (Error code: %d).\n", result);
    }
    
    // Test with 1-byte length buffer (too small, but out[0] should be '\0')
    printf("\nTesting with 1-byte length buffer:\n");
    result = GetCPUVendor(zeroLenBuffer, sizeof(zeroLenBuffer));
    if (result == 0) {
        printf("Unexpected success with 1-byte length.\n");
    } else {
        printf("Correctly failed with 1-byte length (Error code: %d). Buffer content: '%s'\n", result, zeroLenBuffer);
    }

    return 0;
}
*/