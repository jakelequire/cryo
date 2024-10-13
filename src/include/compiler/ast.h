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
#ifndef AST_H
#define AST_H
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
/*---<custom_includes>---*/
#include "compiler/token.h"
#include "compiler/lexer.h"
#include "compiler/symtable.h"
#include "compiler/typedefs.h"
#include "utils/support.h"
#include "utils/arena.h"
#include "common/common.h"
/*---------<end>---------*/

// typedef struct DataType DataType;

// strdup
// extern char *strdup(const char *__s);
// strndup
// extern char *strndup(const char *__string, size_t __n);

/// ### The position struct represents the line and column number of a token in the source code.
typedef struct Position
{
    int line;
    int column;
} Position;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
/// ---
/// ```
///
/// typedef struct CryoModule
/// {
///     ASTNode **astTable;
/// } CryoModule;
///
/// ```
///
typedef struct CryoModule
{
    ASTNode **astTable;
} CryoModule;

/// #### The Namespace for the AST.
/// ---
/// ```
///
/// typedef struct CryoNamespace
/// {
///     char *name;
/// } CryoNamespace;
/// ```
///
typedef struct CryoNamespace
{
    char *name;
    Position position;
} CryoNamespace;

/// #### The CryoProgram struct represents the overall structure of a Cryo program.
/// ---
/// ```
///
/// typedef struct CryoProgram
/// {
///     struct ASTNode **statements;
///     size_t statementCount;
///     size_t statementCapacity;
/// } CryoProgram;
///
/// ```
typedef struct CryoProgram
{
    struct ASTNode **statements;
    size_t statementCount;
    size_t statementCapacity;
} CryoProgram;

/// #### The CryoMetaData struct holds metadata information for AST nodes.
/// ---
/// ```
///
/// typedef struct CryoMetaData
/// {
///     enum CryoNodeType type;      // Node Type
///     int line;                    // Line number for error reporting
///     int column;                  // Column number for error reporting
///     struct ASTNode *firstChild;  // First child node (for linked list structure)
///     struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
///     char *moduleName;            // Current Module
/// } CryoMetaData;
///
/// ```
typedef struct CryoMetaData
{
    enum CryoNodeType type;      // Node Type
    int line;                    // Line number for error reporting
    int column;                  // Column number for error reporting
    struct ASTNode *firstChild;  // First child node (for linked list structure)
    struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
    char *moduleName;            // Current Module
    Position position;
} CryoMetaData;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
/// ---
/// ```
///
/// typedef struct CryoImportNode
/// {
///     char *moduleName;
///     char *subModuleName;
///     bool isStdModule;
/// } CryoImportNode;
///
/// ```
typedef struct CryoImportNode
{
    char *moduleName;
    char *subModuleName;
    bool isStdModule;
} CryoImportNode;

/// #### The CryoScope struct represents the scope of a block of code.
/// ---
/// ```
///
/// typedef struct CryoScope
/// {
///     int scopeLevel;
///     char *scopeName;
///     CryoNodeType scopeType;
/// } CryoScope;
///
/// ```
typedef struct CryoScope
{
    int scopeLevel;
    char *scopeName;
    CryoNodeType scopeType;
} CryoScope;

