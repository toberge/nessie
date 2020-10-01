.PHONY: all run debug install install-local clean

all: nessie

nessie: nessie.c
	$(CC) nessie.c -o nessie \
	    -Wall -Wextra -pedantic -std=c99

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
