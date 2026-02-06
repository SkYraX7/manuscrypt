#include "cipher.h"
#include "keystream.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ===== xorshift32 PRNG for permutation generation ===== */
static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/* Derive permutation seed from key, iteration, and block index */
static uint32_t perm_seed(const EncryptionKey *key, int iteration, uint32_t block_index) {
    uint32_t seed = 0;
    /* Hash the kernel values */
    for (int i = 0; i < KERNEL_SIZE; i++)
        for (int j = 0; j < KERNEL_SIZE; j++)
            seed = seed * 31u + key->W[i][j];
    /* Mix in iteration and block index with different multipliers */
    seed ^= (uint32_t)iteration * 2246822519u;
    seed ^= block_index * 3266489917u;
    return seed ? seed : 1;  /* xorshift requires non-zero state */
}

/* Fisher-Yates shuffle to generate permutation array: perm[i] = destination of byte i */
static void gen_permutation(uint32_t *perm, uint32_t len, uint32_t seed) {
    uint32_t rng = seed;
    for (uint32_t i = 0; i < len; i++)
        perm[i] = i;
    for (uint32_t i = len - 1; i > 0; i--) {
        uint32_t j = xorshift32(&rng) % (i + 1);
        uint32_t tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
}

/* Apply permutation: out[perm[i]] = in[i]  (scatter operation) */
static void apply_perm(uint8_t *out, const uint8_t *in, const uint32_t *perm, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        out[perm[i]] = in[i];
}

/* Apply inverse permutation: out[i] = in[perm[i]]  (gather operation) */
static void apply_inv_perm(uint8_t *out, const uint8_t *in, const uint32_t *perm, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        out[i] = in[perm[i]];
}

/* ===== Block-level encryption ===== */
static void encrypt_block(uint8_t *block, uint32_t len, const EncryptionKey *key, int iter, uint32_t bidx) {
    if (!block || !key || len == 0) return; 

    /* Step 1: XOR with keystream */
    KeystreamState ks;
    keystream_init(&ks, key, iter, bidx);
    for (uint32_t i = 0; i < len; i++)
        block[i] ^= keystream_next(&ks);

    /* Step 2: Apply deterministic byte permutation */
    uint32_t *perm = (uint32_t *)malloc(len * sizeof(uint32_t));
    uint8_t  *temp = (uint8_t  *)malloc(len);
    if (!perm || !temp) {
        fprintf(stderr, "Error: memory allocation failed in encrypt_block\n");
        free(perm);
        free(temp);
        return;
    }

    gen_permutation(perm, len, perm_seed(key, iter, bidx));
    memcpy(temp, block, len);
    apply_perm(block, temp, perm, len);

    free(perm);
    free(temp);
}

/* ===== Block-level decryption ===== */
static void decrypt_block(uint8_t *block, uint32_t len, const EncryptionKey *key, int iter, uint32_t bidx) {
    if (!block || !key || len == 0) return;

    /* Step 1: Apply inverse permutation */
    uint32_t *perm = (uint32_t *)malloc(len * sizeof(uint32_t));
    uint8_t  *temp = (uint8_t  *)malloc(len);
    if (!perm || !temp) {
        fprintf(stderr, "Error: memory allocation failed in decrypt_block\n");
        free(perm);
        free(temp);
        return;
    }

    gen_permutation(perm, len, perm_seed(key, iter, bidx));
    memcpy(temp, block, len);
    apply_inv_perm(block, temp, perm, len);

    free(perm);
    free(temp);

    /* Step 2: XOR with keystream (self-inverse) */
    KeystreamState ks;
    keystream_init(&ks, key, iter, bidx);
    for (uint32_t i = 0; i < len; i++)
        block[i] ^= keystream_next(&ks);
}

/* ===== Full file encryption ===== */
void encrypt(uint8_t *data, uint32_t data_len, const EncryptionKey *key) {
    if (!data || !key || data_len == 0) return;

    uint32_t num_blocks = (data_len + BLOCK_SIZE - 1) / BLOCK_SIZE;

    for (int iter = 1; iter <= key->iterations; iter++) {
        for (uint32_t b = 0; b < num_blocks; b++) {
            uint32_t offset = b * BLOCK_SIZE;
            uint32_t blen = data_len - offset;
            if (blen > BLOCK_SIZE)
                blen = BLOCK_SIZE;
            encrypt_block(data + offset, blen, key, iter, b);
        }
    }
}

/* ===== Full file decryption ===== */
void decrypt(uint8_t *data, uint32_t data_len, const EncryptionKey *key) {
    if (!data || !key || data_len == 0) return;

    uint32_t num_blocks = (data_len + BLOCK_SIZE - 1) / BLOCK_SIZE;

    /* Process iterations in reverse order */
    for (int iter = key->iterations; iter >= 1; iter--) {
        for (uint32_t b = 0; b < num_blocks; b++) {
            uint32_t offset = b * BLOCK_SIZE;
            uint32_t blen = data_len - offset;
            if (blen > BLOCK_SIZE)
                blen = BLOCK_SIZE;
            decrypt_block(data + offset, blen, key, iter, b);
        }
    }
}
