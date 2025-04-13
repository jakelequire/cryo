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
#include "diagnostics/diagnostics.h"
#include "dataTypes/dataTypeManager.h"

/// ---
/// ### Structure
///```
/// typedef struct CryoNamespace
/// {
///     char *name;
///     const char *parentName;
///     bool hasParent;
/// } CryoNamespace;
///```
CryoNamespace *createCryoNamespaceNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoNamespace *node = (CryoNamespace *)ARENA_ALLOC(arena, sizeof(CryoNamespace));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoNamespace node.");
        return NULL;
    }

    node->name = "defaulted";
    node->parentName = (const char *)malloc(64 * sizeof(char));
    node->hasParent = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoProgram
/// {
///     ASTNode **statements;
///     size_t statementCount;
///     size_t statementCapacity;
/// } CryoProgram;
///```
CryoProgram *createCryoProgramContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoProgram *node = (CryoProgram *)ARENA_ALLOC(arena, sizeof(CryoProgram));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoProgram node.");
        return NULL;
    }

    node->statements = (ASTNode **)calloc(PROGRAM_CAPACITY, sizeof(ASTNode *));
    if (!node->statements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate statements array.");
        free(node);
        return NULL;
    }

    node->statementCount = 0;
    node->statementCapacity = PROGRAM_CAPACITY;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoBlockNode
/// {
///     ASTNode **statements;
///     int statementCount;
///     int statementCapacity;
/// } CryoBlockNode;
///```
///
CryoBlockNode *createCryoBlockNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoBlockNode *node = (CryoBlockNode *)ARENA_ALLOC(arena, sizeof(CryoBlockNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoBlockNode node.");
        return NULL;
    }

    int initialCapacity = BLOCK_CAPACITY;
    node->statements = (ASTNode **)ARENA_ALLOC(arena, initialCapacity * sizeof(ASTNode *));
    if (!node->statements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate statements array.");
        free(node);
        return NULL;
    }

    node->statementCount = 0;
    node->statementCapacity = initialCapacity;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoFunctionBlock
/// {
///     ASTNode *function;
///     ASTNode **statements;
///     int statementCount;
///     int statementCapacity;
/// } CryoFunctionBlock;
///```
///
CryoFunctionBlock *createCryoFunctionBlockContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    int initialCapacity = FUNCTION_BLOCK_CAPACITY;
    CryoFunctionBlock *block = (CryoFunctionBlock *)ARENA_ALLOC(arena, sizeof(CryoFunctionBlock));
    if (!block)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoFunctionBlock node.");
        return NULL;
    }

    block->function = NULL;
    block->statements = (ASTNode **)calloc(initialCapacity, sizeof(ASTNode *));
    if (!block->statements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate statements array.");
        free(block);
        return NULL;
    }

    block->statementCount = 0;
    block->statementCapacity = initialCapacity;

    return block;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoModule
/// {
///     ASTNode **astTable;
/// } CryoModule;
///```
///

void CryoModule_resize(ASTNode *self)
{
    __STACK_FRAME__
    if (self->data.module->statementCount >= self->data.module->statementCapacity)
    {
        logMessage(LMI, "INFO", "AST", "Resizing module statement array...");
        self->data.module->statementCapacity *= 2;
        size_t newSize = sizeof(ASTNode *) * self->data.module->statementCapacity;
        self->data.module->statements = (ASTNode **)realloc(self->data.module->statements, newSize);
        if (!self->data.module->statements)
        {
            fprintf(stderr, "[AST] Error: Failed to reallocate memory for module statements.");
            return;
        }
    }
}

void CryoModule_addStatement(ASTNode *self, ASTNode *statement)
{
    __STACK_FRAME__
    CryoModule_resize(self);
    logMessage(LMI, "INFO", "AST", "Adding statement to module...");
    self->data.module->statements[self->data.module->statementCount++] = statement;
}

void CryoModule_setModuleName(ASTNode *self, const char *name)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "AST", "Setting module name...");
    self->data.module->moduleName = name;
}

void CryoModule_importProgramNode(ASTNode *self, ASTNode *program)
{
    logMessage(LMI, "INFO", "AST", "Importing program node...");
    DEBUG_BREAKPOINT;
}

