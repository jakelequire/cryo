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
ASTNode *programNode = NULL;

/* ====================================================================== */
/* @Node_Accessors */

// <printAST>
void printAST(ASTNode *node, int indent)
{
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
        printf(" ");

    switch (node->metaData->type)
    {
    case NODE_PROGRAM:
        printf("Program Node with %zu statements (capacity: %zu)\n",
               node->data.program->statementCount,
               node->data.program->statementCapacity);
        for (size_t i = 0; i < node->data.program->statementCount; i++)
        {
            printAST(node->data.program->statements[i], indent + 2);
        }
        break;

    case NODE_FUNCTION_DECLARATION:
        printf("Function Declaration Node name: %s\n", node->data.functionDecl->name);
        printf("Function Declaration Node returnType: %s\n",
               CryoDataTypeToString(node->data.functionDecl->returnType));
        printf("Function Declaration Node visibility: %s\n",
               CryoVisibilityTypeToString(node->data.functionDecl->visibility));
        printf("Function Declaration Node params:\n");
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            printAST(node->data.functionDecl->params[i], indent + 2);
        }
        printf("Function Declaration Node body:\n");
        printAST(node->data.functionDecl->body, indent + 2);
        break;

    case NODE_VAR_DECLARATION:
        printf("Variable Declaration Node: %s\n", strdup(node->data.varDecl->name));
        printf("Variable Type: %s\n", CryoDataTypeToString(node->data.varDecl->type));
        printf("Is Global: %s\n", node->data.varDecl->isGlobal ? "true" : "false");
        printf("Is Reference: %s\n", node->data.varDecl->isReference ? "true" : "false");
        free(node->data.varDecl->name);
        free(node->data.varDecl);
        break;

    case NODE_EXPRESSION:
        printf("Expression Node\n");
        free(node->data.expression);
        break;

    case NODE_BINARY_EXPR:
        printf("Binary Expression Node: %s\n", CryoOperatorTypeToString(node->data.bin_op->op));
        printAST(node->data.bin_op->left, indent + 2);
        printAST(node->data.bin_op->right, indent + 2);
        break;

    case NODE_UNARY_EXPR:
        printAST(node->data.unary_op->operand, indent + 2);
        break;

    case NODE_LITERAL_EXPR:
        switch (node->data.literal->dataType)
        {
        case DATA_TYPE_INT:
            printf("Integer Literal Node: %d\n", node->data.literal->intValue);
            break;
        case DATA_TYPE_FLOAT:
            printf("Float Literal Node: %f\n", node->data.literal->floatValue);
            break;
        case DATA_TYPE_STRING:
            printf("String Literal Node: %s\n", node->data.literal->stringValue);
            break;
        case DATA_TYPE_BOOLEAN:
            printf("Boolean Literal Node: %s\n", node->data.literal->booleanValue ? "true" : "false");
            break;
        default:
            printf("Unknown Literal Type\n");
        }
        break;
    case NODE_STRING_EXPRESSION:
        printf("String Expression Node: %s\n", node->data.literal->stringValue);
        break;
    case NODE_VAR_NAME:
        printf("Variable Name Node: %s\n", node->data.varName->varName);
        printf("Is Reference: %s\n", node->data.varName->isRef ? "true" : "false");
        break;

    case NODE_FUNCTION_CALL:
        printf("Function Call Node: %s\n", node->data.functionCall->name);
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            printAST(node->data.functionCall->args[i], indent + 2);
        }
        break;

    case NODE_IF_STATEMENT:
        printf("If Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.ifStatement->condition, indent + 2);
        printf("Then Branch:\n");
        printAST(node->data.ifStatement->thenBranch, indent + 2);
        if (node->data.ifStatement->elseBranch)
        {
            printf("Else Branch:\n");
            printAST(node->data.ifStatement->elseBranch, indent + 2);
        }
        break;

    case NODE_WHILE_STATEMENT:
        printf("While Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.whileStatement->condition, indent + 2);
        printf("Body:\n");
        printAST(node->data.whileStatement->body, indent + 2);
        break;

    case NODE_FOR_STATEMENT:
        printf("For Statement Node\n");
        printf("Initializer:\n");
        printAST(node->data.forStatement->initializer, indent + 2);
        printf("Condition:\n");
        printAST(node->data.forStatement->condition, indent + 2);
        printf("Increment:\n");
        printAST(node->data.forStatement->increment, indent + 2);
        printf("Body:\n");
        printAST(node->data.forStatement->body, indent + 2);
        break;

    case NODE_RETURN_STATEMENT:
        printf("Return Statement Node\n");
        if (node->data.returnStatement->returnValue)
        {
            printAST(node->data.returnStatement->returnValue, indent + 2);
        }
        break;

    case NODE_BLOCK:
        printf("Block Node with %d statements (capacity: %d)\n",
               node->data.block->statementCount,
               node->data.block->statementCapacity);
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            printAST(node->data.block->statements[i], indent + 2);
        }
        break;

    case NODE_FUNCTION_BLOCK:
        printf("Function Block Node\n");
        printf("Function:\n");
        printAST(node->data.functionBlock->function, indent + 2);
        printf("Block:\n");
        printAST((ASTNode *)node->data.functionBlock->statements, indent + 2);
        break;

    case NODE_PARAM_LIST:
        printf("Parameter List Node with %d parameters (capacity: %d)\n",
               node->data.paramList->paramCount,
               node->data.paramList->paramCapacity);
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            printAST((ASTNode *)node->data.paramList->params[i], indent + 2);
        }
        break;

    case NODE_ARG_LIST:
        printf("Argument List Node with %d arguments (capacity: %d)\n",
               node->data.argList->argCount,
               node->data.argList->argCapacity);
        for (int i = 0; i < node->data.argList->argCount; i++)
        {
            printAST((ASTNode *)node->data.argList->args[i], indent + 2);
        }
        break;

    case NODE_ARRAY_LITERAL:
        printf("Array Literal Node with %d elements (capacity: %d)\n",
               node->data.array->elementCount,
               node->data.array->elementCapacity);
        for (int i = 0; i < node->data.array->elementCount; i++)
        {
            printAST(node->data.array->elements[i], indent + 2);
        }
        break;

    case NODE_NAMESPACE:
        printf("Namespace Node: %s\n", node->metaData->moduleName);
        break;

    case NODE_UNKNOWN:
        printf("<Unknown Node>\n");
        break;

    default:
        printf("Unhandled Node Type: %s\n", CryoNodeTypeToString(node->metaData->type));
        break;
    }
}
// </printAST>

