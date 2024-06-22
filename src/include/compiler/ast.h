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
/*---------<end>---------*/

#ifdef __cplusplus
extern "C" {
#endif

/*-------<structure_defs>-------*/
typedef struct ASTNode {
    CryoNodeType type;
    int line;  // Line number for error reporting
    struct ASTNode* firstChild;  // First child node (for linked list structure)
    struct ASTNode* nextSibling; // Next sibling node (for linked list structure)
    union {
        struct program {
            struct ASTNode** statements;
            int stmtCount;
            int stmtCapacity;
        } program;

        struct functionDecl {
            char* visibility;
            char* name;
            struct ASTNode* params;
            struct ASTNode* returnType;
            struct ASTNode* body;
        } functionDecl;

        struct paramList {
            struct ASTNode** params;
            int paramCount;
            int paramCapacity;
        } paramList;

        struct varDecl {
            char* name;
            struct ASTNode* type;
            struct ASTNode* initializer;
        } varDecl;

        struct block {
            struct ASTNode** statements;
            int stmtCount;
            int stmtCapacity;
        } block;

        struct stmt {
            struct ASTNode* stmt;
        } stmt;

        struct expr {
            struct ASTNode* expr;
        } expr;

        struct bin_op {
            struct ASTNode* left;
            struct ASTNode* right;
            CryoTokenType op;
            char* operatorText;
        } bin_op;

        struct unary_op {
            CryoTokenType op;
            struct ASTNode* operand;
        } unary_op; // For unary operators, e.g (-5, !true, etc.)

        int value;  // For literal number nodes

        struct varName {
            char* varName;
        } varName;

        struct functionCall {
            char* name;
            struct ASTNode** args;
            int argCount;
        } functionCall;
        
        struct ifStmt {
            struct ASTNode* condition;
            struct ASTNode* thenBranch;
            struct ASTNode* elseBranch;
        } ifStmt;
        
        struct whileStmt {
            struct ASTNode* condition;
            struct ASTNode* body;
        } whileStmt;
        
        struct forStmt {
            struct ASTNode* initializer;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } forStmt;

        struct returnStmt {
            struct ASTNode* returnValue;
        } returnStmt;

        struct str {
            char* str;
        } str;

        struct boolean {
            int value;
        } boolean;
    } data;
} ASTNode;
/*-------<end_defs>-------*/
/*-----<function_prototypes>-----*/
void printAST(ASTNode* node, int indent);
void freeAST(ASTNode* node);
ASTNode* createASTNode(CryoNodeType type);
ASTNode* createLiteralExpr(int value);
ASTNode* createVariableExpr(const char* name);
ASTNode* createBooleanLiteralExpr(int value);
ASTNode* createStringLiteralExpr(const char* str);
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoTokenType op);
ASTNode* createUnaryExpr(CryoTokenType op, ASTNode* operand);
ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body);
ASTNode* createReturnStatement(ASTNode* return_val);
ASTNode* createBlock();
ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body);
ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer, int line);
ASTNode* createExpressionStatement(ASTNode* expression);
ASTNode* createFunctionCallNode(const char* name, ASTNode** args, int argCount);
ASTNode* createStringExpr(const char* str);
void addStatementToBlock(ASTNode* block, ASTNode* statement);
void addFunctionToProgram(ASTNode* program, ASTNode* function);
/*-----<end_prototypes>-----*/


#ifdef __cplusplus
}
#endif

#endif // AST_H