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
ASTNode* createVariableExpr                 (char* name);

/* @Node_Creation - Functions */
ASTNode* createFunctionNode                 (CryoVisibilityType visibility, char* function_name, ASTNode* params, ASTNode* function_body, CryoDataType returnType);
ASTNode* createExternDeclNode               (char* functionName, ASTNode* params, CryoDataType returnType);
ASTNode* createFunctionCallNode             (char* name, ASTNode** args, int argCount);
ASTNode* createReturnNode                   (ASTNode* returnValue);
ASTNode* createReturnStatement              (ASTNode* return_val);

/* @Node_Creation - Parameters */
ASTNode* createParamListNode                (void);
ASTNode* createArgumentListNode             (void);
ASTNode* createParamNode                    (char* name, ASTNode* type);


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