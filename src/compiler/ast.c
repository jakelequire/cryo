#include "include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the global program node
ASTNode* programNode = NULL;

void printAST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; ++i) printf("  ");

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                printAST(node->data.program.statements[i], indent + 1);
            }
            break;
        case NODE_FUNCTION_DECLARATION:
            printf("Function: %s\n", node->data.functionDecl.name);
            printAST(node->data.functionDecl.body, indent + 1);
            break;
        case NODE_BLOCK:
            printf("Block\n");
            for (int i = 0; i < node->data.block.stmtCount; ++i) {
                printAST(node->data.block.statements[i], indent + 1);
            }
            break;
        case NODE_LITERAL:
            printf("Literal: %d\n", node->data.value);
            break;
        case NODE_VAR_NAME:
            printf("Variable: %s\n", node->data.varName.varName);
            break;
        case NODE_BINARY_EXPR:
            printf("Binary Expression: %s\n", node->data.bin_op.operatorText);
            printAST(node->data.bin_op.left, indent + 1);
            printAST(node->data.bin_op.right, indent + 1);
            break;
        case NODE_UNARY_EXPR:
            printf("Unary Expression: %d\n", node->data.unary_op.operator);
            printAST(node->data.unary_op.operand, indent + 1);
            break;
        case NODE_RETURN_STATEMENT:
            printf("Return Statement\n");
            printAST(node->data.returnStmt.returnValue, indent + 1);
            break;
        case NODE_EXPRESSION_STATEMENT:
            printf("Expression Statement\n");
            printAST(node->data.expr.expr, indent + 1);
            break;
        case NODE_FUNCTION_CALL:
            printf("Function Call: %s\n", node->data.functionCall.name);
            break;
        // Handle other node types...
        default:
            printf("Unknown Node\n");
            break;
    }
}

void freeAST(ASTNode* node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            if (node->data.program.statements) {
                for (int i = 0; i < node->data.program.stmtCount; ++i) {
                    freeAST(node->data.program.statements[i]);
                }
                free(node->data.program.statements);
            }
            break;
        case NODE_FUNCTION_DECLARATION:
            free(node->data.functionDecl.name);
            freeAST(node->data.functionDecl.body);
            break;
        case NODE_BLOCK:
            if (node->data.block.statements) {
                for (int i = 0; i < node->data.block.stmtCount; ++i) {
                    freeAST(node->data.block.statements[i]);
                }
                free(node->data.block.statements);
            }
            break;
        // Free other node types...
        default:
            break;
    }
    free(node);
}

ASTNode* createASTNode(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    node->type = type;
    node->line = 0; // Initialize line number
    node->firstChild = NULL;
    node->nextSibling = NULL;

    switch (type) {
        case NODE_PROGRAM:
            node->data.program.statements = NULL;
            node->data.program.stmtCount = 0;
            node->data.program.stmtCapacity = 0;
            break;
        case NODE_FUNCTION_DECLARATION:
            node->data.functionDecl.name = NULL;
            node->data.functionDecl.body = NULL;
            break;
        case NODE_BLOCK:
            node->data.block.statements = NULL;
            node->data.block.stmtCount = 0;
            node->data.block.stmtCapacity = 0;
            break;
        // Initialize other node types...
        default:
            break;
    }

    return node;
}

ASTNode* createLiteralExpr(int value) {
    ASTNode* node = createASTNode(NODE_LITERAL);
    if (!node) {
        return NULL;
    }
    node->data.value = value;
    return node;
}

ASTNode* createVariableExpr(const char* name) {
    ASTNode* node = createASTNode(NODE_VAR_NAME);
    if (!node) {
        return NULL;
    }
    node->data.varName.varName = strdup(name);
    return node;
}

ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoTokenType operator) {
    ASTNode* node = createASTNode(NODE_BINARY_EXPR);
    if (!node) {
        return NULL;
    }
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.operator = operator;

    // Assign descriptive text for the operator
    switch (operator) {
        case TOKEN_PLUS:
            node->data.bin_op.operatorText = "+";
            break;
        case TOKEN_MINUS:
            node->data.bin_op.operatorText = "-";
            break;
        case TOKEN_STAR:
            node->data.bin_op.operatorText = "*";
            break;
        case TOKEN_SLASH:
            node->data.bin_op.operatorText = "/";
            break;
        // Handle other operators...
        default:
            node->data.bin_op.operatorText = "unknown";
            break;
    }

    return node;
}

ASTNode* createUnaryExpr(CryoTokenType operator, ASTNode* operand) {
    ASTNode* node = createASTNode(NODE_UNARY_EXPR);
    if (!node) {
        return NULL;
    }
    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body) {
    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node) {
        return NULL;
    }
    node->data.functionDecl.name = strdup(function_name);
    node->data.functionDecl.body = function_body;
    return node;
}

ASTNode* createReturnStatement(ASTNode* return_val) {
    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.returnStmt.returnValue = return_val;
    return node;
}

ASTNode* createBlock() {
    ASTNode* node = createASTNode(NODE_BLOCK);
    if (!node) {
        return NULL;
    }
    node->data.block.statements = NULL;
    node->data.block.stmtCount = 0;
    node->data.block.stmtCapacity = 0;
    return node;
}

ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = createASTNode(NODE_IF_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;
    return node;
}

ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}

ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_FOR_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;
    return node;
}

ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer) {
    ASTNode* node = createASTNode(NODE_VAR_DECLARATION);
    if (!node) {
        return NULL;
    }
    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.initializer = initializer;
    return node;
}

ASTNode* createExpressionStatement(ASTNode* expression) {
    ASTNode* node = createASTNode(NODE_EXPRESSION_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.expr.expr = expression;
    return node;
}

ASTNode* createFunctionCallNode(const char* name, ASTNode** args, int argCount) {
    ASTNode* node = createASTNode(NODE_FUNCTION_CALL);
    if (!node) {
        return NULL;
    }
    node->data.functionCall.name = strdup(name);
    node->data.functionCall.args = args;
    node->data.functionCall.argCount = argCount;
    return node;
}

void addStatementToBlock(ASTNode* block, ASTNode* statement) {
    if (block->type != NODE_BLOCK) {
        return;
    }
    if (block->data.block.stmtCount >= block->data.block.stmtCapacity) {
        block->data.block.stmtCapacity = block->data.block.stmtCapacity == 0 ? 2 : block->data.block.stmtCapacity * 2;
        block->data.block.statements = realloc(block->data.block.statements, block->data.block.stmtCapacity * sizeof(ASTNode*));
    }
    block->data.block.statements[block->data.block.stmtCount++] = statement;
}

void addFunctionToProgram(ASTNode* program, ASTNode* function) {
    if (program->type != NODE_PROGRAM) {
        return;
    }
    if (program->data.program.stmtCount >= program->data.program.stmtCapacity) {
        program->data.program.stmtCapacity = program->data.program.stmtCapacity == 0 ? 2 : program->data.program.stmtCapacity * 2;
        program->data.program.statements = realloc(program->data.program.statements, program->data.program.stmtCapacity * sizeof(ASTNode*));
    }
    program->data.program.statements[program->data.program.stmtCount++] = function;
}
