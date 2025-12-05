#include <windows.h>
#include <stdio.h>

int Load_From_File(void *file_name, void *buffer) {
  HANDLE hFile;
  DWORD fileSize;
  DWORD bytesRead;

  hFile = CreateFile((LPCTSTR)file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    return 0; // Error opening file
  }

  fileSize = GetFileSize(hFile, NULL);

  if (fileSize == INVALID_FILE_SIZE) {
    CloseHandle(hFile);
    return 0; // Error getting file size
  }

  if (buffer == NULL) {
      CloseHandle(hFile);
      return fileSize;
  }
  
  if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
    CloseHandle(hFile);
    return 0; // Error reading file
  }

  CloseHandle(hFile);

  if (bytesRead != fileSize) {
      return 0;
  }

  return (int)fileSize;
}