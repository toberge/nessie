#include "nessie.h"
#include <stdio.h>
#include <unistd.h>

/**
 * cd builtin - just a wrapper around chdir()
 *
 * @return nonzero on failure, zero on success
 */
int cd(char **argv, int argc) {
    if (argc == 2) {
        if (chdir(argv[1]) == -1) {
            perror("cd");
            return 1;
        } else {
            return 0;
        }
    } else {
        printf("cd: Invalid number of arguments\n");
        return 1;
    }
}

/**
 * exit builtin - exits nessie
 *
 * @return NESSIE_EXIT_SUCCESS to indicate intent
 */
int nessie_exit(__attribute__((unused)) char **argv, __attribute__((unused)) int argc) {
    return NESSIE_EXIT_SUCCESS;
}

/**
 * help builtin - shows some helpful message
 *
 * @return NESSIE_EXIT_SUCCESS to indicate intent
 */
int nessie_help(__attribute__((unused)) char **argv, __attribute__((unused)) int argc) {
    printf("ne[sh]ie – the absurdly stupid shell\n\n");
    printf("Type a command and press enter,\n");
    printf("or run \033[1mnessie -c 'command'\033[m.\n\n");
    printf("For more information, see \033[1mman nessie\033[0m.\n");
    return 0;
}

/**
 * history builtin - displays the current command history
 *
 * If there is no history, nothing will be displayed.
 *
 * @return 0
 */
int nessie_history(__attribute__((unused)) char **argv, __attribute__((unused)) int argc) {
    const char **history = history_get_all();
    if (history == NULL)
        return 0;
    while (*history != NULL) {
        printf("%s\n", *history);
        history++;
    }
    return 0;
}

/**
 * true builtin - returns true
 *
 * @return 0 to indicate success/truth
 */
int nessie_true(__attribute__((unused)) char **argv, __attribute__((unused)) int argc) {
    return 0;
}

/**
 * false builtin - returns false
 *
 * @return 1 to indicate failure/falsehood
 */
int nessie_false(__attribute__((unused)) char **argv, __attribute__((unused)) int argc) {
    return 1;
}
