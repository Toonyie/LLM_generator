#include <windows.h>
#include <stdio.h>

void Delete_File(char *filename) {
  if (DeleteFileA(filename)) {
    // Optionally handle success (e.g., log it)
  } else {
    // Optionally handle error
    DWORD error = GetLastError();
     fprintf(stderr, "Error deleting file: %lu\n", error);
  }
}