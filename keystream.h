#ifndef KEYSTREAM_H
#define KEYSTREAM_H

#include <stdint.h>
#include "keygen.h"

typedef struct {
    uint8_t state[KERNEL_SIZE][KERNEL_SIZE];   /* 3x3 evolving state matrix */
    uint8_t kernel[KERNEL_SIZE][KERNEL_SIZE];  /* 3x3 kernel (constant) */
    uint32_t counter;                           /* byte counter for cycle breaking */
} KeystreamState;

/* Initialize keystream generator for a specific iteration and block */
void keystream_init(KeystreamState *ks, const EncryptionKey *key, int iteration, uint32_t block_index);

/* Generate next keystream byte */
uint8_t keystream_next(KeystreamState *ks);

#endif
