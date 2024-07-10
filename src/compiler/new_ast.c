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
#include "compiler/new_ast.h"

/* ====================================================================== */
// @Global_Variables
ASTNode* programNode = NULL;

/* ====================================================================== */
/* @Node_Accessors */

void printAST(ASTNode* node, int indent) {
    // TODO: Implement
}

void freeAST(ASTNode* node) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Management */

ASTNode* createASTNode(CryoNodeType type) {
    // TODO: Implement
}

void addChildNode(ASTNode* parent, ASTNode* child) {
    // TODO: Implement
}

void addStatementToBlock(ASTNode* block, ASTNode* statement) {
    // TODO: Implement
}

void addFunctionToProgram(ASTNode* program, ASTNode* function) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Expressions & Statements */

ASTNode* createLiteralExpr(int value) {
    // TODO: Implement
}

ASTNode* createExpressionStatement(ASTNode* expression) {
    // TODO: Implement
}

ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoOperatorType op) {
    // TODO: Implement
}

ASTNode* createUnaryExpr(CryoTokenType op, ASTNode* operand) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Literals */

ASTNode* createIntLiteralNode(int value) {
    // TODO: Implement
}

ASTNode* createFloatLiteralNode(float value) {
    // TODO: Implement
}

ASTNode* createStringLiteralNode(char* value) {
    // TODO: Implement
}

ASTNode* createBooleanLiteralNode(int value) {
    // TODO: Implement
}

ASTNode* createIdentifierNode(char* name) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Blocks - Blocks */

ASTNode* createBlockNode() {
    // TODO: Implement
}

ASTNode* createFunctionBlock(ASTNode* function) {
    // TODO: Implement
}

ASTNode* createIfBlock(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    // TODO: Implement
}

ASTNode* createForBlock(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    // TODO: Implement
}

ASTNode* createWhileBlock(ASTNode* condition, ASTNode* body) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Blocks - Literals */

ASTNode* createBooleanLiteralExpr(int value) {
    // TODO: Implement
}

ASTNode* createStringLiteralExpr(char* str) {
    // TODO: Implement
}

ASTNode* createStringExpr(char* str) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Variables */

ASTNode* createVarDeclarationNode(char* var_name, CryoDataType dataType, ASTNode* initializer, int line, bool isGlobal) {
    // TODO: Implement
}

ASTNode* createVariableExpr(char* name) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Functions */

ASTNode* createFunctionNode(CryoVisibilityType visibility, char* function_name, ASTNode* params, ASTNode* function_body, CryoDataType returnType) {
    // TODO: Implement
}

ASTNode* createExternDeclNode(char* functionName, ASTNode* params, CryoDataType returnType) {
    // TODO: Implement
}

ASTNode* createFunctionCallNode(char* name, ASTNode** args, int argCount) {
    // TODO: Implement
}

ASTNode* createReturnNode(ASTNode* returnValue) {
    // TODO: Implement
}

ASTNode* createReturnStatement(ASTNode* return_val) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Parameters */

ASTNode* createParamListNode(void) {
    // TODO: Implement
}

ASTNode* createArgumentListNode(void) {
    // TODO: Implement
}

ASTNode* createParamNode(char* name, ASTNode* type) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Modules & Externals */

ASTNode* createImportNode(ASTNode* importPath) {
    // TODO: Implement
}

ASTNode* createExternNode(ASTNode* externNode) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Conditionals */

ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    // TODO: Implement
}

ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    // TODO: Implement
}

ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body) {
    // TODO: Implement
}


/* ====================================================================== */
/* @Node_Creation - Arrays */

ASTNode* createArrayLiteralNode() {
    // TODO: Implement
}


