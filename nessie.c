#include "nessie.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Second attempt at writing a shell,
 * now with more C knowledge (chapters 1,4,5,6 in K&R + kilo.c tutorial)
 */

struct Option_st O = { 0 };

// TODO: move this tiny function
void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void display_prompt(int status, char *cwd) {
    if (status)
        printf("\033[31;1m[%i]\033[0m \033[32m%s\033[0m \033[34m$\033[0m ", status, cwd);
    else
        printf("\033[32m%s\033[0m \033[34m$\033[0m ", cwd);
}

int single_command_run(char *line) {
    int count, status;
    char **tokens = split_input(line, &count);
    ASTNode *tree = parse_line(tokens, count);
    status = execute_syntax_tree(tree);
    free_ASTNode(tree);
    free_tokens(tokens, count);
    return status;
}

int interactive_run() {
    char **tokens;
    int count, status = 0;

    char *cwd_buffer = (char*) malloc((size_t)PATH_SIZE);

    int len = 0;
    char *line = NULL;

    printf("Welcome to ne[sh]ie, the absurdly stupid shell!\n");

    while (status >= 0) {
        // Display prompt
        display_prompt(status, getcwd(cwd_buffer, (size_t)PATH_SIZE));

        line = read_line(&len);
        if (!line) {
            // simply exit on EOF
            status = NESSIE_EXIT_SUCCESS;
            break;
        }
        // Read and split input
        tokens = split_input(line, &count);
        // Parse and execute
        ASTNode *tree = parse_line(tokens, count);
        status = execute_syntax_tree(tree);
        free_ASTNode(tree);
        free_tokens(tokens, count);
    }

    free(cwd_buffer);

    switch (status) {
        case NESSIE_EXIT_SUCCESS:
            return EXIT_SUCCESS;
        case NESSIE_EXIT_FAILURE:
            return EXIT_FAILURE;
        default:
            fprintf(stderr, "An unknown error occured: %d\n", status);
            return -status;
    }
}

int main(int argc, char **argv) {
    O.debug = 0;

    char *command = NULL;

    while (1) {
        int option_index;
        static struct option long_options[] = {
            { "debug", no_argument, &O.debug, 1 }, // currently does not work
            { "command", required_argument, 0, 'c' },
            { 0, 0, 0, 0 }
        };

        int c = getopt_long (argc, argv, "c:",
                             long_options, &option_index);

        if (c == -1)
          break;

        switch (c) {
            case 0:
                // Long option!
                // ...it must have been --debug
                break;
            case 'c':
                command = optarg;
                break;
            case '?':
                break;
            default:
                abort();
        }
    }

    if (command != NULL) {
        return single_command_run(command);
    } else if (optind < argc) {
        fprintf(stderr, "Support for scripts is not implemented yet\n");
        return 1;
        /* return file_run(argv[optind]); */
    } else {
        return interactive_run();
    }
}
