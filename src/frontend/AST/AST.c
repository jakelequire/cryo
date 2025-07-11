/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
#include "dataTypes/dataTypeManager.h"
#include "frontend/frontendSymbolTable.h"

/* ====================================================================== */
// @Global_Variables
ASTNode *programNode = NULL;

// -------------------------------------------------------------------

// This function takes in a preprocessed AST Node which contains the definitions of the program
// this is apart of the bootstrap process, and will be used to generate the runtime code.
ASTNode *appendASTNodeDefs(ASTNode *root, Arena *arena)
{
    __STACK_FRAME__
    if (!root)
    {
        logMessage(LMI, "ERROR", "AST", "Root node is NULL");
        return NULL;
    }

    int statementCount = root->data.program->statementCount;

    return root;
}

ASTNode *copyASTNode(ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
        return NULL;

    ASTNode *copy = (ASTNode *)malloc(sizeof(ASTNode));
    if (!copy)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for AST node copy");
        return NULL;
    }

    memcpy(copy, node, sizeof(ASTNode));

    return copy;
}

const char *getNamespaceNameFromRootNode(ASTNode *root)
{
    __STACK_FRAME__
    if (!root)
    {
        logMessage(LMI, "ERROR", "AST", "Root node is NULL");
        return NULL;
    }

    // This node is assumed to be the program node.
    // We will have to loop through the program node to find the namespace node.
    ASTNode *currentNode = root;
    while (currentNode->metaData->type != NODE_NAMESPACE)
    {
        currentNode = currentNode->data.program->statements[0];
    }
    const char *name = currentNode->data.cryoNamespace->name;
    printf("Namespace Name: %s\n", name);

    return name;
}

// ======================================================================

ASTNode *createNamespaceNode(char *name, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_NAMESPACE, arena, state, lexer);
    if (!node)
        return NULL;
    assert(name != NULL);
    node->data.cryoNamespace->name = strdup(name);

    logMessage(LMI, "INFO", "AST", "Created namespace node with name: %s", strdup(name));

    return node;
}

void ProgramNode_addStatement(ASTNode *self, ASTNode *statement)
{
    __STACK_FRAME__
    if (!self || !statement)
    {
        logMessage(LMI, "ERROR", "AST", "ProgramNode_addStatement: Self or statement node is NULL");
        return;
    }
    logMessage(LMI, "INFO", "AST", "Adding statement to program node...");

    // Check if the statement array is full
    if (self->data.program->statementCount >= self->data.program->statementCapacity)
    {
        // Resize the statement array
        self->data.program->statementCapacity *= 2;
        self->data.program->statements = (ASTNode **)realloc(self->data.program->statements, sizeof(ASTNode *) * self->data.program->statementCapacity);
        if (!self->data.program->statements)
        {
            logMessage(LMI, "ERROR", "AST", "Failed to reallocate memory for program node statements");
            return;
        }
    }

    // Add the statement to the array
    self->data.program->statements[self->data.program->statementCount++] = statement;
}

void ProgramNode_importAST(ASTNode *self, ASTNode *imported)
{
    __STACK_FRAME__
    if (!self || !imported)
    {
        logMessage(LMI, "ERROR", "AST", "ProgramNode_importAST: Self or imported node is NULL");
        return;
    }

    // This imported node needs to be added to the statements array of the program node.
    // It should be added to the front of the array. The imported node needs to remove its
    // program node, create a module node to be put in the beginning of the primary program node.
    // The module node will contain the imported node.
    ASTNode *moduleNode = (ASTNode *)malloc(sizeof(ASTNode));
    if (!moduleNode)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for module node");
        return;
    }
    logMessage(LMI, "INFO", "AST", "Creating module node from imported node");
    moduleNode->data.module = createCryoModuleContainer(NULL, NULL);
    moduleNode->metaData = createMetaDataContainer(NULL, NULL);
    moduleNode->metaData->type = NODE_MODULE;
    moduleNode->metaData->line = 0;
    moduleNode->metaData->column = 0;
    moduleNode->print = logASTNode;

    logMessage(LMI, "INFO", "AST", "Module node created");
    if (imported->metaData->type == NODE_PROGRAM)
    {
        logMessage(LMI, "INFO", "AST", "Imported node is a program node");
        int statementCount = imported->data.program->statementCount;
        for (int i = 0; i < statementCount; i++)
        {
            moduleNode->data.module->addStatement(moduleNode, imported->data.program->statements[i]);
        }
    }
    else
    {
        moduleNode->data.module->addStatement(moduleNode, imported);
    }

    logMessage(LMI, "INFO", "AST", "Module node created from imported node");

    // Now we add the module node to the program node's statements array.
    // use the `addStatement` function to add the module node to the program node.
    self->data.program->addStatement(self, moduleNode);
    logMessage(LMI, "INFO", "AST", "Added module node to program node");
}

