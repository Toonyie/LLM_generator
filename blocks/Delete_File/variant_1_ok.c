#include <windows.h>

void Delete_File(char *filename) {
    // DeleteFileA is used for ANSI (char*) strings.
    // It returns a non-zero value if the function succeeds, and zero otherwise.
    // Since the function prototype is void, we just call it.
    DeleteFileA(filename);
}