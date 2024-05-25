#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/ast.h"

// Define the global program node
ASTNode* programNode = NULL;

ASTNode* createLiteralExpr(int value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_INT;
    node->data.value = value;
    return node;
}

ASTNode* createVariableExpr(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_IDENTIFIER;
    node->data.varName.varName = strdup(name);
    return node;
}

ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, TokenType operator) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_OP_PLUS; // Or other operators as appropriate
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.operator = operator;
    node->data.bin_op.operatorText = NULL;  // Set operator text if needed
    return node;
}

ASTNode* createUnaryExpr(TokenType operator, ASTNode* operand) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = operator;
    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_FN;
    node->data.functionDecl.name = strdup(function_name);
    node->data.functionDecl.body = function_body;
    return node;
}

ASTNode* createReturnStatement(ASTNode* return_val) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_RETURN;
    node->data.returnStmt.returnValue = return_val;
    return node;
}

ASTNode* createBlock() {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_LBRACE;
    node->data.stmt.stmt = NULL;
    return node;
}

ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_IF;
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;
    return node;
}

ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_WHILE;
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}

ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_FOR;
    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;
    return node;
}

ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_CONST; // Or TOKEN_IDENTIFIER based on your implementation
    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.initializer = initializer;
    return node;
}

ASTNode* createExpressionStatement(ASTNode* expression) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_SEMICOLON; // Assuming expression statements end with a semicolon
    node->data.expr.expr = expression;
    return node;
}

void addStatementToBlock(ASTNode* block, ASTNode* statement) {
    // Assuming block->data.block.stmt is a linked list
    ASTNode* current = block->data.stmt.stmt;
    if (!current) {
        block->data.stmt.stmt = statement;
    } else {
        while (current->next) {
            current = current->next;
        }
        current->next = statement;
    }
}

void addFunctionToProgram(ASTNode* function) {
    // Assuming a global program node exists
    extern ASTNode* programNode; // Define this in your main file or somewhere appropriate
    if (!programNode) {
        programNode = createBlock();
    }
    addStatementToBlock(programNode, function);
}

ASTNode* parseFunctionCall(const char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_IDENTIFIER; // Assuming function calls are identified by their name
    node->data.functionCall.name = strdup(name);
    // node->data.functionCall.arguments = NULL; // Implement argument parsing if needed
    return node;
}