// Create a program node
ASTNode *createProgramNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PROGRAM, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.program->importAST = ProgramNode_importAST;
    node->data.program->addStatement = ProgramNode_addStatement;

    return node;
}

// Create a literal expression node
ASTNode *createLiteralExpr(int value, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.literal = (LiteralNode *)ARENA_ALLOC(arena, sizeof(LiteralNode));
    if (!node->data.literal)
    {
        return NULL;
    }

    logMessage(LMI, "DEBUG", "AST", "Created literal expression node with value: %d", value);
    int intCpy = value;
    node->data.literal->type = DTM->primitives->createInt();
    node->data.literal->value.intValue = value;
    // convert from int to string
    char *buffer = (char *)ARENA_ALLOC(arena, sizeof(char));
    sprintf(buffer, "%d", intCpy);
    node->data.literal->value.stringValue = buffer;

    logMessage(LMI, "DEBUG", "AST", "Literal expression node created with value: %d", value);
    return node;
}

// Create an expression statement node
ASTNode *createExpressionStatement(ASTNode *expression, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_EXPRESSION, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.expression = (CryoExpressionNode *)ARENA_ALLOC(arena, sizeof(CryoExpressionNode));
    if (!node->data.expression)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to allocate memory for expression statement node");
        return NULL;
    }

    logMessage(LMI, "DEBUG", "AST", "Creating expression statement node");

    node->data.expression->nodeType = expression->metaData->type;

    if (expression->metaData->type == NODE_VAR_NAME)
    {
        logMessage(LMI, "DEBUG", "AST", "Expression is a variable name");
        node->data.expression->data.varNameNode = expression->data.varName;
    }
    else if (expression->metaData->type == NODE_LITERAL_EXPR)
    {
        logMessage(LMI, "DEBUG", "AST", "Expression is a literal");
        node->data.expression->data.literalNode = expression->data.literal;
    }
    else
    {
        // Handle other types of expressions if needed
        logMessage(LMI, "ERROR", "AST", "Unsupported expression type");
        return NULL;
    }

    logMessage(LMI, "DEBUG", "AST", "Expression statement node created");
    return node;
}

// Create a binary expression node
ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_BINARY_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.bin_op = (CryoBinaryOpNode *)ARENA_ALLOC(arena, sizeof(CryoBinaryOpNode));
    if (!node->data.bin_op)
    {
        return NULL;
    }

    logMessage(LMI, "DEBUG", "AST", "Created binary expression node with operator: %s", CryoOperatorTypeToString(op));

    node->data.bin_op->left = left;
    node->data.bin_op->right = right;
    node->data.bin_op->op = op;

    return node;
}

ASTNode *createVarNameNode(char *name, DataType *varType, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_VAR_NAME, arena, state, lexer);
    if (!node)
        return NULL;

    logMessage(LMI, "DEBUG", "AST", "Created variable name node with name: %s", name);

    node->data.varName->varName = strdup(name);
    node->data.varName->type = varType;

    return node;
}

// Create a unary expression node
ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_UNARY_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.unary_op->op = op;
    node->data.unary_op->operand = operand;

    return node;
}

