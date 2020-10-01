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

// redo the debug handling somehow
#define DEBUG 0

// {{{ Builtins

int cd(char **argv, int argc);

// }}}


// {{{ Lexer

char *read_line(int *len);
char **split_input(const char *input, const int len, int *num_tokens);

// }}}

// {{{

int launch_command(char **argv, int argc);

// }}}

#endif
