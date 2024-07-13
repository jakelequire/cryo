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


/* ====================================================================== */
// @Global_Variables
ASTNode* programNode = NULL;



/* ====================================================================== */
/* @Node_Accessors */

// <printAST>
void printAST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) {
        printf(" ");
    }

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program Node with %d statements (capacity: %d)\n", node->data.program.stmtCount, node->data.program.stmtCapacity);
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                printAST(node->data.program.statements[i], indent + 2);
            }
            break;
        
        case NODE_FUNCTION_DECLARATION:
            printf("Function Declaration Node name: %s\n", node->data.functionDecl.function->name);
            printf("Function Declaration Node returnType: %s\n", CryoDataTypeToString(node->data.functionDecl.function->returnType));
            printf("Function Declaration Node visibility: %s\n", node->data.functionDecl.function->visibility);
            printf("Function Declaration Node params:\n");
            printAST(node->data.functionDecl.function->params, indent + 2);
            printf("Function Declaration Node body:\n");
            printAST(node->data.functionDecl.function->body, indent + 2);
            break;

        case NODE_VAR_DECLARATION:
            printf("Variable Declaration Node: %s\n", node->data.varDecl.name);
            break;
        
        case NODE_STATEMENT:
            printf("Statement Node\n");
            printAST(node->data.stmt.stmt, indent + 2);
            break;

        case NODE_EXPRESSION:
            printf("Expression Node\n");
            printAST(node->data.expr.expr, indent + 2);
            break;
        
        case NODE_BINARY_EXPR:
            printf("Binary Expression Node: %s\n", node->data.bin_op.operatorText);
            printAST(node->data.bin_op.left, indent + 2);
            printAST(node->data.bin_op.right, indent + 2);
            break;

        case NODE_UNARY_EXPR:
            printf("Unary Expression Node: %s\n", node->data.unary_op.operand);
            printAST(node->data.unary_op.operand, indent + 2);
            break;

        case NODE_LITERAL_EXPR:
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    printf("Integer Literal Node: %d\n", node->data.literalExpression.intValue);
                    break;
                case DATA_TYPE_STRING:
                    printf("Float Literal Node: %f\n", node->data.literalExpression.floatValue);
                    break;
                case DATA_TYPE_BOOLEAN:
                    printf("String Literal Node: %s\n", node->data.literalExpression.stringValue);
                    break;
            }
            break;

        case NODE_VAR_NAME:
            printf("Variable Name Node: %s\n", node->data.varName.varName);
            break;

        case NODE_FUNCTION_CALL:
            printf("Function Call Node: %s\n", node->data.functionCall.name);
            for (int i = 0; i < node->data.functionCall.argCount; i++) {
                printAST(node->data.functionCall.args[i], indent + 2);
            }
            break;
    
        case NODE_IF_STATEMENT:
            printf("If Statement Node\n");
            printAST(node->data.ifStmt.condition, indent + 2);
            printAST(node->data.ifStmt.thenBranch, indent + 2);
            printAST(node->data.ifStmt.elseBranch, indent + 2);
            break;

        case NODE_WHILE_STATEMENT:
            printf("While Statement Node\n");
            printAST(node->data.whileStmt.condition, indent + 2);
            printAST(node->data.whileStmt.body, indent + 2);
            break;

        case NODE_FOR_STATEMENT:
            printf("For Statement Node\n");
            printAST(node->data.forStmt.initializer, indent + 2);
            printAST(node->data.forStmt.condition, indent + 2);
            printAST(node->data.forStmt.increment, indent + 2);
            printAST(node->data.forStmt.body, indent + 2);
            break;

        case NODE_RETURN_STATEMENT:
            printf("Return Statement Node\n");
            printAST(node->data.returnStmt.returnValue, indent + 2);
            break;

        case NODE_BLOCK:
            printf("Block Node with %d statements (capacity: %d)\n", node->data.block.stmtCount, node->data.block.stmtCapacity);
            for (int i = 0; i < node->data.block.stmtCount; i++) {
                printAST(node->data.block.statements[i], indent + 2);
            }
            break;

        case NODE_FUNCTION_BLOCK:
            printf("Function Block Node\n");
            printAST(node->data.functionBlock.function, indent + 2);
            printAST(node->data.functionBlock.block, indent + 2);
            break;

        case NODE_EXPRESSION_STATEMENT:
            printf("Expression Statement Node\n");
            printAST(node->data.expr.expr, indent + 2);
            break;

        case NODE_ASSIGN:
            printf("Assignment Node\n");
            printf("UNIMPLEMENTED\n");
            break;

        case NODE_PARAM_LIST:
            printf("Parameter List Node with %d parameters (capacity: %d)\n", node->data.paramList.paramCount, node->data.paramList.paramCapacity);
            for (int i = 0; i < node->data.paramList.paramCount; i++) {
                printAST(node->data.paramList.params[i], indent + 2);
            }
            break;

        case NODE_TYPE:
            printf("Type Node\n");
            printf("UNIMPLEMENTED\n");
            break;

        case NODE_STRING_LITERAL:
            printf("String Literal Node: %s\n", node->data.literalExpression.stringValue);
            break;

        case NODE_STRING_EXPRESSION:
            printf("String Expression Node: %s\n", node->data.str.str);
            break;

        case NODE_BOOLEAN_LITERAL:
            printf("Boolean Literal Node: %d\n", node->data.literalExpression.booleanValue);
            break;

        case NODE_ARRAY_LITERAL:
            printf("Array Literal Node with %d elements (capacity: %d)\n", node->data.arrayLiteral.elementCount, node->data.arrayLiteral.elementCapacity);
            for (int i = 0; i < node->data.arrayLiteral.elementCount; i++) {
                printAST(node->data.arrayLiteral.elements[i], indent + 2);
            }
            break;

        case NODE_IMPORT_STATEMENT:
            printf("Import Statement Node: %s\n", node->data.importStatementNode.modulePath);
            break;

        case NODE_EXTERN_STATEMENT:
            printf("Extern Statement Node\n");
            if(node->data.externNode.decl.function != NULL) {
                printf("Extern Function Node name: %s\n", node->data.externNode.decl.function->name);
                printf("Extern Function Node returnType: %s\n", node->data.externNode.decl.function->returnType);
                printf("Extern Function Node visibility: %s\n", node->data.externNode.decl.function->visibility);
                printf("Extern Function Node params:\n");
                printAST(node->data.externNode.decl.function->params, indent + 2);
            }
            break;

        case NODE_EXTERN_FUNCTION:
            printf("Function Name: %s\n", node->data.externNode.decl.function->name);
            break;

        case NODE_ARG_LIST:
            printf("Argument List Node with %d arguments (capacity: %d)\n", node->data.argList.argCount, node->data.argList.argCapacity);
            for (int i = 0; i < node->data.argList.argCount; i++) {
                printAST(node->data.argList.args[i], indent + 2);
            }
            break;

        case NODE_UNKNOWN:
            printf("<Unknown Node>\n");
            break;

        default:
            printf("Unknown Node\n");
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
            for (int i = 0; i < node->data.program.stmtCount; i++) {
                freeAST(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
        
        case NODE_FUNCTION_DECLARATION:
            freeAST(node->data.functionDecl.function->body);
            freeAST(node->data.functionDecl.function->params);
            free(node->data.functionDecl.function->name);
            free(node->data.functionDecl.function);
            break;

        case NODE_VAR_DECLARATION:
            free(node->data.varDecl.name);
            freeAST(node->data.varDecl.initializer);
            break;
        
        case NODE_STATEMENT:
            freeAST(node->data.stmt.stmt);
            break;

        case NODE_EXPRESSION:
            freeAST(node->data.expr.expr);
            break;
        
        case NODE_BINARY_EXPR:
            freeAST(node->data.bin_op.left);
            freeAST(node->data.bin_op.right);
            free(node->data.bin_op.operatorText);
            break;

        case NODE_UNARY_EXPR:
            freeAST(node->data.unary_op.operand);
            break;

        case NODE_LITERAL_EXPR:
            switch (node->data.literalExpression.dataType) {
                case DATA_TYPE_INT:
                    break;
                case DATA_TYPE_STRING:
                    free(node->data.literalExpression.stringValue);
                    break;
                case DATA_TYPE_BOOLEAN:
                    break;
            }
            break;

        case NODE_VAR_NAME:
            free(node->data.varName.varName);
            break;

        case NODE_FUNCTION_CALL:
            free(node->data.functionCall.name);
            for (int i = 0; i < node->data.functionCall.argCount; i++) {
                freeAST(node->data.functionCall.args[i]);
            }
            free(node->data.functionCall.args);
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

        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.stmtCount; i++) {
                freeAST(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case NODE_FUNCTION_BLOCK:
            freeAST(node->data.functionBlock.function);
            freeAST(node->data.functionBlock.block);
            break;

        case NODE_EXPRESSION_STATEMENT:
            freeAST(node->data.expr.expr);
            break;

        case NODE_ASSIGN:
            printf("[AST] Assignment Node\n");
            printf("[AST] UNIMPLEMENTED\n");
            break;

        case NODE_PARAM_LIST:
            for (int i = 0; i < node->data.paramList.paramCount; i++) {
                freeAST(node->data.paramList.params[i]);
            }
            free(node->data.paramList.params);
            break;

        case NODE_TYPE:
            break;

        case NODE_STRING_LITERAL:
            free(node->data.literalExpression.stringValue);
            break;

        case NODE_STRING_EXPRESSION:
            free(node->data.str.str);
            break;

        case NODE_BOOLEAN_LITERAL:
            break;

        case NODE_ARRAY_LITERAL:
            for (int i = 0; i < node->data.arrayLiteral.elementCount; i++) {
                freeAST(node->data.arrayLiteral.elements[i]);
            }
            free(node->data.arrayLiteral.elements);
            break;

        case NODE_IMPORT_STATEMENT:
            free(node->data.importStatementNode.modulePath);
            break;

        case NODE_EXTERN_STATEMENT:
            freeAST(node->data.externNode.decl.function);
            break;

        case NODE_EXTERN_FUNCTION:
            free(node->data.externNode.decl.function->name);
            break;

        case NODE_ARG_LIST:
            for (int i = 0; i < node->data.argList.argCount; i++) {
                freeAST(node->data.argList.args[i]);
            }
            free(node->data.argList.args);
            break;

        case NODE_UNKNOWN:
            break;

        default:
            fprintf(stderr, "<!> [AST] Unknown Node\n");
            break;
    }
}
// </freeAST>



/* ====================================================================== */
/* @Node_Management */

// <createASTNode>
ASTNode* createASTNode(CryoNodeType type) {
    printf("[AST_DEBUG] Creating node: %d\n", type);
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "<!> [AST] Error: Failed to allocate memory for AST node\n");
        return NULL;
    }
    node->type = type;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    // Initialize based on node type
    switch (type) {
        case NODE_PROGRAM:
            node->data.program.statements = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.program.statements) {
                fprintf(stderr, "[AST] Failed to allocate memory for program statements\n");
                free(node);
                return NULL;
            }
            node->data.program.stmtCount = 0;
            node->data.program.stmtCapacity = 8;
            break;
            
        case NODE_FUNCTION_DECLARATION:
            node->data.functionDecl.function = (FunctionDeclNode*)calloc(1, sizeof(FunctionDeclNode));
            if (!node->data.functionDecl.function) {
                fprintf(stderr, "[AST] Failed to allocate memory for function declaration\n");
                free(node);
                return NULL;
            }
            node->data.functionDecl.function->name = NULL;
            node->data.functionDecl.function->params = NULL;
            node->data.functionDecl.function->body = NULL;
            node->data.functionDecl.function->returnType = DATA_TYPE_VOID;
            break;
            
        case NODE_FUNCTION_BLOCK:
            node->data.functionBlock.function = NULL;
            node->data.functionBlock.block = createASTNode(NODE_BLOCK);
            if (!node->data.functionBlock.block) {
                fprintf(stderr, "[AST] Failed to create block for function block\n");
                free(node);
                return NULL;
            }
            break;
            
        case NODE_PARAM_LIST:
            node->data.paramList.params = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.paramList.params) {
                fprintf(stderr, "[AST] Failed to allocate memory for parameter list\n");
                free(node);
                return NULL;
            }
            node->data.paramList.paramCount = 0;
            node->data.paramList.paramCapacity = 8;
            break;
            
        case NODE_VAR_DECLARATION:
            node->data.varDecl.name = NULL;
            node->data.varDecl.initializer = NULL;
            break;
            
        case NODE_VAR_NAME:
            node->data.varName.varName = NULL;
            break;
            
        case NODE_RETURN_STATEMENT:
            node->data.returnStmt.returnValue = NULL;
            break;
            
        case NODE_IF_STATEMENT:
            node->data.ifStmt.condition = NULL;
            node->data.ifStmt.thenBranch = NULL;
            node->data.ifStmt.elseBranch = NULL;
            break;
            
        case NODE_FOR_STATEMENT:
            node->data.forStmt.initializer = NULL;
            node->data.forStmt.condition = NULL;
            node->data.forStmt.increment = NULL;
            node->data.forStmt.body = NULL;
            break;
            
        case NODE_BINARY_EXPR:
            node->data.bin_op.left = NULL;
            node->data.bin_op.right = NULL;
            node->data.bin_op.op = OPERATOR_NA;
            break;
            
        case NODE_LITERAL_EXPR:
            node->data.literalExpression.dataType = DATA_TYPE_UNKNOWN;
            break;
            
        case NODE_FUNCTION_CALL:
            node->data.functionCall.name = NULL;
            node->data.functionCall.args = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.functionCall.args) {
                fprintf(stderr, "[AST] Failed to allocate memory for function call arguments\n");
                free(node);
                return NULL;
            }
            node->data.functionCall.argCount = 0;
            break;
            
        case NODE_BLOCK:
            node->data.block.statements = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.block.statements) {
                fprintf(stderr, "[AST] Failed to allocate memory for block statements\n");
                free(node);
                return NULL;
            }
            node->data.block.stmtCount = 0;
            node->data.block.stmtCapacity = 8;
            break;
            
        case NODE_UNARY_EXPR:
            node->data.unary_op.operand = NULL;
            break;
            
        case NODE_STRING_LITERAL:
            node->data.str.str = NULL;
            break;
            
        case NODE_BOOLEAN_LITERAL:
            node->data.boolean.value = 0;
            break;
            
        case NODE_ARRAY_LITERAL:
            node->data.arrayLiteral.elements = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.arrayLiteral.elements) {
                fprintf(stderr, "[AST] Failed to allocate memory for array literal elements\n");
                free(node);
                return NULL;
            }
            node->data.arrayLiteral.elementCount = 0;
            node->data.arrayLiteral.elementCapacity = 8;
            break;
            
        case NODE_IMPORT_STATEMENT:
            node->data.importStatementNode.modulePath = NULL;
            break;
            
        case NODE_EXTERN_STATEMENT:
            node->data.externNode.decl.function = NULL;
            break;
            
        case NODE_EXTERN_FUNCTION:
            node->data.externNode.decl.function = (FunctionDeclNode*)calloc(1, sizeof(FunctionDeclNode));
            if (!node->data.externNode.decl.function) {
                fprintf(stderr, "[AST] Failed to allocate memory for extern function\n");
                free(node);
                return NULL;
            }
            break;
            
        case NODE_ARG_LIST:
            node->data.argList.args = (ASTNode**)calloc(8, sizeof(ASTNode*));
            if (!node->data.argList.args) {
                fprintf(stderr, "[AST] Failed to allocate memory for argument list\n");
                free(node);
                return NULL;
            }
            node->data.argList.argCount = 0;
            node->data.argList.argCapacity = 8;
            break;
            
        default:
            fprintf(stderr, "<!> [AST] Error: Unknown node type during creation: %d\n", type);
            break;
    }

    printf("[AST] Created node of type: %d\n", type);

    return node;
}
// </createASTNode>