/// #### The CryoBlockNode struct represents a block of statements in the AST.
/// ---
/// ```
///
/// typedef struct CryoBlockNode
/// {
///     struct ASTNode **statements;
///     int statementCount;
///     int statementCapacity;
/// } CryoBlockNode;
///
/// ```
typedef struct CryoBlockNode
{
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoBlockNode;

/// #### The CryoFunctionBlock struct represents a function block within the AST.
/// ---
/// ```
///
/// typedef struct CryoFunctionBlock
/// {
///     struct ASTNode *function;
///     struct ASTNode **statements;
///     int statementCount;
///     int statementCapacity;
/// } CryoFunctionBlock;
///
/// ```
typedef struct CryoFunctionBlock
{
    struct ASTNode *function;
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoFunctionBlock;

/// #### The ExternNode struct represents an external node in the AST.
/// ---
/// ```
///
/// typedef struct ExternNode
/// {
///     struct ASTNode *externNode;
/// } ExternNode;
///
/// ```
typedef struct ExternNode
{
    struct ASTNode *externNode;
} ExternNode;

/// #### The ExternFunctionNode struct represents an external function in the AST.
/// ---
/// ```
///
/// typedef struct ExternFunctionNode
/// {
///     char *name;
///     struct ASTNode **params;
///     int paramCount;
///     int paramCapacity;
///     CryoDataType returnType;
/// } ExternFunctionNode;
///
/// ```
typedef struct ExternFunctionNode
{
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    CryoDataType returnType;
} ExternFunctionNode;

/// #### The FunctionDeclNode struct represents a function declaration in the AST.
/// ---
/// ```
///
/// typedef struct FunctionDeclNode
/// {
///     CryoNodeType type;
///     CryoVisibilityType visibility;
///     CryoDataType returnType;
///     char *name;
///     struct ASTNode **params;
///     int paramCount;
///     int paramCapacity;
///     struct ASTNode *body;
/// } FunctionDeclNode;
///
/// ```
typedef struct FunctionDeclNode
{
    CryoNodeType type;
    CryoVisibilityType visibility;
    CryoDataType returnType;
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    struct ASTNode *body;
} FunctionDeclNode;

/// #### The FunctionCallNode struct represents a function call in the AST.
/// ---
/// ```
///
/// typedef struct FunctionCallNode
/// {
///     char *name;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
/// } FunctionCallNode;
///
/// ```
typedef struct FunctionCallNode
{
    char *name;
    struct ASTNode **args;
    int argCount;
    int argCapacity;
} FunctionCallNode;

/// #### The LiteralNode struct represents a literal value in the AST.
/// ---
/// ```
///
/// typedef struct LiteralNode
/// {
///     CryoDataType dataType; // Data type of the literal
///     union
///     {
///         int intValue;
///         float floatValue;
///         char *stringValue;
///         int booleanValue;
///     } value;
/// } LiteralNode;
///
/// ```
typedef struct LiteralNode
{
    CryoDataType dataType; // Data type of the literal
    int length;
    union
    {
        int intValue;
        float floatValue;
        char *stringValue;
        int booleanValue;
    } value;
} LiteralNode;

/// #### The IfStatementNode struct represents an if statement in the AST.
/// ---
/// ```
///
/// typedef struct IfStatementNode
/// {
///     struct ASTNode *condition;
///     struct ASTNode *thenBranch;
///     struct ASTNode *elseBranch;
/// } IfStatementNode;
///
/// ```
typedef struct IfStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
} IfStatementNode;

/// #### The ForStatementNode struct represents a for-loop statement in the AST.
/// ---
/// ```
///
/// typedef struct ForStatementNode
/// {
///     struct ASTNode *initializer;
///     struct ASTNode *condition;
///     struct ASTNode *increment;
///     struct ASTNode *body;
/// } ForStatementNode;
///
/// ```
/// ```
/// for(initializer; condition; increment) { ... }
/// ```
typedef struct ForStatementNode
{
    // `${initializer};`
    struct ASTNode *initializer;
    // `${condition};`
    struct ASTNode *condition;
    // `${increment};`
    struct ASTNode *increment;
    CryoOperatorType incrementOp;
    // { body }
    struct ASTNode *body;
} ForStatementNode;

/// #### The WhileStatementNode struct represents a while-loop statement in the AST.
/// ---
/// ```
///
/// typedef struct WhileStatementNode
/// {
///     struct ASTNode *condition;
///     struct ASTNode *body;
/// } WhileStatementNode;
///
/// ```
typedef struct WhileStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *body;
} WhileStatementNode;

/// #### The VariableNameNode struct represents a variable name in the AST.
/// ---
/// ```
///
/// typedef struct VariableNameNode
/// {
///     CryoDataType refType;
///     bool isRef;
///     char *varName;
/// } VariableNameNode;
///
/// ```
typedef struct VariableNameNode
{
    CryoDataType refType;
    bool isRef;
    char *varName;
} VariableNameNode;

