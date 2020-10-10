#include "nessie.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Second attempt at writing a shell,
 * now with more C knowledge (chapters 1,4,5,6 in K&R + kilo.c tutorial)
 */

// TODO: move this tiny function
void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    char **tokens;
    int count, status = 0;

    char *cwd;
    char *cwd_buffer = (char*) malloc((size_t)PATH_SIZE);

    int len = 0;
    char *line = NULL;

    printf("Welcome to ne[sh]ie, the absurdly stupid shell!\n");

    while (status >= 0) {
        // Display prompt
        cwd = getcwd(cwd_buffer, (size_t)PATH_SIZE);
        if (status)
            printf("\033[31;1m[%i]\033[0m %s \033[33m$\033[0m ", status, cwd);
        else
            printf("%s \033[33m$\033[0m ", cwd);

        line = read_line(&len);
        if (!line) {
            // simply exit on EOF
            status = NESSIE_EXIT_SUCCESS;
            break;
        }
        // Read and split input
        tokens = split_input(line, len, &count);
        status = parse_and_execute(tokens, count);
        free(tokens);
    }

    free(cwd_buffer);

    switch (status) {
        case NESSIE_EXIT_SUCCESS:
            return EXIT_SUCCESS;
        case NESSIE_EXIT_FAILURE:
            return EXIT_FAILURE;
        default:
            fprintf(stderr, "An unknown error occured\n");
            return -status;
    }
}
