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
#ifndef PARSER_H
#define PARSER_H
#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 8
#define MAX_ARGUMENTS 255
/*------ <includes> ------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---<custom_includes>---*/
#include "compiler/lexer.h" // Ensure this is included
#include "compiler/ast.h"
#include "compiler/token.h"
#include "utils/fs.h"
#include "utils/logger.h"
/*---------<end>---------*/
typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;
/*-----<function_prototypes>-----*/
#ifndef HAVE_STRNDUP
char* strndup(const char* s, size_t n);
#endif
int parser(int argc, char* argv[]);
// Helper Functions
void error(const char* message);
void getNextToken(Lexer *lexer);
void consume(Lexer *lexer, CryoTokenType type, const char *message);
int getOperatorPrecedence(CryoTokenType type);
void addStatementToProgram(ASTNode* program, ASTNode* statement);
char* tokenTypeToString(CryoTokenType type);
CryoDataType getCryoDataType(const char* typeStr);
// AST Nodes
ASTNode* createProgramNode(void);
ASTNode* parsePrimaryExpression(Lexer* lexer);
ASTNode* parseExpression(Lexer* lexer);
ASTNode* parseUnaryExpression(Lexer* lexer);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence);
ASTNode* parseExpression(Lexer* lexer);
ASTNode* parseStatement(Lexer* lexer);
ASTNode* parseIfStatement(Lexer* lexer);
ASTNode* parseReturnStatement(Lexer* lexer);
ASTNode* parseExpressionStatement(Lexer* lexer);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseForStatement(Lexer* lexer);
ASTNode* parseVarDeclaration(Lexer* lexer);
ASTNode* parseBlock(Lexer* lexer);
ASTNode* parseStatements(Lexer* lexer);
ASTNode* parseType(Lexer* lexer);
ASTNode* parseFunctionDeclaration(Lexer* lexer, CryoVisibilityType visibility);
ASTNode* parseFunctionBlock(Lexer* lexer);
ASTNode* parseParameterList(Lexer* lexer);
ASTNode* parseParameter(Lexer* lexer);
ASTNode* createTypeNode(CryoTokenType type);
ASTNode* createParamNode(const char* name, ASTNode* type);
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body);
ASTNode* parsePublicDeclaration(Lexer* lexer);
ASTNode* parseFunctionCall(Lexer* lexer, const char* functionName);
// Entry Point
ASTNode* parseProgram(Lexer* lexer);
/*-----<end_prototypes>-----*/

#endif // PARSER_H
