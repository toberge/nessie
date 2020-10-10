#ifndef NESSIE_HEADER
#define NESSIE_HEADER

// for cwd-fetching
#define PATH_SIZE pathconf(".", _PC_PATH_MAX)

// lexer constants
#define NESSIE_TOKEN_LENGTH 64
#define NESSIE_TOKEN_ARRAY_LENGTH 32

// additional status codes
#define NESSIE_EXIT_SUCCESS -3
#define NESSIE_EXIT_FAILURE -4

// alias for indexing fd pair from pipe()
#define PIPE_WRITE 1
#define PIPE_READ 0

// redo the debug handling somehow
#define DEBUG 1

// {{{ Builtins

int cd(char **argv, int argc);

// }}}

// {{{ Lexer

char *read_line(int *len);
char **split_input(const char *input, const int len, int *num_tokens);

// }}}

// {{{ Parser

int parse_and_execute(char **argv, int argc);

// }}}

// {{{ Executor

int launch_command(char **argv, int argc);
int pipe_commands(char **argv1, int argc1, char **argv2, int argc2);

// }}}

#endif
