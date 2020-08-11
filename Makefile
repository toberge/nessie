.PHONY: all
all: nessie

nessie: nessie.c
	$(CC) nessie.c -o nessie \
	    -Wall -Wextra -pedantic -std=c99

.PHONY: install
install: nessie
	cp -f nessie ~/.local/bin
