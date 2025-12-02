#include <windows.h> // Common header for Windows applications (though not strictly needed for __cpuid)
#include <intrin.h>  // Required for the __cpuid intrinsic
#include <string.h>  // Required for memcpy

/**
 * @brief Retrieves the CPU vendor string.
 *
 * This function uses the CPUID instruction to get the CPU vendor identification string.
 * The vendor string is typically 12 characters long (e.g., "GenuineIntel", "AuthenticAMD").
 *
 * @param out Pointer to a character buffer where the vendor string will be stored.
 * @param outLen The size of the output buffer in bytes, including space for the null terminator.
 *               Must be at least 13 bytes (12 characters + 1 null terminator).
 * @return 0 on success, -1 on failure (e.g., invalid buffer, insufficient buffer size).
 */
int GetCPUVendor(char *out, int outLen) {
    // 1. Validate input parameters
    if (out == NULL || outLen <= 0) {
        return -1; // Invalid output buffer pointer or length
    }

    // The CPU vendor string is always 12 characters long (e.g., "GenuineIntel").
    // We need at least 12 characters plus 1 null terminator, so 13 bytes total.
    if (outLen < 13) {
        // Buffer is too small to hold the 12-character vendor string + null terminator
        return -1;
    }

    // 2. Declare an array to hold the results from the CPUID instruction
    // __cpuid fills this array with the values of EAX, EBX, ECX, EDX registers
    // in that order: cpuInfo[0]=EAX, cpuInfo[1]=EBX, cpuInfo[2]=ECX, cpuInfo[3]=EDX.
    int cpuInfo[4]; 

    // 3. Call the __cpuid intrinsic with EAX=0
    // This specific CPUID function (EAX=0) returns:
    //   - EAX: The highest basic CPUID function number supported.
    //   - EBX, EDX, ECX: The 12-character vendor ID string.
    //     The vendor string is composed of EBX (first 4 chars), EDX (middle 4 chars),
    //     and ECX (last 4 chars) in that order.
    __cpuid(cpuInfo, 0);

    // 4. Copy the vendor string components into the output buffer
    // The vendor string is constructed by concatenating EBX, EDX, and then ECX.
    memcpy(&out[0], &cpuInfo[1], 4); // Copy EBX (first 4 bytes)
    memcpy(&out[4], &cpuInfo[3], 4); // Copy EDX (next 4 bytes)
    memcpy(&out[8], &cpuInfo[2], 4); // Copy ECX (last 4 bytes)

    // 5. Null-terminate the string
    out[12] = '\0';

    return 0; // Success
}

/*
// Example Usage (for testing purposes, compile with a main function):
#include <stdio.h>

int main() {
    char vendorString[20]; // Buffer large enough for "AuthenticAMD" + null
    int result = GetCPUVendor(vendorString, sizeof(vendorString));

    if (result == 0) {
        printf("CPU Vendor: %s\n", vendorString);
    } else {
        printf("Failed to get CPU vendor string. Error code: %d\n", result);
    }

    // Test with insufficient buffer size
    char smallBuffer[5];
    result = GetCPUVendor(smallBuffer, sizeof(smallBuffer));
    if (result == -1) {
        printf("Test with small buffer failed as expected.\n");
    }

    // Test with NULL buffer
    result = GetCPUVendor(NULL, 20);
    if (result == -1) {
        printf("Test with NULL buffer failed as expected.\n");
    }

    return 0;
}
*/