// <addChildNode>
void addChildNode(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) {
        fprintf(stderr, "[AST] Parent or child node is NULL\n");
        return;
    }

    if (!parent->firstChild) {
        parent->firstChild = child;
    } else {
        ASTNode* current = parent->firstChild;
        while (current->nextSibling) {
            current = current->nextSibling;
        }
        current->nextSibling = child;
    }
}
// </addChildNode>


// <addStatementToBlock>
void addStatementToBlock(ASTNode* blockNode, ASTNode* statement) {
    if (blockNode->type != NODE_BLOCK && blockNode->type != NODE_FUNCTION_BLOCK) {
        printf("[AST] Error: addStatementToBlock called on non-block node\n");
        return;
    }

    // Debugging initial state
    if (blockNode->data.block.stmtCount >= blockNode->data.block.stmtCapacity) {
        blockNode->data.block.stmtCapacity *= 2;
        blockNode->data.block.statements = (ASTNode**)
        printf("[AST] Reallocated block statement memory: New capacity = %zu\n", blockNode->data.block.stmtCapacity);
    } else {
        printf("[AST] Block statement memory is sufficient\n");
    }

    blockNode->data.block.statements[blockNode->data.block.stmtCount++] = statement;
    // Debugging final state
    printf("[AST] Final state: stmtCount = %zu, stmtCapacity = %zu\n", blockNode->data.block.stmtCount, blockNode->data.block.stmtCapacity);
}
// </addStatementToBlock>


