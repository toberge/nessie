CFLAGS = -Wall -Wextra -pedantic -std=c99

.PHONY: all test run debug install install-local clean

all: nessie

nessie: nessie.c nessie.h lexer.c builtins.c executor.c
	$(CC) *.c -o nessie $(CFLAGS)

install: nessie
	cp -f nessie /usr/local/bin

install-local:
	cp -f nessie ~/.local/bin

clean:
	rm -f nessie

run: nessie
	./nessie

debug: nessie
	./nessie --debug
