To get the CPU vendor string on Windows using intrinsics, we use the `__cpuid` intrinsic. The `CPUID` instruction with `EAX=0` returns the vendor ID string across the `EBX`, `EDX`, and `ECX` registers.

The vendor string is 12 characters long, composed in the order `EBX`, `EDX`, `ECX`.

```c
#include <intrin.h> // Required for the __cpuid intrinsic
#include <string.h> // Required for memcpy and memset

// Function prototype: int GetCPUVendor(char *out, int outLen);
// Fills 'out' with the CPU vendor string (e.g., "GenuineIntel", "AuthenticAMD").
//
// Parameters:
//   out: Pointer to a character buffer where the vendor string will be stored.
//   outLen: The size of the 'out' buffer in bytes.
//
// Returns:
//   0 on success.
//   -1 on failure (e.g., 'out' is NULL or 'outLen' is too small).
int GetCPUVendor(char *out, int outLen) {
    // The CPU vendor string is always 12 characters long (e.g., "GenuineIntel").
    // To store it as a null-terminated C string, we need 12 characters + 1
    // for the null terminator, totaling 13 bytes.
    if (out == NULL || outLen < 13) {
        // If the output buffer is invalid or too small, return failure.
        // It's good practice to clear the buffer if possible to prevent stale data.
        if (out != NULL && outLen > 0) {
            memset(out, 0, outLen);
        }
        return -1;
    }

    // Array to store the results from the CPUID instruction.
    // __cpuid fills this array as [EAX, EBX, ECX, EDX].
    int cpuInfo[4];

    // Execute the CPUID instruction with EAX=0.
    // This leaf function is used to retrieve the vendor ID string and
    // the maximum input value for basic CPUID information.
    //
    // On return from __cpuid(cpuInfo, 0):
    // cpuInfo[0] (EAX register) = Maximum basic CPUID leaf supported
    // cpuInfo[1] (EBX register) = First 4 characters of the vendor ID string
    // cpuInfo[2] (ECX register) = Last 4 characters of the vendor ID string
    // cpuInfo[3] (EDX register) = Middle 4 characters of the vendor ID string
    __cpuid(cpuInfo, 0);

    // The 12-character vendor string is assembled from EBX, EDX, and ECX
    // in that specific order.
    // Example: For "GenuineIntel":
    //   EBX holds "Genu"
    //   EDX holds "ineI"
    //   ECX holds "ntel"

    // Copy the first 4 characters (from EBX, which is cpuInfo[1])
    memcpy(&out[0], &cpuInfo[1], 4);
    // Copy the middle 4 characters (from EDX, which is cpuInfo[3])
    memcpy(&out[4], &cpuInfo[3], 4);
    // Copy the last 4 characters (from ECX, which is cpuInfo[2])
    memcpy(&out[8], &cpuInfo[2], 4);

    // Null-terminate the string to ensure it's a valid C string.
    out[12] = '\0';

    return 0; // Indicate success.
}
```