/* @Node_Creation - Literals */
ASTNode *createIntLiteralNode(int value, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state, lexer);
    if (!node)
    {
        return NULL;
    }
    logMessage(LMI, "INFO", "AST", "Created integer literal node with value: %d", value);

    node->data.literal->value.intValue = value;
    node->data.literal->type = DTM->primitives->createI32();
    node->data.literal->literalType = LITERAL_INT;

    return node;
}

ASTNode *createFloatLiteralNode(float value, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    logMessage(LMI, "INFO", "AST", "Created float literal node with value: %f", value);

    node->data.literal->type = DTM->primitives->createFloat();
    node->data.literal->value.floatValue = value;
    node->data.literal->literalType = LITERAL_FLOAT;

    return node;
}

ASTNode *createStringLiteralNode(const char *value, Arena *arena, CompilerState *state, Lexer *lexer, ParsingContext *parsingContext)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    logMessage(LMI, "INFO", "AST", "Created string literal node with value: %s", value);

    // Trim the `"` characters from the string
    char *trimmedString = strdup(value + 1);
    trimmedString[strlen(trimmedString) - 1] = '\0';

    // Handle any formatting characters
    trimmedString = handleStringFormatting(strdup(trimmedString));

    DEBUG_PRINT_FILTER({
        printf("Manipulated string: %s\n", strdup(trimmedString));
    });

    DataType *contextType = NULL;
    if (parsingContext->stringContextType)
    {
        contextType = parsingContext->stringContextType;
    }
    else
    {
        contextType = DTM->primitives->createStr();
    }

    int length = getStringLength(trimmedString);
    node->data.literal->type = contextType;
    node->data.literal->value.stringValue = strdup(trimmedString);
    node->data.literal->length = strlen(trimmedString);
    node->data.literal->literalType = LITERAL_STRING;

    return node;
}

char *handleStringFormatting(char *value)
{
    __STACK_FRAME__
    // Find the first instance of a format specifier
    char *formatSpecifier = strchr(value, '\\');
    if (!formatSpecifier)
    {
        return value;
    }

    // Find the character after the backslash
    char *formatChar = formatSpecifier + 1;
    char replacement = '\0';

    switch (*formatChar)
    {
    case 'n':
        replacement = '\n';
        break;
    case 't':
        replacement = '\t';
        break;
    case 'r':
        replacement = '\r';
        break;
    case '0':
        replacement = '\0';
        break;
    default:
        return value;
    }

    // Replace the escape sequence and shift the rest of the string left
    *formatSpecifier = replacement;
    memmove(formatSpecifier + 1, formatSpecifier + 2, strlen(formatSpecifier + 2) + 1);

    // Recursively handle any remaining escape sequences
    return handleStringFormatting(value);
}

int getStringLength(char *str)
{
    __STACK_FRAME__
    int length = 0;
    while (*str != '\0')
    {
        length++;
        str++;
    }
    return length;
}

ASTNode *createBooleanLiteralNode(int value, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_LITERAL_EXPR, arena, state, lexer);
    if (!node)
        return NULL;

    logMessage(LMI, "INFO", "AST", "Created boolean literal node with value: %s", value ? "true" : "false");

    node->data.literal->type = DTM->primitives->createPrimBoolean(value);
    node->data.literal->value.booleanValue = value;
    node->data.literal->literalType = LITERAL_BOOLEAN;

    return node;
}

