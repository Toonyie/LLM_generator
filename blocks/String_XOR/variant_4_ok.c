#include <string.h> // Required for strlen

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return; // Handle null pointers gracefully
    }

    size_t key_len = strlen(key);
    if (key_len == 0) {
        return; // Cannot XOR with an empty key
    }

    size_t key_idx = 0;
    while (*string != '\0') {
        *string ^= key[key_idx];
        string++;
        key_idx = (key_idx + 1) % key_len;
    }
}