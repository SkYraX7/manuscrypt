#include "keygen.h"
#include <openssl/sha.h>
#include <string.h>
#include <ctype.h>

int derive_key(const char *passphrase, EncryptionKey *key) {
    if (!passphrase || !key || passphrase[0] == '\0')
        return -1;

    /* SHA-256 hash of passphrase → first 9 bytes form the 3x3 kernel */
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)passphrase, strlen(passphrase), hash);

    for (int i = 0; i < KERNEL_SIZE; i++)
        for (int j = 0; j < KERNEL_SIZE; j++)
            key->W[i][j] = hash[i * KERNEL_SIZE + j];

    /* Iteration count = sum of digit characters in passphrase, clamped to [1, 99] */
    int digit_sum = 0;
    for (int i = 0; passphrase[i]; i++)
        if (isdigit((unsigned char)passphrase[i]))
            digit_sum += passphrase[i] - '0';

    key->iterations = (digit_sum < MIN_ITER) ? MIN_ITER :
                      (digit_sum > MAX_ITER) ? MAX_ITER : digit_sum;

    return 0;
}
