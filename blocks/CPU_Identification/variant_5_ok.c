#include <windows.h> // For BOOL, TRUE, FALSE
#include <string.h>  // For strcmp

BOOL CPU_Identification() {
    unsigned int eax, ebx, ecx, edx;
    char vendor_id[13]; // "GenuineIntel" + null terminator

    // Execute CPUID instruction with EAX=0 to get vendor ID
    __asm__ volatile (
        "cpuid"
        : "=a" (eax), // Output EAX value to 'eax' variable
          "=b" (ebx), // Output EBX value to 'ebx' variable
          "=c" (ecx), // Output ECX value to 'ecx' variable
          "=d" (edx)  // Output EDX value to 'edx' variable
        : "a" (0)     // Input EAX value is 0 (for vendor string)
    );

    // The vendor ID string is "GenuineIntel" stored in EBX, EDX, ECX
    // EBX contains bytes 0-3
    // EDX contains bytes 4-7
    // ECX contains bytes 8-11
    
    // Copy bytes from EBX (first 4 characters)
    vendor_id[0] = (char)(ebx & 0xFF);
    vendor_id[1] = (char)((ebx >> 8) & 0xFF);
    vendor_id[2] = (char)((ebx >> 16) & 0xFF);
    vendor_id[3] = (char)((ebx >> 24) & 0xFF);

    // Copy bytes from EDX (next 4 characters)
    vendor_id[4] = (char)(edx & 0xFF);
    vendor_id[5] = (char)((edx >> 8) & 0xFF);
    vendor_id[6] = (char)((edx >> 16) & 0xFF);
    vendor_id[7] = (char)((edx >> 24) & 0xFF);

    // Copy bytes from ECX (last 4 characters)
    vendor_id[8] = (char)(ecx & 0xFF);
    vendor_id[9] = (char)((ecx >> 8) & 0xFF);
    vendor_id[10] = (char)((ecx >> 16) & 0xFF);
    vendor_id[11] = (char)((ecx >> 24) & 0xFF);

    vendor_id[12] = '\0'; // Null-terminate the string

    // Compare the retrieved vendor ID with "GenuineIntel"
    if (strcmp(vendor_id, "GenuineIntel") == 0) {
        return TRUE; // Running on an Intel CPU
    } else {
        return FALSE; // Not running on an Intel CPU
    }
}