/// #### The CryoExpressionNode struct represents an expression in the AST.
/// ---
/// ```
///
/// typedef struct CryoExpressionNode
/// {
///     CryoNodeType nodeType;
///     union
///     {
///         // For Variable References
///         VariableNameNode *varNameNode;
///         LiteralNode *literalNode;
///         // Unsure what else to put here but I'll figure it out later
///     } data;
///
/// } CryoExpressionNode;
///
/// ```
typedef struct CryoExpressionNode
{
    CryoNodeType nodeType;
    union
    {
        // For Variable References
        VariableNameNode *varNameNode;
        LiteralNode *literalNode;
        // Unsure what else to put here but I'll figure it out later
    } data;
} CryoExpressionNode;

/// #### The CryoVariableNode struct represents a variable declaration in the AST.
/// ---
/// ```
///
/// typedef struct CryoVariableNode
/// {
///     CryoDataType type;
///     struct VariableNameNode *varNameNode;
///     char *name;
///     bool isGlobal;
///     bool isLocal;
///     bool isReference;
///     // This is the data attached to the variable
///     struct ASTNode *initializer;
///     // Optional index expression for array handling
///     bool hasIndexExpr;
///     struct IndexExprNode *indexExpr;
/// } CryoVariableNode;
///
/// ```
typedef struct CryoVariableNode
{
    CryoDataType type;
    struct VariableNameNode *varNameNode;
    char *name;
    bool isGlobal;
    bool isLocal;
    bool isReference;
    bool isMutable;
    bool isIterator;
    // This is the data attached to the variable
    struct ASTNode *initializer;
    // Optional index expression for array handling
    bool hasIndexExpr;
    struct IndexExprNode *indexExpr;
} CryoVariableNode;

/// #### The CryoParameterNode struct represents a parameter in the AST.
/// ---
/// ```
///
/// typedef struct CryoParameterNode
/// {
///     CryoDataType type;
///     char *name;
///     bool hasDefaultValue;
///     struct ASTNode *defaultValue;
/// } CryoParameterNode;
///
/// ```
typedef struct CryoParameterNode
{
    CryoDataType type;
    char *name;
    char *functionName;
    bool hasDefaultValue;
    struct ASTNode *defaultValue;
} CryoParameterNode;

/// #### The ParamNode struct represents a parameter list in the AST.
/// ---
/// ```
///
/// typedef struct ParamNode
/// {
///     enum CryoNodeType nodeType;
///     CryoVariableNode **params;
///     int paramCount;
///     int paramCapacity;
///     char *funcRefName;
/// } ParamNode;
///
/// ```
typedef struct ParamNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **params;
    int paramCount;
    int paramCapacity;
    char *funcRefName;
} ParamNode;

/// #### The ArgNode struct represents an argument list in the AST.
/// ---
/// ```
///
/// typedef struct ArgNode
/// {
///     enum CryoNodeType nodeType;
///     CryoVariableNode **args;
///     int argCount;
///     int argCapacity;
///     char *funcRefName;
/// } ArgNode;
///
/// ```
typedef struct ArgNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **args;
    int argCount;
    int argCapacity;
    char *funcRefName;
} ArgNode;

/// #### The CryoReturnNode struct represents a return statement in the AST.
/// ---
/// ```
///
/// typedef struct CryoReturnNode
/// {
///     struct ASTNode *returnValue;
///     struct ASTNode *expression;
///     CryoDataType returnType;
/// } CryoReturnNode;
///
/// ```
typedef struct CryoReturnNode
{
    struct ASTNode *returnValue;
    struct ASTNode *expression;
    CryoDataType returnType;
} CryoReturnNode;

/// #### The CryoBinaryOpNode struct represents a binary operation in the AST.
/// ---
/// ```
///
/// typedef struct CryoBinaryOpNode
/// {
///     struct ASTNode *left;
///     struct ASTNode *right;
///     CryoOperatorType op;
/// } CryoBinaryOpNode;
///
/// ```
typedef struct CryoBinaryOpNode
{
    struct ASTNode *left;
    struct ASTNode *right;
    CryoOperatorType op;
} CryoBinaryOpNode;

/// #### The CryoUnaryOpNode struct represents a unary operation in the AST.
/// ---
/// ```
///
/// typedef struct CryoUnaryOpNode
/// {
///     CryoTokenType op;
///     struct ASTNode *operand;
/// } CryoUnaryOpNode;
///
/// ```
typedef struct CryoUnaryOpNode
{
    CryoTokenType op;
    struct ASTNode *operand;
    struct ASTNode *expression;
} CryoUnaryOpNode;

