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
void printAST(ASTNode *node, int indent, Arena *arena)
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
            printAST(node->data.program->statements[i], indent + 2, arena);
        }
        break;

    case NODE_EXTERN_FUNCTION:
        printf("\nExtern Function Node: %s\n", strdup(node->data.externFunction->name));
        printf("Extern Function Return Type: %s\n", CryoDataTypeToString(node->data.externFunction->returnType));
        printf("Extern Function Parameters:\n");
        for (int i = 0; i < node->data.externFunction->paramCount; i++)
        {
            printAST(node->data.externFunction->params[i], indent + 2, arena);
        }
        break;

    case NODE_FUNCTION_DECLARATION:
        printf("\nFunction Declaration Node name: %s\n", strdup(node->data.functionDecl->name));
        printf("Function Declaration Node returnType: %s\n",
               CryoDataTypeToString(node->data.functionDecl->returnType));
        printf("Function Declaration Node visibility: %s\n",
               CryoVisibilityTypeToString(node->data.functionDecl->visibility));
        printf("Function Declaration Node params:\n");
        for (int i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            printAST(node->data.functionDecl->params[i], indent + 2, arena);
        }
        printf("Function Declaration Node body:\n");
        printAST(node->data.functionDecl->body, indent + 2, arena);
        break;

    case NODE_VAR_DECLARATION:
        printf("\nVariable Declaration Node: %s\n", strdup(node->data.varDecl->name));
        printf("Variable Type: %s\n", CryoDataTypeToString(node->data.varDecl->type));
        printf("Is Global: %s\n", node->data.varDecl->isGlobal ? "true" : "false");
        printf("Is Reference: %s\n", node->data.varDecl->isReference ? "true" : "false");
        printf("Is Mutable: %s\n", node->data.varDecl->isMutable ? "true" : "false");
        printf("Is Iterator: %s\n", node->data.varDecl->isIterator ? "true" : "false");
        printf("Has Index Expression: %s\n", node->data.varDecl->hasIndexExpr ? "true" : "false");

        if (node->data.varDecl->initializer)
        {
            printf("Initializer:");
            printAST(node->data.varDecl->initializer, indent + 2, arena);
        }
        else
        {
            printf("No Initializer\n");
        }
        break;

    case NODE_EXPRESSION:
        printf("\nExpression Node\n");
        break;

    case NODE_BINARY_EXPR:
        printf("\nBinary Expression Node: %s\n", CryoOperatorTypeToString(node->data.bin_op->op));
        printf("\nLeft Operand:");
        printAST(node->data.bin_op->left, indent + 2, arena);
        printf("\nOperator: %s\n", CryoOperatorTypeToString(node->data.bin_op->op));
        printf("\nRight Operand:");
        printAST(node->data.bin_op->right, indent + 2, arena);
        printf("\n");
        break;

    case NODE_UNARY_EXPR:
        printf("\nUnary Expression Node: %s\n", CryoTokenToString(node->data.unary_op->op));
        break;

    case NODE_LITERAL_EXPR:
        printf("\nLiteral Expression Node: ");
        switch (node->data.literal->dataType)
        {
        case DATA_TYPE_INT:
            printf("Integer Literal Node: %d\n", node->data.literal->value.intValue);
            break;
        case DATA_TYPE_FLOAT:
            printf("Float Literal Node: %f\n", node->data.literal->value.floatValue);
            break;
        case DATA_TYPE_STRING:
            printf("String Literal Node: %s\n", node->data.literal->value.stringValue);
            break;
        case DATA_TYPE_BOOLEAN:
            printf("Boolean Literal Node: %s\n", node->data.literal->value.booleanValue ? "true" : "false");
            break;
        default:
            printf("Unknown Literal Type\n");
        }
        break;
    case NODE_STRING_EXPRESSION:
        printf("\nString Expression Node: %s\n", node->data.literal->value.stringValue);
        break;
    case NODE_VAR_NAME:
        printf("\nVariable Name Node: %s\n", node->data.varName->varName);
        printf("Is Reference: %s\n", node->data.varName->isRef ? "true" : "false");
        break;

    case NODE_FUNCTION_CALL:
        printf("\nFunction Call Node: %s\n", strdup(node->data.functionCall->name));
        printf("Arguments:\n");
        for (int i = 0; i < node->data.functionCall->argCount; i++)
        {
            printAST(node->data.functionCall->args[i], indent + 2, arena);
        }
        break;

    case NODE_IF_STATEMENT:
        printf("\nIf Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.ifStatement->condition, indent + 2, arena);
        printf("Then Branch:\n");
        printAST(node->data.ifStatement->thenBranch, indent + 2, arena);
        if (node->data.ifStatement->elseBranch)
        {
            printf("Else Branch:\n");
            printAST(node->data.ifStatement->elseBranch, indent + 2, arena);
        }
        break;

    case NODE_WHILE_STATEMENT:
        printf("\nWhile Statement Node\n");
        printf("Condition:\n");
        printAST(node->data.whileStatement->condition, indent + 2, arena);
        printf("Body:\n");
        printAST(node->data.whileStatement->body, indent + 2, arena);
        break;

    case NODE_FOR_STATEMENT:
        printf("\nFor Statement Node\n");
        printf("Initializer:\n");
        printAST(node->data.forStatement->initializer, indent + 2, arena);
        printf("Condition:\n");
        printAST(node->data.forStatement->condition, indent + 2, arena);
        printf("Increment:\n");
        printAST(node->data.forStatement->increment, indent + 2, arena);
        printf("Body:\n");
        printAST(node->data.forStatement->body, indent + 2, arena);
        break;

    case NODE_RETURN_STATEMENT:
        printf("\nReturn Statement Node\n");
        if (node->data.returnStatement->returnValue)
        {
            printAST(node->data.returnStatement->returnValue, indent + 2, arena);
        }
        break;

    case NODE_BLOCK:
        printf("\nBlock Node with %d statements (capacity: %d)\n",
               node->data.block->statementCount,
               node->data.block->statementCapacity);
        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            printAST(node->data.block->statements[i], indent + 2, arena);
        }
        break;

    case NODE_FUNCTION_BLOCK:
        printf("\nFunction Block Node\n");
        printf("Function:\n");
        printAST(node->data.functionBlock->function, indent + 2, arena);
        printf("Block:\n");
        for (int i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            printAST(node->data.functionBlock->statements[i], indent + 2, arena);
        }
        break;

    case NODE_PARAM_LIST:
        printf("\nParameter List Node with %d parameters (capacity: %d)\n",
               node->data.paramList->paramCount,
               node->data.paramList->paramCapacity);
        for (int i = 0; i < node->data.paramList->paramCount; i++)
        {
            printAST((ASTNode *)node->data.paramList->params[i], indent + 2, arena);
        }
        break;

    case NODE_ARG_LIST:
        printf("\nArgument List Node with %d arguments (capacity: %d)\n",
               node->data.argList->argCount,
               node->data.argList->argCapacity);
        for (int i = 0; i < node->data.argList->argCount; i++)
        {
            printAST((ASTNode *)node->data.argList->args[i], indent + 2, arena);
        }
        break;

    case NODE_ARRAY_LITERAL:
        printf("\nArray Literal Node with %d elements (capacity: %d)\n",
               node->data.array->elementCount,
               node->data.array->elementCapacity);
        for (int i = 0; i < node->data.array->elementCount; i++)
        {
            printAST(node->data.array->elements[i], indent + 2, arena);
        }
        break;

    case NODE_NAMESPACE:
        printf("\nNamespace Node: %s\n", node->data.cryoNamespace->name);
        break;

    case NODE_INDEX_EXPR:
    {
        printf("\nIndex Expression Node\n");
        printf("Array:\n");
        printAST(node->data.indexExpr->array, indent + 2, arena);
        printf("Index:\n");
        printAST(node->data.indexExpr->index, indent + 2, arena);
        break;
    }

    case NODE_VAR_REASSIGN:
    {
        printf("\nVariable Reassignment Node\n");
        printf("Referenced Variable Name: %s\n", node->data.varReassignment->existingVarName);
        printf("Existing Value:\n");
        printAST(node->data.varReassignment->existingVarNode, indent + 2, arena);
        printf("New Value:\n");
        printAST(node->data.varReassignment->newVarNode, indent + 2, arena);
        break;
    }

    case NODE_PARAM:
    {
        printf("\nParameter Node: %s\n", node->data.param->name);
        printf("Parameter Type: %s\n", CryoDataTypeToString(node->data.param->type));
        printf("Function Name: %s\n", node->data.param->functionName);
        printf("Has Default Value: %s\n", node->data.param->hasDefaultValue ? "true" : "false");
        if (node->data.param->hasDefaultValue)
        {
            printf("Default Value:\n");
            printAST(node->data.param->defaultValue, indent + 2, arena);
        }
        break;
    }

    case NODE_PROPERTY:
    {
        printf("\nProperty Node: %s\n", node->data.property->name);
        printf("Property Type: %s\n", CryoDataTypeToString(node->data.property->type));
        printf("Property Value:\n");
        printAST(node->data.property->value, indent + 2, arena);
        break;
    }

    case NODE_STRUCT_DECLARATION:
    {
        printf("\nStruct Declaration Node: %s\n", node->data.structNode->name);
        printf("Struct Properties:\n");
        for (int i = 0; i < node->data.structNode->propertyCount; i++)
        {
            printAST(node->data.structNode->properties[i], indent + 2, arena);
        }
        break;
    }

    case NODE_IMPORT_STATEMENT:
    {
        printf("\nImport Statement Node: \n");
        printf("Is STD Module: %s\n", node->data.import->isStdModule ? "true" : "false");
        printf("Module Name: %s\n", node->data.import->moduleName);
        if (node->data.import->subModuleName)
        {
            printf("Submodule Name: %s\n", node->data.import->subModuleName);
        }
        break;
    }

    case NODE_EXTERNAL_SYMBOL:
        printf("\nExternal Symbol Node\n");
        break;

    case NODE_SCOPED_FUNCTION_CALL:
        printf("\nScoped Function Call Node\n");
        printf("Function Name: %s\n", node->data.scopedFunctionCall->functionName);
        printf("Scope Name: %s\n", node->data.scopedFunctionCall->scopeName);
        printf("Arg Count: %d\n", node->data.scopedFunctionCall->argCount);
        printf("Arguments:\n");
        for (int i = 0; i < node->data.scopedFunctionCall->argCount; i++)
        {
            printAST(node->data.scopedFunctionCall->args[i], indent + 2, arena);
        }
        break;

    case NODE_UNKNOWN:
        printf("\n<Unknown Node>\n");
        break;

    default:
        printf("\n <!> Unhandled Node Type: %s\n", CryoNodeTypeToString(node->metaData->type));
        break;
    }
}
// </printAST>

