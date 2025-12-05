#include <windows.h>
#include <stdio.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef NTSTATUS (NTAPI *RtlNtStatusToDosError)(NTSTATUS Status);
typedef NTSTATUS (NTAPI *NtQuerySystemInformation)(
    UINT SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    WORD  ProcessorArchitecture;
    WORD  ProcessorLevel;
    WORD  ProcessorRevision;
    BYTE  Reserved;
    DWORD  ProcessorFeatureBits;
    DWORD  Reserved1;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemNotImplemented1,
    SystemProcessesAndThreadsInformation,
    SystemCallCounts,
    SystemConfigurationInformation,
    SystemProcessorPerformanceInformation,
    SystemGlobalFlagInformation,
    SystemNotImplemented2,
    SystemModuleInformation,
    SystemLockInformation,
    SystemNotImplemented3,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemQuotaInformation,
    SystemOverloadCounterInformation,
    SystemPplmInformation,
} SYSTEM_INFORMATION_CLASS;


BOOL CPU_Identification() {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll == NULL) {
        return FALSE;
    }

    NtQuerySystemInformation NtQuerySystemInformationFunc = (NtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    if (NtQuerySystemInformationFunc == NULL) {
        return FALSE;
    }

    SYSTEM_PROCESSOR_INFORMATION processorInfo;
    NTSTATUS status = NtQuerySystemInformationFunc(
        SystemProcessorInformation,
        &processorInfo,
        sizeof(SYSTEM_PROCESSOR_INFORMATION),
        NULL
    );

    if (status != 0) {
      return FALSE;
    }
    
    if (processorInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        return TRUE;
    } else {
        return FALSE;
    }
}