ASTNode *createIdentifierNode(char *name, Arena *arena, CompilerState *state, Lexer *lexer, ParsingContext *context)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_VAR_NAME, arena, state, lexer);
    if (!node)
        return NULL;

    logMessage(LMI, "INFO", "AST", "Created identifier node with name: %s", name);
    char *varName = strdup(name);
    // Testing new symbol table.
    // Replacing the old `findSymbol` from the old symbol table seems to be working fine so far.
    // Attempt to find the symbol in the symbol table (It's okay if it can't find and can just create the node)
    const char *currentScopeID = getCurrentScopeID(context);
    FrontendSymbol *varSym = FEST->lookup(FEST, varName);
    if (varSym)
    {
        logMessage(LMI, "INFO", "AST", "Found symbol in global symbol table: %s", varSym->name);
        node->data.varName->varName = strdup(varSym->name);
        node->data.varName->isRef = true;
        node->data.varName->type = varSym->type;
    }
    else
    {
        // Unsure if I want to keep this, but it's a good fallback for now.
        logMessage(LMI, "INFO", "AST", "Symbol not found in global symbol table: %s", varName);
        node->data.varName->varName = strdup(varName);
        node->data.varName->isRef = true;
        node->data.varName->type = DTM->primitives->createUndefined();
    }

    return node;
}

/* @Node_Blocks - Blocks */
ASTNode *createBlockNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_BLOCK, arena, state, lexer);
    if (!node)
        return NULL;

    return node;
}

ASTNode *createFunctionBlock(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_FUNCTION_BLOCK, arena, state, lexer);
    if (!node)
        return NULL;

    return node;
}

ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_IF_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.ifStatement->condition = condition;
    node->data.ifStatement->thenBranch = then_branch;
    node->data.ifStatement->elseBranch = else_branch;
    return node;
}

ASTNode *createForLoopNode(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_FOR_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.forStatement->initializer = initializer;
    node->data.forStatement->condition = condition;
    node->data.forStatement->increment = increment;
    node->data.forStatement->body = body;

    return node;
}

ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_WHILE_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.whileStatement->condition = condition;
    node->data.whileStatement->body = body;
    return node;
}

/* @Node_Blocks - Literals */
ASTNode *createBooleanLiteralExpr(int value, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    return createBooleanLiteralNode(value, arena, state, lexer);
}

ASTNode *createStringExpr(char *str, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_STRING_EXPRESSION, arena, state, lexer);
    if (!node)
        return NULL;
    int length = strlen(str);
    node->data.literal->type = DTM->primitives->createStr();
    node->data.literal->value.stringValue = strdup(str);
    return node;
}

/* @Node_Creation - Variables */
ASTNode *createVarDeclarationNode(char *var_name, DataType *dataType, ASTNode *initializer,
                                  bool isMutable, bool isGlobal, bool isReference, bool isIterator, bool noInitializer,
                                  Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_VAR_DECLARATION, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create variable declaration node");
        return NULL;
    }

    if (!node->data.varDecl)
    {
        logMessage(LMI, "ERROR", "AST", "Variable declaration node data is NULL");
        return NULL;
    }

    node->data.varDecl->type = dataType;
    node->data.varDecl->name = strdup(var_name);
    node->data.varDecl->varNameNode = createVariableNameNodeContainer(var_name, arena, state);
    node->metaData->line = 0;
    node->data.varDecl->isGlobal = isGlobal;
    node->data.varDecl->isLocal = !isGlobal;
    node->data.varDecl->isReference = isReference;
    node->data.varDecl->isMutable = isMutable;
    node->data.varDecl->isIterator = isIterator;
    node->data.varDecl->noInitializer = noInitializer;

    if (noInitializer)
    {
        node->data.varDecl->initializer = NULL;
    }
    else
    {
        node->data.varDecl->initializer = initializer;
    }

    logMessage(LMI, "INFO", "AST", "Created variable declaration node for %s", var_name);
    return node;
}

ASTNode *createVariableExpr(char *name, bool isReference, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_VAR_NAME, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.varName->varName = strdup(name);
    node->data.varName->isRef = isReference;
    return node;
}

/* @Node_Creation - Functions */
ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, DataType *returnType, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_FUNCTION_DECLARATION, arena, state, lexer);
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
    node->data.functionDecl->type = returnType;
    node->data.functionDecl->functionType = NULL;

    return node;
}

