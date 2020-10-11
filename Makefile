CFLAGS = -O2 -Wall -Wextra -pedantic -std=c99

.PHONY: all test fuzz run debug install install-local clean

all: nessie

nessie: nessie.c nessie.h lexer.c builtins.c parser.c executor.c
	$(CC) *.c -o nessie $(CFLAGS)

fuzz: lexer.c tests/*.c
	for test in tests/*.c; do\
	    clang "$$test" lexer.c -o "$${test%.c}" \
	          $(CFLAGS) -fsanitize=address,fuzzer; \
	    ./"$${test%.c}" -max_total_time=120; \
	done

test: nessie
	bats tests/nessie.bats

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
