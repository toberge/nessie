#include <stdlib.h>
#include <string.h>
#include "nessie.h"

/**
 * Nessie Preprocessor
 * 
 * Handles expansion of variables and command substitution
 * (the latter is just around the corner)
 */

/**
 * Find and expand variables of the form $HEY_whatever
 *
 * @return expanded line – all variables replaced by their value
 */
char *expand_variables(char *line, int len, int *expanded_len) {
    int in_literal = 0;
    char *expanded = malloc(sizeof(char)*len);
    int exp_pos = 0;
    int exp_len = len;

    for (int i = 0; i < len;) {
        // Realloc if needed
        if (exp_pos >= exp_len - 1) {
            exp_len += exp_len / 3;
            expanded = realloc(expanded, exp_len*sizeof(char));
        }

        if (!in_literal && line[i] == '$') {
            // Find length of variable name
            size_t skip = strspn(&line[i+1], NESSIE_VARIABLE_CHARS);
            if (!skip) {
                // If no length... Go on. (TODO: report error better)
                fprintf(stderr, "Syntax error: $ without variable name\n");
                free(expanded);
                return NULL;
            }
            
            // Find variable and its value
            char *var = malloc(sizeof(char)*(skip+1));
            var = strncpy(var, &line[i+1], skip);
            var[skip] = '\0';
            char *value = getenv(var);
            if (O.debug) printf("%s = %s\n", var, value);
            free(var);

            // If there was no such variable, count it as the empty string
            if (!value) value = "";
            int value_len = strlen(value);

            // Extend buffer size based on value length
            if (exp_pos + value_len >= exp_len) {
                exp_len += max(value_len, exp_len / 3);
                expanded = realloc(expanded, exp_len*sizeof(char));
            }

            char *copied = strcpy(&expanded[exp_pos], value);
            if (!copied) die("wtf");
            i += 1 + skip;
            exp_pos += value_len;
        } else {
            if (line[i] == '\'') in_literal = !in_literal;
            expanded[exp_pos++] = line[i++];
        }
    }

    expanded[exp_pos] = '\0'; // finally, terminate
    *expanded_len = exp_pos; // length is UP TO the \0, mind you...
    if (O.debug) printf("Expanded to: %s\n", expanded);
    return expanded;
}
