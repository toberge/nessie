#include "nessie.h"
#include <stdlib.h>

// A NULL-terminated list of commands
const char **history = NULL;
size_t history_next_index = 0;
size_t history_len = 0;

/**
 * Initialize history store
 */
void history_init() {
    history = malloc(NESSIE_HISTORY_LENGTH*sizeof(char *));
    history[history_next_index] = NULL;
    history_len = NESSIE_HISTORY_LENGTH;
}

/**
 * Free up history store
 */
void history_free() {
    for (size_t i = 0; i < history_len; i++)
        free((char *)history[i]);
}

/**
 * Save a command to the history
 * 
 * @param line The line to save
 */
void history_save(const char *line) {
    // TODO: handle max size?
    if (history_next_index >= history_len) {
        history_len += history_len / 3;
        history = realloc(history, history_len*sizeof(char *));
    }
    history[history_next_index++] = line;
    history[history_next_index] = NULL;
}

/**
 * Fetches the current command history (for use in the builtin)
 *
 * @return a const pointer to the history list
 */
const char **history_get_all() {
    return (const char **)history;
}
