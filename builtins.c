#include "nessie.h"
#include <stdio.h>
#include <unistd.h>

/**
 * cd builtin - just a wrapper around chdir()
 *
 * @returns non-zero on failure, zero on success
 */
int cd(char **argv, int argc) {
    if (argc == 2) {
        return -chdir(argv[1]);
    } else {
        printf("cd: Invalid number of arguments\n");
        return 1;
    }
}

/**
 * exit builtin - exits nessie
 *
 * @returns NESSIE_EXIT_SUCCESS to indicate intent
 */
int nessie_exit(char **argv, int argc) {
    return NESSIE_EXIT_SUCCESS;
}
