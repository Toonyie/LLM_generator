#include <windows.h>

#ifndef NTAPI
#define NTAPI __stdcall
#endif

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

int
Decode_Base64(void *encoded, int size, void *decoded)
{
    DWORD decoded_size = 0;
    if (!CryptStringToBinaryA(
            encoded,
            size,
            CRYPT_STRING_BASE64,
            decoded,
            &decoded_size,
            0,
            NULL))
    {
        return 0;
    }
    return (int)decoded_size;
}