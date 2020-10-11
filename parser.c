#include "nessie.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <stdio.h> // shouldn't be necessary

ASTNode *parse_statement(char **tokens, int len);

ASTNode *parse_line(char **argv, int argc) {
    return parse_statement(argv, argc);
}

ASTNode *create_ASTNode() {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = 0;
    node->content = 0;
    node->contentlen = 0;
    node->child_node = NULL;
    node->next_node = NULL;
    return node;
}

// Recursively free an entire syntax tree
void free_ASTNode(ASTNode *node) {
    if (node->child_node)
        free_ASTNode(node->child_node);
    if (node->next_node)
        free_ASTNode(node->next_node);
    free(node);
}

// Parse a command, which may be piped into other commands
ASTNode *parse_command(char **tokens, int len) {
    int i;
    ASTNode *ret = create_ASTNode();
    ret->type = NESSIE_COMMAND;
    ret->content = tokens;
    for (i = 1; i < len-1; i++) {
        /*
         * A A A | B B B B B
         * i = 3
         * rest = 9-3-1 = 5
         */
        if (strcmp(tokens[i], "|") == 0) {
            ret->contentlen = i;
            ret->next_node = parse_command(tokens+i+1, len-i-1);
            return ret;
        }
    }
    ret->contentlen = len;
    if (len > 1 && strcmp(tokens[i], "|") == 0) {
        fprintf(stderr, "Unfinished pipe!\n");
        ret->type = NESSIE_SYNTAX_ERROR;
    }
    return ret;
}

// Parse a statement, which may be followed by other statements
ASTNode *parse_statement(char **tokens, int len) {
    int i;
    ASTNode *ret = create_ASTNode();
    ret->type = NESSIE_STATEMENT;
    for (i = 1; i < len-1; i++) {
        if (strcmp(tokens[i], ";") == 0) {
            ret->child_node = parse_command(tokens, i);
            ret->next_node = parse_statement(tokens+i+1, len-i-1);
            return ret;
        }
    }
    ret->child_node = parse_command(tokens, len);
    return ret;
}