// <freeAST>
void freeAST(ASTNode *node)
{
    if (!node)
    {
        printf("[AST] No node to free\n");
        return;
    }

    switch (node->metaData->type)
    {
    case NODE_PROGRAM:
        for (size_t i = 0; i < node->data.program->statementCount; i++)
        {
            freeAST(node->data.program->statements[i]);
        }
        free(node->data.program->statements);
        free(node->data.program);
        break;

    case NODE_FUNCTION_DECLARATION:
        freeAST(node->data.functionDecl->body);
        free(node->data.functionDecl->name);
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            freeAST(node->data.functionDecl->params[i]);
        }
        free(node->data.functionDecl->params);
        free(node->data.functionDecl);
        break;

    case NODE_VAR_DECLARATION:
        free(node->data.varDecl->name);
        free(node->data.varDecl);
        break;

    case NODE_EXPRESSION:
        free(node->data.expression);
        break;

    case NODE_BINARY_EXPR:
        freeAST(node->data.bin_op->left);
        freeAST(node->data.bin_op->right);
        free(node->data.bin_op);
        break;

    case NODE_UNARY_EXPR:
        freeAST(node->data.unary_op->operand);
        free(node->data.unary_op);
        break;

    case NODE_LITERAL_EXPR:
        if (node->data.literal->dataType == DATA_TYPE_STRING)
        {
            free(node->data.literal->stringValue);
        }
        free(node->data.literal);
        break;

    case NODE_VAR_NAME:
        free(node->data.varName->varName);
        free(node->data.varName);
        break;

    case NODE_FUNCTION_CALL:
        free(node->data.functionCall->name);
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            freeAST(node->data.functionCall->args[i]);
        }
        free(node->data.functionCall->args);
        free(node->data.functionCall);
        break;

    case NODE_IF_STATEMENT:
        freeAST(node->data.ifStatement->condition);
        freeAST(node->data.ifStatement->thenBranch);
        if (node->data.ifStatement->elseBranch)
        {
            freeAST(node->data.ifStatement->elseBranch);
        }
        free(node->data.ifStatement);
        break;

    case NODE_WHILE_STATEMENT:
        freeAST(node->data.whileStatement->condition);
        freeAST(node->data.whileStatement->body);
        free(node->data.whileStatement);
        break;

    case NODE_FOR_STATEMENT:
        freeAST(node->data.forStatement->initializer);
        freeAST(node->data.forStatement->condition);
        freeAST(node->data.forStatement->increment);
        freeAST(node->data.forStatement->body);
        free(node->data.forStatement);
        break;

    case NODE_RETURN_STATEMENT:
        if (node->data.returnStatement->returnValue)
        {
            freeAST(node->data.returnStatement->returnValue);
        }
        free(node->data.returnStatement);
        break;

    case NODE_BLOCK:
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            freeAST(node->data.block->statements[i]);
        }
        free(node->data.block->statements);
        free(node->data.block);
        break;

    case NODE_FUNCTION_BLOCK:
        freeAST(node->data.functionBlock->function);
        freeAST((ASTNode *)node->data.functionBlock->statements);
        free(node->data.functionBlock);
        break;

    case NODE_PARAM_LIST:
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            freeAST((ASTNode *)node->data.paramList->params[i]);
        }
        free(node->data.paramList->params);
        free(node->data.paramList);
        break;

    case NODE_ARG_LIST:
        for (int i = 0; i < node->data.argList->argCount; i++)
        {
            freeAST((ASTNode *)node->data.argList->args[i]);
        }
        free(node->data.argList->args);
        free(node->data.argList);
        break;

    case NODE_ARRAY_LITERAL:
        for (int i = 0; i < node->data.array->elementCount; i++)
        {
            freeAST(node->data.array->elements[i]);
        }
        free(node->data.array->elements);
        free(node->data.array);
        break;

    case NODE_UNKNOWN:
        break;

    default:
        fprintf(stderr, "<!> [AST] Unknown Node Type: %d\n", node->metaData->type);
        break;
    }

    free(node->metaData);
    free(node);
    // printf("[AST] Node of type %s successfully freed.\n", CryoNodeTypeToString(node->metaData->type));
}
// </freeAST>

