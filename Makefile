CFLAGS = -O2 -Wall -Wextra -pedantic -std=c99

.PHONY: all man test fuzz run debug install install-local clean

all: nessie nessie.1

man: nessie.1

nessie.1: man.md
	pandoc man.md --standalone --to=man -o nessie.1

nessie: nessie.c nessie.h lexer.c builtins.c parser.c executor.c
	$(CC) *.c -o nessie $(CFLAGS)

fuzz: lexer.c tests/*.c
	for test in tests/*.c; do\
	    clang "$$test" lexer.c -o "$${test%.c}" \
	          $(CFLAGS) -fsanitize=address,fuzzer; \
	    ./"$${test%.c}" -max_total_time=120 -detect_leaks=0 && \
	    ./"$${test%.c}" -max_total_time=120; \
	done

test: nessie
	bats tests/nessie.bats

run: nessie
	./nessie

debug: nessie
	./nessie --debug

install: nessie nessie.1
	cp -f nessie /usr/local/bin
	mkdir -p /usr/local/man/man1 # (man1 might not be present)
	cp nessie.1 /usr/local/man/man1/nessie.1
	gzip /usr/local/man/man1/nessie.1

install-local:
	cp -f nessie ~/.local/bin

clean:
	rm -f nessie nessie.1
