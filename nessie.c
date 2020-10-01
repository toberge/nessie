// enabling use of getline()
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LENGTH 200
#define MAX_TOKENS 30
#define PATH_SIZE pathconf(".", _PC_PATH_MAX)
#define NESSIE_TOKEN_LENGTH 64
#define NESSIE_TOKEN_ARRAY_LENGTH 32

/*
 * Second attempt at writing a shell,
 * now with more C knowledge (chapters 1,4,5,6 in K&R + kilo.c tutorial)
 */

int cd(char **argv, int argc) {
    if (argc == 2) {
        return -chdir(argv[1]);
    } else {
        perror("cd: Invalid number of arguments");
        return 1;
    }
}

static char* BUILTIN_NAMES[] = {
    "cd"
};

static int (*BUILTINS[]) (char**, int) = {
    cd
};

#define NUM_BUILTINS (sizeof(BUILTINS) / sizeof(BUILTINS[0]))

void die(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
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

char **split_input(const char *input, const int len, int *num_tokens) {
    // note: the len parameter will be useful for validation later.
    int i, n, c, state;

    char **tokens = malloc(NESSIE_TOKEN_ARRAY_LENGTH*sizeof(char*));
    tokens[0] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
    long arrlen = NESSIE_TOKEN_ARRAY_LENGTH;
    long toklen = NESSIE_TOKEN_LENGTH;

    i = 0, n = 0;
    state = OUTSIDE;
    while ((c = *input++) != '\0') {
        // Reallocate memory of array or string if full
        if (n >= arrlen) {
            arrlen += arrlen / 3;
            tokens = realloc(tokens, arrlen);
        }
        if (i >= toklen) {
            toklen += toklen / 3;
            tokens[n] = realloc(tokens[n], toklen);
        }

        switch (state) {
            case OUTSIDE:
                if (c == '"') {
                    state = IN_STRING;
                } else if (c != ' ') {
                    state = IN_WORD;
                    tokens[n][i++] = c;
                }
                break;
            case IN_WORD:
                if (c == ' ') {
                    state = OUTSIDE;
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
                    toklen = NESSIE_TOKEN_LENGTH;
                    i = 0;
                } else {
                    tokens[n][i++] = c;
                }
                break;
            case IN_STRING:
                if (c == '"') { // TODO: check if whitespace follows?
                    state = OUTSIDE;
                    tokens[n++][i] = '\0';
                    // new token! TODO no code duplication
                    tokens[n] = malloc(NESSIE_TOKEN_LENGTH*sizeof(char));
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
        fprintf(stderr, "No matching \" found!\n");
        // Free token array
        *num_tokens = 0;
        free(tokens);
        return NULL;
    }
    if (i) {
        tokens[n][i] = '\0';
        n++;
    }
    *num_tokens = n;
    return tokens;
}

void printarr(char **arr, int len) {
    for (int i = 0; i < len-1; i++) {
        printf("'%s', ", arr[i]);
    }
    printf("'%s'\n", arr[len-1]); // clang complains: bad sign?
}

/* Forks and executes command */
int execute_command(char **tokens, int argc) {
    if (argc < 1) return 0;

    int status;
    pid_t child_pid = fork();
    if (child_pid) {
        // parent process, wait for child to finish
        do {
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else if (child_pid == 0) {
        // child process, execute command!
        char *argv[argc+1]; // create copy with ONLY current amount of args
        for (int i = 0; i < argc; i++)
            argv[i] = tokens[i];
        argv[argc] = NULL; // required by the standard (!!!)
        status = execvp(tokens[0], (char * const*) argv);
    } else {
        // negative pid -> error forking
        printf("Error while forking!\n");
        status = -1;
    }

    return status;
}


int main(int argc, char **argv) {
    char **tokens;
    int count, status = 0, debug = 0, exit_status = 0;
    if (argc == 2 && strcmp("--debug", argv[1]) == 0)
        debug = 1;

    char *cwd;
    char *cwd_buffer = (char*) malloc((size_t)PATH_SIZE);

    int len = 0;
    char *line = NULL;

    printf("Welcome to ne[sh]ie, the absurdly stupid shell!\n");

    while (1) {
        // Display prompt
        cwd = getcwd(cwd_buffer, (size_t)PATH_SIZE);
        if (status)
            printf("\033[31;1m[%i]\033[0m %s \033[33m$\033[0m ", status, cwd);
        else
            printf("%s \033[33m$\033[0m ", cwd);

        status = -1;

        line = read_line(&len);
        if (!line) {
            // simply exit on EOF
            exit_status = EXIT_SUCCESS;
            break;
        }
        // Read and split input
        tokens = split_input(line, len, &count);
        if (count <= 0 || tokens == NULL) {
            free(tokens);
            status = 0;
            continue;
        }
        if (debug) {
            printf("Read %i tokens: ", count);
            printarr(tokens, count);
        }

        // Handle builtins
        for (int i = 0; i < NUM_BUILTINS; i++) {
            if (strcmp(tokens[0], BUILTIN_NAMES[i]) == 0) {
                status = BUILTINS[i]((char **)tokens, count);
                free(tokens);
                continue;
            }
        }

        if (status != -1) { // cmd was a builtin
            free(tokens);
            continue;
        }

        // Handle exit
        if (strcmp(tokens[0], "exit") == 0)
            break;
        else if (tokens[0][0] == 'q')
            break;

        // Turns out it's not a builtin, execute command!
        status = execute_command(tokens, count);
        if (status < 0) {
            exit_status = 1;
            break;
        } else if (chdir(tokens[0]) == 0) {
            status = 0; // since it's not a command, try cd-ing with it!
        } else if (status == 256) {
            printf("No such command or directory: %s\n", tokens[0]);
        }

        free(tokens);
    }

    free(tokens);
    free(cwd_buffer);

    return exit_status;
}
