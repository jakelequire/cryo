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

// *NEW* Untested & Unimplemnted
typedef struct CryoMetaData
{
    enum CryoNodeType type;      // Node Type
    int line;                    // Line number for error reporting
    int column;                  // Column number for error reporting
    struct ASTNode *firstChild;  // First child node (for linked list structure)
    struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
    char *moduleName;            // Current Module
} CryoMetaData;

// *NEW* Untested & Unimplemnted
typedef struct CryoScope
{
    int scopeLevel;
    char *scopeName;
    CryoNodeType scopeType;
} CryoScope;

typedef struct FunctionDeclNode
{
    enum CryoNodeType type;
    enum CryoVisibilityType visibility;
    enum CryoDataType returnType;
    char *name;
    struct ASTNode **params;
    int paramCount;
    int paramCapacity;
    struct ASTNode *body;
} FunctionDeclNode;

// *NEW* Untested & Unimplemnted
typedef struct LiteralNode
{
    enum CryoDataType dataType; // Data type of the literal
    union
    {
        int intValue;
        float floatValue;
        char *stringValue;
        int booleanValue;
    };
} LiteralNode;

// *NEW* Untested & Unimplemnted
typedef struct IfStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
} IfStatementNode;

// *NEW* Untested & Unimplemnted
typedef struct ForStatementNode
{
    struct ASTNode *initializer;
    struct ASTNode *condition;
    struct ASTNode *increment;
    struct ASTNode *body;
} ForStatementNode;

// *NEW* Untested & Unimplemnted
typedef struct WhileStatementNode
{
    struct ASTNode *condition;
    struct ASTNode *body;
} WhileStatementNode;

// *NEW* Untested & Unimplemnted
typedef struct VariableNameNode
{
    enum CryoDataType refType;
    bool isRef;
    char *varName;
} VariableNameNode;

// *NEW* Untested & Unimplemnted
typedef struct CryoExpressionNode
{
    CryoNodeType nodeType;
    union
    {
        // For Variable References
        VariableNameNode *varNameNode;
        LiteralNode *literalNode;
        // Unsure what else to put here but I'll figure it out later
    };

} CryoExpressionNode;

// *NEW* Untested & Unimplemnted
typedef struct CryoVariableNode
{
    enum CryoDataType type;
    struct VarNameNode *varNameNode;
    char *name;
    bool isGlobal;
    bool isLocal;
    bool isReference;
    // This is the data attached to the variable
    union
    {
        LiteralNode *literalNode;
        CryoExpressionNode *expressionNode;
    };
} CryoVariableNode;

// *NEW* Untested & Unimplemnted
typedef struct ParamNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **params;
    int paramCount;
    int paramCapacity;
    char *funcRefName;
} ParamNode;

// *NEW* Untested & Unimplemnted
typedef struct ArgNode
{
    enum CryoNodeType nodeType;
    CryoVariableNode **args;
    int argCount;
    int argCapacity;
    char *funcRefName;
} ArgNode;

// *NEW* Untested & Unimplemnted
typedef struct CryoReturnNode
{
    struct ASTNode *returnValue;
    struct ASTNode *expression;
    CryoDataType returnType;
} CryoReturnNode;

// *NEW* Untested & Unimplemnted
typedef struct CryoArrayNode
{
    struct ASTNode **elements;
    int elementCount;
    int elementCapacity;
} CryoArrayNode;

