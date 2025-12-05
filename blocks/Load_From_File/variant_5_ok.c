#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

int Load_From_File(void *file_name, void *buffer) {
  HANDLE hFile = INVALID_HANDLE_VALUE;
  DWORD fileSize = 0;
  DWORD bytesRead = 0;

  // Ensure inputs are not NULL
  if (file_name == NULL || buffer == NULL) {
    return -1; // Indicate error
  }

  // Convert file_name to wide string if needed
  wchar_t wFileName[MAX_PATH];
  MultiByteToWideChar(CP_UTF8, 0, (char*)file_name, -1, wFileName, MAX_PATH);


  hFile = CreateFileW(wFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return -2; // Indicate file open error
  }

  fileSize = GetFileSize(hFile, NULL);
  if (fileSize == INVALID_FILE_SIZE) {
    CloseHandle(hFile);
    return -3; // Indicate file size error
  }

  if (fileSize == 0) {
    CloseHandle(hFile);
    return 0; //Empty file
  }

  if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
    CloseHandle(hFile);
    return -4; // Indicate read error
  }

  CloseHandle(hFile);

  if (bytesRead != fileSize) {
    return -5; //Indicate that not all file bytes were read
  }

  return (int)fileSize;
}