#include "nessie.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static char* BUILTIN_NAMES[] = {
    "cd",
    "exit",
    "q",
    "help",
    "true",
    "false"
};

static int (*BUILTINS[]) (char**, int) = {
    cd,
    nessie_exit,
    nessie_exit,
    nessie_help,
    nessie_true,
    nessie_false
};

// there will never be more than an int's value of builtins
#define NUM_BUILTINS (int)(sizeof(BUILTINS) / sizeof(BUILTINS[0]))

void printarr(char **arr, int len) {
    for (int i = 0; i < len-1; i++) {
        printf("'%s', ", arr[i]);
    }
    printf("'%s'\n", arr[len-1]);
}

// Call execvp() (or a builtin) with supplied arguments. Will never return.
void child_exec(char **tokens, int argc) {
    // Handle builtins
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(tokens[0], BUILTIN_NAMES[i]) == 0) {
            int status = BUILTINS[i]((char **)tokens, argc);
            // Handle special exit codes...
            // TODO: Extract this to a function?
            if (status == NESSIE_EXIT_SUCCESS)
                exit(EXIT_SUCCESS);
            if (status == NESSIE_EXIT_FAILURE)
                exit(EXIT_FAILURE);
            exit(status);
        }
    }

    // In child process, execute command!
    char *argv[argc+1]; // create copy with ONLY current amount of args
    for (int i = 0; i < argc; i++)
        argv[i] = tokens[i];
    argv[argc] = NULL; // required by the standard (!!!)
    execvp(tokens[0], argv);
    // If execvp returned, it is always a failure.
    exit(127);
}

// Wait for child process to finish
int parent_wait(pid_t child_pid) {
    // In parent process, wait for child to finish
    int status;
    do {
        waitpid(child_pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    // Then get actual exit status!
    return WEXITSTATUS(status);
}

int pipe_start(ASTNode *node);

int execute_syntax_tree(ASTNode *node) {
    switch(node->type) {
        case NESSIE_COMMAND:
            if (node->next_node)
                return pipe_start(node);
            else
                return launch_command(node->content, node->contentlen);
        case NESSIE_STATEMENT:
            if (node->next_node) {
                // Run child command, then proceed
                // (this is where set -e will be handled later)
                int status = execute_syntax_tree(node->child_node);
                if (status == NESSIE_EXIT_SUCCESS) return status; // exit!
                return execute_syntax_tree(node->next_node);
            } else {
                return execute_syntax_tree(node->child_node);
            }
        case NESSIE_OR:
            {
                // Use exit status to determine if execution should continue
                int status = execute_syntax_tree(node->child_node);
                if (status == NESSIE_EXIT_SUCCESS) return status; // exit!
                if (status != 0) // command failed, go on
                    return execute_syntax_tree(node->next_node);
                return status;
            }
        case NESSIE_AND:
            {
                // Use exit status to determine if execution should continue
                int status = execute_syntax_tree(node->child_node);
                if (status == NESSIE_EXIT_SUCCESS) return status; // exit!
                if (status == 0) // command exec'd fine, go on
                    return execute_syntax_tree(node->next_node);
                return status;
            }
        default:
            fprintf(stderr, "Not implemented\n");
            return 1;
    }
}

int pipe_end(ASTNode *node, int fd[2]) {
    int status = -1;
    // Parent will fork again
    int child_pid = fork();

    if (child_pid > 0) {
        // Parent will close fds and wait
        close(fd[PIPE_READ]);
        close(fd[PIPE_WRITE]);
        status = parent_wait(child_pid);
    } else if (child_pid == 0) {
        // Child will route pipe output to stdin
        dup2(fd[PIPE_READ], STDIN_FILENO);
        close(fd[PIPE_WRITE]);
        close(fd[PIPE_READ]);
        child_exec(node->content, node->contentlen);
    } else {
        die("fork");
    }
    return status;
}

int pipe_inner(ASTNode *node, int old_fd[2]) {
    int next_fd[2];
    // Create a *new* pipe - write to next_fd[1], read from next_fd[0]
    if (pipe(next_fd) == -1)
        die("pipe");

    int status = -1;
    // Parent will fork again
    int child_pid = fork();

    if (child_pid > 0) {
        // Parent will close old fds and continue
        close(old_fd[PIPE_READ]);
        close(old_fd[PIPE_WRITE]);
        if (node->next_node->next_node)
            status = pipe_inner(node->next_node, next_fd);
        else
            status = pipe_end(node->next_node, next_fd);
    } else if (child_pid == 0) {
        // Child will route old pipe's output to stdin
        // - and route stdout to next pipe's input
        dup2(old_fd[PIPE_READ], STDIN_FILENO);
        close(old_fd[PIPE_WRITE]);
        close(old_fd[PIPE_READ]);
        dup2(next_fd[PIPE_WRITE], STDOUT_FILENO);
        close(next_fd[PIPE_READ]);
        close(next_fd[PIPE_WRITE]);
        child_exec(node->content, node->contentlen);
    } else {
        die("fork");
    }
    return status;
}

int pipe_start(ASTNode *node) {
    if (node->contentlen < 1) return 0;

    int status = -1;
    int fd[2];
    // create a pipe - write to fd[1], read from fd[0]
    if (pipe(fd) == -1)
        die("pipe");


    pid_t child_pid = fork();

    if (child_pid > 0) {
        if (node->next_node->next_node)
            status = pipe_inner(node->next_node, fd);
        else
            status = pipe_end(node->next_node, fd);
    } else if (child_pid == 0) {
        // Child will route stdin to pipe input
        dup2(fd[PIPE_WRITE], STDOUT_FILENO);
        close(fd[PIPE_READ]);
        close(fd[PIPE_WRITE]);
        child_exec(node->content, node->contentlen);
    } else {
        die("fork");
    }

    return status;
}

/* Forks and executes command */
int execute_command(char **tokens, int argc) {
    if (argc < 1) return 0;

    int status = -1;
    pid_t child_pid = fork();
    if (child_pid > 0) {
        // Is parent, wait!
        status = parent_wait(child_pid);
    } else if (child_pid == 0) {
        // Is child, execute command!
        child_exec(tokens, argc);
    } else {
        // negative pid -> error forking
        fprintf(stderr, "Error while forking!\n");
        status = -1;
    }

    return status;
}

int launch_command(char **tokens, int count) {
    if (count <= 0 || tokens == NULL) {
        return 0;
    }
    if (O.debug) {
        printf("Read %i tokens: ", count);
        printarr(tokens, count);
    }

    // Handle builtins
    for (int i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(tokens[0], BUILTIN_NAMES[i]) == 0) {
            return BUILTINS[i]((char **)tokens, count);
        }
    }

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
