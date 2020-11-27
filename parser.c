#include "nessie.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <stdio.h> // shouldn't be necessary

ASTNode *parse_statement(char **tokens, int len);
ASTNode *create_ASTNode();

// Create a syntax tree from an input line
ASTNode *parse_line(char **argv, int argc) {
    return parse_statement(argv, argc);
}

// Create and initialize an ASTNode struct
ASTNode *create_ASTNode() {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL)
        die("malloc");
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

// Look for an AND, extend the tree if one is found
ASTNode *parse_possible_and(char **argv, int argc) {
    for (int i = 1; i < argc-1; i++) {
        // AND needs to be handled below AND
        // due to its higher priority
        if (strcmp(argv[i], "&&") == 0) {
            ASTNode *ret = create_ASTNode();
            ret->type = NESSIE_AND;
            ret->child_node = parse_command(argv, i);
            ret->next_node = parse_possible_and(argv+i+1, argc-i-1);
            return ret;
        }
    }
    return parse_command(argv, argc);
}

// Look for an OR, extend the tree if one is found
ASTNode *parse_possible_or(char **argv, int argc) {
    for (int i = 1; i < argc-1; i++) {
        // OR needs to be handled at the level above AND
        // due to its lower priority
        if (strcmp(argv[i], "||") == 0) {
            ASTNode *ret = create_ASTNode();
            ret->type = NESSIE_OR;
            ret->child_node = parse_possible_and(argv, i);
            ret->next_node = parse_possible_or(argv+i+1, argc-i-1);
            return ret;
        }
    }
    return parse_possible_and(argv, argc);
}

// Parse a statement, which may be followed by other statements
ASTNode *parse_statement(char **tokens, int len) {
    ASTNode *ret = create_ASTNode();
    ret->type = NESSIE_STATEMENT;
    for (int i = 1; i < len-1; i++) {
        if (strcmp(tokens[i], ";") == 0) {
            ret->child_node = parse_possible_or(tokens, i);
            ret->next_node = parse_statement(tokens+i+1, len-i-1);
            return ret;
        }
    }
    ret->child_node = parse_possible_or(tokens, len);
    return ret;
}
