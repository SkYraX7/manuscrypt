#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keygen.h"
#include "cipher.h"
#include "fileio.h"

#define MAX_PATH 512
#define MAX_PASS 256

int main(void) {
    char filename[MAX_PATH], passphrase[MAX_PASS], outfile[MAX_PATH];
    int choice;

    printf("manuscrypt - convolution-based symmetric encryption\n");
    printf("===================================================\n");

    while (1) {
        printf("\n1. Encrypt file\n");
        printf("2. Decrypt file\n");
        printf("0. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) break;

        if (choice == 0) break;

        if (choice != 1 && choice != 2) {
            printf("Invalid choice.\n");
            continue;
        }

        /* NOTE: scanf with %%s does not handle filenames with spaces.
           For simplicity, we keep this limitation. */
        printf("File path: ");
        if (scanf("%511s", filename) != 1) break;

        printf("Passphrase: ");
        if (scanf("%255s", passphrase) != 1) break;

        /* Derive encryption key from passphrase */
        EncryptionKey key;
        if (derive_key(passphrase, &key) != 0) {
            printf("Error: invalid passphrase (empty or null).\n");
            continue;
        }

        printf("Derived key: %d iterations\n", key.iterations);

        if (choice == 1) {
            /* ===== Encrypt ===== */
            if (strstr(filename, ".msencrptd")) {
                printf("File already has .msencrptd extension.\n");
                continue;
            }

            /* Extract original extension to preserve it */
            const char *ext = strrchr(filename, '.');
            char extension[32] = {0};
            if (ext) {
                snprintf(extension, sizeof(extension), "%s", ext);
            }

            uint32_t flen;
            uint8_t *data = read_file(filename, &flen);
            if (!data) {
                printf("Error: cannot read file '%s'.\n", filename);
                continue;
            }

            printf("Read %u bytes. Encrypting with %d iteration(s)...\n", flen, key.iterations);
            encrypt(data, flen, &key);

            encrypted_name(filename, outfile, sizeof(outfile));
            if (write_encrypted(outfile, data, flen, flen, key.iterations, extension)) {
                printf("Success! Encrypted file: %s\n", outfile);

                /* Delete original file after successful encryption */
                if (remove(filename) == 0) {
                    printf("Original file deleted: %s\n", filename);
                } else {
                    printf("Warning: could not delete original file '%s'.\n", filename);
                }
            } else {
                printf("Error: failed to write encrypted file '%s'.\n", outfile);
            }

            free(data);

        } else {
            /* ===== Decrypt ===== */
            if (!strstr(filename, ".msencrptd")) {
                printf("File does not have .msencrptd extension.\n");
                continue;
            }

            uint32_t data_len, orig_len;
            int stored_iter;
            char stored_extension[32];
            uint8_t *data = read_encrypted(filename, &data_len, &orig_len, &stored_iter,
                                          stored_extension, sizeof(stored_extension));
            if (!data) {
                printf("Error: cannot read encrypted file '%s'.\n", filename);
                continue;
            }

            /* Use iteration count from the file (not from passphrase derivation).
               The file stores the iteration count used during encryption. */
            key.iterations = stored_iter;

            printf("Encrypted data: %u bytes, iterations: %d. Decrypting...\n", data_len, stored_iter);
            decrypt(data, data_len, &key);

            decrypted_name(filename, stored_extension, outfile, sizeof(outfile));
            if (write_file(outfile, data, orig_len)) {
                printf("Success! Decrypted file: %s\n", outfile);

                /* Delete encrypted file after successful decryption */
                if (remove(filename) == 0) {
                    printf("Encrypted file deleted: %s\n", filename);
                } else {
                    printf("Warning: could not delete encrypted file '%s'.\n", filename);
                }
            } else {
                printf("Error: failed to write decrypted file '%s'.\n", outfile);
            }

            free(data);
        }
    }

    printf("\nGoodbye!\n");
    return 0;
}
