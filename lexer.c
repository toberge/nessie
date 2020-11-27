#include "nessie.h"

// enabling use of getline()
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Free all strings in given array
void free_tokens(char **tokens, int len) {
    for (int i = 0; i < len; i++) {
        free(tokens[i]);
    }
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

char **split_input(const char *input, int *num_tokens) {
    // note: the len parameter will be useful for validation later.
    int i, n, c, state;

    char **tokens = malloc(NESSIE_TOKEN_ARRAY_LENGTH*sizeof(char*));
    if (tokens == NULL)
        die("malloc");
    tokens[0] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
    if (tokens[0] == NULL)
        die("malloc");
    long arrlen = NESSIE_TOKEN_ARRAY_LENGTH;
    long toklen = NESSIE_TOKEN_LENGTH;

    char quote = '"'; // quote type for this string

    i = 0, n = 0;
    state = OUTSIDE;
    while ((c = *input++) != '\0') {
        // Reallocate memory of array or string if full
        if (n+1 >= arrlen) {
            arrlen += arrlen / 3;
            tokens = realloc(tokens, arrlen);
            if (!tokens)
                die("realloc");
        }
        if (i+1 >= toklen) {
            toklen += toklen / 3;
            tokens[n] = realloc(tokens[n], toklen);
            if (!tokens[n])
                die("realloc");
        }

        // Crappy handling of operators
        if (state != IN_STRING && strchr(NESSIE_OPERATOR_CHARS, c) != NULL) {
            if (state == IN_WORD) {
                tokens[n] = realloc(tokens[n], (i+2)*sizeof(char));
                if (!tokens[n])
                    die("realloc");
                tokens[n++][i] = '\0';
                tokens[n] = malloc(3*sizeof(char));
                if (!tokens[n])
                    die("malloc");
            } else {
                tokens[n] = realloc(tokens[n], 3*sizeof(char));
                if (!tokens[n])
                    die("realloc");
            }

            tokens[n][0] = c;
            if (*input == c) {
                tokens[n][1] = c;
                tokens[n][2] = '\0';
                input++;
            } else {
                tokens[n] = realloc(tokens[n], 2*sizeof(char));
                if (!tokens[n])
                    die("realloc");
                tokens[n][1] = '\0';
            }

            // new token! TODO no code duplication
            tokens[++n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
            if (!tokens[n])
                die("malloc");
            toklen = NESSIE_TOKEN_LENGTH;
            state = OUTSIDE;
            i = 0;
            continue;
        }

        switch (state) {
            case OUTSIDE:
                if (c == '"' || c == '\'') {
                    state = IN_STRING;
                    quote = c;
                } else if (strchr(NESSIE_WHITESPACE_CHARS, c) == NULL) {
                    state = IN_WORD;
                    tokens[n][i++] = c;
                }
                break;
            case IN_WORD:
                if (strchr(NESSIE_WHITESPACE_CHARS, c) != NULL) {
                    state = OUTSIDE;
                    tokens[n] = realloc(tokens[n], (i+2)*sizeof(char));
                    if (!tokens[n])
                        die("realloc");
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
                    if (!tokens[n])
                        die("malloc");
                    toklen = NESSIE_TOKEN_LENGTH;
                    i = 0;
                } else {
                    tokens[n][i++] = c;
                }
                break;
            case IN_STRING:
                if (c == quote) { // TODO: check if whitespace follows?
                    state = OUTSIDE;
                    tokens[n] = realloc(tokens[n], (i+2)*sizeof(char));
                    if (!tokens[n])
                        die("realloc");
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
                    if (!tokens[n])
                        die("malloc");
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
        fprintf(stderr, "No matching %c found!\n", quote);
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
        free(tokens);
        *num_tokens = 0;
        return NULL;
    } // TODO: Last token might be empty, is that an issue?
    *num_tokens = n;
    return tokens;
}
