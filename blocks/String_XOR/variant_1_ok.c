#include <windows.h>

void String_XOR(char *string, char *key) {
    int string_len = strlen(string);
    int key_len = strlen(key);
    int i;

    for (i = 0; i < string_len; i++) {
        string[i] = string[i] ^ key[i % key_len];
    }
}