/* ====================================================================== */
/* @Node_Management */

// <createASTNode>
ASTNode *createASTNode(CryoNodeType type)
{
    ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ASTNode.\n");
        return NULL;
    }

    node->metaData = createMetaDataContainer();
    if (!node->metaData)
    {
        fprintf(stderr, "[AST] Error: Failed to create metadata container.\n");
        free(node);
        return NULL;
    }

    node->metaData->type = type;

    switch (type)
    {
    case NODE_NAMESPACE:
        break;
    case NODE_PROGRAM:
        node->data.program = createCryoProgramContainer();
        break;
    case NODE_BLOCK:
        node->data.block = createCryoBlockNodeContainer();
        break;
    case NODE_FUNCTION_BLOCK:
        node->data.functionBlock = createCryoFunctionBlockContainer();
        break;
    case NODE_RETURN_STATEMENT:
        node->data.returnStatement = createReturnNodeContainer();
        break;
    case NODE_LITERAL_EXPR:
        node->data.literal = createLiteralNodeContainer();
        break;
    case NODE_VAR_DECLARATION:
        node->data.varDecl = createVariableNodeContainer();
        break;
    case NODE_VAR_NAME:
        node->data.varName = createVariableNameNodeContainer("");
        break;
    case NODE_EXPRESSION:
        node->data.expression = createExpressionNodeContainer();
        break;
    case NODE_FUNCTION_DECLARATION:
        node->data.functionDecl = createFunctionNodeContainer();
        break;
    case NODE_FUNCTION_CALL:
        node->data.functionCall = createFunctionCallNodeContainer();
        break;
    case NODE_IF_STATEMENT:
        node->data.ifStatement = createIfStatementContainer();
        break;
    case NODE_FOR_STATEMENT:
        node->data.forStatement = createForStatementNodeContainer();
        break;
    case NODE_WHILE_STATEMENT:
        node->data.whileStatement = createWhileStatementNodeContainer();
        break;
    case NODE_BINARY_EXPR:
        node->data.bin_op = createBinaryOpNodeContainer();
        break;
    case NODE_UNARY_EXPR:
        node->data.unary_op = createUnaryOpNodeContainer();
        break;
    case NODE_PARAM_LIST:
        node->data.paramList = createParamNodeContainer();
        break;
    case NODE_ARG_LIST:
        node->data.argList = createArgNodeContainer();
        break;
    case NODE_ARRAY_LITERAL:
        node->data.array = createArrayNodeContainer();
        break;
    default:
        fprintf(stderr, "<!> [AST] Error: Unknown node type during creation: %d\n", type);
        free(node->metaData);
        free(node);
        return NULL;
    }

    printf("[AST] Created node of type: %s\n", CryoNodeTypeToString(type));
    return node;
}
// </createASTNode>

