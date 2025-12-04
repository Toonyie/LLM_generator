#include <windows.h> // For VirtualAlloc, VirtualFree
#include <string.h>  // For memcpy

void Run_From_Memory(void *shellcode, int size) {
    LPVOID allocated_memory;

    // Allocate executable memory.
    // MEM_COMMIT | MEM_RESERVE allocates and commits pages.
    // PAGE_EXECUTE_READWRITE allows code execution, reading, and writing to the memory region.
    allocated_memory = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (allocated_memory == NULL) {
        // In a production scenario, error handling (e.g., logging, throwing an exception)
        // would be performed here if memory allocation fails.
        return;
    }

    // Copy the provided shellcode into the newly allocated executable memory.
    memcpy(allocated_memory, shellcode, size);

    // Execute the shellcode.
    // The memory address is cast to a function pointer that takes no arguments and returns void,
    // then the function is called.
    ((void(*)())allocated_memory)();

    // Free the allocated memory.
    // This line will only be reached if the shellcode execution returns control to the caller.
    // Many shellcodes might terminate the current process or transition control elsewhere
    // without returning, in which case this memory will be cleaned up by the OS upon process exit.
    VirtualFree(allocated_memory, 0, MEM_RELEASE);
}