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
#define INITIAL_STATEMENT_CAPACITY 256
/*------ <includes> ------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*---<custom_includes>---*/
#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "utils.h"
/*---------<end>---------*/


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
// AST Nodes
ASTNode* parsePrimaryExpression(Lexer* lexer);
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
ASTNode* parseFunctionDeclaration(Lexer* lexer);
ASTNode* parseParamList(Lexer* lexer);
ASTNode* createTypeNode(CryoTokenType type);
ASTNode* parseType(Lexer* lexer);
ASTNode* createParamNode(const char* name, ASTNode* type);
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body);
// Entry Point
ASTNode* parseProgram(Lexer* lexer);
/*-----<end_prototypes>-----*/
#endif // PARSER_H