/* ====================================================================== */
/* @Node_Management */

// <createASTNode>
ASTNode *createASTNode(CryoNodeType type, Arena *arena, CompilerState *state)
{
    ASTNode *node = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST node");
        return NULL;
    }

    node->metaData = createMetaDataContainer(arena, state);
    if (!node->metaData)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for AST node metadata");
        return NULL;
    }

    char *moduleName = getCurrentNamespace(state->table);
    if (moduleName)
    {
        node->metaData->moduleName = strdup(moduleName);
    }
    node->metaData->type = type;
    node->metaData->position = getPosition(state->lexer);

    switch (type)
    {
    case NODE_NAMESPACE:
        node->data.cryoNamespace = createCryoNamespaceNodeContainer(arena, state);
        break;
    case NODE_PROGRAM:
        node->data.program = createCryoProgramContainer(arena, state);
        break;
    case NODE_IMPORT_STATEMENT:
        node->data.import = createCryoImportNodeContainer(arena, state);
        break;
    case NODE_BLOCK:
        node->data.block = createCryoBlockNodeContainer(arena, state);
        break;
    case NODE_FUNCTION_BLOCK:
        node->data.functionBlock = createCryoFunctionBlockContainer(arena, state);
        break;
    case NODE_RETURN_STATEMENT:
        node->data.returnStatement = createReturnNodeContainer(arena, state);
        break;
    case NODE_LITERAL_EXPR:
        node->data.literal = createLiteralNodeContainer(arena, state);
        break;
    case NODE_VAR_DECLARATION:
        node->data.varDecl = createVariableNodeContainer(arena, state);
        break;
    case NODE_VAR_NAME:
        node->data.varName = createVariableNameNodeContainer("", arena, state);
        break;
    case NODE_EXPRESSION:
        node->data.expression = createExpressionNodeContainer(arena, state);
        break;
    case NODE_FUNCTION_DECLARATION:
        node->data.functionDecl = createFunctionNodeContainer(arena, state);
        break;
    case NODE_EXTERN_FUNCTION:
        node->data.externNode = createExternNodeContainer(NODE_EXTERN_FUNCTION, arena, state);
        break;
    case NODE_FUNCTION_CALL:
        node->data.functionCall = createFunctionCallNodeContainer(arena, state);
        break;
    case NODE_IF_STATEMENT:
        node->data.ifStatement = createIfStatementContainer(arena, state);
        break;
    case NODE_FOR_STATEMENT:
        node->data.forStatement = createForStatementNodeContainer(arena, state);
        break;
    case NODE_WHILE_STATEMENT:
        node->data.whileStatement = createWhileStatementNodeContainer(arena, state);
        break;
    case NODE_BINARY_EXPR:
        node->data.bin_op = createBinaryOpNodeContainer(arena, state);
        break;
    case NODE_UNARY_EXPR:
        node->data.unary_op = createUnaryOpNodeContainer(arena, state);
        break;
    case NODE_PARAM_LIST:
        node->data.paramList = createParamNodeContainer(arena, state);
        break;
    case NODE_ARG_LIST:
        node->data.argList = createArgNodeContainer(arena, state);
        break;
    case NODE_ARRAY_LITERAL:
        node->data.array = createArrayNodeContainer(arena, state);
        break;
    case NODE_INDEX_EXPR:
        node->data.indexExpr = createIndexExprNodeContainer(arena, state);
        break;
    case NODE_VAR_REASSIGN:
        node->data.varReassignment = createVariableReassignmentNodeContainer(arena, state);
        break;
    case NODE_PARAM:
        node->data.param = createParameterNodeContainer(arena, state);
        break;
    case NODE_PROPERTY:
        node->data.property = createPropertyNodeContainer(arena, state);
        break;
    case NODE_STRUCT_DECLARATION:
        node->data.structNode = createStructNodeContainer(arena, state);
        break;
    case NODE_SCOPED_FUNCTION_CALL:
        node->data.scopedFunctionCall = createScopedFunctionCallNode(arena, state);
        break;
    default:
        logMessage("ERROR", __LINE__, "AST", "Unknown Node Type: %s", CryoNodeTypeToString(type));
        return NULL;
    }

    logMessage("INFO", __LINE__, "AST", "Created AST node of type: %s", CryoNodeTypeToString(type));
    return node;
}
// </createASTNode>

