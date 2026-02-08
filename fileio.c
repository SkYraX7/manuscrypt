#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *read_file(const char *path, uint32_t *out_len) {
    if (!path || !out_len) return NULL; 

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        return NULL;
    }

    uint8_t *buf = (uint8_t *)malloc((size_t)size);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return NULL;
    }

    fclose(f);
    *out_len = (uint32_t)size;
    return buf;
}

int write_file(const char *path, const uint8_t *data, uint32_t len) {
    if (!path || !data) return 0;

    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    int ok = (fwrite(data, 1, len, f) == len);
    fclose(f);
    return ok;
}

int write_encrypted(const char *path, const uint8_t *data, uint32_t data_len,
                    uint32_t orig_len, int iterations, const char *extension) {
    if (!path || !data) return 0;

    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    /* Write encrypted data */
    if (fwrite(data, 1, data_len, f) != data_len) {
        fclose(f);
        return 0;
    }

    /* Append original length as 4 bytes big-endian */
    uint8_t hdr[4] = {
        (uint8_t)((orig_len >> 24) & 0xFF),
        (uint8_t)((orig_len >> 16) & 0xFF),
        (uint8_t)((orig_len >>  8) & 0xFF),
        (uint8_t)( orig_len        & 0xFF)
    };
    fwrite(hdr, 1, 4, f);

    /* Append iteration count as 3 ASCII digits (zero-padded) */
    char iter[4];
    snprintf(iter, sizeof(iter), "%03d", iterations);
    fwrite(iter, 1, 3, f);

    /* Append extension: extension string + length (1 byte) - length goes LAST for easy reading */
    size_t ext_strlen = extension ? strlen(extension) : 0;
    uint8_t ext_len = (ext_strlen > 255) ? 255 : (uint8_t)ext_strlen;  /* Clamp to 255 */
    if (ext_len > 0) {
        fwrite(extension, 1, ext_len, f);
    }
    fwrite(&ext_len, 1, 1, f);  /* Write length last so we can read it from a fixed position */

    fclose(f);
    return 1;
}

uint8_t *read_encrypted(const char *path, uint32_t *out_data_len,
                        uint32_t *out_orig_len, int *out_iterations,
                        char *out_extension, size_t ext_buf_size) {
    if (!path || !out_data_len || !out_orig_len || !out_iterations || !out_extension)
        return NULL;

    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long total = ftell(f);

    /* Minimum valid encrypted file: 1 byte data + 4 (len) + 3 (iter) + 1 (ext_len) = 9 bytes */
    if (total < 9) {
        fclose(f);
        return NULL;
    }

    /* Read extension length (last byte) */
    fseek(f, total - 1, SEEK_SET);
    uint8_t ext_len;
    fread(&ext_len, 1, 1, f);

    /* Read extension if present */
    if (ext_len > 0 && ext_len < ext_buf_size) {
        fseek(f, total - 1 - ext_len, SEEK_SET);
        fread(out_extension, 1, ext_len, f);
        out_extension[ext_len] = '\0';
    } else {
        out_extension[0] = '\0';
    }

    /* Read iteration count (3 bytes before extension) */
    fseek(f, total - 1 - ext_len - 3, SEEK_SET);
    char iter_str[4] = {0};
    fread(iter_str, 1, 3, f);
    *out_iterations = atoi(iter_str);

    /* Read original length (4 bytes before iteration count) */
    fseek(f, total - 1 - ext_len - 3 - 4, SEEK_SET);
    uint8_t hdr[4];
    fread(hdr, 1, 4, f);
    *out_orig_len = ((uint32_t)hdr[0] << 24) | ((uint32_t)hdr[1] << 16) |
                    ((uint32_t)hdr[2] <<  8) |  (uint32_t)hdr[3];

    /* Data length = total - (4 + 3 + 1 + ext_len) */
    uint32_t trailer_size = 4 + 3 + 1 + ext_len;
    uint32_t data_len = (uint32_t)(total - trailer_size);
    *out_data_len = data_len;

    /* Read encrypted data */
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc(data_len);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, data_len, f) != data_len) {
        free(buf);
        fclose(f);
        return NULL;
    }

    fclose(f);
    return buf;
}

void encrypted_name(const char *in, char *out_buf, size_t buf_size) {
    if (!in || !out_buf || buf_size == 0) return;

    /* Replace extension with .msencrptd, or append if no extension */
    const char *dot = strrchr(in, '.');
    int base_len = dot ? (int)(dot - in) : (int)strlen(in);
    snprintf(out_buf, buf_size, "%.*s.msencrptd", base_len, in);
}

void decrypted_name(const char *in, const char *extension, char *out_buf, size_t buf_size) {
    if (!in || !out_buf || buf_size == 0) return;

    /* Strip .msencrptd and append the original extension */
    const char *ext = strstr(in, ".msencrptd");
    if (ext) {
        /* Found .msencrptd - strip it and add original extension */
        int base_len = (int)(ext - in);
        if (extension && extension[0] != '\0') {
            snprintf(out_buf, buf_size, "%.*s%s", base_len, in, extension);
        } else {
            snprintf(out_buf, buf_size, "%.*s", base_len, in);
        }
    } else {
        /* No .msencrptd extension - just copy as is */
        snprintf(out_buf, buf_size, "%s", in);
    }
}
