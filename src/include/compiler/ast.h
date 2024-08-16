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
#include <string.h>
#include <malloc.h>
/*---<custom_includes>---*/
#include "compiler/token.h"
#include "compiler/lexer.h"
#include "compiler/symtable.h"
/*---------<end>---------*/

// *NEW* Untested & Unimplemnted
typedef struct CryoModule
{
    ASTNode **astTable;
} CryoModule;

typedef struct CryoProgram
{
    struct ASTNode **statements;
    size_t statementCount;
    size_t statementCapacity;
} CryoProgram;

typedef struct CryoMetaData
{
    enum CryoNodeType type;      // Node Type
    int line;                    // Line number for error reporting
    int column;                  // Column number for error reporting
    struct ASTNode *firstChild;  // First child node (for linked list structure)
    struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
    char *moduleName;            // Current Module
} CryoMetaData;

typedef struct CryoScope
{
    int scopeLevel;
    char *scopeName;
    CryoNodeType scopeType;
} CryoScope;

typedef struct CryoBlockNode
{
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoBlockNode;

typedef struct CryoFunctionBlock
{
    struct ASTNode *function;
    struct ASTNode **statements;
    int statementCount;
    int statementCapacity;
} CryoFunctionBlock;

typedef struct ExternNode
{
    struct ASTNode *externNode;
} ExternNode;

typedef struct ExternFunctionNode
{
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    CryoDataType returnType;
} ExternFunctionNode;

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

typedef struct FunctionCallNode
{
    char *name;
    struct ASTNode **args;
    int argCount;
    int argCapacity;
} FunctionCallNode;

typedef struct LiteralNode
{
    CryoDataType dataType; // Data type of the literal
    union
    {
        int intValue;
        float floatValue;
        char *stringValue;
        int booleanValue;
    };
} LiteralNode;

typedef struct IfStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
} IfStatementNode;

typedef struct ForStatementNode
{
    struct ASTNode *initializer;
    struct ASTNode *condition;
    struct ASTNode *increment;
    struct ASTNode *body;
} ForStatementNode;

typedef struct WhileStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *body;
} WhileStatementNode;

typedef struct VariableNameNode
{
    CryoDataType refType;
    bool isRef;
    char *varName;
} VariableNameNode;

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

typedef struct CryoVariableNode
{
    CryoDataType type;
    struct VariableNameNode *varNameNode;
    char *name;
    bool isGlobal;
    bool isLocal;
    bool isReference;
    // This is the data attached to the variable
    struct ASTNode *initializer;
} CryoVariableNode;

typedef struct ParamNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **params;
    int paramCount;
    int paramCapacity;
    char *funcRefName;
} ParamNode;

typedef struct ArgNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **args;
    int argCount;
    int argCapacity;
    char *funcRefName;
} ArgNode;

typedef struct CryoReturnNode
{
    struct ASTNode *returnValue;
    struct ASTNode *expression;
    CryoDataType returnType;
} CryoReturnNode;

typedef struct CryoBinaryOpNode
{
    struct ASTNode *left;
    struct ASTNode *right;
    CryoOperatorType op;
} CryoBinaryOpNode;

typedef struct CryoUnaryOpNode
{
    CryoTokenType op;
    struct ASTNode *operand;
} CryoUnaryOpNode;

typedef struct CryoArrayNode
{
    struct ASTNode **elements;
    int elementCount;
    int elementCapacity;
} CryoArrayNode;

typedef struct ASTNode
{
    CryoMetaData *metaData;

    union
    {
        // For the main program
        CryoProgram *program;
        // For Blocks
        CryoBlockNode *block;
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
        // For Arrays
        CryoArrayNode *array;
    } data;
} ASTNode;

#define INITIAL_CAPACITY 8