// <addChildNode>
void addChildNode(ASTNode *parent, ASTNode *child, Arena *arena, CompilerState *state)
{
    if (!parent || !child)
    {
        logMessage("ERROR", __LINE__, "AST", "Parent or child node is NULL");
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
void addStatementToBlock(ASTNode *blockNode, ASTNode *statement, Arena *arena, CompilerState *state)
{
    if (blockNode->metaData->type != NODE_BLOCK && blockNode->metaData->type != NODE_FUNCTION_BLOCK)
    {
        logMessage("ERROR", __LINE__, "AST", "Invalid block node");
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
            logMessage("ERROR", __LINE__, "AST", "Failed to reallocate memory for block statements");
            return;
        }
        logMessage("INFO", __LINE__, "AST", "Block statement memory reallocated");
    }
    else
    {
        logMessage("INFO", __LINE__, "AST", "Block statement memory is sufficient");
    }

    // THIS IS THROWING :)
    block->statements[block->statementCount++] = statement;
    // Debugging final state
    logMessage("INFO", __LINE__, "AST", "Final state: stmtCount = %d, stmtCapacity = %d", block->statementCount);
}
// </addStatementToBlock>

void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement, Arena *arena, CompilerState *state)
{
    if (!functionBlock || !statement || !functionBlock->metaData || functionBlock->metaData->type != NODE_FUNCTION_BLOCK)
    {
        logMessage("ERROR", __LINE__, "AST", "Invalid function block node");
        return;
    }

    CryoFunctionBlock *block = functionBlock->data.functionBlock;
    if (!block)
    {
        logMessage("ERROR", __LINE__, "AST", "Function block data is NULL");
        return;
    }

    // Initialize statements array if it doesn't exist
    if (!block->statements)
    {
        block->statementCapacity = 64; // Start with a reasonable capacity
        block->statementCount = 0;
        block->statements = (ASTNode **)ARENA_ALLOC(arena, sizeof(ASTNode *) * block->statementCapacity);
        if (!block->statements)
        {
            logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for function block statements");
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
            logMessage("ERROR", __LINE__, "AST", "Failed to reallocate memory for function block statements");
            return;
        }
        block->statements = newStatements;
        block->statementCapacity = newCapacity;
    }

    // Add the new statement
    block->statements[block->statementCount++] = statement;

    // Debug output
    logMessage("INFO", __LINE__, "AST", "Debug: block=%p, statementCount=%d, statementCapacity=%d",
               (void *)block, block->statementCount, block->statementCapacity);

    addChildNode(functionBlock, statement, arena, state);
}

