#ifndef FILEIO_H
#define FILEIO_H

#include <stdint.h>
#include <stddef.h>

/* Read entire file into heap-allocated buffer. Caller must free. Returns NULL on error. */
uint8_t *read_file(const char *path, uint32_t *out_len);

/* Write buffer to file. Returns 1 on success, 0 on failure. */
int write_file(const char *path, const uint8_t *data, uint32_t len);

/* Write encrypted file with format: [data][orig_len: 4B BE][iterations: 3B ASCII][ext_len: 1B][ext] */
int write_encrypted(const char *path, const uint8_t *data, uint32_t data_len,
                    uint32_t orig_len, int iterations, const char *extension);

/* Read encrypted file, extracting data, original length, iterations, and extension.
   Returns allocated data buffer (caller must free), or NULL on error.
   Extension is written to out_extension buffer (size ext_buf_size). */
uint8_t *read_encrypted(const char *path, uint32_t *out_data_len,
                        uint32_t *out_orig_len, int *out_iterations,
                        char *out_extension, size_t ext_buf_size);

/* Generate .msencrptd filename from input. out_buf must be large enough (strlen(in) + 11). */
void encrypted_name(const char *in, char *out_buf, size_t buf_size);

/* Generate decrypted filename using stored extension. */
void decrypted_name(const char *in, const char *extension, char *out_buf, size_t buf_size);

#endif
