#include "include/ast.h"
#include "include/semantics.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool analyzeNode(ASTNode* node);

bool analyze(ASTNode* root) {
    if (!root) return false;

    return analyzeNode(root);
}

bool analyzeVariableDeclaration(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing variable declaration: %s\n", node->data.varDecl.name);
    // Check if variable is redeclared in the same scope
    // Ensure the initialization expression is valid
    if (node->data.varDecl.initializer) {
        if (!analyzeNode(node->data.varDecl.initializer)) {
            printf("{Semantics} [ERROR] Invalid initialization for variable: %s\n", node->data.varDecl.name);
            return false;
        }
    }
    // Additional semantic checks for variable declaration
    return true;
}

bool analyzeFunction(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing function declaration: %s\n", node->data.functionDecl.name);
    // Check function parameters, return type, and body
    if (!analyzeNode(node->data.functionDecl.body)) {
        printf("{Semantics} [ERROR] Invalid function body for function: %s\n", node->data.functionDecl.name);
        return false;
    }
    return true;
}

bool analyzeIfStatement(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing if statement\n");
    // Check condition expression
    if (!analyzeNode(node->data.ifStmt.condition)) {
        printf("{Semantics} [ERROR] Invalid condition in if statement\n");
        return false;
    }
    // Analyze then branch
    if (!analyzeNode(node->data.ifStmt.thenBranch)) {
        printf("{Semantics} [ERROR] Invalid then branch in if statement\n");
        return false;
    }
    // Analyze else branch if it exists
    if (node->data.ifStmt.elseBranch && !analyzeNode(node->data.ifStmt.elseBranch)) {
        printf("{Semantics} [ERROR] Invalid else branch in if statement\n");
        return false;
    }
    return true;
}

bool analyzeWhileStatement(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing while statement\n");
    // Check condition expression
    if (!analyzeNode(node->data.whileStmt.condition)) {
        printf("{Semantics} [ERROR] Invalid condition in while statement\n");
        return false;
    }
    // Analyze body
    if (!analyzeNode(node->data.whileStmt.body)) {
        printf("{Semantics} [ERROR] Invalid body in while statement\n");
        return false;
    }
    return true;
}

bool analyzeForStatement(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing for statement\n");
    // Check initialization, condition, and iteration expressions
    if (node->data.forStmt.initializer && !analyzeNode(node->data.forStmt.initializer)) {
        printf("{Semantics} [ERROR] Invalid initialization in for statement\n");
        return false;
    }
    if (node->data.forStmt.condition && !analyzeNode(node->data.forStmt.condition)) {
        printf("{Semantics} [ERROR] Invalid condition in for statement\n");
        return false;
    }
    if (node->data.forStmt.increment && !analyzeNode(node->data.forStmt.increment)) {
        printf("{Semantics} [ERROR] Invalid iteration in for statement\n");
        return false;
    }
    // Analyze body
    if (!analyzeNode(node->data.forStmt.body)) {
        printf("{Semantics} [ERROR] Invalid body in for statement\n");
        return false;
    }
    return true;
}

bool analyzeBinaryExpression(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing binary expression with operator: %s\n", node->data.bin_op.operatorText);
    // Check left and right operand types
    if (!analyzeNode(node->data.bin_op.left)) {
        printf("{Semantics} [ERROR] Invalid left operand in binary expression\n");
        return false;
    }
    if (!analyzeNode(node->data.bin_op.right)) {
        printf("{Semantics} [ERROR] Invalid right operand in binary expression\n");
        return false;
    }
    // Additional semantic checks for binary expression
    return true;
}

bool analyzeUnaryExpression(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing unary expression with operator: %d\n", node->data.unary_op.operator);
    // Check operand type
    if (!analyzeNode(node->data.unary_op.operand)) {
        printf("{Semantics} [ERROR] Invalid operand in unary expression\n");
        return false;
    }
    // Additional semantic checks for unary expression
    return true;
}

bool analyzeReturn(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing return statement\n");
    // Check return expression type matches function return type
    if (node->data.returnStmt.returnValue && !analyzeNode(node->data.returnStmt.returnValue)) {
        printf("{Semantics} [ERROR] Invalid return value in return statement\n");
        return false;
    }
    return true;
}

bool analyzeBlock(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing block statement\n");
    for (int i = 0; i < node->data.block.stmtCount; ++i) {
        if (!analyzeNode(node->data.block.statements[i])) {
            printf("{Semantics} [ERROR] Failed to analyze block statement\n");
            return false;
        }
    }
    return true;
}

bool analyzeProgram(ASTNode* node) {
    printf("{Semantics} [DEBUG] Analyzing program\n");
    for (int i = 0; i < node->data.program.stmtCount; ++i) {
        if (!analyzeNode(node->data.program.statements[i])) {
            printf("{Semantics} [ERROR] Failed to analyze program statement\n");
            return false;
        }
    }
    return true;
}

bool analyzeNode(ASTNode* node) {
    if (!node) return true;

    printf("{Semantics} [DEBUG] Analyzing node of type %d...\n", node->type);

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                if (!analyzeNode(node->data.program.statements[i])) {
                    return false;
                }
            }
            break;
        case NODE_FUNCTION_DECLARATION:
            return analyzeFunction(node);

        case NODE_VAR_DECLARATION:
            return analyzeVariableDeclaration(node);

        case NODE_IF_STATEMENT:
            return analyzeIfStatement(node);

        case NODE_WHILE_STATEMENT:
            return analyzeWhileStatement(node);

        case NODE_FOR_STATEMENT:
            return analyzeForStatement(node);

        case NODE_BINARY_EXPR:
            return analyzeBinaryExpression(node);

        case NODE_UNARY_EXPR:
            return analyzeUnaryExpression(node);

        case NODE_RETURN_STATEMENT:
            return analyzeReturn(node);

        case NODE_BLOCK:
            return analyzeBlock(node);

        default:
            printf("{Semantics} [ERROR] Unknown node type %d\n", node->type);
            return false;
    }

    return analyzeNode(node->nextSibling);
}
