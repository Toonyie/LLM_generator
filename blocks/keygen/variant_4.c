#include <stdint.h> // For uint8_t
#include <stdlib.h> // For rand(), srand()
#include <time.h>   // For time()

void keygen(uint8_t *output_key_buffer, size_t key_byte_length) {
    static int generator_initialized_flag = 0;
    if (!generator_initialized_flag) {
        srand((unsigned int)time(NULL)); // Seed the random number generator once
        generator_initialized_flag = 1;
    }

    size_t byte_position = 0;
    if (key_byte_length > 0) {
        do {
            // Populate the buffer with random bytes using bitwise AND to ensure 0-255 range
            output_key_buffer[byte_position] = (uint8_t)(rand() & 0xFF);
            byte_position++;
        } while (byte_position < key_byte_length);
    }
}