/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *  
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          * 
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "compiler/ast.h"




// Define the global program node
ASTNode* programNode = NULL;


// <printAST>
void printAST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program:\n");
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                printAST(node->data.program.statements[i], indent + 1);
            }
            break;
            
        case NODE_VAR_DECLARATION:
            printf("Variable Declaration: %s\n", node->data.varDecl.name);
            printAST(node->data.varDecl.initializer, indent + 1);
            break;

        case NODE_LITERAL_EXPR:
            printf("Literal: %d\n", node->data.value);
            break;

        case NODE_FUNCTION_DECLARATION:
            printf("Function Declaration: %s\n", node->data.functionDecl.name);
            printAST(node->data.functionDecl.body, indent + 1);
            break;

        case NODE_BOOLEAN_LITERAL:
            printf("Boolean Literal: %s\n", node->data.value ? "true" : "false");
            break;

        case NODE_STRING_LITERAL:
            printf("String Literal: '%s'\n", node->data.str.str);
            break;
        // Add more cases for other node types
        default:
            printf("Unknown node type: %d\n", node->type);
            break;
    }
}
// </printAST>


// <freeAST>
void freeAST(ASTNode* node) {
    if (!node) {
        printf("[AST] No node to free\n");
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            if (node->data.program.statements) {
                for (int i = 0; i < node->data.program.stmtCount; ++i) {
                    printf("[AST] Freeing program statement: %d\n", node->data.program.statements[i]->type);
                    freeAST(node->data.program.statements[i]);
                }
                printf("[AST] Freeing program statements\n");
                free(node->data.program.statements);
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            printf("[AST] Freeing Function Declaration Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            free(node->data.functionDecl.name);
            freeAST(node->data.functionDecl.body);
            break;

        case NODE_BLOCK:
            printf("[AST] Freeing Block Node\n");
            if (node->data.block.statements) {
                for (int i = 0; i < node->data.block.stmtCount; ++i) {
                    printf("[AST] Freeing block statement: %d\n", node->data.block.statements[i]->type);
                    freeAST(node->data.block.statements[i]);
                }
                printf("[AST] Freeing block statements\n");
                free(node->data.block.statements);
            }
            printf("[AST] Freeing node: %d\n", node->type);
            break;

        case NODE_BINARY_EXPR:
            printf("[AST] Freeing Binary Expression Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            freeAST(node->data.bin_op.left);
            freeAST(node->data.bin_op.right);
            break;

        case NODE_LITERAL_EXPR:
            printf("[AST] Freeing Literal Expression Node\n");
            // No dynamic memory to free
            break;

        case NODE_VAR_DECLARATION:
            printf("[AST] Freeing Variable Declaration Node\n");
            printf("[AST] Freeing node: %d\n", node->type);
            free(node->data.varDecl.name);
            freeAST(node->data.varDecl.initializer);
            break;
            
        case NODE_STRING_LITERAL:
            printf("[AST] Freeing String Literal Node\n");
            free(node->data.str.str);
            break;
        // Free other node types...
        default:
            printf("[AST] Unknown Node Type. <DEFAULTED>\n");
            exit(0);
            break;
    }
    printf("[AST] Freeing node: %d\n", node->type);
    // free(node);
}
// </freeAST>


// <createASTNode>
ASTNode* createASTNode(CryoNodeType type) {
    printf("[AST] Creating node: %d\n", type);
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));  // Use calloc to initialize memory
    if (!node) {
        printf("[AST] [ERROR] Failed to allocate memory for AST node\n");
        return NULL;
    }
    node->type = type;
    node->line = 0; // Initialize line number
    node->firstChild = NULL;
    node->nextSibling = NULL;

    printf("[AST] Initialized node: %d\n", type);

    switch (type) {
        case NODE_FUNCTION_DECLARATION:
            node->data.functionDecl.returnType = NULL;
            node->data.functionDecl.params = NULL;
            node->data.functionDecl.name = NULL;
            node->data.functionDecl.body = NULL;
            break;
        case NODE_VAR_DECLARATION:
            node->data.varDecl.name = NULL;
            node->data.varDecl.initializer = NULL;
            break;
        case NODE_BLOCK:
            node->data.block.stmtCount = 0;
            node->data.block.stmtCapacity = 0;
            node->data.block.statements = NULL;
            break;
        // Initialize other node types...
        default:
            break;
    }

    return node;
}
// </createASTNode>


// <createLiteralExpr>
ASTNode* createLiteralExpr(int value) {
    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        return NULL;
    }
    node->data.value = value;
    return node;
}
// </createLiteralExpr>