// <addFunctionToProgram>
void addFunctionToProgram(ASTNode* program, ASTNode* function) {
    if (!program || !function) {
        fprintf(stderr, "[AST] Program or function node is NULL\n");
        return;
    }

    if (program->data.program.stmtCount >= program->data.program.stmtCapacity) {
        program->data.program.stmtCapacity *= 2;
        program->data.program.statements = (ASTNode**)realloc(program->data.program.statements, sizeof(ASTNode*) * program->data.program.stmtCapacity);
        if (!program->data.program.statements) {
            fprintf(stderr, "[AST] Failed to reallocate memory for program statements\n");
            return;
        }
    }

    program->data.program.statements[program->data.program.stmtCount++] = function;
    addChildNode(program, function);
}
// </addFunctionToProgram>



/* ====================================================================== */
/* @Node_Creation - Expressions & Statements */


ASTNode* createProgramNode() {
    printf("[AST] Creating Program Node\n");

    ASTNode* node = createASTNode(NODE_PROGRAM);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program node\n");
        return NULL;
    }

    node->data.program.statements = (ASTNode**)calloc(8, sizeof(ASTNode*));
    if (!node->data.program.statements) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program statements\n");
        free(node);
        return NULL;
    }

    node->data.program.stmtCount = 0;
    node->data.program.stmtCapacity = 8;

    printf("[AST] Created Program Node\n");
    return node;
}


