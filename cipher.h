#ifndef CIPHER_H
#define CIPHER_H

#include <stdint.h>
#include "keygen.h"

#define BLOCK_SIZE 65536  /* 64 KB blocks for memory-efficient processing */

/* Encrypt data in-place. Supports up to 4GB files (uint32_t limit). */
void encrypt(uint8_t *data, uint32_t data_len, const EncryptionKey *key);

/* Decrypt data in-place. Must use same key as encryption. */
void decrypt(uint8_t *data, uint32_t data_len, const EncryptionKey *key);

#endif