// <addFunctionToProgram>
void addFunctionToProgram(ASTNode *program, ASTNode *function, Arena *arena, CompilerState *state)
{
    if (!program || !function)
    {
        logMessage("ERROR", __LINE__, "AST", "Program or function node is NULL");
        return;
    }

    if (program->metaData->type != NODE_PROGRAM)
    {
        logMessage("ERROR", __LINE__, "AST", "Invalid program node");
        return;
    }

    CryoProgram *prog = program->data.program;

    if (prog->statementCount >= prog->statementCapacity)
    {
        prog->statementCapacity *= 2;
        prog->statements = (ASTNode **)realloc(prog->statements, sizeof(ASTNode *) * prog->statementCapacity);
        if (!prog->statements)
        {
            logMessage("ERROR", __LINE__, "AST", "Failed to reallocate memory for program statements");
            return;
        }
    }

    prog->statements[prog->statementCount++] = function;
    addChildNode(program, function, arena, state);
}
// </addFunctionToProgram>

// -------------------------------------------------------------------

ASTNode *createNamespaceNode(char *name, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_NAMESPACE, arena, state);
    if (!node)
        return NULL;
    assert(name != NULL);
    node->data.cryoNamespace->name = strdup(name);

    logMessage("INFO", __LINE__, "AST", "Created namespace node with name: %s", strdup(name));

    return node;
}