// <createLiteralExpr>
ASTNode* createLiteralExpr(int value) {
    printf("[AST] Creating Literal Expression Node: %d\n", value);

    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create literal expression node\n");
        return NULL;
    }

    node->data.literalExpression.dataType = DATA_TYPE_INT;
    node->data.literalExpression.intValue = value;

    printf("[AST] Created Literal Expression Node: %d\n", value);
    return node;
}
// </createLiteralExpr>


// <createExpressionStatement>
ASTNode* createExpressionStatement(ASTNode* expression) {
    printf("[AST] Creating Expression Statement Node\n");

    ASTNode* node = createASTNode(NODE_EXPRESSION_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create expression statement node\n");
        return NULL;
    }

    node->data.expr.expr = expression;

    printf("[AST] Created Expression Statement Node\n");
    return node;
}
// </createExpressionStatement>


// <createBinaryExpr>
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoOperatorType op) {
    printf("[AST] Creating Binary Expression Node\n");

    ASTNode* node = createASTNode(NODE_BINARY_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create binary expression node\n");
        return NULL;
    }

    node->data.bin_op.left = left;
    node->data.bin_op.right = right;
    node->data.bin_op.op = op;
    node->data.bin_op.operatorText = CryoOperatorTypeToString(op);

    printf("[AST] Created Binary Expression Node\n");
    return node;
}
// </createBinaryExpr>