// <addChildNode>
void addChildNode(ASTNode *parent, ASTNode *child)
{
    if (!parent || !child)
    {
        fprintf(stderr, "[AST] Parent or child node is NULL\n");
        return;
    }

    if (!parent->metaData->firstChild)
    {
        parent->metaData->firstChild = child;
    }
    else
    {
        ASTNode *current = parent->metaData->firstChild;
        while (current->metaData->nextSibling)
        {
            current = current->metaData->nextSibling;
        }
        current->metaData->nextSibling = child;
    }
}
// </addChildNode>

// <addStatementToBlock>
void addStatementToBlock(ASTNode *blockNode, ASTNode *statement)
{
    if (blockNode->metaData->type != NODE_BLOCK && blockNode->metaData->type != NODE_FUNCTION_BLOCK)
    {
        printf("[AST] Error: addStatementToBlock called on non-block node\n");
        return;
    }

    CryoBlockNode *block = blockNode->data.block;

    // Debugging initial state
    if (block->statementCount >= block->statementCapacity)
    {
        block->statementCapacity *= 2;
        block->statements = (ASTNode **)realloc(block->statements, sizeof(ASTNode *) * block->statementCapacity);
        if (!block->statements)
        {
            fprintf(stderr, "[AST] Failed to reallocate memory for block statements\n");
            return;
        }
        printf("[AST] Reallocated block statement memory: New capacity = %d\n", block->statementCapacity);
    }
    else
    {
        printf("[AST] Block statement memory is sufficient\n");
    }

    // THIS IS THROWING :)
    block->statements[block->statementCount++] = statement;
    // Debugging final state
    printf("[AST] Final state: stmtCount = %d, stmtCapacity = %d\n", block->statementCount, block->statementCapacity);
}
// </addStatementToBlock>

