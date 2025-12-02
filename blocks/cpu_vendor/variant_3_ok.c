#include <string.h> // Required for memcpy
#include <intrin.h> // Required for the __cpuid intrinsic

/**
 * @brief Retrieves the CPU vendor string.
 *
 * This function uses the CPUID instruction (via the __cpuid intrinsic)
 * to obtain the CPU vendor ID string, such as "GenuineIntel" or "AuthenticAMD".
 *
 * @param out A pointer to a character array where the vendor string will be stored.
 * @param outLen The maximum size of the output buffer, including space for the null terminator.
 *               The vendor string is 12 characters long. A buffer of at least 13 bytes
 *               (12 for the string + 1 for null terminator) is required for success.
 * @return 0 on success, -1 on failure. Failure occurs if:
 *         - 'out' is NULL.
 *         - 'outLen' is zero or negative.
 *         - 'outLen' is too small to hold the entire 12-character vendor string
 *           plus a null terminator (i.e., less than 13 bytes).
 */
int GetCPUVendor(char *out, int outLen) {
    // Check for invalid input buffer or length
    if (out == NULL || outLen <= 0) {
        return -1; // Failure: invalid buffer or length
    }

    // The CPU vendor string is exactly 12 characters long (e.g., "GenuineIntel").
    // We need 12 characters for the string + 1 for the null terminator.
    const int VENDOR_STRING_LENGTH = 12;
    const int REQUIRED_BUFFER_SIZE = VENDOR_STRING_LENGTH + 1;

    // Check if the provided buffer is large enough to store the full string
    // plus its null terminator.
    if (outLen < REQUIRED_BUFFER_SIZE) {
        // Buffer is too small. To ensure 'out' is always a valid string
        // (even if empty) in case of an error where outLen > 0, we null-terminate it.
        out[0] = '\0';
        return -1; // Failure: buffer too small
    }

    // Array to store the CPUID results.
    // cpuInfo[0] will contain EAX, cpuInfo[1] EBX, cpuInfo[2] ECX, cpuInfo[3] EDX.
    int cpuInfo[4];

    // Call the CPUID instruction with EAX=0.
    // This function ID is used to obtain the highest basic CPUID function
    // supported by the processor, and also the vendor ID string.
    // The vendor string is returned in EBX, EDX, and ECX.
    __cpuid(cpuInfo, 0);

    // The vendor string is composed of 12 characters, stored across EBX, EDX, and ECX.
    // The order is specific: EBX (first 4 chars), EDX (middle 4 chars), ECX (last 4 chars).

    // Copy the first 4 bytes (from EBX)
    memcpy(out, &cpuInfo[1], 4);

    // Copy the next 4 bytes (from EDX)
    memcpy(out + 4, &cpuInfo[3], 4);

    // Copy the last 4 bytes (from ECX)
    memcpy(out + 8, &cpuInfo[2], 4);

    // Null-terminate the string to make it a valid C string.
    out[VENDOR_STRING_LENGTH] = '\0';

    return 0; // Success
}