// <createUnaryExpr>
ASTNode* createUnaryExpr(CryoTokenType op, ASTNode* operand) {
    printf("[AST] Creating Unary Expression Node\n");

    ASTNode* node = createASTNode(NODE_UNARY_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create unary expression node\n");
        return NULL;
    }

    node->data.unary_op.op = op;
    node->data.unary_op.operand = operand;

    printf("[AST] Created Unary Expression Node\n");
    return node;
}
// </createUnaryExpr>



/* ====================================================================== */
/* @Node_Creation - Literals */


// <createIntLiteralNode>
ASTNode* createIntLiteralNode(int value) {
    printf("[AST] Creating Integer Literal Node: %d\n", value);

    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create integer literal node\n");
        return NULL;
    }

    node->data.literalExpression.dataType = DATA_TYPE_INT;
    node->data.literalExpression.intValue = value;

    printf("[AST] Created Integer Literal Node: %d\n", value);
    return node;
}
// </createIntLiteralNode>


// <createFloatLiteralNode>
ASTNode* createFloatLiteralNode(float value) {
    printf("[AST] Creating Float Literal Node: %f\n", value);

    ASTNode* node = createASTNode(NODE_LITERAL_EXPR);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create float literal node\n");
        return NULL;
    }

    node->data.literalExpression.dataType = DATA_TYPE_FLOAT;
    node->data.literalExpression.floatValue = value;

    printf("[AST] Created Float Literal Node: %f\n", value);
    return node;
}
// </createFloatLiteralNode>


