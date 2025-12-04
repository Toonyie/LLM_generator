#include <windows.h> // For BOOL, TRUE, FALSE
#include <string.h>  // For strcmp

// MinGW-w64 typically provides __cpuid via intrin.h for MSVC compatibility.
// If not found, __cpuid can also be found in <x86intrin.h> or implemented via inline assembly.
#include <intrin.h>  // For __cpuid intrinsic

BOOL CPU_Identification() {
    int cpuInfo[4];   // EAX, EBX, ECX, EDX registers
    char vendorID[13]; // "GenuineIntel\0" (12 characters + null terminator)

    // Call CPUID with EAX=0 to get the vendor ID string
    // The __cpuid intrinsic stores EAX, EBX, ECX, EDX into cpuInfo[0-3] respectively.
    __cpuid(cpuInfo, 0);

    // The vendor ID string is returned in EBX, EDX, ECX.
    // Order of concatenation: EBX (bits 0-31), EDX (bits 32-63), ECX (bits 64-95).
    // cpuInfo[1] = EBX
    // cpuInfo[3] = EDX
    // cpuInfo[2] = ECX

    // Copy the registers into the char array
    // Note: Using memcpy or direct pointer assignment is typical.
    // Ensure byte order is correct for the string.
    *(int*)(vendorID + 0) = cpuInfo[1]; // EBX (first 4 chars)
    *(int*)(vendorID + 4) = cpuInfo[3]; // EDX (middle 4 chars)
    *(int*)(vendorID + 8) = cpuInfo[2]; // ECX (last 4 chars)
    vendorID[12] = '\0'; // Null-terminate the string

    // Compare the extracted vendor ID with "GenuineIntel"
    if (strcmp(vendorID, "GenuineIntel") == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}