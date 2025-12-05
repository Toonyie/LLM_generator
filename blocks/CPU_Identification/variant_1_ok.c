#include <windows.h>
#include <stdio.h>

// Define UNICODE_STRING if not available (MinGW)
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
  WORD  wReserved1;
  WORD  wReserved2;
  DWORD dwReserved3;
  DWORD dwReserved4;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

BOOL CPU_Identification() {
  INT CPUInfo[4] = {0};
  __cpuid(CPUInfo, 0);

  if (CPUInfo[0] >= 1) {
    __cpuid(CPUInfo, 1);
  } else {
    return FALSE;
  }

  char Vendor[13] = {0};
  __cpuid(CPUInfo, 0);
  memcpy(Vendor + 0, &CPUInfo[1], 4);
  memcpy(Vendor + 4, &CPUInfo[3], 4);
  memcpy(Vendor + 8, &CPUInfo[2], 4);

  if (strcmp(Vendor, "GenuineIntel") == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}