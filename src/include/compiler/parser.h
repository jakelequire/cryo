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
#include <assert.h>
/*---<custom_includes>---*/
#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "compiler/token.h"
#include "compiler/symtable.h"
#include "utils/fs.h"
#include "utils/logger.h"
/*---------<end>---------*/
typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;

typedef struct {
    bool isParsingIfCondition;
    int scopeLevel;
    // Add other context flags as needed
} ParsingContext;

/* @Macros */
#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 8
#define MAX_ARGUMENTS 255


#ifndef HAVE_STRNDUP
char* strndup(const char* s, size_t n);
#endif

/* =========================================================== */
/* @Function_Prototypes                                        */

void printLine(const char* source, int line);

/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */
ASTNode* parseProgram                       (Lexer* lexer);

/* @Helper_Functions | Debugging, Errors, Walkers */
void consume                                (Lexer *lexer, CryoTokenType type, const char* message, const char* functionName);
void getNextToken                           (Lexer *lexer);
void error                                  (char* message, char* functionName);
void debugCurrentToken                      (void);
Token peekNextUnconsumedToken               (Lexer *lexer);

/* @DataType_Management */
CryoDataType getCryoDataType                (const char* typeStr);
CryoDataType parseType                      (Lexer* lexer, ParsingContext* context);
int getOperatorPrecedence                   (CryoTokenType type);

/* @Parser_Management */
// ASTNode* createProgramNode               (void);     !! Move to AST
void addStatementToProgram                  (ASTNode* program, ASTNode* statement);

/* @ASTNode_Parsing - Expressions & Statements*/
ASTNode* parseStatement                     (Lexer *lexer, ParsingContext *context);
ASTNode* parsePrimaryExpression             (Lexer *lexer, ParsingContext *context);
ASTNode* parseExpression                    (Lexer *lexer, ParsingContext *context);
ASTNode* parseExpressionStatement           (Lexer *lexer, ParsingContext *context);
ASTNode* parseBinaryExpression              (Lexer *lexer, ParsingContext *context, ASTNode* left, int precedence);
ASTNode* parseUnaryExpression               (Lexer *lexer, ParsingContext *context);
ASTNode* parsePublicDeclaration             (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Blocks*/
ASTNode* parseBlock                         (Lexer *lexer, ParsingContext *context);
ASTNode* parseFunctionBlock                 (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Variables*/
ASTNode* parseVarDeclaration                (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Functions*/
ASTNode* parseFunctionDeclaration           (Lexer *lexer, ParsingContext *context, CryoVisibilityType visibility);
ASTNode* parseExternFunctionDeclaration     (Lexer *lexer, ParsingContext *context);
ASTNode* parseFunctionCall                  (Lexer *lexer, ParsingContext *context, char* functionName);
ASTNode* parseReturnStatement               (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Parameters */
ASTNode* parseParameter                     (Lexer *lexer, ParsingContext *context);
ASTNode* parseParameterList                 (Lexer *lexer, ParsingContext *context);
ASTNode* parseArgumentList                  (Lexer *lexer, ParsingContext *context);
void addParameterToList                     (ASTNode* paramListNode, ASTNode* param);
void addArgumentToList                      (ASTNode* argListNode, ASTNode* arg);

/* @ASTNode_Parsing - Modules & Externals */
ASTNode* parseImport                        (Lexer *lexer, ParsingContext *context);
ASTNode* parseExtern                        (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Conditionals */
ASTNode* parseIfStatement                   (Lexer *lexer, ParsingContext *context);
ASTNode* parseForLoop                       (Lexer *lexer, ParsingContext *context);
ASTNode* parseWhileStatement                (Lexer *lexer, ParsingContext *context);

/* @ASTNode_Parsing - Arrays */
ASTNode* parseArrayLiteral                  (Lexer *lexer, ParsingContext *context);
void addElementToArrayLiteral               (ASTNode* arrayLiteral, ASTNode* element);

/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */
int parser(int argc, char* argv[]);


#endif // PARSER_H