/// #### The CryoArrayNode struct represents an array in the AST.
/// ---
/// ```
///
/// typedef struct CryoArrayNode
/// {
///     struct ASTNode **elements;
///     int elementCount;
///     int elementCapacity;
/// } CryoArrayNode;
///
/// ```
typedef struct CryoArrayNode
{
    struct ASTNode **elements;
    int elementCount;
    int elementCapacity;
} CryoArrayNode;

/// #### The IndexExprNode struct represents an index expression in the AST.
/// ---
///
/// ```
/// typedef struct IndexExprNode
/// {
///     char *name;
///     struct ASTNode *array;
///     struct ASTNode *index;
/// } IndexExprNode;
///
/// ```
typedef struct IndexExprNode
{
    // Name of the array reference
    char *name;
    // Should be the array node itself
    struct ASTNode *array;
    // Should be a literal expression node with an int value
    struct ASTNode *index;
} IndexExprNode;

/// #### The VariableReassignmentNode struct represents a variable reassignment in the AST.
/// ---
///
/// ```
/// typedef struct VariableReassignmentNode
/// {
///     char *existingVarName;
///     ASTNode *existingVarNode;
///     CryoDataType existingVarType;
///     ASTNode *newVarNode;
/// } VariableReassignmentNode;
///
/// ```
typedef struct VariableReassignmentNode
{
    char *existingVarName;
    ASTNode *existingVarNode;
    CryoDataType existingVarType;
    ASTNode *newVarNode;
} VariableReassignmentNode;

/// #### The PropertyNode struct represents a property in the AST.
/// ---
///
/// ```
/// typedef struct PropertyNode
/// {
///     char *name;
///     ASTNode *value;
///     CryoDataType type;
/// } PropertyNode;
///
typedef struct PropertyNode
{
    char *name;
    ASTNode *value;
    CryoDataType type;
} PropertyNode;

/// #### The StructNode struct represents a struct in the AST.
/// ---
///
/// ```
/// typedef struct StructNode
/// {
///     char *name;
///     PropertyNode **properties;
///     int propertyCount;
///     int propertyCapacity;
/// } StructNode;
///
typedef struct StructNode
{
    char *name;
    ASTNode **properties;
    int propertyCount;
    int propertyCapacity;
} StructNode;

// typedef struct CustomTypeNode
// {
//     DataType type;
//     char *typeString;
//
// } CustomTypeNode;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
/// ---
/// ```
/// typedef struct ScopedFunctionCallNode {
///     char *scopeName;           // e.g., "Math"
///     char *functionName;        // e.g., "add"
///     struct ASTNode **args;     // Array of argument nodes
///     int argCount;
///     int argCapacity;
/// } ScopedFunctionCallNode;
/// ```
typedef struct ScopedFunctionCallNode
{
    char *scopeName;       // e.g., "Math"
    char *functionName;    // e.g., "add"
    struct ASTNode **args; // Array of argument nodes
    int argCount;
    int argCapacity;
} ScopedFunctionCallNode;

typedef struct ASTNode
{
    CryoMetaData *metaData;

    union
    {
        // For Namespaces
        CryoNamespace *cryoNamespace;
        // For the main program
        CryoProgram *program;
        // For Import Statements
        CryoImportNode *import;
        // For Blocks
        CryoBlockNode *block;
        // For Scoped Function Calls
        ScopedFunctionCallNode *scopedFunctionCall;
        // For Functions
        CryoFunctionBlock *functionBlock;
        // For Externs
        ExternNode *externNode;
        // Extern Functions
        ExternFunctionNode *externFunction;
        // For Return Statements
        CryoReturnNode *returnStatement;
        // For Literals
        LiteralNode *literal;
        // For Variable Declarations
        CryoVariableNode *varDecl;
        // For Variable Names
        VariableNameNode *varName;
        // For Expressions
        CryoExpressionNode *expression;
        // For Function Declarations
        FunctionDeclNode *functionDecl;
        // For Function Calls
        FunctionCallNode *functionCall;
        // For If Statements
        IfStatementNode *ifStatement;
        // For For Statements
        ForStatementNode *forStatement;
        // For While Statements
        WhileStatementNode *whileStatement;
        // For Binary Operations
        CryoBinaryOpNode *bin_op;
        // For Unary Operations
        CryoUnaryOpNode *unary_op;
        // For Parameters
        ParamNode *paramList;
        // For Arguments
        ArgNode *argList;
        // Single Parameter Node
        CryoParameterNode *param;
        // For Arrays
        CryoArrayNode *array;
        // For Index Expressions
        IndexExprNode *indexExpr;
        // For Variable Reassignments
        VariableReassignmentNode *varReassignment;
        // For Structs
        StructNode *structNode;
        // For Properties
        PropertyNode *property;
    } data;
} ASTNode;