// Create a program node
ASTNode *createProgramNode(Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_PROGRAM, arena, state);
    if (!node)
        return NULL;

    return node;
}

// Create a literal expression node
ASTNode *createLiteralExpr(int value, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state);
    if (!node)
        return NULL;

    node->data.literal = (LiteralNode *)ARENA_ALLOC(arena, sizeof(LiteralNode));
    if (!node->data.literal)
    {
        return NULL;
    }

    logMessage("DEBUG", __LINE__, "AST", "Created literal expression node with value: %d", value);
    int intCpy = value;
    node->data.literal->dataType = DATA_TYPE_INT;
    node->data.literal->value.intValue = value;
    // convert from int to string
    char *buffer = (char *)ARENA_ALLOC(arena, sizeof(char));
    sprintf(buffer, "%d", intCpy);
    node->data.literal->value.stringValue = buffer;

    logMessage("DEBUG", __LINE__, "AST", "Literal expression node created with value: %d", value);
    return node;
}

// Create an expression statement node
ASTNode *createExpressionStatement(ASTNode *expression, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_EXPRESSION, arena, state);
    if (!node)
        return NULL;

    node->data.expression = (CryoExpressionNode *)ARENA_ALLOC(arena, sizeof(CryoExpressionNode));
    if (!node->data.expression)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to allocate memory for expression statement node");
        return NULL;
    }

    logMessage("DEBUG", __LINE__, "AST", "Creating expression statement node");

    node->data.expression->nodeType = expression->metaData->type;

    if (expression->metaData->type == NODE_VAR_NAME)
    {
        logMessage("DEBUG", __LINE__, "AST", "Expression is a variable name");
        node->data.expression->data.varNameNode = expression->data.varName;
    }
    else if (expression->metaData->type == NODE_LITERAL_EXPR)
    {
        logMessage("DEBUG", __LINE__, "AST", "Expression is a literal");
        node->data.expression->data.literalNode = expression->data.literal;
    }
    else
    {
        // Handle other types of expressions if needed
        logMessage("ERROR", __LINE__, "AST", "Unsupported expression type");
        return NULL;
    }

    logMessage("DEBUG", __LINE__, "AST", "Expression statement node created");
    return node;
}

// Create a binary expression node
ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_BINARY_EXPR, arena, state);
    if (!node)
        return NULL;

    node->data.bin_op = (CryoBinaryOpNode *)ARENA_ALLOC(arena, sizeof(CryoBinaryOpNode));
    if (!node->data.bin_op)
    {
        return NULL;
    }

    logMessage("DEBUG", __LINE__, "AST", "Created binary expression node with operator: %s", CryoOperatorTypeToString(op));

    node->data.bin_op->left = left;
    node->data.bin_op->right = right;
    node->data.bin_op->op = op;

    return node;
}

// Create a unary expression node
ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_UNARY_EXPR, arena, state);
    if (!node)
        return NULL;

    node->data.unary_op->op = op;
    node->data.unary_op->operand = operand;

    return node;
}

