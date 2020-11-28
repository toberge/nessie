#include "nessie.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Second attempt at writing a shell,
 * now with more C knowledge (chapters 1,4,5,6 in K&R + kilo.c tutorial)
 */

struct Option_st O = { 0 };

/**
 * Display a colored prompt with current working directory
 * and exit status of the last command, if it was nonzero
 *
 * @param status Exit status of last command
 * @param cwd    Current working directory
 */
void display_prompt(int status, char *cwd) {
    if (status)
        printf("\033[31;1m[%i]\033[0m ", status);
    printf("\033[32m%s\033[0m \033[34m$\033[0m ", cwd);
}

/**
 * Run nessie in non-interactive mode
 *
 * @return Exit code for the shell
 */
int single_command_run(char *line) {
    int count, status;
    char **tokens = split_input(line, &count);
    ASTNode *tree = parse_line(tokens, count);
    status = execute_syntax_tree(tree);
    free_ASTNode(tree);
    free_tokens(tokens, count);

    // Just in case some negative status appears
    switch (status) {
        case NESSIE_EXIT_SUCCESS:
            return EXIT_SUCCESS;
        case NESSIE_EXIT_FAILURE:
            return EXIT_FAILURE;
        default:
            return status;
    }
}

/**
 * Run nessie in interactive mode
 *
 * @return Exit code for the shell
 */
int interactive_run() {
    char **tokens;
    int count, status = 0;

    char *cwd_buffer = (char*) malloc((size_t)PATH_SIZE);

    int len = 0;
    char *line = NULL;
    history_init();

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
        // Save history
        history_save(line);
    }

    free(cwd_buffer);
    history_free();

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
            { "debug", no_argument, &O.debug, 'd' },
            { "command", required_argument, 0, 'c' },
            { "help", no_argument, 0, 'h' },
            { 0, 0, 0, 0 }
        };

        int c = getopt_long (argc, argv, "dc:h",
                             long_options, &option_index);

        if (c == -1)
          break;

        switch (c) {
            case 0:
                // long option
                break;
            case 'c':
                command = optarg;
                break;
            case 'h':
                nessie_help(NULL, 0);
                exit(EXIT_SUCCESS);
                break;
            case '?':
                break;
            default:
                fprintf(stderr, "Unknown option(s).");
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
