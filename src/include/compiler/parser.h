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

typedef struct {
    bool isParsingIfCondition;
    // Add other context flags as needed
} ParsingContext;

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
ASTNode* parsePrimaryExpression(Lexer* lexer, ParsingContext* context);
ASTNode* parseExpression(Lexer* lexer, ParsingContext* context);
ASTNode* parseUnaryExpression(Lexer* lexer, ParsingContext* context);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence, ParsingContext* context);
ASTNode* parseStatement(Lexer* lexer, ParsingContext* context);
ASTNode* parseReturnStatement(Lexer* lexer, ParsingContext* context);
ASTNode* parseExpressionStatement(Lexer* lexer, ParsingContext* context);

ASTNode* parseVarDeclaration(Lexer* lexer, ParsingContext* context);
ASTNode* parseBlock(Lexer* lexer, ParsingContext* context);
ASTNode* parseStatements(Lexer* lexer, ParsingContext* context);
ASTNode* parseType(Lexer* lexer, ParsingContext* context);
ASTNode* parseFunctionDeclaration(Lexer* lexer, CryoVisibilityType visibility, ParsingContext* context);
ASTNode* parseFunctionBlock(Lexer* lexer, ParsingContext* context);
ASTNode* parseParameterList(Lexer* lexer, ParsingContext* context);
ASTNode* createParamNode(const char* name, ASTNode* type);
ASTNode* parsePublicDeclaration(Lexer* lexer, ParsingContext* context);
ASTNode* parseFunctionCall(Lexer* lexer, const char* functionName, ParsingContext* context);

ASTNode* createTypeNode(CryoTokenType type);
ASTNode* parseParameter(Lexer* lexer, ParsingContext* context);
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body);

ASTNode* parseIfStatement(Lexer* lexer, ParsingContext* context);
ASTNode* parseForLoop(Lexer* lexer, ParsingContext* context);
ASTNode* parseWhileStatement(Lexer* lexer);
ASTNode* parseProgram(Lexer* lexer);

#endif // PARSER_H

// Entry Point
/*-----<end_prototypes>-----*/