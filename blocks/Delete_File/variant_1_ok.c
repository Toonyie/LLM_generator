#include <windows.h>
#include <stdio.h>

void Delete_File(char *filename) {
    if (DeleteFileA(filename) == 0) {
        DWORD error = GetLastError();
        fprintf(stderr, "Error deleting file: %s, Error code: %lu\n", filename, error);
    } else {
        printf("File deleted successfully: %s\n", filename);
    }
}