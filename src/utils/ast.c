#include <stdio.h>
#include <stdlib.h>

#include "include/lexer.h"
#include "include/parser.h"


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