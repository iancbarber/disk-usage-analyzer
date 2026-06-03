CC      := cc
CFLAGS  := -std=c11 -Wall -Wextra -g
BIN     := dua
SRC     := src/main.c

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

# Build with AddressSanitizer to catch memory bugs (leaks, OOB, use-after-free)
.PHONY: asan
asan: $(SRC)
	$(CC) $(CFLAGS) -fsanitize=address -o $(BIN) $(SRC)

.PHONY: clean
clean:
	rm -f $(BIN) src/*.o
