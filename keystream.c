#include "keystream.h"
#include <string.h>

void keystream_init(KeystreamState *ks, const EncryptionKey *key, int iteration, uint32_t block_index) {
    if (!ks || !key) return;

    /* Start with kernel copied into both state and kernel */
    memcpy(ks->kernel, key->W, sizeof(key->W));
    memcpy(ks->state, key->W, sizeof(key->W));

    /* Mix (iteration, block_index) into all 9 state bytes using an LCG */
    uint32_t mix = (uint32_t)iteration * 2654435761u + block_index * 340573321u;
    for (int i = 0; i < KERNEL_SIZE * KERNEL_SIZE; i++) {
        mix = mix * 1103515245u + 12345u;  /* LCG step */
        ks->state[i / KERNEL_SIZE][i % KERNEL_SIZE] ^= (uint8_t)((mix >> 16) & 0xFF);
    }

    ks->counter = 0;
}

uint8_t keystream_next(KeystreamState *ks) {
    if (!ks) return 0;

    /* Output: center element of state */
    uint8_t output = ks->state[1][1];

    /* Advance state via 2D circular convolution with kernel (mod 256) */
    uint8_t new_state[KERNEL_SIZE][KERNEL_SIZE];
    for (int i = 0; i < KERNEL_SIZE; i++) {
        for (int j = 0; j < KERNEL_SIZE; j++) {
            uint32_t sum = 0;
            for (int m = 0; m < KERNEL_SIZE; m++)
                for (int n = 0; n < KERNEL_SIZE; n++)
                    sum += (uint32_t)ks->state[(i + m) % KERNEL_SIZE][(j + n) % KERNEL_SIZE]
                         * (uint32_t)ks->kernel[m][n];
            new_state[i][j] = (uint8_t)(sum & 0xFF);
        }
    }

    /* Inject counter to break linear periodicity */
    new_state[0][0] ^= (uint8_t)(ks->counter & 0xFF);

    memcpy(ks->state, new_state, sizeof(new_state));
    ks->counter++;

    return output;
}
