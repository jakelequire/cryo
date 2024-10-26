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
#include "frontend/AST.h"

/* ====================================================================== */
// @Global_Variables
ASTNode *programNode = NULL;


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
    char *trimmedString = strdup(value + 1);
    trimmedString[strlen(trimmedString) - 1] = '\0';

    // Handle any formatting characters
    trimmedString = handleStringFormatting(strdup(trimmedString));

    printf("Manipulated string: %s\n", strdup(trimmedString));

    node->data.literal->dataType = DATA_TYPE_STRING;
    node->data.literal->value.stringValue = strdup(trimmedString);
    node->data.literal->length = strlen(trimmedString);

    return node;
}

char *handleStringFormatting(char *value)
{
    // Find the first instance of a format specifier
    // i.e `\n`, `\t`, etc.
    char *formatSpecifier = strchr(value, '\\');
    if (!formatSpecifier)
    {
        return value;
    }

    // Find the character after the backslash
    char *formatChar = formatSpecifier + 1;
    switch (*formatChar)
    {
    case 'n':
        *formatSpecifier = '\n';
        break;
    case 't':
        *formatSpecifier = '\t';
        break;
    case 'r':
        *formatSpecifier = '\r';
        break;
    case '0':
        *formatSpecifier = '\0';
        break;
    default:
        break;
    }

    // Recursively call the function to handle the next format specifier
    return handleStringFormatting(formatSpecifier);
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
        logMessage("WARN", __LINE__, "AST", "Return statement has void return type");
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