ASTNode *createExternFuncNode(char *function_name, ASTNode **params, int paramCount, DataType *functionType, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_EXTERN_FUNCTION, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.externFunction->name = strdup(function_name);
    if (!node->data.externFunction->name)
    {
        return NULL;
    }

    node->data.externFunction->params = params;
    node->data.externFunction->paramCount = paramCount;
    node->data.externFunction->type = functionType;

    return node;
}

ASTNode *createFunctionCallNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    return createASTNode(NODE_FUNCTION_CALL, arena, state, lexer);
}

ASTNode *createReturnNode(ASTNode *returnValue, DataType *returnType, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.returnStatement->returnValue = returnValue;
    node->data.returnStatement->expression = returnValue;
    node->data.returnStatement->type = returnType;

    return node;
}

ASTNode *createReturnExpression(ASTNode *returnExpression, DataType *returnType, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_RETURN_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.returnStatement->expression = returnExpression;
    node->data.returnStatement->type = returnType;
    return node;
}

/* @Node_Creation - Parameters */
ASTNode *createParamListNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    return createASTNode(NODE_PARAM_LIST, arena, state, lexer);
}

ASTNode *createArgumentListNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    return createASTNode(NODE_ARG_LIST, arena, state, lexer);
}

ASTNode *createParamNode(char *name, char *functionName, DataType *type, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PARAM, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.param->name = strdup(name);
    node->data.param->type = type;
    node->data.param->hasDefaultValue = false;
    node->data.param->isMutable = true;
    node->data.param->functionName = strdup(functionName);
    node->data.param->defaultValue = NULL;
    return node;
}

ASTNode *createArgsNode(char *name, DataType *type, CryoNodeType nodeType, bool isLiteral, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(nodeType, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create args node");
        return NULL;
    }

    switch (nodeType)
    {
    case NODE_LITERAL_EXPR:
    {
        node->data.literal->type = type;
        switch (type->container->primitive)
        {
        case PRIM_INT:
            node->data.literal->value.intValue = atoi(name);
            break;
        case PRIM_FLOAT:
            node->data.literal->value.floatValue = atof(name);
            break;
        case PRIM_STRING:
            node->data.literal->value.stringValue = strdup(name);
            break;
        case PRIM_BOOLEAN:
            node->data.literal->value.booleanValue = strcmp(name, "true") == 0 ? true : false;
            break;
        case PRIM_I8:
        case PRIM_I16:
        case PRIM_I32:
        case PRIM_I64:
        case PRIM_I128:
            node->data.literal->value.intValue = atoi(name);
            break;
        case PRIM_OBJECT:
        case PRIM_NULL:
        case PRIM_VOID:
            break;
        default:
            logMessage(LMI, "ERROR", "AST", "Unknown data type: %s", DTM->debug->dataTypeToString(type));
            CONDITION_FAILED;
        }
        break;
    }
    case NODE_VAR_NAME:
    {
        node->data.varName->varName = strdup(name);
        node->data.varName->isRef = false;
        node->data.varName->type = type;
        break;
    }
    case NODE_VAR_DECLARATION:
    {
        node->data.varDecl->name = strdup(name);
        node->data.varDecl->varNameNode = createVariableNameNodeContainer(name, arena, state);
        node->data.varDecl->isGlobal = false;
        node->data.varDecl->isLocal = true;
        node->data.varDecl->isReference = true;
        node->data.varDecl->isMutable = false;
        node->data.varDecl->isIterator = false;
        node->data.varDecl->initializer = NULL;
        node->data.varDecl->type = type;
        break;
    }
    default:
        logMessage(LMI, "ERROR", "AST", "Unknown node type: %s", CryoNodeTypeToString(nodeType));
        CONDITION_FAILED;
    }

    return node;
}

/* @Node_Creation - Modules & Externals */
ASTNode *createImportNode(char *module, char *subModule, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_IMPORT_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;

    node->data.import->moduleName = strdup(module);
    node->data.import->subModuleName = subModule ? strdup(subModule) : NULL;
    node->data.import->isStdModule = false;

    return node;
}

