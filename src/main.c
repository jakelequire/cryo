#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>


void printAST(ASTNode* node);
char* readFile(const char* path); 


void printASTIndented(ASTNode* node, int indent) {
    if (node == NULL) return;

    // Create an indent string based on the depth level
    char* indentStr = (char*)malloc(indent + 1);
    for (int i = 0; i < indent; i++) indentStr[i] = ' ';
    indentStr[indent] = '\0';

    switch (node->type) {
        case NODE_NUMBER:
            printf("%s%d\n", indentStr, node->data.value);
            break;
        case NODE_BINARY_OP:
            printf("%s(\n", indentStr);
            printASTIndented(node->data.bin_op.left, indent + 2);
            printf("%s %s\n", indentStr, node->data.bin_op.operatorText);
            printASTIndented(node->data.bin_op.right, indent + 2);
            printf("%s)\n", indentStr);
            break;
        default:
            printf("%sUnknown node type\n", indentStr);
            break;
    }

    free(indentStr);
}

void printAST(ASTNode* node) {
    printASTIndented(node, 0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);
    Token token;
    nextToken(&lexer, &token);

    ASTNode* ast = parseExpression(&lexer);
    if (ast == NULL) {
        fprintf(stderr, "Failed to parse expression.\n");
        free(source);
        return 1;
    }

    printAST(ast);  // Print the AST
    freeAST(ast);  // Free the AST
    free(source);  // Free the source buffer
    return 0;
}