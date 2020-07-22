#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LENGTH 100
#define MAX_TOKENS 20
#define NUM_BUILTINS 1
#define PATH_SIZE pathconf(".", _PC_PATH_MAX)

/*
 * Second attempt at writing a shell,
 * now with more C knowledge (chapter 1...)
 */

int cd(char argv[][MAX_LENGTH], int argc) {
    if (argc == 2)
        return -chdir(argv[1]);
    else {
        perror("cd: Invalid number of arguments");
        return 1;
    }
}

static char* BUILTIN_NAMES[NUM_BUILTINS] = {
    "cd"
};

static int (*BUILTINS[NUM_BUILTINS]) (char[][MAX_LENGTH], int) = {
    cd
};

int split_input(char tokens[][MAX_LENGTH]) {
    int i, n, c, state;
    enum { OUTSIDE, IN_WORD, IN_STRING };

    i = 0, n = 0;
    state = OUTSIDE;
    while ((c = getchar()) != EOF && c != '\n') {
        if (c == ' '  && state == IN_WORD) {
            state = OUTSIDE;
            /* tokens[n] = token; */
            tokens[n][i] = '\0';
            /* char newtoken[MAX_LENGTH]; */
            /* token = newtoken; // okay whatever */
            n++;
            i = 0;
        } else if (c == '"') {
            if (state == OUTSIDE) {
                state = IN_STRING;
            } else if (state == IN_STRING) {
                state = OUTSIDE;
                tokens[n][i] = '\0';
                n++;
                i = 0;
            } else {
                state = IN_WORD;
                /* token[i] = c; */
                tokens[n][i] = c;
                i++;
            }
        } else {
            if (state != IN_STRING) state = IN_WORD;
            tokens[n][i] = c;
            i++;
        }
    }
    if (i) {
        tokens[n][i] = '\0';
        n++;
    }
    return n;
}

void printarr(char arr[][MAX_LENGTH], int len) {
    for (int i = 0; i < len-1; i++) {
        printf("'%s', ", arr[i]);
    }
    printf("'%s'\n", arr[len-1]);
}

void arrcpy(char from[][MAX_LENGTH], char to[][MAX_LENGTH], int amount) {
    for (int i = 0; i < amount; i++) {
        int j = 0;
        while (from[i][j] != '\0')
            to[i][j] = from[i][j], j++;
        to[i][j] = '\0';
    }
}

/* Forks and executes command */
int execute_command(char tokens[][MAX_LENGTH], int argc) {
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
        perror("Error while forking!");
        status = -1;
    }

    return status;
}

int main() {
    char tokens[MAX_TOKENS][MAX_LENGTH] = {{'\0'}};
    int count, status = 0, debug = 0, exit_status = 0;

    char *cwd;
    char *cwd_buffer = (char*)malloc((size_t)PATH_SIZE);

    printf("Welcome to ne[sh]ie, the absurdly stupid shell!\n");

    while (1) {
        // Display prompt
        cwd = getcwd(cwd_buffer, (size_t)PATH_SIZE);
        if (status)
            printf("\033[31;1m[%i]\033[0m %s \033[33m$\033[0m ", status, cwd);
        else
            printf("%s \033[33m$\033[0m ", cwd);

        // Read and split input
        count = split_input(tokens);
        if (debug) {
            printf("Read %i tokens: ", count);
            printarr(tokens, count);
        }

        // Handle builtins
        for (int i = 0; i < NUM_BUILTINS; i++) {
            if (strcmp(tokens[0], BUILTIN_NAMES[i]) == 0) {
                status = BUILTINS[i](tokens, count);
                continue;
            }
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
        } else if (chdir(tokens[0]) == 0)
            status = 0; // since it's not a command, try cd-ing with it!
        else if (status == 256)
            printf("No such command or directory: %s\n", tokens[0]);
    }

    free(cwd_buffer);

    return exit_status;
}