CryoModule *createCryoModuleContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoModule *node = (CryoModule *)malloc(sizeof(CryoModule));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate createCryoModuleContainer node.");
        return NULL;
    }

    node->statements = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    if (!node->statements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate statements array.");
        free(node);
        return NULL;
    }

    node->moduleName = "defaulted";
    node->statementCount = 0;
    node->statementCapacity = 64;

    node->addStatement = CryoModule_addStatement;
    node->resize = CryoModule_resize;
    node->setModuleName = CryoModule_setModuleName;
    node->importProgramNode = CryoModule_importProgramNode;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoMetaData
/// {
///     enum CryoNodeType type;      // Node Type
///     int line;                    // Line number for error reporting
///     int column;                  // Column number for error reporting
///     struct ASTNode *firstChild;  // First child node (for linked list structure)
///     struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
///     char *moduleName;            // Current Module
/// } CryoMetaData;
///```
///
CryoMetaData *createMetaDataContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoMetaData *node = (CryoMetaData *)malloc(sizeof(CryoMetaData));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoMetaData node.");
        return NULL;
    }

    node->type = NODE_UNKNOWN;
    node->line = 0;
    node->column = 0;

    node->moduleName = "defaulted";

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoImportNode
/// {
///     char *moduleName;
///     char *subModuleName;
///     bool isStdModule;
/// } CryoImportNode;
///```
///
CryoImportNode *createCryoImportNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoImportNode *node = (CryoImportNode *)ARENA_ALLOC(arena, sizeof(CryoImportNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoImportNode node.");
        return NULL;
    }

    node->moduleName = (char *)calloc(1, sizeof(char));
    node->subModuleName = (char *)calloc(1, sizeof(char));
    node->isStdModule = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoScope
/// {
///     int scopeLevel;
///     char *scopeName;
///     CryoNodeType *scopeType;
///     bool isGlobal;
/// } CryoScope;
///```
///
CryoScope *createCryoScopeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoScope *node = (CryoScope *)ARENA_ALLOC(arena, sizeof(CryoScope));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoScope node.");
        return NULL;
    }

    node->scopeLevel = 0;
    node->scopeName = "unknown_scope";
    node->scopeType = NODE_UNKNOWN;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ExternNode
