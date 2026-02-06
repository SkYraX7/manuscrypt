# manuscrypt

A symmetric file encryption tool using convolution-inspired keystream generation.

## Algorithm Overview

### How It Works

1. **Key Derivation**: The passphrase is hashed using SHA-256. The first 9 bytes of the hash form a 3×3 kernel matrix. The iteration count is derived from the sum of digit characters in the passphrase (clamped to 1-99).

2. **Keystream Generation**:  A 3×3 state matrix is initialized from the kernel and advanced using **2D circular convolution** with the kernel (mod 256). This operation is performed for each keystream byte:
   ```
   new_state[i][j] = Σ state[(i+m)%3][(j+n)%3] × kernel[m][n]  (mod 256)
   ```
   The center element of the state is output as the keystream byte. A counter is XORed into the state to prevent linear periodicity.

3. **Encryption**: Data is processed in 64 KB blocks. For each iteration:
   - XOR every byte with the keystream (generated fresh for each iteration/block)
   - Apply a deterministic byte permutation (Fisher-Yates shuffle seeded from the key)

4. **Decryption**: Iterations are reversed (last to first). For each block:
   - Apply inverse permutation
   - XOR with the same keystream (self-inverse operation)

### Encrypted File Format

```
[encrypted data: L bytes]
[original file length: 4 bytes, big-endian uint32]
[iteration count: 3 bytes ASCII, e.g. "042"]
[extension length: 1 byte]
[extension: variable length string]
```

Total overhead: 8 + extension_length bytes (minimum 8 bytes if no extension). The `.msencrptd` extension is used for encrypted files, and the original file extension is preserved in the metadata for restoration upon decryption.

## Why Convolutions Are Lossy (And How We Fixed It)

A standard convolution computes `output[i] = Σ input[i+k] × kernel[k]` — a many-to-one mapping that destroys information. This project solves this by using convolution operations only to **generate a pseudorandom keystream**, while the actual encryption uses XOR (which is perfectly reversible). This preserves the mathematical elegance of convolution-based diffusion while being cryptographically correct.

## Build Instructions

### Prerequisites

- **MinGW** (or any C compiler supporting C11)
- **OpenSSL** development libraries (for SHA-256)

**Windows users with MSYS2/MinGW64**: Install OpenSSL via pacman:
```bash
pacman -S mingw-w64-x86_64-openssl
```

For standalone MinGW installations, OpenSSL is typically at `C:\Program Files\OpenSSL-Win64`. Update `OPENSSL_DIR` in the Makefile if needed.

### Compile

**On Windows** - use the provided build script (auto-detects MinGW):
```bash
.\build.bat
```

**On Unix-like systems** - use make:
```bash
make
```

Or compile manually with gcc:

```bash
gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c keygen.c
gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c keystream.c
gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c cipher.c
gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c fileio.c
gcc -Wall -Wextra -O2 -std=c11 -I"C:/Program Files/OpenSSL-Win64/include" -c main.c
gcc *.o -o manuscrypt.exe -L"C:/Program Files/OpenSSL-Win64/lib" -lssl -lcrypto
```

This produces `manuscrypt.exe` (or `manuscrypt` on Unix-like systems).

## Usage

Run the interactive menu:

```bash
./manuscrypt
```

### Example Session

```
manuscrypt - convolution-based symmetric encryption
===================================================

1. Encrypt file
2. Decrypt file
0. Exit
Choice: 1
File path: sample.txt
Passphrase: hello123
Derived key: 6 iterations
Read 42 bytes. Encrypting with 6 iteration(s)...
Success! Encrypted file: sample.msencrptd
Original file deleted: sample.txt
```

To decrypt:

```
Choice: 2
File path: sample.msencrptd
Passphrase: hello123
Encrypted data: 42 bytes, iterations: 6. Decrypting...
Success! Decrypted file: sample.txt
Encrypted file deleted: sample.msencrptd
```

**Note**: Encryption and decryption operations are in-place - the original file is automatically deleted after successful encryption, and the encrypted file is deleted after successful decryption. The original filename and extension are fully restored.

### Notes

- **Iteration count** is derived from the sum of digits in your passphrase. For example, `"hello123"` → 1+2+3 = 6 iterations.
- Filenames with spaces are not supported by the current CLI (scanf limitation).
- Maximum file size: 4 GB (uint32_t limit).

## Scalability

| File Size | Memory Usage | Time Complexity |
|-----------|--------------|-----------------|
| Any size  | ~320 KB      | O(iterations × file_size) |

Files are processed in 64 KB blocks, so even gigabyte-sized files require minimal memory (just one block + permutation array = ~320 KB working memory).

## Security Notes

⚠️ **This is an educational project, not production-grade encryption.**

- The keystream generator (linear recurrence mod 256) is not cryptographically strong
- XOR-based stream ciphers are vulnerable to known-plaintext attacks
- No authentication (no HMAC/MAC to detect tampering)
- The permutation step uses xorshift32, which is not a CSPRNG

**For real security, use proven algorithms like AES-GCM or ChaCha20-Poly1305.**

This project is valuable for learning about:
- How convolution operations can be used in cryptography
- Why reversibility matters in encryption design
- Block cipher design principles (substitution + permutation)
- Keystream generation and deterministic key derivation

## License

GNU General Public License v3.0 (see [LICENSE](LICENSE))