// <createStringLiteralNode>
ASTNode* createStringLiteralNode(char* value) {
    printf("[AST] Creating String Literal Node: %s\n", value);

    ASTNode* node = createASTNode(NODE_STRING_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create string literal node\n");
        return NULL;
    }

    node->data.str.str = strdup(value);

    printf("[AST] Created String Literal Node: %s\n", value);
    return node;
}
// </createStringLiteralNode>


// <createBooleanLiteralNode>
ASTNode* createBooleanLiteralNode(int value) {
    printf("[AST] Creating Boolean Literal Node: %d\n", value);

    ASTNode* node = createASTNode(NODE_BOOLEAN_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create boolean literal node\n");
        return NULL;
    }

    node->data.boolean.value = value;

    printf("[AST] Created Boolean Literal Node: %d\n", value);
    return node;
}
// </createBooleanLiteralNode>


// <createIdentifierNode>
ASTNode* createIdentifierNode(char* name) {
    printf("[AST] Creating Identifier Node: %s\n", name);

    ASTNode* node = createASTNode(NODE_VAR_NAME);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create identifier node\n");
        return NULL;
    }

    node->data.varName.varName = strdup(name);

    printf("[AST] Created Identifier Node: %s\n", name);
    return node;
}
// </createIdentifierNode>



/* ====================================================================== */
/* @Node_Blocks - Blocks */

// <createBlockNode>
ASTNode* createBlockNode() {
    printf("[AST] Creating Block Node\n");

    ASTNode* node = createASTNode(NODE_BLOCK);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create block node\n");
        return NULL;
    }

    node->data.block.statements = (ASTNode**)calloc(8, sizeof(ASTNode*));
    if (!node->data.block.statements) {
        fprintf(stderr, "[AST] Failed to allocate memory for block statements\n");
        free(node);
        return NULL;
    }

    node->data.block.stmtCount = 0;
    node->data.block.stmtCapacity = 8;

    printf("\n[AST] Created Block Node\n");
    return node;
}
// </createBlockNode>


// <createFunctionBlock>
ASTNode* createFunctionBlock(ASTNode* function) {
    printf("[AST] Creating Function Block Node\n");

    ASTNode* node = createASTNode(NODE_FUNCTION_BLOCK);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create function block node\n");
        return NULL;
    }

    node->data.functionBlock.function = function;
    node->data.functionBlock.block = createBlockNode();

    printf("\n[AST] Created Function Block Node\n");
    return node;
}
// </createFunctionBlock>


// <createIfBlock>
ASTNode* createIfBlock(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    printf("[AST] Creating If Block Node\n");

    ASTNode* node = createASTNode(NODE_IF_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create if block node\n");
        return NULL;
    }

    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;

    printf("[AST] Created If Block Node\n");
    return node;
}
// </createIfBlock>


// <createForBlock>
ASTNode* createForBlock(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    printf("[AST] Creating For Block Node\n");

    ASTNode* node = createASTNode(NODE_FOR_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create for block node\n");
        return NULL;
    }

    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;

    printf("[AST] Created For Block Node\n");
    return node;
}
// </createForBlock>


// <createWhileBlock>
ASTNode* createWhileBlock(ASTNode* condition, ASTNode* body) {
    printf("[AST] Creating While Block Node\n");

    ASTNode* node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create while block node\n");
        return NULL;
    }

    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;

    printf("[AST] Created While Block Node\n");
    return node;
}
// </createWhileBlock>



/* ====================================================================== */
/* @Node_Blocks - Literals */

// <createBooleanLiteralExpr>
ASTNode* createBooleanLiteralExpr(int value) {
    printf("[AST] Creating Boolean Literal Expression Node: %d\n", value);

    ASTNode* node = createASTNode(NODE_BOOLEAN_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create boolean literal expression node\n");
        return NULL;
    }

    node->data.literalExpression.dataType = DATA_TYPE_BOOLEAN;
    node->data.literalExpression.booleanValue = value;

    return node;
}
// </createBooleanLiteralExpr>


// <createStringLiteralExpr>
ASTNode* createStringLiteralExpr(char* str) {
    printf("[AST] Creating String Literal Expression Node: %s\n", str);

    ASTNode* node = createASTNode(NODE_STRING_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create string literal expression node\n");
        return NULL;
    }

    node->data.literalExpression.dataType = DATA_TYPE_STRING;
    node->data.literalExpression.stringValue = strdup(str);

    return node;
}
// </createStringLiteralExpr>


