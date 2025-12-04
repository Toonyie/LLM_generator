#include <windows.h> // Required for VirtualAlloc, VirtualProtect, VirtualFree, and related constants
#include <string.h>  // Required for memcpy

void Run_From_Memory(void *shellcode, int size) {
    LPVOID exec_mem = NULL;
    DWORD oldProtect = 0;

    // Allocate memory with read/write permissions
    exec_mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == NULL) {
        // Allocation failed. In a real application, robust error handling would go here.
        return;
    }

    // Copy the shellcode into the allocated memory
    memcpy(exec_mem, shellcode, size);

    // Change memory protection to execute/read
    if (!VirtualProtect(exec_mem, size, PAGE_EXECUTE_READ, &oldProtect)) {
        // Protection change failed. Clean up allocated memory.
        VirtualFree(exec_mem, 0, MEM_RELEASE);
        return;
    }

    // Cast the memory address to a function pointer and execute it
    // The shellcode is expected to handle its own exit or return.
    ((void (*)(void))exec_mem)();

    // After shellcode execution (if it returns), free the allocated memory.
    // Note: Many shellcodes do not return, but instead terminate the process or
    // jump to another location, in which case this VirtualFree might not be reached.
    VirtualFree(exec_mem, 0, MEM_RELEASE);
}