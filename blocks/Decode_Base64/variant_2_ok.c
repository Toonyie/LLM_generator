#include <windows.h>

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

int
Decode_Base64(void *encoded, int size, void *decoded)
{
    DWORD decoded_size = 0;
    BOOL result = CryptStringToBinaryA(
        encoded,
        size,
        CRYPT_STRING_BASE64,
        decoded,
        &decoded_size,
        0,
        NULL
    );

    if (!result) {
        return -1; // Indicate failure
    }

    return (int)decoded_size;
}