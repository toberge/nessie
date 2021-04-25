#ifndef NESSIE_HEADER
#define NESSIE_HEADER

#include <stdio.h> // for FILE

// for cwd-fetching
#define PATH_SIZE pathconf(".", _PC_PATH_MAX)

// lexer constants
#define NESSIE_LINE_LENGTH 256
#define NESSIE_TOKEN_LENGTH 64
#define NESSIE_TOKEN_ARRAY_LENGTH 32
#define NESSIE_HISTORY_LENGTH 256

// additional status codes
#define NESSIE_EXIT_SUCCESS -3
#define NESSIE_EXIT_FAILURE -4

// semantic exit codes
#define EXIT_SYNTAX_ERROR 2

// alias for indexing fd pair from pipe()
#define PIPE_WRITE 1
#define PIPE_READ 0

#define NESSIE_WHITESPACE_CHARS " \x9a\t\n"
#define NESSIE_OPERATOR_CHARS "&|;"
#define NESSIE_VARIABLE_CHARS "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

// struct for varous options
struct Option_st {
    int debug;
};

extern struct Option_st O;

// {{{ Utils

int min(int a, int b);
int max(int a, int b);
void die(char *msg);

// }}}

// {{{ Builtins

int nessie_cd(char **argv, int argc);
int nessie_setenv(char **argv, int argc);
int nessie_exit(char **argv, int argc);
int nessie_help(char **argv, int argc);
int nessie_true(char **argv, int argc);
int nessie_false(char **argv, int argc);
int nessie_history(char **argv, int argc);

// }}}

// {{{ Lexer

char *expand_variables(char *line, int len, int *expanded_len); // TODO remove
char *read_line(int *len, FILE *file);
char **split_input(const char *input, const int input_len, int *num_tokens);
void free_tokens(char **tokens, int len);

// }}}

// {{{ Parser

enum nessieSyntax {
    NESSIE_SYNTAX_ERROR,
    NESSIE_STATEMENT,
    NESSIE_COMMAND,
    NESSIE_AND,
    NESSIE_OR
};

struct ASTNode_st;
typedef struct ASTNode_st {
    int type;
    char **content;
    int contentlen;
    struct ASTNode_st *child_node;
    struct ASTNode_st *next_node;
} ASTNode;

ASTNode *parse_line(char **argv, int argc);
void free_ASTNode(ASTNode *node);

// }}}

// {{{ Executor

int execute_syntax_tree(ASTNode *node);
int launch_command(char **argv, int argc);
int pipe_commands(char **argv1, int argc1, char **argv2, int argc2);

// }}}

// {{{ History

void history_init();
void history_free();
void history_save(const char *line);
const char **history_get_all();

// }}}

#endif
