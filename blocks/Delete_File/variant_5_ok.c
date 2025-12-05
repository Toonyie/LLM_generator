#include <windows.h>
#include <stdio.h>

void Delete_File(char *filename) {
    if (DeleteFileA(filename)) {
        // File deleted successfully
    } else {
        // Handle the error if the file could not be deleted
        DWORD error = GetLastError();
        fprintf(stderr, "Error deleting file: %s, Error code: %lu\n", filename, error);
    }
}