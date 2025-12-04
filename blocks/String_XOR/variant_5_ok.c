#include <string.h>

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return;
    }

    size_t string_len = strlen(string);
    size_t key_len = strlen(key);

    if (key_len == 0) {
        return; // Cannot XOR with an empty key
    }

    for (size_t i = 0; i < string_len; i++) {
        string[i] ^= key[i % key_len];
    }
}