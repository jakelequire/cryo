#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"

void freeAST(ASTNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_BINARY_EXPR:
            freeAST(node->data.bin_op.left);
            freeAST(node->data.bin_op.right);
            free(node->data.bin_op.operatorText);
            break;
        case NODE_UNARY_EXPR:
            freeAST(node->data.unary_op.operand);
            break;
        case NODE_LITERAL_EXPR:
            break;
        case NODE_VARIABLE_EXPR:
            free(node->data.varName.varName);
            break;
        case NODE_FUNCTION_CALL:
            free(node->data.functionCall.name);
            break;
        case NODE_FUNCTION_DECL:
            free(node->data.functionDecl.name);
            freeAST(node->data.functionDecl.body);
            break;
        case NODE_IF_STATEMENT:
            freeAST(node->data.ifStmt.condition);
            freeAST(node->data.ifStmt.thenBranch);
            freeAST(node->data.ifStmt.elseBranch);
            break;
        case NODE_WHILE_STATEMENT:
            freeAST(node->data.whileStmt.condition);
            freeAST(node->data.whileStmt.body);
            break;
        case NODE_FOR_STATEMENT:
            freeAST(node->data.forStmt.initializer);
            freeAST(node->data.forStmt.condition);
            freeAST(node->data.forStmt.increment);
            freeAST(node->data.forStmt.body);
            break;
        case NODE_RETURN_STATEMENT:
            freeAST(node->data.returnStmt.returnValue);
            break;
        default:
            break;
    }
    free(node);
}

void printASTIndented(ASTNode* node, int indent) {
    if (node == NULL) return;

    char* indentStr = (char*)malloc((indent + 1) * sizeof(char));
    memset(indentStr, ' ', indent);
    indentStr[indent] = '\0';

    switch (node->type) {
        case NODE_BINARY_EXPR:
            printf("%sBinary Expression: %s\n", indentStr, node->data.bin_op.operatorText);
            printASTIndented(node->data.bin_op.left, indent + 2);
            printASTIndented(node->data.bin_op.right, indent + 2);
            break;
        case NODE_UNARY_EXPR:
            printf("%sUnary Expression: %d\n", indentStr, node->data.unary_op.operator);
            printASTIndented(node->data.unary_op.operand, indent + 2);
            break;
        case NODE_LITERAL_EXPR:
            printf("%sLiteral: %d\n", indentStr, node->data.value);
            break;
        case NODE_VARIABLE_EXPR:
            printf("%sVariable: %s\n", indentStr, node->data.varName.varName);
            break;
        case NODE_FUNCTION_CALL:
            printf("%sFunction Call: %s\n", indentStr, node->data.functionCall.name);
            break;
        case NODE_FUNCTION_DECL:
            printf("%sFunction Declaration: %s\n", indentStr, node->data.functionDecl.name);
            printASTIndented(node->data.functionDecl.body, indent + 2);
            break;
        case NODE_IF_STATEMENT:
            printf("%sIf Statement\n", indentStr);
            printASTIndented(node->data.ifStmt.condition, indent + 2);
            printASTIndented(node->data.ifStmt.thenBranch, indent + 2);
            if (node->data.ifStmt.elseBranch) {
                printASTIndented(node->data.ifStmt.elseBranch, indent + 2);
            }
            break;
        case NODE_WHILE_STATEMENT:
            printf("%sWhile Statement\n", indentStr);
            printASTIndented(node->data.whileStmt.condition, indent + 2);
            printASTIndented(node->data.whileStmt.body, indent + 2);
            break;
        case NODE_FOR_STATEMENT:
            printf("%sFor Statement\n", indentStr);
            printASTIndented(node->data.forStmt.initializer, indent + 2);
            printASTIndented(node->data.forStmt.condition, indent + 2);
            printASTIndented(node->data.forStmt.increment, indent + 2);
            printASTIndented(node->data.forStmt.body, indent + 2);
            break;
        case NODE_RETURN_STATEMENT:
            printf("%sReturn Statement\n", indentStr);
            printASTIndented(node->data.returnStmt.returnValue, indent + 2);
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

ASTNode* createLiteralExpr(int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_LITERAL_EXPR;
    node->data.value = value;
    return node;
}

ASTNode* createVariableExpr(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE_EXPR;
    node->data.varName.varName = strdup(name);
    return node;
}

ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, TokenType operator) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_EXPR;
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.operator = operator;
    node->data.bin_op.operatorText = (operator == TOKEN_PLUS) ? strdup("+") :
                                     (operator == TOKEN_MINUS) ? strdup("-") :
                                     (operator == TOKEN_STAR) ? strdup("*") : strdup("/");
    return node;
}

ASTNode* createUnaryExpr(TokenType operator, ASTNode* operand) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_EXPR;
    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* parseFunctionCall(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->data.functionCall.name = strdup(name);
    return node;
}

ASTNode* parseIfStatement() {
    // Implementation for if statement parsing
    return NULL; // Placeholder
}

ASTNode* parseReturnStatement() {
    // Implementation for return statement parsing
    return NULL; // Placeholder
}

ASTNode* parseExpressionStatement() {
    // Implementation for expression statement parsing
    return NULL; // Placeholder
}

ASTNode* createFunctionNode(const char* name, ASTNode* body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DECL;
    node->data.functionDecl.name = strdup(name);
    node->data.functionDecl.body = body;
    return node;
}