/// {
///     struct ASTNode *externNode;
/// } ExternNode;
///```
///
ExternNode *createExternNodeContainer(CryoNodeType type, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ExternNode *node = (ExternNode *)ARENA_ALLOC(arena, sizeof(ExternNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ExternNode node.");
        return NULL;
    }

    switch (type)
    {
    case NODE_EXTERN_FUNCTION:
    {
        node->externNode = (ASTNode *)ARENA_ALLOC(arena, sizeof(ExternFunctionNode));
        break;
    }
    default:
    {
        fprintf(stderr, "[AST] Error: Unknown node type for ExternNode."); // Should never happen
        return NULL;
    }
    }

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ExternFunctionNode
/// {
///     char *name;
///     struct ASTNode **params;
///     int paramCount;
///     int paramCapacity;
///     CryoDataType returnType;
/// } ExternFunctionNode;
///```
///
ExternFunctionNode *createExternFunctionNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ExternFunctionNode *node = (ExternFunctionNode *)malloc(sizeof(ExternFunctionNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ExternFunctionNode node.");
        return NULL;
    }

    node->name = (char *)malloc(sizeof(char) * 1024);
    node->params = (ASTNode **)malloc(sizeof(ASTNode *) * PARAM_CAPACITY);
    node->paramCount = 0;
    node->paramCapacity = PARAM_CAPACITY;
    node->type = DTM->functionTypes->createFunctionTemplate();

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct FunctionDeclNode
/// {
///     enum CryoNodeType type;
///     enum CryoVisibilityType visibility;
///     enum CryoDataType returnType;
///     char *name;
///     struct ASTNode **params;
///     int paramCount;
///     int paramCapacity;
///     struct ASTNode *body;
/// } FunctionDeclNode;
///```
///
FunctionDeclNode *createFunctionNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    FunctionDeclNode *node = (FunctionDeclNode *)ARENA_ALLOC(arena, sizeof(FunctionDeclNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate FunctionDeclNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->params = NULL;
    node->body = NULL;
    node->visibility = VISIBILITY_PUBLIC;
    node->type = DTM->functionTypes->createFunctionTemplate();
    node->functionType = DTM->functionTypes->createFunctionTemplate();
    node->paramCount = 0;
    node->paramCapacity = PARAM_CAPACITY;
    node->paramTypes = NULL;

    node->parentScopeID = (const char *)malloc(sizeof(char) * 65);
    node->functionScopeID = (const char *)malloc(sizeof(char) * 65);

    node->isVariadic = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct FunctionCallNode
/// {
///     char *name;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
///     DataType *eturnType;
/// } FunctionCallNode;
///```
///
FunctionCallNode *createFunctionCallNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    FunctionCallNode *node = (FunctionCallNode *)ARENA_ALLOC(arena, sizeof(FunctionCallNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate FunctionCallNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;
    node->returnType = DTM->primitives->createVoid();
    node->isVariadic = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct LiteralNode
/// {
///     enum CryoDataType dataType; // Data type of the literal
///     union
///     {
///         int intValue;
///         float floatValue;
///         char *stringValue;
///         int booleanValue;
///     };
/// } LiteralNode;
///```
///
LiteralNode *createLiteralNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    LiteralNode *node = (LiteralNode *)ARENA_ALLOC(arena, sizeof(LiteralNode) * 2);
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate LiteralNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->length = 0;
    node->value.intValue = 0;
    node->value.floatValue = 0;
    node->value.stringValue = (const char *)calloc(1, sizeof(char *));
    node->value.booleanValue = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct IfStatementNode
/// {
///     struct ASTNode *condition;
///     struct ASTNode *thenBranch;
///     struct ASTNode *elseBranch;
/// } IfStatementNode;
///```
///
IfStatementNode *createIfStatementContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    IfStatementNode *node = (IfStatementNode *)ARENA_ALLOC(arena, sizeof(IfStatementNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate IfStatementNode node.");
        return NULL;
    }

    node->condition = NULL;
    node->elseBranch = NULL;
    node->thenBranch = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ForStatementNode
/// {
///     struct ASTNode *initializer;
///     struct ASTNode *condition;
///     struct ASTNode *increment;
///     struct ASTNode *body;
/// } ForStatementNode;
///```
///
ForStatementNode *createForStatementNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ForStatementNode *node = (ForStatementNode *)ARENA_ALLOC(arena, sizeof(ForStatementNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ForStatementNode node.");
        return NULL;
    }

    node->initializer = NULL;
    node->condition = NULL;
    node->increment = NULL;
    node->incrementOp = OPERATOR_UNKNOWN;
    node->body = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct WhileStatementNode
/// {
///     struct ASTNode *condition;
///     struct ASTNode *body;
/// } WhileStatementNode;
///```
///
WhileStatementNode *createWhileStatementNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    WhileStatementNode *node = (WhileStatementNode *)ARENA_ALLOC(arena, sizeof(WhileStatementNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate WhileStatementNode node.");
        return NULL;
    }

    node->condition = NULL;
    node->body = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoExpressionNode
/// {
///     CryoNodeType nodeType;
///     union
///     {
///         struct VariableNameNode *varNameNode;
///         struct LiteralNode *literalNode;
///     } data;
/// } CryoExpressionNode;
///```
///
CryoExpressionNode *createExpressionNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoExpressionNode *node = (CryoExpressionNode *)ARENA_ALLOC(arena, sizeof(CryoExpressionNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ExpressionNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    ;
    node->data.varNameNode = NULL;
    node->data.literalNode = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoVariableNode
/// {
///     enum CryoDataType type;
///     struct VariableNameNode *varNameNode;
///     char *name;
///     bool isGlobal;
///     bool isLocal;
///     bool isReference;
///     bool isMutable;
///     bool isIterator;
///     struct ASTNode *initializer;
///     struct ASTNode *indexExpr;
///     bool hasIndexExpr;
///     ASTNode *indexExpr;
/// } CryoVariableNode;
///```
///
CryoVariableNode *createVariableNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoVariableNode *node = (CryoVariableNode *)ARENA_ALLOC(arena, sizeof(CryoVariableNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->varNameNode = NULL;
    node->name = (char *)calloc(1, sizeof(char));
    node->isGlobal = false;
    node->isLocal = false;
    node->isReference = false;
    node->isMutable = false;
    node->isIterator = false;
    node->initializer = NULL;
    node->hasIndexExpr = false;
    node->indexExpr = NULL;

    node->isNewInstance = false;
    node->hasUnaryOp = false;
    node->unaryOp = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct VariableNameNode
/// {
///     enum CryoDataType refType;
///     bool isRef;
///     char *varName;
/// } VariableNameNode;
///```
///
VariableNameNode *createVariableNameNodeContainer(char *varName, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    VariableNameNode *node = (VariableNameNode *)ARENA_ALLOC(arena, sizeof(VariableNameNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableNameNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->isRef = false;
    node->varName = strdup(varName);
    node->scopeID = (char *)calloc(1, sizeof(char));

    node->hasUnaryOp = false;
    node->unaryOp = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoParameterNode
/// {
///     enum CryoDataType type;
///     char *name;
///     bool hasDefaultValue;
///     bool isMutable;
///     struct ASTNode *defaultValue;
/// } CryoParameterNode;
///```
///
CryoParameterNode *createParameterNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoParameterNode *node = (CryoParameterNode *)ARENA_ALLOC(arena, sizeof(CryoParameterNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ParameterNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));
    node->functionName = (char *)calloc(1, sizeof(char));
    node->hasDefaultValue = false;
    node->isMutable = true;
    node->defaultValue = NULL;
    node->isVariadic = false;
    node->variadicElementType = DTM->primitives->createUndefined();

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ParamNode
/// {
///     enum CryoNodeType nodeType;
///     struct VariableNode **params;
///     int paramCount;
///     int paramCapacity;
///     char *funcRefName;
/// } ParamNode;
///```
///
ParamNode *createParamNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ParamNode *node = (ParamNode *)ARENA_ALLOC(arena, sizeof(ParamNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ParamNode node.");
        return NULL;
    }

    node->nodeType = NODE_UNKNOWN;
    node->params = NULL;
    node->paramCount = 0;
    node->paramCapacity = PARAM_CAPACITY;
    node->funcRefName = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ArgNode
/// {
///     enum CryoNodeType nodeType;
///     struct VariableNode **args;
///     int argCount;
///     int argCapacity;
///     char *funcRefName;
/// } ArgNode;
///```
///
ArgNode *createArgNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ArgNode *node = (ArgNode *)ARENA_ALLOC(arena, sizeof(ArgNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ArgNode node.");
        return NULL;
    }

    node->nodeType = NODE_UNKNOWN;
    node->args = (ASTNode **)calloc(ARG_CAPACITY, sizeof(ASTNode *));
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;
    node->funcRefName = (char *)calloc(1, sizeof(char));

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoReturnNode
/// {
///     struct ASTNode *returnValue;
///     struct ASTNode *expression;
///     CryoDataType returnType;
/// } CryoReturnNode;
///```
///
CryoReturnNode *createReturnNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoReturnNode *node = (CryoReturnNode *)ARENA_ALLOC(arena, sizeof(CryoReturnNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ReturnNode node.");
        return NULL;
    }

    node->returnValue = (ASTNode *)malloc(sizeof(ASTNode));
    node->expression = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = DTM->primitives->createUndefined();

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoBinaryOpNode
/// {
///     struct ASTNode *left;
///     struct ASTNode *right;
///     CryoOperatorType op;
/// } CryoBinaryOpNode;
///```
///
CryoBinaryOpNode *createBinaryOpNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoBinaryOpNode *node = (CryoBinaryOpNode *)ARENA_ALLOC(arena, sizeof(CryoBinaryOpNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate BinaryOpNode node.");
        return NULL;
    }

    node->left = NULL;
    node->right = NULL;
    node->op = OPERATOR_UNKNOWN;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoUnaryOpNode
/// {
///     CryoTokenType op;
///     struct ASTNode *operand;
/// } CryoUnaryOpNode;
///```
///
CryoUnaryOpNode *createUnaryOpNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoUnaryOpNode *node = (CryoUnaryOpNode *)ARENA_ALLOC(arena, sizeof(CryoUnaryOpNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate UnaryOpNode node.");
        return NULL;
    }

    node->op = TOKEN_UNKNOWN;
    node->operand = NULL;
    node->expression = (ASTNode *)malloc(sizeof(ASTNode));
    node->resultType = DTM->primitives->createUndefined();

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct CryoArrayNode
/// {
///     struct ASTNode **elements;
///     int elementCount;
///     int elementCapacity;
/// } CryoArrayNode;
///```
///
CryoArrayNode *createArrayNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoArrayNode *node = (CryoArrayNode *)ARENA_ALLOC(arena, sizeof(CryoArrayNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ArrayNode node.");
        return NULL;
    }

    const int initialCapacity = ARRAY_CAPACITY;
    node->elements = (ASTNode **)ARENA_ALLOC(arena, initialCapacity * sizeof(ASTNode *));
    if (!node->elements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate elements array.");
        free(node);
        return NULL;
    }

    node->elementCount = 0;
    node->elementCapacity = initialCapacity;
    node->type = DTM->primitives->createUndefined();
    node->elementTypes = (DataType **)calloc(initialCapacity, sizeof(DataType *));

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct IndexExprNode
/// {
///     char *name;
///     struct ASTNode *array;
///     struct ASTNode *index;
/// } IndexExprNode;
///```
///
IndexExprNode *createIndexExprNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    IndexExprNode *node = (IndexExprNode *)ARENA_ALLOC(arena, sizeof(IndexExprNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate IndexExprNode node.");
        return NULL;
    }

    node->name = (char *)malloc(sizeof(char));
    node->array = NULL;
    node->index = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct VariableReassignmentNode
/// {
///     char *existingVarName;
///     struct ASTNode *existingVarNode;
///     CryoDataType existingVarType;
///     struct ASTNode *newVarNode;
/// } VariableReassignmentNode;
///```
///
VariableReassignmentNode *createVariableReassignmentNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    VariableReassignmentNode *node = (VariableReassignmentNode *)ARENA_ALLOC(arena, sizeof(VariableReassignmentNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableReassignmentNode node.");
        return NULL;
    }

    node->existingVarName = (char *)calloc(1, sizeof(char));
    node->existingVarNode = NULL;
    node->existingVarType = DTM->primitives->createUndefined();
    node->newVarNode = NULL;

    logMessage(LMI, "INFO", "Containers", "Created VariableReassignmentNode");

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct StructNode
/// {
///     char *name;
///     ASTNode **properties;
///     ASTNode **methods;
///     ASTNode *constructor;
///     int propertyCount;
///     int propertyCapacity;
///     int methodCount;
///     int methodCapacity;
///     bool hasConstructor;
///     bool hasDefaultValue;
///     DataType *type;
///     bool isStatic;
/// } StructNode;
///```
///
StructNode *createStructNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    StructNode *node = (StructNode *)ARENA_ALLOC(arena, sizeof(StructNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate StructNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));

    node->properties = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    node->methods = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->constructor = NULL;
    node->ctorArgCount = 0;
    node->ctorArgCapacity = ARG_CAPACITY;
    node->ctorArgs = (ASTNode **)calloc(1, sizeof(ASTNode *));

    node->hasConstructor = false;
    node->hasDefaultValue = false;
    node->isStatic = false;
    node->type = DTM->primitives->createUndefined();

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct PropertyNode
/// {
///     char *name;
///     struct ASTNode *value;
///     CryoDataType type;
///     bool defaultProperty;
///     const char *parentName;
///     CryoNodeType parentNodeType;
/// } PropertyNode;
///```
///
PropertyNode *createPropertyNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    PropertyNode *node = (PropertyNode *)ARENA_ALLOC(arena, sizeof(PropertyNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate PropertyNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->value = NULL;
    node->type = DTM->primitives->createUndefined();
    node->defaultProperty = false;
    node->parentName = (char *)calloc(1, sizeof(char));
    node->parentNodeType = NODE_UNKNOWN;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ScopedFunctionCallNode
/// {
///     char *functionName;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
/// } ScopedFunctionCallNode;
///```
///
ScopedFunctionCallNode *createScopedFunctionCallNode(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ScopedFunctionCallNode *node = (ScopedFunctionCallNode *)ARENA_ALLOC(arena, sizeof(ScopedFunctionCallNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ScopedFunctionCallNode node.");
        return NULL;
    }

    node->functionName = (char *)calloc(1, sizeof(char));
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct StructConstructorNode
/// {
///     char *name;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
///     struct ConstructorMetaData *metaData;
/// } StructConstructorNode;
///```
///
StructConstructorNode *createStructConstructorNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    StructConstructorNode *node = (StructConstructorNode *)ARENA_ALLOC(arena, sizeof(StructConstructorNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate StructConstructorNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;
    node->metaData = createConstructorMetaDataContainer(arena, state);
    node->constructorBody = NULL;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ConstructorMetaData
/// {
///     enum CryoNodeType parentNodeType;
///     const char *parentName;
///     bool hasDefaultFlag;
/// } ConstructorMetaData;
///```
///
ConstructorMetaData *createConstructorMetaDataContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ConstructorMetaData *node = (ConstructorMetaData *)ARENA_ALLOC(arena, sizeof(ConstructorMetaData));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ConstructorMetaData node.");
        return NULL;
    }

    node->parentNodeType = NODE_UNKNOWN;
    node->parentName = (char *)calloc(1, sizeof(char));
    node->hasDefaultFlag = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct PropertyAccessNode
/// {
///     struct ASTNode *object;
///     struct ASTNode *object;
///     const char *property;
///     struct ASTNode *property;
///     int propertyIndex;
/// } PropertyAccessNode;
///```
///
PropertyAccessNode *createPropertyAccessNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    PropertyAccessNode *node = (PropertyAccessNode *)ARENA_ALLOC(arena, sizeof(PropertyAccessNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate PropertyAccessNode node.");
        return NULL;
    }

    node->object = NULL;
    node->propertyName = (char *)calloc(1, sizeof(char));
    node->property = NULL;
    node->propertyIndex = 0;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ThisNode
/// {
///     const char *name;
/// } ThisNode;
///```
///
ThisNode *createThisNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ThisNode *node = (ThisNode *)ARENA_ALLOC(arena, sizeof(ThisNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ThisNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct PropertyReassignmentNode
/// {
///     char *name;
///     struct ASTNode *object;
///     struct ASTNode *value;
/// } PropertyReassignmentNode;
///```
///
PropertyReassignmentNode *createPropertyReassignmentNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    PropertyReassignmentNode *node = (PropertyReassignmentNode *)ARENA_ALLOC(arena, sizeof(PropertyReassignmentNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate PropertyReassignmentNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->object = NULL;
    node->value = NULL;
    node->objectType = DTM->primitives->createUndefined();
    node->propertyIndex = 0;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct MethodNode
/// {
///     enum CryoVisibilityType visibility;
///     enum CryoDataType returnType;
///     char *name;
///     struct ASTNode **params;
///     int paramCount;
///     int paramCapacity;
///     struct ASTNode *body;
///     bool isStatic;
/// } MethodNode;
///```
///
MethodNode *createMethodNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    MethodNode *node = (MethodNode *)ARENA_ALLOC(arena, sizeof(MethodNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate MethodNode node.");
        return NULL;
    }

    node->name = NULL;
    node->params = NULL;
    node->paramCount = 0;
    node->paramCapacity = PARAM_CAPACITY;
    node->paramTypes = NULL;
    node->body = NULL;
    node->visibility = VISIBILITY_PUBLIC;
    node->type = DTM->primitives->createUndefined();
    node->functionType = DTM->primitives->createUndefined();
    node->isStatic = false;
    node->parentName = (char *)calloc(1, sizeof(char));

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct MethodCallNode
/// {
///     DataType *instanceType;
///     DataType *returnType;
///     const char *instanceName;
///     ASTNode *accessorObj;
///     const char *name;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
///     bool isStatic;
/// } MethodCallNode;
///```
///
MethodCallNode *createMethodCallNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    MethodCallNode *node = (MethodCallNode *)ARENA_ALLOC(arena, sizeof(MethodCallNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate MethodCallNode node.");
        return NULL;
    }

    node->instanceType = DTM->primitives->createUndefined();
    node->returnType = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));
    node->instanceName = (char *)calloc(1, sizeof(char));
    node->accessorObj = NULL;
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;
    node->isStatic = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct GenericDeclNode
/// {
///     DataType *type;             // e.g., List<T>
///     const char *name;           // e.g., "List"
///     ASTNode **properties;       // e.g., <T>
///     int propertyCount;          // e.g., 1
///     DataType **constraintTypes; // e.g., T extends Number
///     bool hasConstraint;
/// } GenericDeclNode;
///```
///
GenericDeclNode *createGenericDeclNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    GenericDeclNode *node = (GenericDeclNode *)ARENA_ALLOC(arena, sizeof(GenericDeclNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate GenericDeclNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));
    node->properties = NULL;
    node->propertyCount = 0;
    node->constraintTypes = NULL;
    node->hasConstraint = false;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct GenericInstNode
/// {
///     const char *baseName;     // e.g., "List"
///     DataType **typeArguments; // e.g., <int>
///     int argumentCount;
///     DataType *resultType; // The concrete type after substitution
/// } GenericInstNode;
///```
///
GenericInstNode *createGenericInstNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    GenericInstNode *node = (GenericInstNode *)ARENA_ALLOC(arena, sizeof(GenericInstNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate GenericInstNode node.");
        return NULL;
    }

    node->baseName = (char *)calloc(1, sizeof(char));
    node->typeArguments = NULL;
    node->argumentCount = 0;
    node->resultType = DTM->primitives->createUndefined();

    return node;
}

struct PublicMembers *createPublicMembersContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    struct PublicMembers *node = (struct PublicMembers *)malloc(sizeof(struct PublicMembers));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ProtectedMembers node.");
        return NULL;
    }

    node->methods = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->properties = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    return node;
}

struct PrivateMembers *createPrivateMembersContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    struct PrivateMembers *node = (struct PrivateMembers *)malloc(sizeof(struct PrivateMembers));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ProtectedMembers node.");
        return NULL;
    }

    node->methods = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->properties = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    return node;
}

struct ProtectedMembers *createProtectedMembersContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    struct ProtectedMembers *node = (struct ProtectedMembers *)malloc(sizeof(struct ProtectedMembers));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ProtectedMembers node.");
        return NULL;
    }

    node->methods = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->properties = (ASTNode **)calloc(1, sizeof(ASTNode *));
    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    return node;
}

/// ---
/// ### Structure
///```
/// typedef struct ClassNode
/// {
///     DataType *type;
///     const char *name;
///     ASTNode *constructor;
///     int propertyCount;
///     int propertyCapacity;
///     int methodCount;
///     int methodCapacity;
///     bool hasConstructor;
///     bool hasDefaultValue;
///     bool isStatic;
///     // For Private Members
///     PrivateMembers *privateMembers;
///     // For Public Members
///     PublicMembers *publicMembers;
///     // For Protected Members
///     ProtectedMembers *protectedMembers;
/// } ClassNode;
///```
///
ClassNode *createClassNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ClassNode *node = (ClassNode *)ARENA_ALLOC(arena, sizeof(ClassNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ClassNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));

    node->hasConstructor = false;
    node->constructors = (ASTNode **)malloc(sizeof(ASTNode *) * ARG_CAPACITY);
    node->constructorCount = 0;
    node->constructorCapacity = ARG_CAPACITY;

    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->hasDefaultValue = false;
    node->isStatic = false;

    node->privateMembers = createPrivateMembersContainer(arena, state);
    node->publicMembers = createPublicMembersContainer(arena, state);
    node->protectedMembers = createProtectedMembersContainer(arena, state);

    return node;
}

ClassConstructorNode *createClassConstructorNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    StructConstructorNode *node = (StructConstructorNode *)ARENA_ALLOC(arena, sizeof(StructConstructorNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate StructConstructorNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->args = (ASTNode **)calloc(ARG_CAPACITY, sizeof(ASTNode *));
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;
    node->metaData = createConstructorMetaDataContainer(arena, state);
    node->constructorBody = NULL;

    return node;
}

ObjectNode *createObjectNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ObjectNode *node = (ObjectNode *)ARENA_ALLOC(arena, sizeof(ObjectNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ObjectNode node.");
        return NULL;
    }

    node->objType = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));
    node->isNewInstance = false;

    node->args = (ASTNode **)calloc(ARG_CAPACITY, sizeof(ASTNode *));
    node->argCount = 0;
    node->argCapacity = ARG_CAPACITY;

    node->genericTypes = (DataType **)calloc(1, sizeof(DataType *) * GENERIC_CAPACITY);
    node->genericCount = 0;
    node->genericCapacity = GENERIC_CAPACITY;

    return node;
}

NullNode *createNullNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    NullNode *node = (NullNode *)ARENA_ALLOC(arena, sizeof(NullNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate NullNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createNull();

    return node;
}

TypeofNode *createTypeofNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    TypeofNode *node = (TypeofNode *)ARENA_ALLOC(arena, sizeof(TypeofNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate TypeofNode node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->expression = (ASTNode *)calloc(1, sizeof(ASTNode));

    return node;
}

UsingNode *createUsingNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    UsingNode *node = (UsingNode *)ARENA_ALLOC(arena, sizeof(UsingNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate UsingNode node.");
        return NULL;
    }

    node->primaryModule = (const char *)calloc(1, sizeof(char));
    node->secondaryModuleCount = 0;
    node->secondaryModuleCapacity = MAX_USING_MODULES;
    node->secondaryModules = (const char **)calloc(1, sizeof(char *) * MAX_USING_MODULES);

    node->filePath = (char *)calloc(1, sizeof(char));

    return node;
}

ModuleNode *createModuleNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ModuleNode *node = (ModuleNode *)ARENA_ALLOC(arena, sizeof(ModuleNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ModuleNode node.");
        return NULL;
    }

    node->moduleName = (char *)calloc(1, sizeof(char));
    node->moduleDir = (char *)calloc(1, sizeof(char));
    node->modulePath = (char *)calloc(1, sizeof(char));
    node->moduleFile = (char *)calloc(1, sizeof(char));

    return node;
}

AnnotationNode *createAnnotationNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    AnnotationNode *node = (AnnotationNode *)ARENA_ALLOC(arena, sizeof(AnnotationNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate AnnotationNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->value = (char *)calloc(1, sizeof(char));

    return node;
}

TypeDecl *createTypeDeclContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    TypeDecl *node = (TypeDecl *)ARENA_ALLOC(arena, sizeof(TypeDecl));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate TypeDecl node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->name = (char *)calloc(1, sizeof(char));

    return node;
}

TypeCast *createTypeCastContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    TypeCast *node = (TypeCast *)ARENA_ALLOC(arena, sizeof(TypeCast));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate TypeCast node.");
        return NULL;
    }

    node->type = DTM->primitives->createUndefined();
    node->expression = (ASTNode *)calloc(1, sizeof(ASTNode));

    return node;
}

ImplementNode *createImplementationNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    ImplementNode *node = (ImplementNode *)malloc(sizeof(ImplementNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate Implementation node.");
        return NULL;
    }

    node->interfaceName = NULL;
    node->interfaceType = DTM->primitives->createUndefined();

    node->properties = (ASTNode **)malloc(sizeof(ASTNode *) * PROPERTY_CAPACITY);
    node->propertyCount = 0;
    node->propertyCapacity = PROPERTY_CAPACITY;

    node->methods = (ASTNode **)malloc(sizeof(ASTNode *) * METHOD_CAPACITY);
    node->methodCount = 0;
    node->methodCapacity = METHOD_CAPACITY;

    node->constructors = (ASTNode **)malloc(sizeof(ASTNode *) * METHOD_CAPACITY);
    node->constructorCount = 0;
    node->constructorCapacity = METHOD_CAPACITY;

    return node;
}

void *createDiscardNodeContainer(Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    void *node = (void *)malloc(sizeof(void *));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate DiscardNode node.");
        return NULL;
    }

    return node;
}