CryoProgram *createCryoProgramContainer();
CryoBlockNode *createCryoBlockNodeContainer();
CryoFunctionBlock *createCryoFunctionBlockContainer();
CryoModule *createCryoModuleContainer();
CryoMetaData *createMetaDataContainer();
CryoScope *createCryoScopeContainer();
ExternNode *createExternNodeContainer(CryoNodeType type);
FunctionDeclNode *createFunctionNodeContainer();
FunctionCallNode *createFunctionCallNodeContainer();
LiteralNode *createLiteralNodeContainer();
IfStatementNode *createIfStatementContainer();
ForStatementNode *createForStatementNodeContainer();
WhileStatementNode *createWhileStatementNodeContainer();
CryoExpressionNode *createExpressionNodeContainer();
CryoVariableNode *createVariableNodeContainer();
VariableNameNode *createVariableNameNodeContainer(char *varName);
ParamNode *createParamNodeContainer();
ArgNode *createArgNodeContainer();
CryoReturnNode *createReturnNodeContainer();
CryoBinaryOpNode *createBinaryOpNodeContainer();
CryoUnaryOpNode *createUnaryOpNodeContainer();
CryoArrayNode *createArrayNodeContainer();

#ifdef __cplusplus
extern "C"
{
#endif

    /* @Node_Accessors */
    void printAST(ASTNode *node, int indent);
    void freeAST(ASTNode *node);

    /* @Node_Management */
    ASTNode *createASTNode(CryoNodeType type);
    void addChildNode(ASTNode *parent, ASTNode *child);
    void addStatementToBlock(ASTNode *block, ASTNode *statement);
    void addStatementToFunctionBlock(ASTNode *functionBlock, ASTNode *statement);
    void addFunctionToProgram(ASTNode *program, ASTNode *function);

    /* @Node_Creation - Expressions & Statements */
    ASTNode *createProgramNode(void);
    ASTNode *createNamespaceNode(char *name);
    ASTNode *createLiteralExpr(int value);
    ASTNode *createExpressionStatement(ASTNode *expression);
    ASTNode *createBinaryExpr(ASTNode *left, ASTNode *right, CryoOperatorType op);
    ASTNode *createUnaryExpr(CryoTokenType op, ASTNode *operand);

    /* @Node_Creation - Literals */
    ASTNode *createIntLiteralNode(int value);
    ASTNode *createFloatLiteralNode(float value);
    ASTNode *createStringLiteralNode(char *value);
    ASTNode *createBooleanLiteralNode(int value);
    ASTNode *createIdentifierNode(char *name);

    /* @Node_Blocks - Blocks */
    ASTNode *createBlockNode(void);
    ASTNode *createFunctionBlock(void);
    ASTNode *createIfBlock(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
    ASTNode *createForBlock(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body);
    ASTNode *createWhileBlock(ASTNode *condition, ASTNode *body);

    /* @Node_Blocks - Literals */
    ASTNode *createBooleanLiteralExpr(int value);
    ASTNode *createStringLiteralExpr(char *str);
    ASTNode *createStringExpr(char *str);

    /* @Node_Creation - Variables */
    ASTNode *createVarDeclarationNode(char *var_name, CryoDataType dataType, ASTNode *initializer, int line, bool isGlobal, bool isReference);
    ASTNode *createVariableExpr(char *name, bool isReference);

    /* @Node_Creation - Functions */
    ASTNode *createFunctionNode(CryoVisibilityType visibility, char *function_name, ASTNode **params, ASTNode *function_body, CryoDataType returnType);
    ASTNode *createExternFuncNode(char *function_name, ASTNode **params, CryoDataType returnType);
    ASTNode *createFunctionCallNode(void);
    ASTNode *createReturnNode(ASTNode *returnValue);
    ASTNode *createReturnExpression(ASTNode *returnExpression, CryoDataType returnType);

    /* @Node_Creation - Parameters */
    ASTNode *createParamListNode(void);
    ASTNode *createArgumentListNode(void);
    ASTNode *createParamNode(char *name, CryoDataType type);
    ASTNode *createArgsNode(char *name, CryoDataType type, bool isLiteral);

    /* @Node_Creation - Modules & Externals */
    ASTNode *createImportNode(char *importPath);
    ASTNode *createExternNode(ASTNode *externNode);

    /* @Node_Creation - Conditionals */
    ASTNode *createIfStatement(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
    ASTNode *createForStatement(ASTNode *initializer, ASTNode *condition, ASTNode *increment, ASTNode *body);
    ASTNode *createWhileStatement(ASTNode *condition, ASTNode *body);

    /* @Node_Creation - Arrays */
    ASTNode *createArrayLiteralNode(void);

#ifdef __cplusplus
}
#endif

#endif // AST_H