ASTNode *createExternNode(ASTNode *externNode, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_EXTERN_STATEMENT, arena, state, lexer);
    if (!node)
        return NULL;
    node->data.externNode->externNode = externNode;
    return node;
}

/* @Node_Creation - Conditionals */
ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createIfBlock(condition, then_branch, else_branch, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create if statement node");
        return NULL;
    }
    return node;
}

ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createForLoopNode(initializer, condition, increment, body, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create for statement node");
        return NULL;
    }
    return node;
}

ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createWhileBlock(condition, body, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create while statement node");
        return NULL;
    }
    return node;
}

/* @Node_Creation - Arrays */
ASTNode *createArrayLiteralNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_ARRAY_LITERAL, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create array literal node");
        return NULL;
    }
    return node;
}

// Add a new function to create an index expression node
ASTNode *createIndexExprNode(char *arrayName, ASTNode *arrayRef, ASTNode *index, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_INDEX_EXPR, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create index expression node");
        return NULL;
    }

    node->data.indexExpr->name = strdup(arrayName);
    node->data.indexExpr->array = arrayRef;
    node->data.indexExpr->index = index;

    return node;
}

ASTNode *createVarReassignment(char *varName, ASTNode *existingVarNode, ASTNode *newVarNode, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_VAR_REASSIGN, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create variable reassignment node");
        return NULL;
    }

    node->data.varReassignment->existingVarName = strdup(varName);
    node->data.varReassignment->existingVarNode = existingVarNode;
    node->data.varReassignment->newVarNode = newVarNode;

    return node;
}

ASTNode *createFieldNode(const char *fieldName, DataType *type, const char *parentName, CryoNodeType parentNodeType, ASTNode *fieldValue, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PROPERTY, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create field node");
        return NULL;
    }

    node->data.property->name = fieldName;
    node->data.property->value = fieldValue;
    node->data.property->type = type;
    node->data.property->parentName = parentName;
    node->data.property->parentNodeType = parentNodeType;

    return node;
}

ASTNode *createStructNode(const char *structName, ASTNode **properties, int propertyCount, ASTNode *constructor,
                          ASTNode **methods, int methodCount,
                          Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_STRUCT_DECLARATION, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create struct node");
        return NULL;
    }

    node->data.structNode->name = structName;
    node->data.structNode->properties = properties;
    node->data.structNode->propertyCount = propertyCount;
    node->data.structNode->propertyCapacity = 64;
    node->data.structNode->methods = methods;
    node->data.structNode->methodCount = methodCount;
    node->data.structNode->methodCapacity = 64;
    node->data.structNode->constructor = constructor;
    node->data.structNode->ctorArgs = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    node->data.structNode->ctorArgCount = 0;

    return node;
}

ASTNode *createStructConstructor(char *structName, ASTNode **fields, int argCount, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_STRUCT_CONSTRUCTOR, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create struct constructor node");
        return NULL;
    }

    node->data.structConstructor->name = strdup(structName);
    node->data.structConstructor->args = fields;
    node->data.structConstructor->argCount = argCount;
    node->data.structConstructor->argCapacity = 64;
    return node;
}

/* @Node_Creation - Scoped Calls */
ASTNode *createScopedFunctionCall(Arena *arena, CompilerState *state, const char *functionName, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_SCOPED_FUNCTION_CALL, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create scoped function call node");
        return NULL;
    }

    node->data.scopedFunctionCall->functionName = strdup(functionName);

    return node;
}

ASTNode *createPropertyAccessNode(ASTNode *object, const char *property, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PROPERTY_ACCESS, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create property access node");
        return NULL;
    }

    node->data.propertyAccess->object = object;
    node->data.propertyAccess->propertyName = property;

    return node;
}

ASTNode *createThisNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_THIS, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create 'this' node");
        return NULL;
    }

    return node;
}

ASTNode *createPropertyReassignmentNode(ASTNode *object, const char *property, ASTNode *newValue, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PROPERTY_REASSIGN, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create property reassignment node");
        return NULL;
    }

    node->data.propertyReassignment->object = object;
    node->data.propertyReassignment->name = strdup(property);
    node->data.propertyReassignment->value = newValue;

    return node;
}

