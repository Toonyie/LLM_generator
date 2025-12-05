#include <windows.h>
#include <stdio.h>

void Delete_File(char *filename) {
  if (DeleteFileA(filename)) {
    // Optionally, handle success
    // printf("File deleted successfully.\n");
  } else {
    // Optionally, handle failure
    DWORD error = GetLastError();
    //printf("Error deleting file: %lu\n", error);
  }
}