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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include "frontend/tokens.h"
#include "frontend/lexer.h"
#include "frontend/symTable.h"
#include "tools/arena/arena.h"
#include "tools/utils/c_logger.h"
#include "tools/macros/consoleColors.h"
#include "frontend/typeTable.h"
#include "common/common.h"

typedef struct DataType DataType;

#define INITIAL_CAPACITY 8

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
///     DataType *type;
/// } ExternFunctionNode;
///
/// ```
typedef struct ExternFunctionNode
{
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    DataType *type;
} ExternFunctionNode;

/// #### The FunctionDeclNode struct represents a function declaration in the AST.
/// ---
/// ```
///
/// typedef struct FunctionDeclNode
/// {
///     CryoNodeType type;
///     CryoVisibilityType visibility;
///     DataType *type;
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
    CryoNodeType nodeType;
    CryoVisibilityType visibility;
    DataType *type;

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
///     DataType *type;
///     int length;
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
    DataType *type;
    int length;
    union
    {
        int intValue;
        float floatValue;
        const char *stringValue;
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
///     DataType *type;
///     bool isRef;
///     char *varName;
/// } VariableNameNode;
///
/// ```
typedef struct VariableNameNode
{
    DataType *type;
    bool isRef; // Remove Later
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
    DataType *type;
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
///     DataType *type;
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
    DataType *type;
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
///     char *name;
///     bool hasDefaultValue;
///     bool isMutable;
///     struct ASTNode *defaultValue;
/// } CryoParameterNode;
///
/// ```
typedef struct CryoParameterNode
{
    DataType *type;
    char *name;
    char *functionName;
    bool hasDefaultValue;
    bool isMutable;
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
///     DataType *type;
/// } CryoReturnNode;
///
/// ```
typedef struct CryoReturnNode
{
    struct ASTNode *returnValue;
    struct ASTNode *expression;
    DataType *type;
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
///     ASTNode *newVarNode;
/// } VariableReassignmentNode;
///
/// ```
typedef struct VariableReassignmentNode
{
    char *existingVarName;
    ASTNode *existingVarNode;
    DataType *existingVarType;
    DataType *type;
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
///     DataType *type;
///     bool defaultProperty;
/// } PropertyNode;
///
typedef struct PropertyNode
{
    char *name;
    ASTNode *value;
    DataType *type;
    bool defaultProperty;
} PropertyNode;

typedef struct MethodNode
{
    DataType *type;
    char *name;
    ASTNode **params;
    int paramCount;
    int paramCapacity;
    ASTNode *body;
} MethodNode;

/// #### The StructNode struct represents a struct in the AST.
/// ---
///
/// ```
/// typedef struct StructNode
/// {
///     char *name;
///     PropertyNode **properties;
///     ASTNode **methods;
///     ASTNode *constructor;
///     int propertyCount;
///     int propertyCapacity;
/// } StructNode;
///
typedef struct StructNode
{
    char *name;
    ASTNode **properties;
    ASTNode **methods;
    ASTNode *constructor;
    int propertyCount;
    int propertyCapacity;
    int methodCount;
    int methodCapacity;
    bool hasConstructor;
    bool hasDefaultValue;
    DataType *type;
} StructNode;

// typedef struct CustomTypeNode
// {
//     DataType *type;
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

typedef struct ConstructorMetaData
{
    CryoNodeType parentNodeType;
    const char *parentName;
    bool hasDefaultFlag;
} ConstructorMetaData;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
/// ---
/// ```
/// typedef struct StructConstructorNode
/// {
///     char *name;
///     struct ASTNode **args;
///     int argCount;
///     int argCapacity;
///     ConstructorMetaData *metaData;
///     ASTNode *constructorBody;
/// } StructConstructorNode;
/// ```
typedef struct StructConstructorNode
{
    char *name;
    struct ASTNode **args;
    int argCount;
    int argCapacity;
    ConstructorMetaData *metaData;
    ASTNode *constructorBody;
} StructConstructorNode;

typedef struct ThisNode
{
    const char *name;
    ASTNode *object;
} ThisNode;

typedef struct PropertyAccessNode
{
    DataType *objType;
    ASTNode *object;
    const char *property;
} PropertyAccessNode;

typedef struct PropertyReassignmentNode
{
    const char *name;
    ASTNode *object;
    ASTNode *value;
} PropertyReassignmentNode;

/// #### The ASTNode struct is the primary data structure for the Abstract Syntax Tree.
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
        // For Struct Constructors
        StructConstructorNode *structConstructor;
        // For Property Access
        PropertyAccessNode *propertyAccess;
        // For This Keyword
        ThisNode *thisNode;
        // For Property Reassignment
        PropertyReassignmentNode *propertyReassignment;
        // For Methods
        MethodNode *method;
    } data;
} ASTNode;

// # ============================================================ # //
// # AST Creation (./src/frontend/AST/AST.c)                      # //
// # ============================================================ # //
#ifdef __cplusplus
extern "C"
{
#endif
    /* @Node_Accessors */
    void printAST(ASTNode *node, int indent, Arena *arena);
    ASTNode *copyASTNode(ASTNode *node);

    /* @Node_Management */
    ASTNode *createASTNode(CryoNodeType type, Arena *arena, CompilerState *state, TypeTable *typeTable);
    void addChildNode(ASTNode *parent, ASTNode *child, Arena *arena, CompilerState *state, TypeTable *typeTable);
    void addStatementToBlock(ASTNode *block, ASTNode *statement, Arena *arena, CompilerState *state);
    void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement, Arena *arena, CompilerState *state);
    void addFunctionToProgram(ASTNode *program, ASTNode *function, Arena *arena, CompilerState *state);

    /* @Node_Creation - Expressions & Statements */
    ASTNode *createProgramNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createNamespaceNode(char *name, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createLiteralIntExpr(int value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createExpressionStatement(ASTNode *expression, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Literals */
    ASTNode *createIntLiteralNode(int value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createFloatLiteralNode(float value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createStringLiteralNode(const char *value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    char *handleStringFormatting(char *value);
    ASTNode *createBooleanLiteralNode(int value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createIdentifierNode(char *name, CryoSymbolTable *symTable, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Blocks - Blocks */
    ASTNode *createBlockNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createFunctionBlock(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createForBlock(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Blocks - Literals */
    ASTNode *createBooleanLiteralExpr(int value, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createStringLiteralExpr(char *str, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createStringExpr(char *str, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Variables */
    ASTNode *createVarDeclarationNode(char *var_name, DataType *dataType, ASTNode *initializer, bool isMutable, bool isGlobal, bool isReference, bool isIterator, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createVariableExpr(char *name, bool isReference, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Functions */
    ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, DataType *returnType, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createExternFuncNode(char *function_name, ASTNode **params, DataType *returnType, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createFunctionCallNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createReturnNode(ASTNode *returnValue, DataType *returnType, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createReturnExpression(ASTNode *returnExpression, DataType *returnType, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Parameters */
    ASTNode *createParamListNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createArgumentListNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createParamNode(char *name, char *functionName, DataType *type, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createArgsNode(char *name, DataType *type, CryoNodeType nodeType, bool isLiteral, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Modules & Externals */
    ASTNode *createImportNode(char *module, char *subModule, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createExternNode(ASTNode *externNode, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Conditionals */
    ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Arrays */
    ASTNode *createArrayLiteralNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createIndexExprNode(char *arrayName, ASTNode *arrayRef, ASTNode *index, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Variable Reassignments */
    ASTNode *createVarReassignment(char *varName, ASTNode *existingVarNode, ASTNode *newVarNode, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Structs */
    ASTNode *createFieldNode(char *fieldName, DataType *type, ASTNode *fieldValue, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createStructNode(char *structName, ASTNode **properties, int propertyCount, ASTNode *constructor, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createConstructorNode(char *structName, ASTNode *body, ASTNode **fields, int argCount, Arena *arena, CompilerState *state, TypeTable *typeTable);

    /* @Node_Creation - Scoped Calls */
    ASTNode *createScopedFunctionCall(Arena *arena, CompilerState *state, const char *functionName, TypeTable *typeTable);

    /* @Node_Creation - This Keyword */
    ASTNode *createThisNode(Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createPropertyAccessNode(ASTNode *object, const char *property, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createPropertyReassignmentNode(ASTNode *object, const char *property, ASTNode *newValue, Arena *arena, CompilerState *state, TypeTable *typeTable);
    ASTNode *createStructPropertyAccessNode(ASTNode *object, const char *property, Arena *arena, CompilerState *state, TypeTable *typeTable);

#ifdef __cplusplus
}
#endif

// # ============================================================ #
// # Node Containers (./src/frontend/AST/ASTContainers.c)         #
// # ============================================================ #
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
StructConstructorNode *createStructConstructorNodeContainer(Arena *arena, CompilerState *state);
ConstructorMetaData *createConstructorMetaDataContainer(Arena *arena, CompilerState *state);
PropertyAccessNode *createPropertyAccessNodeContainer(Arena *arena, CompilerState *state);
ThisNode *createThisNodeContainer(Arena *arena, CompilerState *state);
PropertyReassignmentNode *createPropertyReassignmentNodeContainer(Arena *arena, CompilerState *state);

// # ============================================================ #
// # AST Debug Output (./src/frontend/AST/debugOutputAST.c)       #
// # ============================================================ #

typedef struct ASTDebugNode
{
    const char *nodeType;
    const char *nodeName;
    DataType *dataType;
    int line;
    int column;
    struct ASTDebugNode *children;
    int childCount;
    int indent;
    const char *namespaceName;
    ASTNode *sourceNode;
    ASTNode *value;
    ASTNode **args;
    int argCount;
} ASTDebugNode;

typedef struct DebugASTOutput
{
    const char *fileName;
    const char *short_fileName;
    const char *filePath;
    const char *fileExt;
    const char *cwd;
    ASTDebugNode *nodes;
    int nodeCount;
} DebugASTOutput;

int initASTDebugOutput(ASTNode *root, CompilerSettings *settings);
void initASTConsoleOutput(ASTNode *root, const char *filePath);
void logASTNodeDebugView(ASTNode *node);

DebugASTOutput *createDebugASTOutput(const char *fileName, const char *filePath, const char *fileExt, const char *cwd);
ASTDebugNode *createASTDebugNode(const char *nodeType, const char *nodeName, DataType *dataType, int line, int column, int indent, ASTNode *sourceNode);
void createASTDebugView(ASTNode *node, DebugASTOutput *output, int indentLevel);
DebugASTOutput *addDebugNodesToOutput(ASTDebugNode *node, DebugASTOutput *output);
void createASTDebugOutputFile(DebugASTOutput *output);
void removePrevASTOutput(const char *filePath);
char *seekNamespaceName(ASTNode *node);
bool propHasDefault(PropertyNode *prop);

// Output File Buffer
char *getASTBuffer(DebugASTOutput *output, bool console);
char *logASTBuffer(DebugASTOutput *output, bool console);

char *ASTNodeValueBuffer(ASTNode *node);

// Formatting Functions
char *formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel, bool console);
char *formatProgramNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatParamListNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatBlockNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatParamNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatStructNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output);
char *formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output);
char *formatThisNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);

char *CONSOLE_formatASTNode(ASTDebugNode *node, DebugASTOutput *output, int indentLevel);
char *CONSOLE_formatProgramNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatParamListNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatBlockNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatVarDeclNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatExpressionNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatLiteralExprNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatReturnStatementNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionCallNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatParamNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatPropertyNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatVarNameNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatStructNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatExternFunctionNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatFunctionBlock(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatStructConstructor(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatThisNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatThisAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);
char *CONSOLE_formatPropertyAssignmentNode(ASTDebugNode *node, DebugASTOutput *output);

#endif // AST_H