void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement)
{
    if (!functionBlock || !statement || !functionBlock->metaData || functionBlock->metaData->type != NODE_FUNCTION_BLOCK)
    {
        fprintf(stderr, "[AST] Error: Invalid function block or statement\n");
        return;
    }

    CryoFunctionBlock *block = functionBlock->data.functionBlock;
    if (!block)
    {
        fprintf(stderr, "[AST] Error: Function block is NULL\n");
        return;
    }

    // Initialize statements array if it doesn't exist
    if (!block->statements)
    {
        block->statementCapacity = 8; // Start with a reasonable capacity
        block->statementCount = 0;
        block->statements = (ASTNode **)malloc(sizeof(ASTNode *) * block->statementCapacity);
        if (!block->statements)
        {
            fprintf(stderr, "[AST] Failed to allocate memory for function block statements\n");
            return;
        }
    }
    // Resize if necessary
    else if (block->statementCount >= block->statementCapacity)
    {
        size_t newCapacity = block->statementCapacity * 2;
        ASTNode **newStatements = (ASTNode **)realloc(block->statements, sizeof(ASTNode *) * newCapacity);
        if (!newStatements)
        {
            fprintf(stderr, "[AST] Failed to reallocate memory for function block statements\n");
            return;
        }
        block->statements = newStatements;
        block->statementCapacity = newCapacity;
    }

    // Add the new statement
    block->statements[block->statementCount++] = statement;

    // Debug output
    printf("[AST] Debug: block=%p, statementCount=%d, statementCapacity=%d\n",
           (void *)block, block->statementCount, block->statementCapacity);

    addChildNode(functionBlock, statement);
}

// <addFunctionToProgram>
void addFunctionToProgram(ASTNode *program, ASTNode *function)
{
    if (!program || !function)
    {
        fprintf(stderr, "[AST] Program or function node is NULL\n");
        return;
    }

    if (program->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[AST] Error: First argument is not a program node\n");
        return;
    }

    CryoProgram *prog = program->data.program;

    if (prog->statementCount >= prog->statementCapacity)
    {
        prog->statementCapacity *= 2;
        prog->statements = (ASTNode **)realloc(prog->statements, sizeof(ASTNode *) * prog->statementCapacity);
        if (!prog->statements)
        {
            fprintf(stderr, "[AST] Failed to reallocate memory for program statements\n");
            return;
        }
    }

    prog->statements[prog->statementCount++] = function;
    addChildNode(program, function);
}
// </addFunctionToProgram>

// -------------------------------------------------------------------

ASTNode *createNamespaceNode(char *name)
{
    ASTNode *node = createASTNode(NODE_NAMESPACE);
    if (!node)
        return NULL;
    node->metaData->moduleName = strdup(name);

    return node;
}

// Create a program node
ASTNode *createProgramNode(void)
{
    ASTNode *node = createASTNode(NODE_PROGRAM);
    if (!node)
        return NULL;

    return node;
}

// Create a literal expression node
ASTNode *createLiteralExpr(int value)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR);
    if (!node)
        return NULL;

    node->data.literal = (LiteralNode *)malloc(sizeof(LiteralNode));
    if (!node->data.literal)
    {
        free(node);
        return NULL;
    }

    node->data.literal->dataType = DATA_TYPE_INT;
    node->data.literal->intValue = value;

    return node;
}

// Create an expression statement node
ASTNode *createExpressionStatement(ASTNode *expression)
{
    ASTNode *node = createASTNode(NODE_EXPRESSION);
    if (!node)
        return NULL;

    node->data.expression = (CryoExpressionNode *)malloc(sizeof(CryoExpressionNode));
    if (!node->data.expression)
    {
        free(node);
        return NULL;
    }

    node->data.expression->nodeType = expression->metaData->type;

    if (expression->metaData->type == NODE_VAR_NAME)
    {
        node->data.expression->data.varNameNode = expression->data.varName;
    }
    else if (expression->metaData->type == NODE_LITERAL_EXPR)
    {
        node->data.expression->data.literalNode = expression->data.literal;
    }
    else
    {
        // Handle other types of expressions if needed
        free(node->data.expression);
        free(node);
        return NULL;
    }

    return node;
}

