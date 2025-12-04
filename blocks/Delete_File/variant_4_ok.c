#include <windows.h>

void Delete_File(char *filename) {
    // DeleteFileA is used to delete a file specified by an ANSI (char*) string.
    // It returns TRUE on success, FALSE on failure.
    // As the function is declared void, we do not return its status.
    DeleteFileA(filename);
}