// <createStringExpr>
ASTNode* createStringExpr(char* str) {
    printf("[AST] Creating String Expression Node: %s\n", str);

    ASTNode* node = createASTNode(NODE_STRING_EXPRESSION);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create string expression node\n");
        return NULL;
    }

    node->data.str.str = strdup(str);

    return node;
}
// </createStringExpr>



/* ====================================================================== */
/* @Node_Creation - Variables */

// <createVarDeclarationNode>
ASTNode* createVarDeclarationNode(char* var_name, CryoDataType dataType, ASTNode* initializer, int line, bool isGlobal) {
    printf("[AST] Creating Variable Declaration Node: %s\n", var_name);

    ASTNode* node = createASTNode(NODE_VAR_DECLARATION);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create variable declaration node\n");
        return NULL;
    }

    node->data.varDecl.name = strdup(var_name);
    node->data.varDecl.dataType = dataType;
    node->data.varDecl.initializer = initializer;
    node->data.varDecl.isGlobal = isGlobal;
    node->data.varDecl.isReference = false;
    node->data.varDecl.scopeLevel = 0;

    printf("[AST] Created Variable Declaration Node: %s\n", var_name);
    return node;
}
// </createVarDeclarationNode>


// <createVariableExpr>
ASTNode* createVariableExpr(char* name, bool isReference) {
    printf("[AST] Creating Variable Expression Node: %s\n", name);

    ASTNode* node = createASTNode(NODE_VAR_NAME);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create variable expression node\n");
        return NULL;
    }

    node->data.varName.varName = strdup(name);
    node->data.varName.isReference = isReference;

    printf("[AST] Created Variable Expression Node: %s\n", name);
    return node;
}
// </createVariableExpr>



/* ====================================================================== */
/* @Node_Creation - Functions */

// <createFunctionNode>
ASTNode* createFunctionNode(CryoVisibilityType visibility, char* function_name, ASTNode* params, ASTNode* function_body, CryoDataType returnType) {
    printf("[AST] Creating Function Node: %s\n", function_name);

    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create function node\n");
        return NULL;
    }

    node->data.functionDecl.function->visibility = visibility;
    node->data.functionDecl.function->name = strdup(function_name);
    node->data.functionDecl.function->params = params;
    node->data.functionDecl.function->body = function_body;
    node->data.functionDecl.function->returnType = returnType;

    printf("[AST] Created Function Node: %s\n", function_name);
    return node;
}
// </createFunctionNode>


// <createExternDeclNode>
ASTNode* createExternDeclNode(char* functionName, ASTNode* params, CryoDataType returnType) {
    printf("[AST] Creating Extern Declaration Node: %s\n", functionName);

    ASTNode* node = createASTNode(NODE_EXTERN_FUNCTION);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create extern declaration node\n");
        return NULL;
    }

    node->data.externNode.decl.function->name = strdup(functionName);
    node->data.externNode.decl.function->params = params;
    node->data.externNode.decl.function->returnType = returnType;

    printf("[AST] Created Extern Declaration Node: %s\n", functionName);
    return node;
}
// </createExternDeclNode>


// <createFunctionCallNode>
ASTNode* createFunctionCallNode(char* name, ASTNode** args, int argCount) {
    printf("[AST] Creating Function Call Node: %s\n", name);

    ASTNode* node = createASTNode(NODE_FUNCTION_CALL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create function call node\n");
        return NULL;
    }

    node->data.functionCall.name = strdup(name);
    node->data.functionCall.args = args;
    node->data.functionCall.argCount = argCount;

    printf("[AST] Created Function Call Node: %s\n", name);
    return node;
}
// </createFunctionCallNode>


// <createReturnNode>
ASTNode* createReturnNode(ASTNode* returnValue) {
    printf("[AST] Creating Return Node\n");

    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create return node\n");
        return NULL;
    }

    node->data.returnStmt.returnValue = returnValue;

    printf("[AST] Created Return Node\n");
    return node;
}
// </createReturnNode>


// <createReturnStatement>
ASTNode* createReturnExpression(ASTNode* returnExpression, CryoDataType returnType) {
    printf("[AST] Creating Return Expression Node\n");

    ASTNode* node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create return expression node\n");
        return NULL;
    }
    
    node->data.returnStmt.returnType = returnType;
    node->data.returnStmt.expression = returnExpression;
}
// </createReturnStatement>



