#include <intrin.h> // Required for __cpuidex intrinsic
#include <string.h> // Required for memcpy and strncpy_s/strncpy

// Note: For Visual Studio, <windows.h> is often included but not strictly
// necessary for this particular function as it uses intrinsics, not direct WinAPI calls.

/**
 * @brief Retrieves the CPU vendor string.
 *
 * This function uses the CPUID instruction (via the __cpuidex intrinsic) to query
 * the CPU vendor string (e.g., "GenuineIntel", "AuthenticAMD").
 * The string is copied into the provided buffer, ensuring null-termination
 * and respecting the buffer's size.
 *
 * @param out Pointer to the character buffer where the vendor string will be stored.
 * @param outLen The maximum size of the output buffer, including the null terminator.
 *               Must be at least 1 to allow for a null terminator.
 * @return 0 on success, -1 on failure (e.g., out is NULL, outLen is too small).
 */
int GetCPUVendor(char *out, int outLen) {
    // Validate input buffer parameters
    if (out == NULL || outLen <= 0) {
        return -1; // Invalid output buffer or insufficient length
    }

    int cpuInfo[4]; // Array to hold the EAX, EBX, ECX, EDX registers' values
                    // cpuInfo[0] will store EAX
                    // cpuInfo[1] will store EBX
                    // cpuInfo[2] will store ECX
                    // cpuInfo[3] will store EDX

    // Call the CPUID instruction with EAX=0 and ECX=0 (subfunction for basic info).
    // This CPUID call returns the highest basic CPUID function supported (in EAX)
    // and the 12-character vendor ID string in EBX, EDX, ECX registers.
    // The order of the string parts is EBX, then EDX, then ECX.
    __cpuidex(cpuInfo, 0, 0);

    // The CPU vendor string is always 12 characters long.
    // We need a temporary buffer of 13 bytes to hold the 12 characters plus a null terminator.
    char vendorString[13]; 

    // Copy the contents of EBX (cpuInfo[1]) to the first 4 bytes of vendorString.
    memcpy(&vendorString[0], &cpuInfo[1], 4); 
    // Copy the contents of EDX (cpuInfo[3]) to the next 4 bytes.
    memcpy(&vendorString[4], &cpuInfo[3], 4);
    // Copy the contents of ECX (cpuInfo[2]) to the last 4 bytes.
    memcpy(&vendorString[8], &cpuInfo[2], 4);
    
    // Explicitly null-terminate the temporary vendor string buffer.
    vendorString[12] = '\0'; 

    // Copy the collected vendor string into the user-provided output buffer.
    // We use platform-specific (compiler-specific) string copy functions for safety and compatibility.
#ifdef _MSC_VER
    // For Microsoft Visual C++ compilers, use the secure CRT function strncpy_s.
    // The _TRUNCATE argument ensures that if the source string is too long,
    // it will be truncated and the destination buffer will always be null-terminated.
    strncpy_s(out, outLen, vendorString, _TRUNCATE);
#else
    // For other compilers (e.g., GCC/Clang on Windows via MinGW),
    // use the standard strncpy and manually ensure null-termination.
    // strncpy does not guarantee null-termination if the source string is larger
    // than or equal to the specified copy length.
    strncpy(out, vendorString, outLen - 1);
    // Always null-terminate the output buffer to prevent reading garbage or buffer overflows.
    // This is safe because outLen is guaranteed to be >= 1 by the initial check.
    out[outLen - 1] = '\0';
#endif

    return 0; // Indicate success
}