/* @Node_Creation - Literals */
ASTNode *createIntLiteralNode(int value, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state);
    if (!node)
    {
        return NULL;
    }
    logMessage("INFO", __LINE__, "AST", "Created integer literal node with value: %d", value);

    node->data.literal->value.intValue = value;
    node->data.literal->dataType = DATA_TYPE_INT;

    return node;
}

ASTNode *createFloatLiteralNode(float value, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state);
    if (!node)
        return NULL;

    logMessage("INFO", __LINE__, "AST", "Created float literal node with value: %f", value);

    node->data.literal->dataType = DATA_TYPE_FLOAT;
    node->data.literal->value.floatValue = value;
    return node;
}

ASTNode *createStringLiteralNode(char *value, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state);
    if (!node)
        return NULL;

    logMessage("INFO", __LINE__, "AST", "Created string literal node with value: %s", value);

    // Trim the `"` characters from the string
    char *trimmedString = strdup(value);
    trimmedString++;
    trimmedString[strlen(trimmedString) - 1] = '\0';

    node->data.literal->dataType = DATA_TYPE_STRING;
    node->data.literal->value.stringValue = strdup(trimmedString);
    node->data.literal->length = strlen(trimmedString);

    return node;
}

ASTNode *createBooleanLiteralNode(int value, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state);
    if (!node)
        return NULL;

    logMessage("INFO", __LINE__, "AST", "Created boolean literal node with value: %s", value ? "true" : "false");

    node->data.literal->dataType = DATA_TYPE_BOOLEAN;
    node->data.literal->value.booleanValue = value;
    return node;
}

ASTNode *createIdentifierNode(char *name, CryoSymbolTable *symTable, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_VAR_NAME, arena, state);
    if (!node)
        return NULL;

    logMessage("INFO", __LINE__, "AST", "Created identifier node with name: %s", name);
    char *varName = strdup(name);
    // Attempt to find the symbol in the symbol table (It's okay if it can't find and can just create the node)
    CryoSymbol *sym = findSymbol(symTable, varName, arena);
    if (sym)
    {
        logMessage("INFO", __LINE__, "AST", "Found symbol in symbol table: %s", sym->name);
        node->data.varName->varName = strdup(sym->name);
        node->data.varName->isRef = true;
        node->data.varName->refType = sym->valueType;
    }
    else
    {
        logMessage("INFO", __LINE__, "AST", "Symbol not found in symbol table: %s", varName);
        node->data.varName->varName = strdup(varName);
        node->data.varName->isRef = true;
        node->data.varName->refType = DATA_TYPE_UNKNOWN;
    }
    return node;
}

/* @Node_Blocks - Blocks */
ASTNode *createBlockNode(Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_BLOCK, arena, state);
    if (!node)
        return NULL;

    return node;
}

ASTNode *createFunctionBlock(Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_FUNCTION_BLOCK, arena, state);
    if (!node)
        return NULL;

    return node;
}

ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_IF_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.ifStatement->condition = condition;
    node->data.ifStatement->thenBranch = then_branch;
    node->data.ifStatement->elseBranch = else_branch;
    return node;
}

ASTNode *createForBlock(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_FOR_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.forStatement->initializer = initializer;
    node->data.forStatement->condition = condition;
    node->data.forStatement->increment = increment;
    node->data.forStatement->body = body;
    return node;
}

ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_WHILE_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.whileStatement->condition = condition;
    node->data.whileStatement->body = body;
    return node;
}

/* @Node_Blocks - Literals */
ASTNode *createBooleanLiteralExpr(int value, Arena *arena, CompilerState *state)
{
    return createBooleanLiteralNode(value, arena, state);
}

ASTNode *createStringLiteralExpr(char *str, Arena *arena, CompilerState *state)
{
    return createStringLiteralNode(str, arena, state);
}

ASTNode *createStringExpr(char *str, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_STRING_EXPRESSION, arena, state);
    if (!node)
        return NULL;
    node->data.literal->dataType = DATA_TYPE_STRING;
    node->data.literal->value.stringValue = strdup(str);
    return node;
}