typedef struct ASTNode
{
    enum CryoNodeType type;
    int line;                    // Line number for error reporting
    struct ASTNode *firstChild;  // First child node (for linked list structure)
    struct ASTNode *nextSibling; // Next sibling node (for linked list structure)
    // *NEW* Untested & Unimplemnted
    char *moduleName;
    // *NEW* Untested & Unimplemnted
    int column;

    union
    {
        int value; // For literal number nodes

        struct
        {
            char *modulePath;
        } importStatementNode;

        struct externNode
        {
            CryoNodeType type;
            union decl
            {
                FunctionDeclNode *function;
                // Will add more types here
            } decl;
        } externNode;

        struct
        {
            struct ASTNode **statements;
            int stmtCount;
            int stmtCapacity;
        } program;

        struct
        {
            struct ASTNode **statements;
            size_t stmtCount;
            size_t stmtCapacity;
        } block;

        // >=------<Function Declaration>------=<
        struct
        {
            CryoNodeType type;
            FunctionDeclNode *function;
        } functionDecl;

        struct
        {
            char *name;
            struct ASTNode **args;
            int argCount;
            int argCapacity;
        } functionCall;

        struct
        {
            struct ASTNode *function;
            struct ASTNode *block;
        } functionBlock;

        struct
        {
            struct ASTNode **params;
            int paramCount;
            int paramCapacity;
        } paramList;

        struct
        {
            struct ASTNode **args;
            int argCount;
            int argCapacity;
        } argList;

        struct
        {
            struct ASTNode *returnValue;
            struct ASTNode *expression;
            CryoDataType returnType;
        } returnStmt;
        // >=--------------<End>--------------=<

        // >=------<Variable Declaration>------=<
        struct varDecl
        {
            CryoDataType dataType; // Data type of the variable
            char *name;
            struct ASTNode *initializer;
            bool isGlobal;
            bool isReference;
            int scopeLevel;
        } varDecl;

        struct varName
        {
            char *varName;
            bool isReference;
        } varName;
        // >=--------------<End>--------------=<

        // >=------<Control Flow>------=<
        struct
        {
            struct ASTNode *condition;
            struct ASTNode *thenBranch;
            struct ASTNode *elseBranch;
        } ifStmt;

        struct
        {
            struct ASTNode *condition;
            struct ASTNode *body;
        } whileStmt;

        struct
        {
            struct ASTNode *initializer;
            struct ASTNode *condition;
            struct ASTNode *increment;
            struct ASTNode *body;
        } forStmt;
        // >=--------------<End>--------------=<

        struct
        {
            struct ASTNode *stmt;
        } stmt;

        struct
        {
            CryoDataType dataType;
            struct ASTNode *expression;
        } expr;

        struct
        {
            CryoDataType dataType; // Data type of the literal
            union
            {
                int intValue;
                float floatValue;
                char *stringValue;
                int booleanValue;
            };
        } literalExpression;

        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
            CryoOperatorType op;
            char *operatorText;
        } bin_op;

        struct
        {
            CryoTokenType op;
            struct ASTNode *operand;
        } unary_op; // For unary operators, e.g (-5, !true, etc.)

        struct
        {
            char *str;
        } str;

        struct
        {
            CryoDataType dataType;
            int value;
        } boolean;

        struct
        {
            CryoArrayNode *array;
        } arrayLiteral; // Add this for array literals

    } data;
} ASTNode;

#define INITIAL_CAPACITY 8

static inline CryoModule *createCryoModuleContainer();
static inline CryoMetaData *createMetaDataContainer();
static inline CryoScope *createCryoScopeContainer();
static inline FunctionDeclNode *createFunctionNodeContainer();
static inline LiteralNode *createLiteralNodeContainer();
static inline IfStatementNode *createIfStatementContainer();
static inline ForStatementNode *createForStatementNodeContainer();
static inline WhileStatementNode *createWhileStatementNodeContainer();
static inline CryoExpressionNode *createExpressionNodeContainer();
static inline CryoVariableNode *createVariableNodeContainer();
static inline VariableNameNode *createVariableNameNodeContainer();
static inline ParamNode *createParamNodeContainer();
static inline ArgNode *createArgNodeContainer();
static inline CryoReturnNode *createReturnNodeContainer();
static inline CryoArrayNode *createArrayNodeContainer();

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
    void addFunctionToProgram(ASTNode *program, ASTNode *function);

    /* @Node_Creation - Expressions & Statements */
    ASTNode *createProgramNode(void);
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
    ASTNode *createFunctionBlock(ASTNode *function);
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
    ASTNode *createExternFuncNode(void);
    ASTNode *createFunctionCallNode(void);
    ASTNode *createReturnNode(ASTNode *returnValue);
    ASTNode *createReturnExpression(ASTNode *returnExpression, CryoDataType returnType);

    /* @Node_Creation - Parameters */
    ASTNode *createParamListNode(void);
    ASTNode *createArgumentListNode(void);
    ASTNode *createParamNode(char *name, CryoDataType type);
    ASTNode *createArgsNode(char *name, CryoDataType type);

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