ASTNode *createConstructorNode(char *structName, ASTNode *body, ASTNode **fields, int argCount, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_STRUCT_CONSTRUCTOR, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create constructor node");
        return NULL;
    }

    node->data.structConstructor->name = strdup(structName);
    node->data.structConstructor->args = fields;
    node->data.structConstructor->argCount = argCount;
    node->data.structConstructor->argCapacity = 64;
    node->data.structConstructor->constructorBody = body;

    return node;
}

ASTNode *createStructPropertyAccessNode(ASTNode *object, ASTNode *property, const char *propertyName, DataType *type, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_PROPERTY_ACCESS, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create struct property access node");
        return NULL;
    }

    node->data.propertyAccess->object = object;
    node->data.propertyAccess->property = property;
    node->data.propertyAccess->propertyName = propertyName;
    node->data.propertyAccess->propertyIndex = -1; // Need to find a replacement for `getPropertyAccessIndex(type, propertyName);`

    return node;
}

ASTNode *createMethodNode(DataType *type, ASTNode *body, const char *methodName, ASTNode **args, int argCount, const char *parentName, bool isStatic,
                          Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_METHOD, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create method node");
        return NULL;
    }

    node->data.method->name = methodName;
    node->data.method->body = body;
    node->data.method->params = args;
    node->data.method->paramCount = argCount;
    node->data.method->type = type;
    node->data.method->functionType = type;
    node->data.method->parentName = parentName;
    node->data.method->isStatic = isStatic;

    return node;
}

ASTNode *createMethodCallNode(ASTNode *accessorObj, DataType *returnType, DataType *instanceType, const char *methodName,
                              ASTNode **args, int argCount, bool isStatic,
                              Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_METHOD_CALL, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create method call node");
        return NULL;
    }

    node->data.methodCall->returnType = returnType;
    node->data.methodCall->instanceType = instanceType;
    node->data.methodCall->accessorObj = accessorObj;
    node->data.methodCall->name = strdup(methodName);
    node->data.methodCall->args = args;
    node->data.methodCall->argCount = argCount;
    node->data.methodCall->instanceName = DTM->debug->dataTypeToString(instanceType);
    node->data.methodCall->isStatic = isStatic;

    return node;
}

ASTNode *createGenericDeclNode(DataType *type, const char *name, GenericType **properties, int propertyCount,
                               DataType **constraintTypes, bool hasConstraint,
                               Arena *arena, CompilerState *state, Lexer *lexer)

{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_GENERIC_DECL, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create method call node");
        return NULL;
    }

    node->data.genericDecl->name = strdup(name);
    node->data.genericDecl->properties = properties;
    node->data.genericDecl->propertyCount = propertyCount;
    node->data.genericDecl->constraintTypes = constraintTypes;
    node->data.genericDecl->hasConstraint = hasConstraint;
    node->data.genericDecl->type = type;

    return node;
}

ASTNode *createGenericInstNode(const char *baseName, DataType **typeArguments, int argumentCount, DataType *resultType,
                               Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_GENERIC_INST, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create method call node");
        return NULL;
    }

    node->data.genericInst->baseName = baseName;
    node->data.genericInst->typeArguments = typeArguments;
    node->data.genericInst->argumentCount = argumentCount;
    node->data.genericInst->resultType = resultType;

    return node;
}

ASTNode *createClassDeclarationNode(const char *className,
                                    Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_CLASS, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create class declaration node");
        return NULL;
    }

    node->data.classNode->name = className;
    node->data.classNode->propertyCount = 0;
    node->data.classNode->methodCount = 0;

    return node;
}

ASTNode *createClassConstructor(const char *className, ASTNode *body, ASTNode **fields, int argCount, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_CLASS_CONSTRUCTOR, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create class constructor node");
        return NULL;
    }

    node->data.classConstructor->name = strdup(className);
    node->data.classConstructor->args = fields;
    node->data.classConstructor->argCount = argCount;
    node->data.classConstructor->argCapacity = ARG_CAPACITY;
    node->data.classConstructor->constructorBody = body;

    return node;
}

