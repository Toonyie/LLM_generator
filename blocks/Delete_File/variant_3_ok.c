#include <windows.h>

void Delete_File(char *filename) {
    DeleteFileA(filename);
}