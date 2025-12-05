#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

void Run_From_Memory(void *shellcode, int size) {
    DWORD oldProtect;
    void (*func)();

    VirtualProtect(shellcode, size, PAGE_EXECUTE_READWRITE, &oldProtect);

    func = (void (*)())shellcode;
    func();

    VirtualProtect(shellcode, size, oldProtect, &oldProtect);
}