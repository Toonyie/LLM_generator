// Code in C
// Code for Windows
// Use the Windows API or intrinsics

#include <intrin.h> // Required for the __cpuid intrinsic on MSVC
#include <string.h> // Required for memcpy

// Function prototype: int GetCPUVendor(char *out, int outLen);
//
// Fills 'out' with the CPU vendor string (e.g., "GenuineIntel", "AuthenticAMD").
// The vendor string is typically 12 characters long.
//
// Parameters:
//   out:    A pointer to a character buffer where the vendor string will be stored.
//   outLen: The size of the 'out' buffer, including space for the null-terminator.
//
// Returns:
//   0 on success.
//  -1 on failure (e.g., 'out' is NULL or 'outLen' is too small/zero).
int GetCPUVendor(char *out, int outLen) {
    // 1. Validate input parameters.
    // 'out' must not be NULL, and 'outLen' must be at least 1 to accommodate
    // a null-terminator (even if no string content can fit).
    if (out == NULL || outLen <= 0) {
        return -1; // Invalid output buffer provided.
    }

    // 2. Declare an array to store the CPUID results.
    // The __cpuid intrinsic fills this array:
    // cpuInfo[0] = EAX
    // cpuInfo[1] = EBX
    // cpuInfo[2] = ECX
    // cpuInfo[3] = EDX
    int cpuInfo[4];

    // 3. Call the __cpuid intrinsic with EAX=0.
    // This CPUID leaf function (0x0) is specifically used to retrieve the
    // CPU vendor ID string and the maximum supported CPUID leaf function.
    // The 12-character vendor ID string is returned across EBX, EDX, and ECX.
    __cpuid(cpuInfo, 0);

    // 4. Assemble the 12-character vendor string from the CPUID results.
    // The specific order for the vendor string is:
    // First 4 characters from EBX (cpuInfo[1])
    // Next 4 characters from EDX (cpuInfo[3])
    // Last 4 characters from ECX (cpuInfo[2])
    char vendorString[13]; // Allocate space for 12 characters + 1 for null-terminator

    // Copy the contents of EBX to the first 4 bytes of vendorString
    memcpy(&vendorString[0], &cpuInfo[1], 4);
    // Copy the contents of EDX to the next 4 bytes
    memcpy(&vendorString[4], &cpuInfo[3], 4);
    // Copy the contents of ECX to the last 4 bytes
    memcpy(&vendorString[8], &cpuInfo[2], 4);

    // Null-terminate the temporary vendor string to ensure it's a valid C string.
    vendorString[12] = '\0';

    // 5. Copy the assembled string to the caller's buffer, respecting 'outLen'.
    // Determine how many characters can actually be copied. This must be
    // 'outLen - 1' at maximum to leave space for the null-terminator.
    int charsToCopy = 12; // The full length of the CPU vendor string.
    
    // If the provided output buffer is smaller than the full vendor string plus null-terminator,
    // truncate the string to fit.
    if (outLen - 1 < charsToCopy) {
        charsToCopy = outLen - 1;
    }

    // Only perform the copy if there's actually space for at least one character.
    if (charsToCopy > 0) {
        memcpy(out, vendorString, charsToCopy);
    }
    
    // Always null-terminate the output buffer at the correct position.
    // This ensures that 'out' is always a valid C string, even if empty or truncated.
    out[charsToCopy] = '\0';

    return 0; // Indicate success.
}

/*
// Example Usage (for testing purposes):
#include <stdio.h> // For printf

int main() {
    char vendor[32]; // Buffer to hold the vendor string

    printf("Attempting to get CPU vendor string...\n");

    int result = GetCPUVendor(vendor, sizeof(vendor));

    if (result == 0) {
        printf("CPU Vendor: %s\n", vendor);
    } else {
        printf("Failed to get CPU vendor string. Error code: %d\n", result);
    }

    // --- Test Cases for different buffer sizes ---

    // Test with a buffer just large enough for "GenuineIntel" + null (13 bytes)
    char vendor_exact[13];
    result = GetCPUVendor(vendor_exact, sizeof(vendor_exact));
    if (result == 0) {
        printf("Exact buffer test (13 bytes): %s\n", vendor_exact);
    } else {
        printf("Exact buffer test failed. Error code: %d\n", result);
    }

    // Test with a smaller buffer (e.g., 5 bytes to fit "Genu" + null)
    char vendor_small[5];
    result = GetCPUVendor(vendor_small, sizeof(vendor_small));
    if (result == 0) {
        printf("Small buffer test (5 bytes): %s\n", vendor_small); // Should print "Genu" or similar prefix
    } else {
        printf("Small buffer test failed. Error code: %d\n", result);
    }

    // Test with a buffer only for the null terminator (1 byte)
    char vendor_tiny[1];
    result = GetCPUVendor(vendor_tiny, sizeof(vendor_tiny));
    if (result == 0) {
        printf("Tiny buffer test (1 byte): '%s'\n", vendor_tiny); // Should print '' (empty string)
    } else {
        printf("Tiny buffer test failed. Error code: %d\n", result);
    }

    // --- Test Cases for error handling ---

    // Test with a NULL output buffer
    result = GetCPUVendor(NULL, 10);
    if (result == -1) {
        printf("NULL buffer test: Correctly failed.\n");
    } else {
        printf("NULL buffer test: Unexpectedly succeeded.\n");
    }

    // Test with a zero-length output buffer
    result = GetCPUVendor(vendor, 0);
    if (result == -1) {
        printf("Zero length buffer test: Correctly failed.\n");
    } else {
        printf("Zero length buffer test: Unexpectedly succeeded.\n");
    }

    return 0;
}
*/