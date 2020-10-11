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

// struct for varous options
struct Option_st {
    int debug;
};

extern struct Option_st O;

// {{{ Builtins

int cd(char **argv, int argc);

// }}}

// {{{ Lexer

char *read_line(int *len);
char **split_input(const char *input, int *num_tokens);
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

#endif