/* @Node_Creation - Variables */
ASTNode *createVarDeclarationNode(char *var_name, CryoDataType dataType, ASTNode *initializer, bool isMutable, bool isGlobal, bool isReference, bool isIterator, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_VAR_DECLARATION, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create variable declaration node");
        return NULL;
    }

    if (!node->data.varDecl)
    {
        logMessage("ERROR", __LINE__, "AST", "Variable declaration node data is NULL");
        return NULL;
    }

    printf("\n\nData Type in Var Decl: %s\n\n", CryoDataTypeToString(dataType));

    node->data.varDecl->type = dataType;
    node->data.varDecl->name = strdup(var_name);
    node->data.varDecl->varNameNode = createVariableNameNodeContainer(var_name, arena, state);
    node->metaData->line = 0;
    node->data.varDecl->isGlobal = isGlobal;
    node->data.varDecl->isLocal = !isGlobal;
    node->data.varDecl->isReference = isReference;
    node->data.varDecl->isMutable = isMutable;
    node->data.varDecl->isIterator = isIterator;
    node->data.varDecl->initializer = initializer;

    logMessage("INFO", __LINE__, "AST", "Created variable declaration node for %s", var_name);
    return node;
}

ASTNode *createVariableExpr(char *name, bool isReference, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_VAR_NAME, arena, state);
    if (!node)
        return NULL;
    node->data.varName->varName = strdup(name);
    node->data.varName->isRef = isReference;
    return node;
}

/* @Node_Creation - Functions */
ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, CryoDataType returnType, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_FUNCTION_DECLARATION, arena, state);
    if (!node)
    {
        return NULL;
    }
    int paramCount = 0;
    for (int i = 0; params[i] != NULL; i++)
    {
        paramCount++;
    }
    int paramCapacity = paramCount + 1;
    node->data.functionDecl->visibility = visibility;
    node->data.functionDecl->name = strdup(function_name);
    node->data.functionDecl->params = params;
    node->data.functionDecl->paramCount = paramCount;
    node->data.functionDecl->paramCapacity = paramCapacity;
    node->data.functionDecl->body = function_body;
    node->data.functionDecl->returnType = returnType;
    return node;
}

ASTNode *createExternFuncNode(char *function_name, ASTNode **params, CryoDataType returnType, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_EXTERN_FUNCTION, arena, state);
    if (!node)
        return NULL;

    int paramCount = 0;
    for (int i = 0; params[i] != NULL; i++)
    {
        paramCount++;
    }

    node->data.externFunction = (ExternFunctionNode *)ARENA_ALLOC(arena, sizeof(ExternFunctionNode));
    if (!node->data.externFunction)
    {
        return NULL;
    }

    node->data.externFunction->name = strdup(function_name);
    if (!node->data.externFunction->name)
    {
        return NULL;
    }

    node->data.externFunction->params = NULL;
    node->data.externFunction->paramCount = paramCount;

    if (paramCount > 0 && params)
    {
        node->data.externFunction->params = (ASTNode **)ARENA_ALLOC(arena, paramCount * sizeof(ASTNode *));
        if (!node->data.externFunction->params)
        {
            return NULL;
        }
        memcpy(node->data.externFunction->params, params, paramCount * sizeof(ASTNode *));
    }

    node->data.externFunction->returnType = returnType;

    return node;
}

ASTNode *createFunctionCallNode(Arena *arena, CompilerState *state)
{
    return createASTNode(NODE_FUNCTION_CALL, arena, state);
}

ASTNode *createReturnNode(ASTNode *returnValue, CryoDataType returnType, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.returnStatement->returnValue = returnValue;
    node->data.returnStatement->expression = returnValue;
    node->data.returnStatement->returnType = returnType;

    if (returnType == DATA_TYPE_VOID)
    {
        logMessage("WARN", __LINE__, "AST", node, "Return statement has void return type");
    }

    return node;
}

ASTNode *createReturnExpression(ASTNode *returnExpression, CryoDataType returnType, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.returnStatement->expression = returnExpression;
    node->data.returnStatement->returnType = returnType;
    return node;
}

/* @Node_Creation - Parameters */
ASTNode *createParamListNode(Arena *arena, CompilerState *state)
{
    return createASTNode(NODE_PARAM_LIST, arena, state);
}

ASTNode *createArgumentListNode(Arena *arena, CompilerState *state)
{
    return createASTNode(NODE_ARG_LIST, arena, state);
}

ASTNode *createParamNode(char *name, char *functionName, CryoDataType type, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_PARAM, arena, state);
    if (!node)
        return NULL;
    node->data.param->name = strdup(name);
    node->data.param->type = type;
    node->data.param->hasDefaultValue = false;
    node->data.param->isMutable = true;
    node->data.param->functionName = functionName;
    node->data.param->defaultValue = NULL;
    return node;
}

