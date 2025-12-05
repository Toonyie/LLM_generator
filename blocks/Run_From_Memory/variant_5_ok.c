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
    PVOID DefaultUiLanguage;
    PVOID OriginalBase;
    ULONG DllCharacteristics;
    ULONG CheckSum;
    ULONG ImageType;
    PVOID ImageAddressMode;
    ULONG PreferredBase;
    PVOID DataDirectory;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef struct _PEB_LDR_DATA {
    BYTE Length;
    BYTE Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BYTE ShutdownInProgress;
    PVOID ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;


typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Ldr;
    PVOID ProcessParameters;
    PVOID Reserved3[3];
    PVOID AtlThunkSListPtr;
    PVOID Reserved4;
    PVOID Reserved5[47];
    PVOID PostProcessInitRoutine;
    PVOID Reserved6[128];
    PVOID Reserved7;
    ULONG SessionId;
} PEB, *PPEB;

void Run_From_Memory(void *shellcode, int size) {
    DWORD oldProtect;
    SIZE_T shellcode_size = (SIZE_T)size;

    if (shellcode == NULL || size <= 0) {
        return;
    }

    if (!VirtualProtect(shellcode, shellcode_size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return;
    }

    ((void (*)())shellcode)();

    VirtualProtect(shellcode, shellcode_size, oldProtect, &oldProtect);
}