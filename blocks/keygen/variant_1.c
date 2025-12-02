#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    uint64_t private_part_a;
    uint64_t private_part_b;
    uint64_t public_part_x;
    uint64_t public_part_y;
} KeyPair;

int keygen_generate_keypair(KeyPair *key_pair_out) {
    if (key_pair_out == NULL) {
        return -1;
    }

    srand((unsigned int)time(NULL));

    key_pair_out->private_part_a = ((uint64_t)rand() << 32) | rand();
    key_pair_out->private_part_b = ((uint64_t)rand() << 32) | rand();

    key_pair_out->public_part_x = (key_pair_out->private_part_a * 3 + 7) % 0xFFFFFFFFFFFFFFFFULL;
    key_pair_out->public_part_y = (key_pair_out->private_part_b * 5 - 11) % 0xFFFFFFFFFFFFFFFFULL;

    return 0;
}