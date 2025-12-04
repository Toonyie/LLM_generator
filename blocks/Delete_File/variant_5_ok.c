#include <windows.h>

void Delete_File(char *filename) {
    // DeleteFileA is used for ANSI strings (char*)
    // DeleteFileW would be used for Unicode strings (wchar_t*)
    DeleteFileA(filename);
}