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

/// ---
/// ### Structure
///```
/// typedef struct CryoNamespace
/// {
///     char *name;
/// } CryoNamespace;
///```
CryoNamespace *createCryoNamespaceNodeContainer(Arena *arena)
{
    CryoNamespace *node = (CryoNamespace *)ARENA_ALLOC(arena, sizeof(CryoNamespace));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoNamespace node.");
        return NULL;
    }

    node->name = "defaulted";

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
CryoProgram *createCryoProgramContainer(Arena *arena)
{
    CryoProgram *node = (CryoProgram *)ARENA_ALLOC(arena, sizeof(CryoProgram));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoProgram node.");
        return NULL;
    }

    const int initialCapacity = 128;
    node->statements = (ASTNode **)calloc(initialCapacity, sizeof(ASTNode *));
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
/// typedef struct CryoBlockNode
/// {
///     ASTNode **statements;
///     int statementCount;
///     int statementCapacity;
/// } CryoBlockNode;
///```
///
CryoBlockNode *createCryoBlockNodeContainer(Arena *arena)
{
    CryoBlockNode *node = (CryoBlockNode *)ARENA_ALLOC(arena, sizeof(CryoBlockNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoBlockNode node.");
        return NULL;
    }

    int initialCapacity = 128; // Or any other small, non-zero value
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
CryoFunctionBlock *createCryoFunctionBlockContainer(Arena *arena)
{
    CryoFunctionBlock *block = (CryoFunctionBlock *)ARENA_ALLOC(arena, sizeof(CryoFunctionBlock));
    if (block)
    {
        block->function = NULL;
        block->statements = NULL;
        block->statementCount = 0;
        block->statementCapacity = 128;
    }
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
CryoModule *createCryoModuleContainer(Arena *arena)
{
    CryoModule *node = (CryoModule *)ARENA_ALLOC(arena, sizeof(CryoModule));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate createCryoModuleContainer node.");
        return NULL;
    }

    node->astTable = (ASTNode **)calloc(1, sizeof(ASTNode *));

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
CryoMetaData *createMetaDataContainer(Arena *arena)
{
    CryoMetaData *node = (CryoMetaData *)ARENA_ALLOC(arena, sizeof(CryoMetaData));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate CryoMetaData node.");
        return NULL;
    }

    node->type = NODE_UNKNOWN;
    node->line = 0;
    node->column = 0;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    node->moduleName = "defaulted";

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
CryoScope *createCryoScopeContainer(Arena *arena)
{
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
ExternNode *createExternNodeContainer(CryoNodeType type, Arena *arena)
{
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
        node->externNode = NULL;
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
ExternFunctionNode *createExternFunctionNodeContainer(Arena *arena)
{
    ExternFunctionNode *node = (ExternFunctionNode *)ARENA_ALLOC(arena, sizeof(ExternFunctionNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ExternFunctionNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->params = NULL;
    node->paramCount = 0;
    node->paramCapacity = 128;
    node->returnType = DATA_TYPE_UNKNOWN;

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
FunctionDeclNode *createFunctionNodeContainer(Arena *arena)
{
    FunctionDeclNode *node = (FunctionDeclNode *)ARENA_ALLOC(arena, sizeof(FunctionDeclNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate FunctionDeclNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->params = NULL;
    node->body = NULL;
    node->returnType = DATA_TYPE_VOID;
    node->visibility = VISIBILITY_PUBLIC;

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
/// } FunctionCallNode;
///```
///
FunctionCallNode *createFunctionCallNodeContainer(Arena *arena)
{
    FunctionCallNode *node = (FunctionCallNode *)ARENA_ALLOC(arena, sizeof(FunctionCallNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate FunctionCallNode node.");
        return NULL;
    }

    node->name = (char *)calloc(1, sizeof(char));
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = 128;

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
LiteralNode *createLiteralNodeContainer(Arena *arena)
{
    LiteralNode *node = (LiteralNode *)ARENA_ALLOC(arena, sizeof(LiteralNode) * 2);
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate LiteralNode node.");
        return NULL;
    }

    node->dataType = DATA_TYPE_UNKNOWN;
    node->value.intValue = 0;
    node->value.floatValue = 0;
    node->value.stringValue = (char *)ARENA_ALLOC(arena, sizeof(char));
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
IfStatementNode *createIfStatementContainer(Arena *arena)
{
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
ForStatementNode *createForStatementNodeContainer(Arena *arena)
{
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
WhileStatementNode *createWhileStatementNodeContainer(Arena *arena)
{
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
CryoExpressionNode *createExpressionNodeContainer(Arena *arena)
{
    CryoExpressionNode *node = (CryoExpressionNode *)ARENA_ALLOC(arena, sizeof(CryoExpressionNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ExpressionNode node.");
        return NULL;
    }

    node->nodeType = NODE_UNKNOWN;
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
CryoVariableNode *createVariableNodeContainer(Arena *arena)
{
    CryoVariableNode *node = (CryoVariableNode *)ARENA_ALLOC(arena, sizeof(CryoVariableNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableNode node.");
        return NULL;
    }

    node->type = DATA_TYPE_UNKNOWN;
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
VariableNameNode *createVariableNameNodeContainer(char *varName, Arena *arena)
{
    VariableNameNode *node = (VariableNameNode *)ARENA_ALLOC(arena, sizeof(VariableNameNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableNameNode node.");
        return NULL;
    }

    node->refType = DATA_TYPE_UNKNOWN;
    node->isRef = false;
    node->varName = strdup(varName);

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
///     struct ASTNode *defaultValue;
/// } CryoParameterNode;
///```
///
CryoParameterNode *createParameterNodeContainer(Arena *arena)
{
    CryoParameterNode *node = (CryoParameterNode *)ARENA_ALLOC(arena, sizeof(CryoParameterNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ParameterNode node.");
        return NULL;
    }

    node->type = DATA_TYPE_UNKNOWN;
    node->name = (char *)calloc(1, sizeof(char));
    node->functionName = (char *)calloc(1, sizeof(char));
    node->hasDefaultValue = false;
    node->defaultValue = NULL;

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
ParamNode *createParamNodeContainer(Arena *arena)
{
    ParamNode *node = (ParamNode *)ARENA_ALLOC(arena, sizeof(ParamNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ParamNode node.");
        return NULL;
    }

    node->nodeType = NODE_UNKNOWN;
    node->params = NULL;
    node->paramCount = 0;
    node->paramCapacity = 128;
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
ArgNode *createArgNodeContainer(Arena *arena)
{
    ArgNode *node = (ArgNode *)ARENA_ALLOC(arena, sizeof(ArgNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ArgNode node.");
        return NULL;
    }

    node->nodeType = NODE_UNKNOWN;
    node->args = NULL;
    node->argCount = 0;
    node->argCapacity = 128;
    node->funcRefName = NULL;

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
CryoReturnNode *createReturnNodeContainer(Arena *arena)
{
    CryoReturnNode *node = (CryoReturnNode *)ARENA_ALLOC(arena, sizeof(CryoReturnNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ReturnNode node.");
        return NULL;
    }

    node->returnValue = NULL;
    node->expression = NULL;
    node->returnType = DATA_TYPE_UNKNOWN;

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
CryoBinaryOpNode *createBinaryOpNodeContainer(Arena *arena)
{
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
CryoUnaryOpNode *createUnaryOpNodeContainer(Arena *arena)
{
    CryoUnaryOpNode *node = (CryoUnaryOpNode *)ARENA_ALLOC(arena, sizeof(CryoUnaryOpNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate UnaryOpNode node.");
        return NULL;
    }

    node->op = TOKEN_UNKNOWN;
    node->operand = NULL;
    node->expression = NULL;

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
CryoArrayNode *createArrayNodeContainer(Arena *arena)
{
    CryoArrayNode *node = (CryoArrayNode *)ARENA_ALLOC(arena, sizeof(CryoArrayNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate ArrayNode node.");
        return NULL;
    }

    const int initialCapacity = 64; // Or any other small, non-zero value
    node->elements = (ASTNode **)ARENA_ALLOC(arena, initialCapacity * sizeof(ASTNode *));
    if (!node->elements)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate elements array.");
        free(node);
        return NULL;
    }

    node->elementCount = 0;
    node->elementCapacity = initialCapacity;

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
IndexExprNode *createIndexExprNodeContainer(Arena *arena)
{
    IndexExprNode *node = (IndexExprNode *)ARENA_ALLOC(arena, sizeof(IndexExprNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate IndexExprNode node.");
        return NULL;
    }

    node->name = ARENA_ALLOC(arena, sizeof(char));
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
VariableReassignmentNode *createVariableReassignmentNodeContainer(Arena *arena)
{
    VariableReassignmentNode *node = (VariableReassignmentNode *)ARENA_ALLOC(arena, sizeof(VariableReassignmentNode));
    if (!node)
    {
        fprintf(stderr, "[AST] Error: Failed to allocate VariableReassignmentNode node.");
        return NULL;
    }

    node->existingVarName = (char *)calloc(1, sizeof(char));
    node->existingVarNode = NULL;
    node->existingVarType = DATA_TYPE_UNKNOWN;
    node->newVarNode = NULL;

    logMessage("INFO", __LINE__, "Containers", "Created VariableReassignmentNode");

    return node;
}
