#include "parser.h"
#include <stdlib.h> // For dynamic memory management
#include <string.h> // For atoi function
#include <stdio.h>

static ASTNode* parseTerm(Lexer* lexer, Token* token);
static ASTNode* parseFactor(Lexer* lexer, Token* token);

void nextTokenWrapper(Lexer* lexer, Token* token) {
    nextToken(lexer, token);
    while (token->type == TOKEN_COMMENT || token->type == TOKEN_WHITESPACE) { // Skip comments or whitespaces if needed
        nextToken(lexer, token);
    }
}

ASTNode* parseExpression(Lexer* lexer) {
    Token token;
    nextTokenWrapper(lexer, &token);
    ASTNode* node = parseTerm(lexer, &token);

    while (token.type == TOKEN_PLUS || token.type == TOKEN_MINUS) {
        ASTNode* new_node = malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.bin_op.operator = token.type;
        new_node->data.bin_op.left = node;

        nextTokenWrapper(lexer, &token);
        new_node->data.bin_op.right = parseTerm(lexer, &token);

        node = new_node;
    }
    return node;
}

static ASTNode* parseTerm(Lexer* lexer, Token* token) {
    ASTNode* node = parseFactor(lexer, token);

    while (token->type == TOKEN_STAR || token->type == TOKEN_SLASH) {
        ASTNode* new_node = malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.bin_op.operator = token->type;
        new_node->data.bin_op.left = node;

        nextTokenWrapper(lexer, token);
        new_node->data.bin_op.right = parseFactor(lexer, token);

        node = new_node;
    }
    return node;
}

static ASTNode* parseFactor(Lexer* lexer, Token* token) {
    if (token->type == TOKEN_INT) {
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        node->data.value = atoi(token->start);  // Convert string to integer
        nextTokenWrapper(lexer, token);
        return node;
    } else if (token->type == TOKEN_LEFT_PAREN) {
        nextTokenWrapper(lexer, token);
        ASTNode* node = parseExpression(lexer);

        if (token->type != TOKEN_RIGHT_PAREN) {
            // Handle error, expected right parenthesis
            fprintf(stderr, "Error: Expected ')' got '%.*s'\n", token->length, token->start);
        }
        nextTokenWrapper(lexer, token);
        return node;
    }
    // Handle error, unexpected token
    fprintf(stderr, "Error: Unexpected token '%.*s'\n", token->length, token->start);
    return NULL;
}

void freeAST(ASTNode* node) {
    if (node->type == NODE_BINARY_OP) {
        freeAST(node->data.bin_op.left);
        freeAST(node->data.bin_op.right);
    }
    free(node);
}