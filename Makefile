CFLAGS := -O2 -Wall -Wextra -pedantic -std=c99

SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/*.c)
LIB := $(wildcard $(SRC_DIR)/*.h)
FUZZ_FILES := tests/setup.c $(filter-out $(SRC_DIR)/nessie.c, $(SRC))

.PHONY: all man test fuzz run debug install install-local clean

all: nessie nessie.1

man: nessie.1

nessie.1: man.md
	pandoc man.md --standalone --to=man -o nessie.1

nessie: $(LIB) $(SRC)
	$(CC) $(SRC) -o nessie $(CFLAGS)

fuzz: $(SRC_DIR)/lexer.c tests/*.c
	for test in tests/*_fuzzer_test.c; do\
	    clang "$$test" $(FUZZ_FILES) -o "$${test%.c}" \
	          $(CFLAGS) -fsanitize=address,fuzzer; \
	    ./"$${test%.c}" -max_total_time=120 -detect_leaks=0 && \
	    ./"$${test%.c}" -max_total_time=120; \
	done

test: nessie
	bats tests/nessie.bats

run: nessie
	@./nessie

debug: nessie
	@./nessie --debug

install: nessie nessie.1
	cp -f nessie /usr/local/bin
	mkdir -p /usr/local/man/man1 # (man1 might not be present)
	cp -f nessie.1 /usr/local/man/man1/nessie.1
	gzip -f /usr/local/man/man1/nessie.1

install-local: nessie
	cp -f nessie ~/.local/bin

install-ci: nessie
	# This does not require pandoc
	cp -f nessie /usr/local/bin

clean:
	rm -f nessie nessie.1
