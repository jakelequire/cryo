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

/*------ <includes> ------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
/*---<custom_includes>---*/
#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "compiler/token.h"
#include "compiler/symtable.h"
#include "compiler/error.h"
#include "utils/fs.h"
#include "utils/logger.h"
#include "utils/utility.h"
#include "utils/arena.h"
#include "common/common.h"
#include "compiler.h"
/*---------<end>---------*/
typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct CryoSymbolTable CryoSymbolTable;
typedef struct CompilerState CompilerState;
typedef struct Position Position;

typedef struct
{
    bool isParsingIfCondition;
    int scopeLevel;
    // Add other context flags as needed
} ParsingContext;

/* @Macros */
#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 8
#define MAX_ARGUMENTS 255

// #ifndef HAVE_STRNDUP
// char* strndup(const char* s, size_t n);
// #endif

/* =========================================================== */
/* @Function_Prototypes                                        */

Position getPosition(Lexer *lexer);
void printPosition(Position position);

void printLine(const char *source, int line, Arena *arena, CompilerState *state);

/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */
ASTNode *parseProgram(Lexer *lexer, CryoSymbolTable *table, Arena *arena, CompilerState *state);

/* @Helper_Functions | Debugging, Errors, Walkers */
void consume(Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state);
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state);
void error(char *message, char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state);
void debugCurrentToken(Lexer *lexer, Arena *arena, CompilerState *state);
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state);
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state);

/* @DataType_Management */
CryoDataType getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state);
CryoDataType parseType(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena, CompilerState *state);
int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state);

/* @Parser_Management */
void addStatementToProgram(ASTNode *program, CryoSymbolTable *table, ASTNode *statement, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Expressions & Statements*/
void parseDebugger(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseScopeCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseScopedFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName);
ASTNode *parseNamespace(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state);
ASTNode *parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Blocks*/
ASTNode *parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Variables*/
ASTNode *parseVarDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Functions*/
ASTNode *parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state);
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *functionName, Arena *arena, CompilerState *state);
ASTNode *parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
/* @ASTNode_Parsing - Parameters */
ASTNode *parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state);
ASTNode **parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state);
ASTNode *parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoDataType expectedType, Arena *arena, CompilerState *state);
void addParameterToList(CryoSymbolTable *table, ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state);
void addArgumentToList(CryoSymbolTable *table, ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state);
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode *functionCall, ASTNode *arg, Arena *arena, CompilerState *state);
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Modules & Externals */
ASTNode *parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
void importTypeDefinitions(const char *module, const char *subModule, CryoSymbolTable *table, Arena *arena, CompilerState *state);
ASTNode *parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Conditionals */
ASTNode *parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseIfCondition(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Arrays */
ASTNode *parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state);
ASTNode *parseArrayIndexing(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state);

ASTNode *parseAssignment(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *varName, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Structures */
ASTNode *parseStructDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);
ASTNode *parseStructField(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state);

#endif // PARSER_H