ASTNode *createObject(const char *objectName, DataType *objectType, bool isNew,
                      ASTNode **args, int argCount,
                      Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_OBJECT_INST, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create object node");
        return NULL;
    }

    node->data.objectNode->name = strdup(objectName);
    node->data.objectNode->objType = objectType;
    node->data.objectNode->isNewInstance = isNew;
    node->data.objectNode->args = args;
    node->data.objectNode->argCount = argCount;

    return node;
}

ASTNode *createObjectWithGenerics(const char *objectName, DataType *objectType, bool isNew,
                                  ASTNode **args, int argCount,
                                  DataType **generics, int genericCount,
                                  Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_OBJECT_INST, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create object node");
        return NULL;
    }

    node->data.objectNode->name = strdup(objectName);
    node->data.objectNode->objType = objectType;
    node->data.objectNode->isNewInstance = isNew;

    node->data.objectNode->args = args;
    node->data.objectNode->argCount = argCount;

    node->data.objectNode->genericTypes = generics;
    node->data.objectNode->genericCount = genericCount;

    return node;
}

ASTNode *createNullNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_NULL_LITERAL, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create null node");
        return NULL;
    }

    return node;
}

ASTNode *createTypeofNode(ASTNode *expression, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_TYPEOF, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create typeof node");
        return NULL;
    }

    node->data.typeofNode->expression = expression;

    return node;
}

ASTNode *createUsingNode(const char *primaryModule, const char *secondaryModules[],
                         int secondaryModuleCount, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_USING, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create using node");
        return NULL;
    }

    node->data.usingNode->primaryModule = strdup(primaryModule);
    node->data.usingNode->secondaryModules = secondaryModules;
    node->data.usingNode->secondaryModuleCount = secondaryModuleCount;

    return node;
}

ASTNode *createModuleNode(const char *moduleName, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_MODULE, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create module node");
        return NULL;
    }

    node->data.moduleNode->moduleName = strdup(moduleName);

    return node;
}

ASTNode *createAnnotationNode(const char *annotationName, const char *annotationValue, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_ANNOTATION, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create annotation node");
        return NULL;
    }

    node->data.annotation->name = strdup(annotationName);
    node->data.annotation->value = strdup(annotationValue);

    return node;
}

ASTNode *createTypeDeclNode(const char *typeName, DataType *type, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "AST", "Creating type declaration node for %s", typeName);
    ASTNode *node = createASTNode(NODE_TYPE, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create type declaration node");
        return NULL;
    }

    node->data.typeDecl->name = strdup(typeName);
    node->data.typeDecl->type = type;

    return node;
}

ASTNode *createTypeCastNode(DataType *type, ASTNode *expression, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_TYPE_CAST, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create type cast node");
        return NULL;
    }

    node->data.typeCast->type = type;
    node->data.typeCast->expression = expression;

    return node;
}

ASTNode *createDiscardNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_DISCARD, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create discard node");
        return NULL;
    }

    return node;
}

ASTNode *createImplementationNode(const char *interfaceName, ASTNode **properties, int propertyCount,
                                  ASTNode **methods, int methodCount,
                                  Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_IMPLEMENTATION, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create implementation node");
        return NULL;
    }

    node->data.implementation->interfaceName = interfaceName;
    node->data.implementation->properties = properties;
    node->data.implementation->propertyCount = propertyCount;
    node->data.implementation->methods = methods;
    node->data.implementation->methodCount = methodCount;

    return node;
}

ASTNode *createBreakNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_BREAK, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create break node");
        return NULL;
    }

    return node;
}

ASTNode *createContinueNode(Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    ASTNode *node = createASTNode(NODE_CONTINUE, arena, state, lexer);
    if (!node)
    {
        logMessage(LMI, "ERROR", "AST", "Failed to create continue node");
        return NULL;
    }

    return node;
}