#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

BOOL CPU_Identification() {
  INT cpuInfo[4];
  __cpuid(cpuInfo, 0);

  char vendor[13];
  memset(vendor, 0, sizeof(vendor));
  memcpy(vendor, &cpuInfo[1], 4);
  memcpy(vendor + 4, &cpuInfo[3], 4);
  memcpy(vendor + 8, &cpuInfo[2], 4);

  if (strcmp(vendor, "GenuineIntel") == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}