// Create a binary expression node
ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op)
{
    ASTNode *node = createASTNode(NODE_BINARY_EXPR);
    if (!node)
        return NULL;

    node->data.bin_op = (CryoBinaryOpNode *)malloc(sizeof(CryoBinaryOpNode));
    if (!node->data.bin_op)
    {
        free(node);
        return NULL;
    }

    node->data.bin_op->left = left;
    node->data.bin_op->right = right;
    node->data.bin_op->op = op;

    return node;
}

// Create a unary expression node
ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand)
{
    ASTNode *node = createASTNode(NODE_UNARY_EXPR);
    if (!node)
        return NULL;

    node->data.unary_op = (CryoUnaryOpNode *)malloc(sizeof(CryoUnaryOpNode));
    if (!node->data.unary_op)
    {
        free(node);
        return NULL;
    }

    node->data.unary_op->op = op;
    node->data.unary_op->operand = operand;

    return node;
}

/* @Node_Creation - Literals */
ASTNode *createIntLiteralNode(int value)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_INT;
    node->data.literal->intValue = value;
    return node;
}

ASTNode *createFloatLiteralNode(float value)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_FLOAT;
    node->data.literal->floatValue = value;
    return node;
}

ASTNode *createStringLiteralNode(char *value)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_STRING;
    node->data.literal->stringValue = strdup(value);
    return node;
}

ASTNode *createBooleanLiteralNode(int value)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_BOOLEAN;
    node->data.literal->booleanValue = value;
    return node;
}

ASTNode *createIdentifierNode(char *name)
{
    ASTNode *node = createASTNode(NODE_VAR_NAME);
    if (!node)
        return NULL;
    node->data.varName->varName = strdup(name);
    return node;
}

/* @Node_Blocks - Blocks */
ASTNode *createBlockNode(void)
{
    ASTNode *node = createASTNode(NODE_BLOCK);
    if (!node)
        return NULL;
    node->data.block->statements = NULL;
    node->data.block->statementCount = 0;
    node->data.block->statementCapacity = 0;
    return node;
}

ASTNode *createFunctionBlock()
{
    ASTNode *node = createASTNode(NODE_FUNCTION_BLOCK);
    if (!node)
        return NULL;

    return node;
}

ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch)
{
    ASTNode *node = createASTNode(NODE_IF_STATEMENT);
    if (!node)
        return NULL;
    node->data.ifStatement->condition = condition;
    node->data.ifStatement->thenBranch = then_branch;
    node->data.ifStatement->elseBranch = else_branch;
    return node;
}

ASTNode *createForBlock(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body)
{
    ASTNode *node = createASTNode(NODE_FOR_STATEMENT);
    if (!node)
        return NULL;
    node->data.forStatement->initializer = initializer;
    node->data.forStatement->condition = condition;
    node->data.forStatement->increment = increment;
    node->data.forStatement->body = body;
    return node;
}

ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body)
{
    ASTNode *node = createASTNode(NODE_WHILE_STATEMENT);
    if (!node)
        return NULL;
    node->data.whileStatement->condition = condition;
    node->data.whileStatement->body = body;
    return node;
}

/* @Node_Blocks - Literals */
ASTNode *createBooleanLiteralExpr(int value)
{
    return createBooleanLiteralNode(value);
}

ASTNode *createStringLiteralExpr(char *str)
{
    return createStringLiteralNode(str);
}

ASTNode *createStringExpr(char *str)
{
    ASTNode *node = createASTNode(NODE_STRING_EXPRESSION);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_STRING;
    node->data.literal->stringValue = strdup(str);
    return node;
}