/* ====================================================================== */
/* @Node_Creation - Parameters */

// <createParamListNode>
ASTNode* createParamListNode(void) {
    printf("[AST] Creating Parameter List Node\n");

    ASTNode* node = createASTNode(NODE_PARAM_LIST);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create parameter list node\n");
        return NULL;
    }

    node->data.paramList.params = NULL;
    node->data.paramList.paramCount = 0;
    node->data.paramList.paramCapacity = 0;

    printf("[AST] Created Parameter List Node\n");
    return node;    
}
// </createParamListNode>


// </createArgumentListNode>
ASTNode* createArgumentListNode(void) {
    printf("[AST] Creating Argument List Node\n");

    ASTNode* node = createASTNode(NODE_ARG_LIST);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create argument list node\n");
        return NULL;
    }

    node->data.argList.args = NULL;
    node->data.argList.argCount = 0;
    node->data.argList.argCapacity = 0;

    printf("[AST] Created Argument List Node\n");
    return node;
}
// </createArgumentListNode>


// <createParamNode>
ASTNode* createParamNode(char* name, CryoDataType type) {
    printf("[AST] Creating Parameter Node: %s\n", name);

    ASTNode* node = createASTNode(NODE_VAR_DECLARATION);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create parameter node\n");
        return NULL;
    }

    node->data.varDecl.name = strdup(name);
    node->data.varDecl.initializer = NULL;
    node->data.varDecl.isReference = false;
    node->data.varDecl.scopeLevel = 1;

    printf("[AST] Created Parameter Node: %s\n", name);
    return node;
}
// </createParamNode>



/* ====================================================================== */
/* @Node_Creation - Modules & Externals */

// <createImportNode>
ASTNode* createImportNode(ASTNode* importPath) {
    printf("[AST] Creating Import Node\n");

    ASTNode* node = createASTNode(NODE_IMPORT_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create import node\n");
        return NULL;
    }

    node->data.importStatementNode.modulePath = (char*)importPath;
    
    return node;
}
// </createImportNode>


// <createExternNode>
ASTNode* createExternNode(ASTNode* externNode) {
    printf("[AST] Creating Extern Node\n");

    ASTNode* node = createASTNode(NODE_EXTERN_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create extern node\n");
        return NULL;
    }

    node->data.externNode.type = NODE_UNKNOWN;
    node->data.externNode.decl.function = externNode;
    
    return node;
}
// </createExternNode>



/* ====================================================================== */
/* @Node_Creation - Conditionals */


// <createIfStatement>
ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    printf("[AST] Creating If Statement Node\n");

    ASTNode* node = createASTNode(NODE_IF_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create if statement node\n");
        return NULL;
    }

    node->data.ifStmt.condition = condition;
    node->data.ifStmt.thenBranch = then_branch;
    node->data.ifStmt.elseBranch = else_branch;

    printf("[AST] Created If Statement Node\n");
    return node;
}
// </createIfStatement>


// <createForStatement>
ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    printf("[AST] Creating For Statement Node\n");

    ASTNode* node = createASTNode(NODE_FOR_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create for statement node\n");
        return NULL;
    }

    node->data.forStmt.initializer = initializer;
    node->data.forStmt.condition = condition;
    node->data.forStmt.increment = increment;
    node->data.forStmt.body = body;

    printf("[AST] Created For Statement Node\n");
    return node;
}
// </createForStatement>


// <createWhileStatement>
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    printf("[AST] Creating While Statement Node\n");

    ASTNode* node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create while statement node\n");
        return NULL;
    }

    node->data.whileStmt.condition = condition;
    node->data.whileStmt.body = body;

    printf("[AST] Created While Statement Node\n");
    return node;
}
// </createWhileStatement>


/* ====================================================================== */
/* @Node_Creation - Arrays */

// <createArrayLiteralNode>
ASTNode* createArrayLiteralNode() {
    printf("[AST] Creating Array Literal Node\n");

    ASTNode* node = createASTNode(NODE_ARRAY_LITERAL);
    if (!node) {
        fprintf(stderr, "[AST] Failed to create array literal node\n");
        return NULL;
    }

    node->data.arrayLiteral.elements = NULL;
    node->data.arrayLiteral.elementCount = 0;
    node->data.arrayLiteral.elementCapacity = 0;

    printf("[AST] Created Array Literal Node\n");
    return node;
}
// </createArrayLiteralNode>

