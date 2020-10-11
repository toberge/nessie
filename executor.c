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

// Call execvp() with supplied arguments. Will never return.
void child_exec(char **tokens, int argc) {
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
                execute_syntax_tree(node->child_node);
                return execute_syntax_tree(node->next_node);
            } else {
                return execute_syntax_tree(node->child_node);
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
        fprintf(stderr, "Error while forking!\n");
        status = -1;
    }
    return status;
}

int pipe_inner(ASTNode *node, int old_fd[2]) {
    // Create a new pipe
    int next_fd[2];
    pipe(next_fd);
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
        fprintf(stderr, "Error while forking!\n");
        status = -1;
    }
    return status;
}

int pipe_start(ASTNode *node) {
    if (node->contentlen < 1) return 0;

    int status = -1;
    int fd[2];
    pipe(fd); // create a pipe - write to fd[1], read from fd[0]

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
        fprintf(stderr, "Error while forking!\n");
        status = -1;
    }

    return status;
}

// Pipe output from cmd1 to cmd2 to stdout
int pipe_commands(char **argv1, int argc1, char **argv2, int argc2) {
    if (argc1 < 1 || argc2 < 1) return 0;

    int status = -1;

    int fd[2];
    pipe(fd); // create a pipe - write to fd[0], read from fd[1]

    pid_t child_pid = fork();

    if (child_pid > 0) {
        // Parent will fork again
        child_pid = fork();
        // TODO: This should _end_ dynamically
        //       The shell should _only_ wait at the tail end
        //       of the pipe chain. Recursion FTW!

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
            child_exec(argv2, argc2);
        } else {
            fprintf(stderr, "Error while forking!\n");
            status = -1;
        }
    } else if (child_pid == 0) {
        // Child will route stdin to pipe input
        dup2(fd[PIPE_WRITE], STDOUT_FILENO);
        close(fd[PIPE_READ]);
        close(fd[PIPE_WRITE]);
        child_exec(argv1, argc1);
    } else {
        fprintf(stderr, "Error while forking!\n");
        status = -1;
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
