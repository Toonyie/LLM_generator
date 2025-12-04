#include <stddef.h> // For size_t
#include <string.h> // For strlen

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return;
    }

    size_t key_len = strlen(key);
    if (key_len == 0) {
        return; // Cannot XOR with an empty key
    }

    size_t string_idx = 0;
    size_t key_idx = 0;

    while (string[string_idx] != '\0') {
        string[string_idx] ^= key[key_idx];
        string_idx++;
        key_idx++;
        if (key_idx >= key_len) {
            key_idx = 0; // Wrap around the key
        }
    }
}