#include <windows.h>

void String_XOR(char *string, char *key) {
    int string_length = 0;
    int key_length = 0;

    // Calculate string length
    while (string[string_length] != '\0') {
        string_length++;
    }

    // Calculate key length
    while (key[key_length] != '\0') {
        key_length++;
    }

    for (int i = 0; i < string_length; i++) {
        string[i] = string[i] ^ key[i % key_length];
    }
}