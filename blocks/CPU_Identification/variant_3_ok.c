#include <windows.h>
#include <intrin.h> // For __cpuid intrinsic
#include <string.h> // For strcmp

BOOL CPU_Identification() {
    int cpuInfo[4];
    char vendorId[13]; // 12 characters + null terminator

    // EAX = 0 retrieves the vendor ID string and the maximum basic CPUID input value.
    __cpuid(cpuInfo, 0);

    // The vendor ID string is returned in EBX, EDX, and ECX.
    // EBX (cpuInfo[1]) contains the first 4 characters.
    // EDX (cpuInfo[3]) contains the next 4 characters.
    // ECX (cpuInfo[2]) contains the last 4 characters.

    // Copy EBX part (first 4 bytes)
    *(int*)(vendorId + 0) = cpuInfo[1];
    // Copy EDX part (next 4 bytes)
    *(int*)(vendorId + 4) = cpuInfo[3];
    // Copy ECX part (last 4 bytes)
    *(int*)(vendorId + 8) = cpuInfo[2];
    vendorId[12] = '\0'; // Null-terminate the string

    // Compare with the known Intel vendor ID string
    if (strcmp(vendorId, "GenuineIntel") == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}