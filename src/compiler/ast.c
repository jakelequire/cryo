#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "include/ast.h"


// Define the global program node
ASTNode* programNode = NULL;

void printAST(ASTNode* node, int indent) {
    if (!node) {
        printf("%*s[AST_PRINT] Node is NULL\n", indent, "");
        return;
    }

    printf("%*s[AST_PRINT] Node type: %d\n", indent, "", node->type);

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                printAST(node->data.program.statements[i], indent + 2);
            }
            break;
        case NODE_FUNCTION_DECLARATION:
            printf("%*s[AST_PRINT] Function name: %s\n", indent + 2, "", node->data.functionDecl.name);
            printAST(node->data.functionDecl.body, indent + 2);
            break;
        case NODE_VAR_DECLARATION:
            printf("%*s[AST_PRINT] Variable name: %s\n", indent + 2, "", node->data.varDecl.name);
            printAST(node->data.varDecl.initializer, indent + 2);
            break;
        case NODE_RETURN_STATEMENT:
            printf("%*s[AST_PRINT] Return statement\n", indent + 2, "");
            printAST(node->data.returnStmt.returnValue, indent + 2);
            break;
        case NODE_BLOCK:
            printf("%*s[AST_PRINT] Block statement with %d statements\n", indent + 2, "", node->data.block.stmtCount);
            for (int i = 0; i < node->data.block.stmtCount; i++) {
                printAST(node->data.block.statements[i], indent + 2);
            }
            break;
        // Handle other node types...
        default:
            printf("%*s[AST_PRINT] Unknown node type: %d\n", indent + 2, "", node->type);
            break;
    }
}



void freeAST(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_IDENTIFIER:
            free(node->data.varName.varName);
            break;
        case TOKEN_OP_PLUS:
        case TOKEN_OP_MINUS:
        case TOKEN_OP_MUL_ASSIGN:
        case TOKEN_OP_DIV_ASSIGN:
            freeAST(node->data.bin_op.left);
            freeAST(node->data.bin_op.right);
            break;
        case TOKEN_KW_FN:
            free(node->data.functionDecl.name);
            freeAST(node->data.functionDecl.body);
            break;
        case TOKEN_KW_RETURN:
            freeAST(node->data.returnStmt.returnValue);
            break;
        case TOKEN_LBRACE:
            freeAST(node->data.stmt.stmt);
            break;
        case TOKEN_KW_IF:
            freeAST(node->data.ifStmt.condition);
            freeAST(node->data.ifStmt.thenBranch);
            freeAST(node->data.ifStmt.elseBranch);
            break;
        case TOKEN_KW_WHILE:
            freeAST(node->data.whileStmt.condition);
            freeAST(node->data.whileStmt.body);
            break;
        case TOKEN_KW_FOR:
            freeAST(node->data.forStmt.initializer);
            freeAST(node->data.forStmt.condition);
            freeAST(node->data.forStmt.increment);
            freeAST(node->data.forStmt.body);
            break;
        case TOKEN_KW_CONST:
            free(node->data.varDecl.name);
            freeAST(node->data.varDecl.initializer);
            break;
        case TOKEN_SEMICOLON:
            freeAST(node->data.expr.expr);
            break;
        default:
            break;
    }
    free(node);
}

ASTNode* createASTNode(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    return node;
}

ASTNode* createLiteralExpr(int value) {
    printf("[DEBUG] Creating literal expression with value: %d\n", value);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_LITERAL;
    node->data.value = value;
    node->next = NULL;
    return node;
}

ASTNode* createVariableExpr(const char* name) {
    printf("[DEBUG] Creating variable expression with name: %s\n", name);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_IDENTIFIER;
    node->data.varName.varName = strdup(name);
    return node;
}

ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoTokenType operator) {
    printf("[DEBUG] Creating binary expression with operator: %d\n", operator);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_OP_PLUS; // Or other operators as appropriate
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.operator = operator;
    node->data.bin_op.operatorText = NULL;  // Set operator text if needed
    return node;
}

ASTNode* createUnaryExpr(CryoTokenType operator, ASTNode* operand) {
    printf("[DEBUG] Creating unary expression with operator: %d\n", operator);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = operator;
    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}

ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body) {
    printf("[DEBUG] Creating function node with name: %s\n", function_name);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_DECLARATION;
    node->data.functionDecl.name = function_name;
    node->data.functionDecl.body = function_body;
    node->next = NULL;
    return node;
}

ASTNode* createReturnStatement(ASTNode* return_val) {
    printf("[DEBUG] Creating return statement\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_RETURN;
    node->data.returnStmt.returnValue = return_val;
    return node;
}

ASTNode* createBlock() {
    printf("[DEBUG] Creating block node\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->data.block.statements = NULL;
    node->data.block.stmtCount = 0;
    node->next = NULL;
    return node;
}

ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    printf("[DEBUG] Creating if statement\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_IF;
    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;
    return node;
}

ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    printf("[DEBUG] Creating while statement\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_WHILE;
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}

ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    printf("[DEBUG] Creating for statement\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_FOR;
    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;
    return node;
}

ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer) {
    printf("[DEBUG] Creating variable declaration node\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_KW_CONST; // Or TOKEN_IDENTIFIER based on your implementation
    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.initializer = initializer;
    return node;
}

ASTNode* createExpressionStatement(ASTNode* expression) {
    printf("[DEBUG] Creating expression statement\n");
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_SEMICOLON; // Assuming expression statements end with a semicolon
    node->data.expr.expr = expression;
    return node;
}

void addStatementToBlock(ASTNode* block, ASTNode* statement) {
    if (!block || block->type != NODE_BLOCK) {
        printf("[ERROR] Invalid block node\n");
        return;
    } else {
        printf("[DEBUG] Adding statement to block\n");
    }

    printf("[DEBUG] Adding statement to block\n");
    block->data.block.statements = realloc(block->data.block.statements, sizeof(ASTNode*) * (block->data.block.stmtCount + 1));
    block->data.block.statements[block->data.block.stmtCount++] = statement;
}

void addFunctionToProgram(ASTNode* function) {
    // Assuming a global program node exists
    printf("[DEBUG] Adding function to program\n");
    extern ASTNode* programNode; // Define this in your main file or somewhere appropriate
    if (!programNode) {
        printf("[DEBUG] Creating program node\n");
        programNode = createBlock();
    } else {
        printf("[DEBUG] Program node already exists\n");
    }
    printf("[DEBUG] Adding function to program\n");
    addStatementToBlock(programNode, function);
}

ASTNode* parseFunctionCall(const char* name) {
    printf("[DEBUG] Parsing function call: %s\n", name);
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = TOKEN_IDENTIFIER; // Assuming function calls are identified by their name
    node->data.functionCall.name = strdup(name);
    // node->data.functionCall.arguments = NULL; // Implement argument parsing if needed
    return node;
}
