#include <string.h> // Required for strlen

void String_XOR(char *string, char *key) {
    if (string == NULL || key == NULL) {
        return;
    }

    size_t key_len = strlen(key);
    if (key_len == 0) {
        // Cannot XOR with an empty key.
        return;
    }

    size_t i = 0;
    while (string[i] != '\0') {
        string[i] = string[i] ^ key[i % key_len];
        i++;
    }
}