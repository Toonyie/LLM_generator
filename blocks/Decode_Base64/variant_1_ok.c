#include <windows.h>

#ifndef NTAPI
#define NTAPI __stdcall
#endif

#ifndef UNICODE
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR  Buffer;
} STRING, *PSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#endif

int
Decode_Base64(void *encoded, int size, void *decoded)
{
    DWORD decoded_size = 0;

    if (!encoded || !decoded || size <= 0) {
        return 0;
    }

    if (!CryptStringToBinaryA(encoded, size, CRYPT_STRING_BASE64, decoded, &decoded_size, 0, NULL)) {
        return 0;
    }

    return (int)decoded_size;
}