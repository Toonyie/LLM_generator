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
    PVOID DllCharacteristics;
    PVOID DllCheckSum;
    PVOID LoadedImports;
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
    PVOID ContextInformation;
    PVOID OriginalBase;
    LARGE_INTEGER LoadTime;
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
    BYTE InheritedAddressSpace;
    BYTE ReadImageFileExecOptions;
    BYTE BeingDebugged;
    BYTE SpareBool;
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PVOID ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    ULONG CrossProcessFlags;
    ULONG UserSharedData;
    ULONG SpareUlong;
    ULONG nNumberOfProcessors;
    LARGE_INTEGER AlignedMask;
    union {
        ULONG MemoryManagementFlags;
        struct {
            ULONG LowFragmentation : 1;
            ULONG HardenedVa : 1;
        };
    };
    ULONG ImageBaseAddressSpaceUsage;
    ULONG ImageBaseAddressSpaceUsageMask;
    ULONG AppCompatInfo;
    ULARGE_INTEGER CSDVersion;
    PVOID ActivationContextData;
    PVOID ProcessAssemblyStorageMap;
    PVOID SystemDependencies;
    PVOID SxsContextList;
    PVOID ShimData;
    PVOID AppCompatFlags;
    PVOID AppCompatFlagsUser;
    PVOID pShimInfo;
    ULONG AppCompatFlagsProcess;
    ULONG AppVerifierFlags;
    PVOID AppVerifierFlagsUser;
    PVOID pfnAllocateUserPhysicalPages;
    PVOID pfnFreeUserPhysicalPages;
    PVOID pPhysicalVad;
    ULONG pProtectedProcesses;
    PVOID SessionId;
    ULARGE_INTEGER AppCompatCacheFlags;
    ULONG AppCompatFlagsUserReg;
    ULONG SpareUlong2;
    ULARGE_INTEGER TxnIsolationLevel;
    ULONG TxnIsolationFlags;
    ULONG SpareUlong3;
    ULONG SkipPatchingAlso;
    PVOID ShimEngine;
    PVOID MaxVcrRuntime;
    ULONG FilterBitmap;
    ULONG ActivityId;
    PVOID pHeapReplacementFunction;
    DWORD dwHotpatchGeneration;
    PVOID pContextData;
    PVOID pReserved[2];
    PVOID* pInstrumentationCallbackSpares;
    ULONG pInstrumentationFunction;
} PEB, *PPEB;

#ifdef _WIN64
#define PEB_OFFSET 0x60
#else
#define PEB_OFFSET 0x30
#endif

void Run_From_Memory(void *shellcode, int size) {
  DWORD oldProtect;
  void (*func)();

  VirtualProtect(shellcode, size, PAGE_EXECUTE_READWRITE, &oldProtect);

  func = (void (*)())shellcode;
  func();
}