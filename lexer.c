#include "nessie.h"

// enabling use of getline()
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

// Free all strings in given array
void free_tokens(char **tokens, int len) {
    for (int i = 0; i < len; i++)
        free(tokens[i]);
    free(tokens);
}

/* Reads a line and stores its length in len */
char *read_line(int *len) {
    char *line = NULL;
    size_t linelen;
    ssize_t charsread;
    if ((charsread = getline(&line, &linelen, stdin)) == -1) {
        free(line); // as the manual says, it must be freed.
        return NULL; // indicate that getline failed (or reached EOF)
    } else if (charsread > 0 && line[charsread - 1] == '\n') {
        line[--charsread] = '\0'; // delete the newline
    }
    (*len) = (int) charsread;
    return line;
}

enum { OUTSIDE, IN_WORD, IN_STRING };

char **split_input(const char *input, const int len, int *num_tokens) {
    // note: the len parameter will be useful for validation later.
    int i, n, c, state;

    char **tokens = malloc(NESSIE_TOKEN_ARRAY_LENGTH*sizeof(char*));
    tokens[0] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
    long arrlen = NESSIE_TOKEN_ARRAY_LENGTH;
    long toklen = NESSIE_TOKEN_LENGTH;

    i = 0, n = 0;
    state = OUTSIDE;
    while ((c = *input++) != '\0') {
        // Reallocate memory of array or string if full
        if (n >= arrlen) {
            arrlen += arrlen / 3;
            tokens = realloc(tokens, arrlen);
        }
        if (i >= toklen) {
            toklen += toklen / 3;
            tokens[n] = realloc(tokens[n], toklen);
        }

        switch (state) {
            case OUTSIDE:
                if (c == '"') {
                    state = IN_STRING;
                } else if (c != ' ') {
                    state = IN_WORD;
                    tokens[n][i++] = c;
                }
                break;
            case IN_WORD:
                if (c == ' ') {
                    state = OUTSIDE;
                    tokens[n] = realloc(tokens[n], (i+2)*sizeof(char));
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
                    toklen = NESSIE_TOKEN_LENGTH;
                    i = 0;
                } else {
                    tokens[n][i++] = c;
                }
                break;
            case IN_STRING:
                if (c == '"') { // TODO: check if whitespace follows?
                    state = OUTSIDE;
                    tokens[n] = realloc(tokens[n], (i+2)*sizeof(char));
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
                    toklen = NESSIE_TOKEN_LENGTH;
                    i = 0;
                } else {
                    tokens[n][i++] = c;
                }
                break;
            default:
                fprintf(stderr, "Invalid state\n");
        }
    }
    if (state == IN_STRING) {
        fprintf(stderr, "No matching \" found!\n");
        // Free token array
        *num_tokens = 0;
        free_tokens(tokens, n+1);
        return NULL;
    }
    if (i) {
        tokens[n][i] = '\0';
        n++;
    } else if (!n) { // no tokens acquired
        free(tokens[0]);
    }
    *num_tokens = n;
    return tokens;
}