/* @Node_Creation - Variables */
ASTNode *createVarDeclarationNode(char *var_name, CryoDataType dataType, ASTNode *initializer, int line, bool isGlobal, bool isReference)
{
    ASTNode *node = createASTNode(NODE_VAR_DECLARATION);
    if (!node)
    {
        printf("DEBUG: Failed to create AST node\n");
        return NULL;
    }

    if (!node->data.varDecl)
    {
        printf("DEBUG: varDecl is NULL\n");
        free(node);
        return NULL;
    }

    node->data.varDecl->type = dataType;
    node->data.varDecl->name = strdup(var_name);
    node->data.varDecl->varNameNode = createVariableNameNodeContainer(var_name);
    node->metaData->line = line;
    node->data.varDecl->isGlobal = isGlobal;
    node->data.varDecl->isLocal = !isGlobal;
    node->data.varDecl->isReference = isReference;
    node->data.varDecl->initializer = initializer;

    printf("DEBUG [AST] Final node type: %s\n", CryoDataTypeToString(node->data.varDecl->type));
    return node;
}

ASTNode *createVariableExpr(char *name, bool isReference)
{
    ASTNode *node = createASTNode(NODE_VAR_NAME);
    if (!node)
        return NULL;
    node->data.varName->varName = strdup(name);
    node->data.varName->isRef = isReference;
    return node;
}

/* @Node_Creation - Functions */
ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, CryoDataType returnType)
{
    ASTNode *node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node)
        return NULL;
    node->data.functionDecl->visibility = visibility;
    node->data.functionDecl->name = strdup(function_name);
    node->data.functionDecl->params = params;
    node->data.functionDecl->body = function_body;
    node->data.functionDecl->returnType = returnType;
    return node;
}

ASTNode *createExternFuncNode(void)
{
    return createASTNode(NODE_EXTERN_FUNCTION);
}

ASTNode *createFunctionCallNode(void)
{
    return createASTNode(NODE_FUNCTION_CALL);
}

ASTNode *createReturnNode(ASTNode *returnValue)
{
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node)
        return NULL;
    node->data.returnStatement->returnValue = returnValue;
    return node;
}

ASTNode *createReturnExpression(ASTNode *returnExpression, CryoDataType returnType)
{
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT);
    if (!node)
        return NULL;
    node->data.returnStatement->expression = returnExpression;
    node->data.returnStatement->returnType = returnType;
    return node;
}

/* @Node_Creation - Parameters */
ASTNode *createParamListNode(void)
{
    return createASTNode(NODE_PARAM_LIST);
}

ASTNode *createArgumentListNode(void)
{
    return createASTNode(NODE_ARG_LIST);
}

ASTNode *createParamNode(char *name, CryoDataType type)
{
    ASTNode *node = createASTNode(NODE_VAR_DECLARATION);
    if (!node)
        return NULL;
    node->data.varDecl->name = strdup(name);
    node->data.varDecl->type = type;
    node->data.varDecl->isGlobal = false;
    node->data.varDecl->isReference = false;
    return node;
}

ASTNode *createArgsNode(char *name, CryoDataType type)
{
    ASTNode *node = createASTNode(NODE_VAR_DECLARATION);
    if (!node)
        return NULL;
    node->data.varDecl->name = strdup(name);
    node->data.varDecl->type = type;
    node->data.varDecl->isGlobal = false;
    node->data.varDecl->isReference = true;
    return node;
}

/* @Node_Creation - Modules & Externals */
ASTNode *createImportNode(char *importPath)
{
    ASTNode *node = createASTNode(NODE_IMPORT_STATEMENT);
    if (!node)
        return NULL;
    // node->data.importStatement->modulePath = strdup(importPath);
    return node;
}

ASTNode *createExternNode(ASTNode *externNode)
{
    ASTNode *node = createASTNode(NODE_EXTERN_STATEMENT);
    if (!node)
        return NULL;
    node->data.externNode->externNode = externNode;
    return node;
}

/* @Node_Creation - Conditionals */
ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch)
{
    return createIfBlock(condition, then_branch, else_branch);
}

ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body)
{
    return createForBlock(initializer, condition, increment, body);
}

ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body)
{
    return createWhileBlock(condition, body);
}

/* @Node_Creation - Arrays */
ASTNode *createArrayLiteralNode(void)
{
    return createASTNode(NODE_ARRAY_LITERAL);
}