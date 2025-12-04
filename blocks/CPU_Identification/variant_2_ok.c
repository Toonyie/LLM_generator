#include <windows.h>
#include <intrin.h> // For __cpuid

// Function prototype: BOOL CPU_Identification();
// Identifies if running under an Intel CPU or not
BOOL CPU_Identification() {
    int cpuInfo[4]; // EAX, EBX, ECX, EDX
    char vendorID[13]; // "GenuineIntel" + null terminator

    // EAX=0: Get vendor ID string
    __cpuid(cpuInfo, 0);

    // Copy EBX (cpuInfo[1])
    *(int*)&vendorID[0] = cpuInfo[1];
    // Copy EDX (cpuInfo[3])
    *(int*)&vendorID[4] = cpuInfo[3];
    // Copy ECX (cpuInfo[2])
    *(int*)&vendorID[8] = cpuInfo[2];
    vendorID[12] = '\0'; // Null-terminate the string

    // Compare with Intel's vendor ID
    if (lstrcmpA(vendorID, "GenuineIntel") == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}