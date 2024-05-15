#ifndef PARSER_H
#define PARSER_H

#include "include/lexer.h"  // Include lexer for token stream

typedef enum {
    NODE_NUMBER,
    NODE_BINARY_OP,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line;  // Line number for error reporting
    union {
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            TokenType operator;
            char* operatorText;  // Descriptive text for the operator
        } bin_op;
        int value;  // For number nodes
    } data;
} ASTNode;

ASTNode* parseExpression(Lexer* lexer);
void freeAST(ASTNode* node);

#endif