#define INITIAL_CAPACITY 8

CryoNamespace *createCryoNamespaceNodeContainer(Arena *arena, CompilerState *state);
CryoProgram *createCryoProgramContainer(Arena *arena, CompilerState *state);
CryoBlockNode *createCryoBlockNodeContainer(Arena *arena, CompilerState *state);
CryoFunctionBlock *createCryoFunctionBlockContainer(Arena *arena, CompilerState *state);
CryoModule *createCryoModuleContainer(Arena *arena, CompilerState *state);
CryoMetaData *createMetaDataContainer(Arena *arena, CompilerState *state);
CryoImportNode *createCryoImportNodeContainer(Arena *arena, CompilerState *state);
CryoScope *createCryoScopeContainer(Arena *arena, CompilerState *state);
ExternNode *createExternNodeContainer(CryoNodeType type, Arena *arena, CompilerState *state);
FunctionDeclNode *createFunctionNodeContainer(Arena *arena, CompilerState *state);
FunctionCallNode *createFunctionCallNodeContainer(Arena *arena, CompilerState *state);
LiteralNode *createLiteralNodeContainer(Arena *arena, CompilerState *state);
IfStatementNode *createIfStatementContainer(Arena *arena, CompilerState *state);
ForStatementNode *createForStatementNodeContainer(Arena *arena, CompilerState *state);
WhileStatementNode *createWhileStatementNodeContainer(Arena *arena, CompilerState *state);
CryoExpressionNode *createExpressionNodeContainer(Arena *arena, CompilerState *state);
CryoVariableNode *createVariableNodeContainer(Arena *arena, CompilerState *state);
VariableNameNode *createVariableNameNodeContainer(char *varName, Arena *arena, CompilerState *state);
CryoParameterNode *createParameterNodeContainer(Arena *arena, CompilerState *state);
ParamNode *createParamNodeContainer(Arena *arena, CompilerState *state);
ArgNode *createArgNodeContainer(Arena *arena, CompilerState *state);
CryoReturnNode *createReturnNodeContainer(Arena *arena, CompilerState *state);
CryoBinaryOpNode *createBinaryOpNodeContainer(Arena *arena, CompilerState *state);
CryoUnaryOpNode *createUnaryOpNodeContainer(Arena *arena, CompilerState *state);
CryoArrayNode *createArrayNodeContainer(Arena *arena, CompilerState *state);
IndexExprNode *createIndexExprNodeContainer(Arena *arena, CompilerState *state);
VariableReassignmentNode *createVariableReassignmentNodeContainer(Arena *arena, CompilerState *state);
StructNode *createStructNodeContainer(Arena *arena, CompilerState *state);
PropertyNode *createPropertyNodeContainer(Arena *arena, CompilerState *state);
ScopedFunctionCallNode *createScopedFunctionCallNode(Arena *arena, CompilerState *state);

