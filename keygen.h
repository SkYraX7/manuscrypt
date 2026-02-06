#ifndef KEYGEN_H
#define KEYGEN_H

#include <stdint.h>

#define KERNEL_SIZE  3
#define MAX_ITER     99
#define MIN_ITER     1

typedef struct {
    uint8_t W[KERNEL_SIZE][KERNEL_SIZE];  /* 3x3 kernel from passphrase hash */
    int iterations;                        /* number of encryption passes */
} EncryptionKey;

/* Derive encryption key from passphrase. Returns 0 on success, -1 on error. */
int derive_key(const char *passphrase, EncryptionKey *key);

#endif
