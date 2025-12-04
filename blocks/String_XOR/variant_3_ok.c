#include <stddef.h> // For size_t
#include <string.h> // For strlen

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return; // Handle NULL pointers gracefully
    }

    size_t key_len = strlen(key);
    if (key_len == 0) {
        return; // Nothing to XOR with if key is empty
    }

    size_t key_idx = 0;
    while (*string != '\0') {
        *string ^= key[key_idx];
        key_idx = (key_idx + 1) % key_len;
        string++;
    }
}