#ifdef __cplusplus
extern "C"
{
#endif

    /* @Node_Accessors */
    void printAST(ASTNode *node, int indent, Arena *arena);

    /* @Node_Management */
    ASTNode *createASTNode(CryoNodeType type, Arena *arena, CompilerState *state);
    void addChildNode(ASTNode *parent, ASTNode *child, Arena *arena, CompilerState *state);
    void addStatementToBlock(ASTNode *block, ASTNode *statement, Arena *arena, CompilerState *state);
    void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement, Arena *arena, CompilerState *state);
    void addFunctionToProgram(ASTNode *program, ASTNode *function, Arena *arena, CompilerState *state);

    /* @Node_Creation - Expressions & Statements */
    ASTNode *createProgramNode(Arena *arena, CompilerState *state);
    ASTNode *createNamespaceNode(char *name, Arena *arena, CompilerState *state);
    ASTNode *createLiteralIntExpr(int value, Arena *arena, CompilerState *state);
    ASTNode *createExpressionStatement(ASTNode *expression, Arena *arena, CompilerState *state);
    ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op, Arena *arena, CompilerState *state);
    ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand, Arena *arena, CompilerState *state);

    /* @Node_Creation - Literals */
    ASTNode *createIntLiteralNode(int value, Arena *arena, CompilerState *state);
    ASTNode *createFloatLiteralNode(float value, Arena *arena, CompilerState *state);
    ASTNode *createStringLiteralNode(char *value, Arena *arena, CompilerState *state);
    ASTNode *createBooleanLiteralNode(int value, Arena *arena, CompilerState *state);
    ASTNode *createIdentifierNode(char *name, CryoSymbolTable *symTable, Arena *arena, CompilerState *state);

    /* @Node_Blocks - Blocks */
    ASTNode *createBlockNode(Arena *arena, CompilerState *state);
    ASTNode *createFunctionBlock(Arena *arena, CompilerState *state);
    ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state);
    ASTNode *createForBlock(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state);
    ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state);

    /* @Node_Blocks - Literals */
    ASTNode *createBooleanLiteralExpr(int value, Arena *arena, CompilerState *state);
    ASTNode *createStringLiteralExpr(char *str, Arena *arena, CompilerState *state);
    ASTNode *createStringExpr(char *str, Arena *arena, CompilerState *state);

    /* @Node_Creation - Variables */
    ASTNode *createVarDeclarationNode(char *var_name, CryoDataType dataType, ASTNode *initializer, bool isMutable, bool isGlobal, bool isReference, bool isIterator, Arena *arena, CompilerState *state);
    ASTNode *createVariableExpr(char *name, bool isReference, Arena *arena, CompilerState *state);

    /* @Node_Creation - Functions */
    ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, CryoDataType returnType, Arena *arena, CompilerState *state);
    ASTNode *createExternFuncNode(char *function_name, ASTNode **params, CryoDataType returnType, Arena *arena, CompilerState *state);
    ASTNode *createFunctionCallNode(Arena *arena, CompilerState *state);
    ASTNode *createReturnNode(ASTNode *returnValue, CryoDataType returnType, Arena *arena, CompilerState *state);
    ASTNode *createReturnExpression(ASTNode *returnExpression, CryoDataType returnType, Arena *arena, CompilerState *state);

    /* @Node_Creation - Parameters */
    ASTNode *createParamListNode(Arena *arena, CompilerState *state);
    ASTNode *createArgumentListNode(Arena *arena, CompilerState *state);
    ASTNode *createParamNode(char *name, char *functionName, CryoDataType type, Arena *arena, CompilerState *state);
    ASTNode *createArgsNode(char *name, CryoDataType type, bool isLiteral, Arena *arena, CompilerState *state);

    /* @Node_Creation - Modules & Externals */
    ASTNode *createImportNode(char *module, char *subModule, Arena *arena, CompilerState *state);
    ASTNode *createExternNode(ASTNode *externNode, Arena *arena, CompilerState *state);

    /* @Node_Creation - Conditionals */
    ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state);
    ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state);
    ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state);

    /* @Node_Creation - Arrays */
    ASTNode *createArrayLiteralNode(Arena *arena, CompilerState *state);
    ASTNode *createIndexExprNode(char *arrayName, ASTNode *arrayRef, ASTNode *index, Arena *arena, CompilerState *state);

    /* @Node_Creation - Variable Reassignments */
    ASTNode *createVarReassignment(char *varName, ASTNode *existingVarNode, ASTNode *newVarNode, Arena *arena, CompilerState *state);

    /* @Node_Creation - Structs */
    ASTNode *createFieldNode(char *fieldName, CryoDataType type, ASTNode *fieldValue, Arena *arena, CompilerState *state);
    ASTNode *createStructNode(char *structName, ASTNode **properties, int propertyCount, Arena *arena, CompilerState *state);

    /* @Node_Creation - Scoped Calls */
    ASTNode *createScopedFunctionCall(Arena *arena, CompilerState *state, const char *functionName);

#ifdef __cplusplus
}
#endif

#endif // AST_H
