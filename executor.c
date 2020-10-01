#include "nessie.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static char* BUILTIN_NAMES[] = {
    "cd"
};

static int (*BUILTINS[]) (char**, int) = {
    cd
};

// there will never be more than an int's value of builtins
#define NUM_BUILTINS (int)(sizeof(BUILTINS) / sizeof(BUILTINS[0]))

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
    if (child_pid > 0) {
        // In parent process, wait for child to finish
        do {
            waitpid(child_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        // Then get actual exit status!
        status = WEXITSTATUS(status);
    } else if (child_pid == 0) {
        // In child process, execute command!
        char *argv[argc+1]; // create copy with ONLY current amount of args
        for (int i = 0; i < argc; i++)
            argv[i] = tokens[i];
        argv[argc] = NULL; // required by the standard (!!!)
        execvp(tokens[0], (char * const*) argv);
        // If execvp returned, it is always a failure.
        exit(127);
    } else {
        // negative pid -> error forking
        printf("Error while forking!\n");
        status = -1;
    }

    return status;
}

int launch_command(char **tokens, int count) {
    if (count <= 0 || tokens == NULL) {
        return 0;
    }
    if (DEBUG) {
        printf("Read %i tokens: ", count);
        printarr(tokens, count);
    }

    // Handle builtins
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(tokens[0], BUILTIN_NAMES[i]) == 0) {
            return BUILTINS[i]((char **)tokens, count);
        }
    }

    // Handle exit
    if (strcmp(tokens[0], "exit") == 0)
        return NESSIE_EXIT_SUCCESS;
    else if (tokens[0][0] == 'q')
        return NESSIE_EXIT_SUCCESS;

    // Turns out it's not a builtin, execute command!
    int status = execute_command(tokens, count);
    if (status < 0) {
        return NESSIE_EXIT_FAILURE;
    } else if (status == 127) { // Unknown command!
        if (chdir(tokens[0]) == 0) // Try cd-ing with it!
            return 0;
        // Otherwise, it is neither a command nor a directory.
        printf("No such command or directory: %s\n", tokens[0]);
        return status;
    }
    return status; // return point for normal exit codes
}
