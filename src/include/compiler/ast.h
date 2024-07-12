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
/*---<custom_includes>---*/
#include "compiler/token.h"
#include "compiler/lexer.h"
#include "compiler/symtable.h"
/*---------<end>---------*/


typedef struct {
    CryoVisibilityType visibility;
    CryoDataType returnType;
    char* name;
    struct ASTNode** params;
    int paramCount;
    struct ASTNode* body;
} FunctionDeclNode;


typedef struct ASTNode {
    enum CryoNodeType type;
    int line;  // Line number for error reporting
    struct ASTNode* firstChild;  // First child node (for linked list structure)
    struct ASTNode* nextSibling; // Next sibling node (for linked list structure)

    union {
        int value;  // For literal number nodes

        struct {
            char* modulePath;
        } importStatementNode;

        struct externNode {
            CryoNodeType type;
            union decl {
                FunctionDeclNode* function;
                // Will add more types here
            } decl;
        } externNode;

        struct {
            struct ASTNode** statements;
            int stmtCount;
            int stmtCapacity;
        } program;

        struct {
            struct ASTNode** statements;
            size_t stmtCount;
            size_t stmtCapacity;
        } block;
        
        // >=------<Function Declaration>------=<
        struct {
            CryoNodeType type;
            FunctionDeclNode* function;
        } functionDecl;

        struct {
            char* name;
            struct ASTNode** args;
            int argCount;
        } functionCall;

        struct {
            struct ASTNode* function;
            struct ASTNode* block;
        } functionBlock;

        struct {
            struct ASTNode** params;
            int paramCount;
            int paramCapacity;
        } paramList;

        struct {
            struct ASTNode** args;
            int argCount;
            int argCapacity;
        } argList;

        struct {
            struct ASTNode* returnValue;
            struct ASTNode* expression;
            CryoDataType returnType;
        } returnStmt;
        // >=--------------<End>--------------=<

        
        // >=------<Variable Declaration>------=<
        struct {
            CryoDataType dataType;   // Data type of the variable
            char* name;
            struct ASTNode* initializer;
            bool isGlobal;
            bool isReference;
            int scopeLevel;
        } varDecl;

        struct {
            char* varName;
            bool isReference;
        } varName;
        // >=--------------<End>--------------=<
        

        // >=------<Control Flow>------=<
        struct {
            struct ASTNode* condition;
            struct ASTNode* thenBranch;
            struct ASTNode* elseBranch;
        } ifStmt;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } whileStmt;
        
        struct {
            struct ASTNode* initializer;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } forStmt;
        // >=--------------<End>--------------=<

        struct {
            struct ASTNode* stmt;
        } stmt;

        struct {
            struct ASTNode* expr;
        } expr;

        struct {
            CryoDataType dataType;  // Data type of the literal
            union {
                int intValue;
                float floatValue;
                char* stringValue;
                int booleanValue;
            };
        } literalExpression;

        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            CryoOperatorType op;
            char* operatorText;
        } bin_op;

        struct {
            CryoTokenType op;
            struct ASTNode* operand;
        } unary_op; // For unary operators, e.g (-5, !true, etc.)

        struct {
            char* str;
        } str;

        struct {
            CryoDataType dataType;
            int value;
        } boolean;

        struct {
            struct ASTNode** elements;
            int elementCount;
            int elementCapacity;
        } arrayLiteral; // Add this for array literals

    } data;
} ASTNode;


#define INITIAL_CAPACITY 8

#ifdef __cplusplus
extern "C" {
#endif



/* @Node_Accessors */
void printAST                               (ASTNode* node, int indent);
void freeAST                                (ASTNode* node);

/* @Node_Management */
ASTNode* createASTNode                      (CryoNodeType type);
void addChildNode                           (ASTNode* parent, ASTNode* child);
void addStatementToBlock                    (ASTNode* block, ASTNode* statement);
void addFunctionToProgram                   (ASTNode* program, ASTNode* function);

/* @Node_Creation - Expressions & Statements */
ASTNode* createProgramNode                  (void);
ASTNode* createLiteralExpr                  (int value);
ASTNode* createExpressionStatement          (ASTNode* expression);
ASTNode* createBinaryExpr                   (ASTNode* left, ASTNode* right, CryoOperatorType op);
ASTNode* createUnaryExpr                    (CryoTokenType op, ASTNode* operand);

/* @Node_Creation - Literals */
ASTNode* createIntLiteralNode               (int value);
ASTNode* createFloatLiteralNode             (float value);
ASTNode* createStringLiteralNode            (char* value);
ASTNode* createBooleanLiteralNode           (int value);
ASTNode* createIdentifierNode               (char* name);

/* @Node_Blocks - Blocks */
ASTNode* createBlockNode                    (void);
ASTNode* createFunctionBlock                (ASTNode* function);
ASTNode* createIfBlock                      (ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* createForBlock                     (ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* createWhileBlock                   (ASTNode* condition, ASTNode* body);


/* @Node_Blocks - Literals */
ASTNode* createBooleanLiteralExpr           (int value);
ASTNode* createStringLiteralExpr            (char* str);
ASTNode* createStringExpr                   (char* str);


/* @Node_Creation - Variables */
ASTNode* createVarDeclarationNode           (char* var_name, CryoDataType dataType, ASTNode* initializer, int line, bool isGlobal);
ASTNode* createVariableExpr                 (char* name, bool isReference);

/* @Node_Creation - Functions */
ASTNode* createFunctionNode                 (CryoVisibilityType visibility, char* function_name, ASTNode* params, ASTNode* function_body, CryoDataType returnType);
ASTNode* createExternDeclNode               (char* functionName, ASTNode* params, CryoDataType returnType);
ASTNode* createFunctionCallNode             (char* name, ASTNode** args, int argCount);
ASTNode* createReturnNode                   (ASTNode* returnValue);
ASTNode* createReturnExpression             (ASTNode* returnExpression, CryoDataType returnType);

/* @Node_Creation - Parameters */
ASTNode* createParamListNode                (void);
ASTNode* createArgumentListNode             (void);
ASTNode* createParamNode                    (char* name, CryoDataType type);


/* @Node_Creation - Modules & Externals */
ASTNode* createImportNode                   (ASTNode* importPath);
ASTNode* createExternNode                   (ASTNode* externNode);


/* @Node_Creation - Conditionals */
ASTNode* createIfStatement                  (ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* createForStatement                 (ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* createWhileStatement               (ASTNode* condition, ASTNode* body);

/* @Node_Creation - Arrays */
ASTNode* createArrayLiteralNode             (void);



#ifdef __cplusplus
}
#endif

#endif // AST_H
