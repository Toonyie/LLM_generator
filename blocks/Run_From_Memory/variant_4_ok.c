#include <windows.h>
#include <string.h>

void Run_From_Memory(void *shellcode, int size) {
    LPVOID allocated_memory;
    HANDLE thread_handle;

    // Allocate memory with read, write, and execute permissions
    allocated_memory = VirtualAlloc(
        NULL,           // System determines where to allocate the region
        size,           // Size of the region
        MEM_COMMIT | MEM_RESERVE, // Allocate and reserve memory
        PAGE_EXECUTE_READWRITE    // Memory protection: Execute, Read, Write
    );

    if (allocated_memory == NULL) {
        // Failed to allocate memory
        return;
    }

    // Copy the shellcode into the allocated memory
    memcpy(allocated_memory, shellcode, size);

    // Create a new thread to execute the shellcode
    thread_handle = CreateThread(
        NULL,           // Default security attributes
        0,              // Default stack size
        (LPTHREAD_START_ROUTINE)allocated_memory, // Thread start address (shellcode entry point)
        NULL,           // No parameter to pass to the thread
        0,              // Creation flags (0 = run immediately)
        NULL            // Don't care about the thread ID
    );

    if (thread_handle == NULL) {
        // Failed to create thread, free allocated memory
        VirtualFree(allocated_memory, 0, MEM_RELEASE);
        return;
    }

    // Close the thread handle. The shellcode will continue to execute in its own thread.
    // If the caller needs to wait for the shellcode to finish,
    // they would typically use WaitForSingleObject on the handle before closing.
    CloseHandle(thread_handle);
}