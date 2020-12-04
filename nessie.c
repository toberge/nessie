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
 * Process and execute a command
 *
 * @return exit status of command
 */
int parse_and_run_command(char *line, int len) {
        int count, status;
        // Insert variables
        line = expand_variables(line, len);
        if (!line) return EXIT_SYNTAX_ERROR; // syntax error
        // Read and split input
        char **tokens = split_input(line, &count);
        // Parse and execute
        ASTNode *tree = parse_line(tokens, count);
        status = execute_syntax_tree(tree);
        free_ASTNode(tree);
        free_tokens(tokens, count);
        return status;
}

/**
 * Run nessie in non-interactive mode
 *
 * @return Exit code for the shell
 */
int single_command_run(char *line, int len) {
    int status = parse_and_run_command(line, len);

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
 * Run a script file
 *
 * @return Exit code for the shell
 */
int script_run(FILE *file) {
    int len, status = NESSIE_EXIT_SUCCESS;
    char *line = read_line(&len, file);
    while (line != NULL) {
        status = single_command_run(line, len);
        if (status < 0) // if exit was called, exit.
            return status;
        line = read_line(&len, file);
    }
    return status;
}

/**
 * Wrapper around script_run for actual files
 *
 * @return Exit code for the shell
 */
int file_run(char *filename) {
    FILE *file = fopen(filename, "r");
    int status = script_run(file);
    fclose(file);
    return status;
}

/**
 * Run nessie in interactive mode
 *
 * @return Exit code for the shell
 */
int interactive_run() {
    char *cwd_buffer = (char*) malloc((size_t)PATH_SIZE);

    int len = 0;
    char *line = NULL;
    int status = 0;
    history_init();

    printf("Welcome to ne[sh]ie, the absurdly stupid shell!\n");

    while (status >= 0) {
        // Display prompt
        display_prompt(status, getcwd(cwd_buffer, (size_t)PATH_SIZE));


        line = read_line(&len, stdin);
        if (!line) {
            // simply exit on EOF, but print a newline first!
            printf("\n");
            status = NESSIE_EXIT_SUCCESS;
            break;
        }

        status = parse_and_run_command(line, len);
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
        return single_command_run(command, strlen(command));
    } else if (optind < argc) {
        return file_run(argv[optind]);
    } else if (!isatty(STDIN_FILENO)) {
        return script_run(stdin);
    } else {
        return interactive_run();
    }
}
