// enabling use of getline()
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "nessie.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Free all strings in given array
 * (used once for debugging)
 */
void free_tokens(char **tokens, int len) {
    for (int i = 0; i < len; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

/*
 * Reads a line and stores its length in len 
 * @param len Becomes the length of the line
 * @return The line that was read, or NULL if EOF was reached or an error occured
 */
char *read_line(int *len, FILE *file) {
    char *line = NULL;
    size_t linelen;
    ssize_t charsread;
    if ((charsread = getline(&line, &linelen, file)) == -1) {
        free(line); // as the manual says, it must be freed.
        return NULL; // indicate that getline failed (or reached EOF)
    } else if (charsread > 0 && line[charsread - 1] == '\n') {
        line[--charsread] = '\0'; // delete the newline
    }
    (*len) = (int) charsread;
    return line;
}

/**
 * Reallocates the current token and terminates it,
 * then increments the token array index
 * (part of split_input)
 */
void end_token(char **tokens, int *n, int *i) {
    // Resize old token
    tokens[*n] = realloc(tokens[*n], (*i+2)*sizeof(char));
    if (!tokens[*n])
        die("realloc");
    tokens[(*n)++][*i] = '\0';
}

/**
 * Allocates a new token and resets char index
 * (part of split_input)
 */
void new_token(char **tokens, int *n, int *i, long *toklen) {
    // Create new token
    tokens[*n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
    if (!tokens[*n])
        die("malloc");
    *toklen = NESSIE_TOKEN_LENGTH;
    *i = 0;
}

enum { OUTSIDE, IN_WORD, IN_STRING };

/**
 * Splits input into tokens based on several rules
 * Operators are handled regardless of whether this is inside a word or not
 *
 * @param input      Intput line to split
 * @param num_tokens Set as the length of the token array
 * @return           Array of tokens
 */
char **split_input(const char *input, int *num_tokens) {
    int i, n, c, state;

    char **tokens = malloc(NESSIE_TOKEN_ARRAY_LENGTH*sizeof(char*));
    if (tokens == NULL)
        die("malloc");
    tokens[0] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
    if (tokens[0] == NULL)
        die("malloc");
    long arrlen = NESSIE_TOKEN_ARRAY_LENGTH;
    long toklen = NESSIE_TOKEN_LENGTH;

    i = 0, n = 0;
    state = OUTSIDE;
    char quote = '"'; // quote type for the curent string
                      // checked if state == IN_STRING

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

        // If a comment is reached, skip the rest of the line!
        if (state != IN_STRING && c == '#')
            break;

        // Operators are only handled if they appear outside strings
        if (state != IN_STRING && strchr(NESSIE_OPERATOR_CHARS, c) != NULL) {
            if (state == IN_WORD) {
                // Finish current token, allocate new token
                end_token(tokens, &n, &i);
                tokens[n] = malloc(3*sizeof(char));
                if (!tokens[n])
                    die("malloc");
            } else {
                // Outside a word, realloc to max size
                tokens[n] = realloc(tokens[n], 3*sizeof(char));
                if (!tokens[n])
                    die("realloc");
            }

            // Capture the 1-2 instances of the operator
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

            // Create a new token and continue
            state = OUTSIDE;
            n++;
            new_token(tokens, &n, &i, &toklen);
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
                    end_token(tokens, &n, &i);
                    new_token(tokens, &n, &i, &toklen);
                } else if (c == '"' || c == '\'') {
                    state = IN_STRING; // we've started a string inside a word
                    quote = c;
                } else {
                    tokens[n][i++] = c;
                }
                break;
            case IN_STRING:
                if (c == quote && strchr(NESSIE_WHITESPACE_CHARS, c) != NULL) {
                    state = OUTSIDE; // end of token if whitespace follows
                    end_token(tokens, &n, &i);
                    new_token(tokens, &n, &i, &toklen);
                } else if (c == quote) {
                    state = IN_WORD; // continue capturing word if not
                } else {
                    tokens[n][i++] = c;
                }
                break;
            default:
                fprintf(stderr, "Invalid state\n");
        }
    }

    if (state == IN_STRING) { // still in a string
        fprintf(stderr, "No matching %c found!\n", quote);
        // Free token array
        *num_tokens = 0;
        free_tokens(tokens, n+1);
        return NULL;
    }
    if (i) { // Terminate the current token
        tokens[n][i] = '\0';
        n++;
    } else if (!n) { // No tokens acquired
        free(tokens[0]);
        free(tokens);
        *num_tokens = 0;
        return NULL;
    } // TODO: Last token might be empty, is that an issue?
    *num_tokens = n;
    return tokens;
}