ASTNode *createArgsNode(char *name, CryoDataType type, CryoNodeType nodeType, bool isLiteral, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(nodeType, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create args node");
        return NULL;
    }

    switch (nodeType)
    {
    case NODE_LITERAL_EXPR:
    {
        node->data.literal->dataType = type;
        switch (type)
        {
        case DATA_TYPE_INT:
            node->data.literal->value.intValue = atoi(name);
            break;
        case DATA_TYPE_FLOAT:
            node->data.literal->value.floatValue = atof(name);
            break;
        case DATA_TYPE_STRING:
            node->data.literal->value.stringValue = strdup(name);
            break;
        case DATA_TYPE_BOOLEAN:
            node->data.literal->value.booleanValue = strcmp(name, "true") == 0 ? true : false;
            break;
        case DATA_TYPE_VOID:
            break;
        default:
            logMessage("ERROR", __LINE__, "AST", "Unknown data type: %s", CryoDataTypeToString(type));
            CONDITION_FAILED;
        }
        break;
    }
    case NODE_VAR_NAME:
    {
        node->data.varName->varName = strdup(name);
        node->data.varName->isRef = false;
        break;
    }
    default:
        logMessage("ERROR", __LINE__, "AST", "Unknown node type: %s", CryoNodeTypeToString(nodeType));
        CONDITION_FAILED;
    }

    return node;
}

/* @Node_Creation - Modules & Externals */
ASTNode *createImportNode(char *module, char *subModule, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_IMPORT_STATEMENT, arena, state);
    if (!node)
        return NULL;

    node->data.import->moduleName = strdup(module);
    node->data.import->subModuleName = subModule ? strdup(subModule) : NULL;
    node->data.import->isStdModule = false;

    return node;
}

ASTNode *createExternNode(ASTNode *externNode, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_EXTERN_STATEMENT, arena, state);
    if (!node)
        return NULL;
    node->data.externNode->externNode = externNode;
    return node;
}

/* @Node_Creation - Conditionals */
ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state)
{
    return createIfBlock(condition, then_branch, else_branch, arena, state);
}

ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state)
{
    return createForBlock(initializer, condition, increment, body, arena, state);
}

ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state)
{
    return createWhileBlock(condition, body, arena, state);
}

/* @Node_Creation - Arrays */
ASTNode *createArrayLiteralNode(Arena *arena, CompilerState *state)
{
    return createASTNode(NODE_ARRAY_LITERAL, arena, state);
}

// Add a new function to create an index expression node
ASTNode *createIndexExprNode(char *arrayName, ASTNode *arrayRef, ASTNode *index, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_INDEX_EXPR, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create index expression node");
        return NULL;
    }

    node->data.indexExpr->name = strdup(arrayName);
    node->data.indexExpr->array = arrayRef;
    node->data.indexExpr->index = index;

    return node;
}

ASTNode *createVarReassignment(char *varName, ASTNode *existingVarNode, ASTNode *newVarNode, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_VAR_REASSIGN, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create variable reassignment node");
        return NULL;
    }

    node->data.varReassignment->existingVarName = strdup(varName);
    node->data.varReassignment->existingVarNode = existingVarNode;
    node->data.varReassignment->newVarNode = newVarNode;

    return node;
}

ASTNode *createFieldNode(char *fieldName, CryoDataType type, ASTNode *fieldValue, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_PROPERTY, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create field node");
        return NULL;
    }

    node->data.property->name = strdup(fieldName);
    node->data.property->value = fieldValue;
    node->data.property->type = type;

    return node;
}

ASTNode *createStructNode(char *structName, ASTNode **properties, int propertyCount, Arena *arena, CompilerState *state)
{
    ASTNode *node = createASTNode(NODE_STRUCT_DECLARATION, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create struct node");
        return NULL;
    }

    node->data.structNode->name = strdup(structName);
    node->data.structNode->properties = properties;
    node->data.structNode->propertyCount = propertyCount;
    node->data.structNode->propertyCapacity = 64;

    return node;
}

/* @Node_Creation - Scoped Calls */
ASTNode *createScopedFunctionCall(Arena *arena, CompilerState *state, const char *functionName)
{
    ASTNode *node = createASTNode(NODE_SCOPED_FUNCTION_CALL, arena, state);
    if (!node)
    {
        logMessage("ERROR", __LINE__, "AST", "Failed to create scoped function call node");
        return NULL;
    }

    node->data.scopedFunctionCall->functionName = strdup(functionName);

    return node;
}