// <createVariableExpr>
ASTNode* createVariableExpr(const char* name) {
    ASTNode* node = createASTNode(NODE_VAR_NAME);
    if (!node) {
        return NULL;
    }
    node->data.varName.varName = strdup(name);
    return node;
}
// </createVariableExpr>


// <createStringLiteralExpr>
ASTNode* createStringLiteralExpr(const char* str) {
    ASTNode* node = createASTNode(NODE_STRING_LITERAL);
    if (!node) {
        return NULL;
    }
    node->data.str.str = strdup(str);
    return node;
}
// </createStringLiteralExpr


// <createBinaryExpr>
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoTokenType operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_EXPR; // Ensure the correct node type is set
    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.operator = operator;
    printf("Created Binary Expression Node: left=%p, right=%p, operator=%d\n", left, right, operator);
    return node;
}
// </createBinaryExpr>


// <createUnaryExpr>
ASTNode* createUnaryExpr(CryoTokenType operator, ASTNode* operand) {
    ASTNode* node = createASTNode(NODE_UNARY_EXPR);
    if (!node) {
        return NULL;
    }
    node->data.unary_op.operator = operator;
    node->data.unary_op.operand = operand;
    return node;
}
// </createUnaryExpr>


// <createFunctionNode>
ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body) {
    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node) {
        return NULL;
    }
    node->data.functionDecl.name = strdup(function_name);
    node->data.functionDecl.body = function_body;
    return node;
}
// </createFunctionNode>


// <createReturnStatement>
ASTNode* createReturnStatement(ASTNode* return_val) {
    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.returnStmt.returnValue = return_val;
    return node;
}
// </createReturnStatement>


// <createBlock>
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
// </createBlock>


// <createIfStatement>
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
// </createIfStatement>


// <createWhileStatement>
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;
    return node;
}
// </createWhileStatement>


// <createForStatement>
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
// </createForStatement>


// <createVarDeclarationNode>
ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer, int line) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));  // Use calloc to initialize memory
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for variable declaration node\n");
        return NULL;
    }

    node->type = NODE_VAR_DECLARATION;
    node->line = line;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.initializer = initializer;
    node->data.varDecl.type = NULL; // Initialize type as NULL

    return node;
}
// </createVarDeclarationNode>


// <createStringExpr>
ASTNode* createStringExpr(const char* str) {
    ASTNode* node = createASTNode(NODE_STRING_LITERAL);
    if (!node) {
        return NULL;
    }
    node->data.str.str = strdup(str);
    return node;
}
// </createStringExpr>


// <createBooleanLiteralExpr>
ASTNode* createBooleanLiteralExpr(int value) {
    ASTNode* node = createASTNode(NODE_BOOLEAN_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for boolean literal node\n");
        return NULL;
    }
    node->data.value = value;
    printf("[AST] Boolean Literal Node Created: %s\n", value ? "true" : "false");
    return node;
}
// </createBooleanLiteralExpr>


// <createExpressionStatement>
ASTNode* createExpressionStatement(ASTNode* expression) {
    ASTNode* node = createASTNode(NODE_EXPRESSION_STATEMENT);
    if (!node) {
        return NULL;
    }
    node->data.expr.expr = expression;
    return node;
}
// </createExpressionStatement>


// <createFunctionCallNode>
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
// </createFunctionCallNode>


// <addStatementToBlock>
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
// </addStatementToBlock>


// <addFunctionToProgram>
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
// </addFunctionToProgram>
