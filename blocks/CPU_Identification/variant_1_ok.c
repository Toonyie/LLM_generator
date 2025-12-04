#include <windows.h> // For BOOL, TRUE, FALSE
#include <string.h>  // For memcmp

// __cpuid intrinsic for GCC (MinGW-w64) and MSVC compatibility
#ifdef __GNUC__
#include <stdint.h> // For uint32_t
// Define a wrapper for __cpuid using inline assembly for GCC/Clang
static inline void __cpuid(int cpuInfo[4], int infoType) {
    // EAX = infoType, ECX = 0
    // Output: EAX (cpuInfo[0]), EBX (cpuInfo[1]), ECX (cpuInfo[2]), EDX (cpuInfo[3])
    __asm__ __volatile__ (
        "cpuid\n"
        : "=a" (cpuInfo[0]), "=b" (cpuInfo[1]), "=c" (cpuInfo[2]), "=d" (cpuInfo[3])
        : "a" (infoType), "c" (0) // Input EAX = infoType, ECX = 0
    );
}
#elif _MSC_VER
#include <intrin.h> // For __cpuid intrinsic in MSVC
#endif

BOOL CPU_Identification() {
    int cpuInfo[4];       // Array to store EAX, EBX, ECX, EDX registers
    char vendorID[13];    // Buffer for 12-character vendor ID string + null terminator
    const char intelVendorString[] = "GenuineIntel"; // Expected Intel vendor string

    // Call CPUID with leaf 0 to get the vendor ID string
    // The vendor ID is returned in EBX, EDX, ECX (in that order)
    __cpuid(cpuInfo, 0);

    // Copy the vendor ID parts into the vendorID buffer
    // cpuInfo[1] holds EBX
    // cpuInfo[3] holds EDX
    // cpuInfo[2] holds ECX
    memcpy(vendorID, &cpuInfo[1], 4);     // Copy EBX to vendorID[0-3]
    memcpy(vendorID + 4, &cpuInfo[3], 4); // Copy EDX to vendorID[4-7]
    memcpy(vendorID + 8, &cpuInfo[2], 4); // Copy ECX to vendorID[8-11]
    vendorID[12] = '\0';                  // Null-terminate the string

    // Compare the extracted vendor string with "GenuineIntel"
    if (memcmp(vendorID, intelVendorString, 12) == 0) {
        return TRUE; // It's an Intel CPU
    } else {
        return FALSE; // It's not an Intel CPU
    }
}