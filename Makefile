# manuscrypt - convolution-based symmetric encryption
# Configure OpenSSL path if needed (default for MinGW on Windows)
OPENSSL_DIR ?= C:/Program Files/OpenSSL-Win64

SRC     = main.c keygen.c keystream.c cipher.c fileio.c
OBJ     = $(SRC:.c=.o)
PROG    = manuscrypt
CFLAGS  = -Wall -Wextra -O2 -std=c11 -I"$(OPENSSL_DIR)/include"
LFLAGS  = -L"$(OPENSSL_DIR)/lib" -lssl -lcrypto

$(PROG): $(OBJ)
	gcc $(OBJ) -o $(PROG) $(LFLAGS)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROG) $(PROG).exe

.PHONY: clean
