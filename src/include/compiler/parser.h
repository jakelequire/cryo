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
/*---------<end>---------*/
typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct CryoSymbolTable CryoSymbolTable;


typedef struct {
    bool isParsingIfCondition;
    int scopeLevel;
    // Add other context flags as needed
} ParsingContext;

/* @Macros */
#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 8
#define MAX_ARGUMENTS 255


//#ifndef HAVE_STRNDUP
//char* strndup(const char* s, size_t n);
//#endif

/* =========================================================== */
/* @Function_Prototypes                                        */

void printLine(const char* source, int line);

/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */
ASTNode* parseProgram                       (Lexer* lexer, CryoSymbolTable *table);

/* @Helper_Functions | Debugging, Errors, Walkers */
void consume                                (Lexer *lexer, CryoTokenType type, const char* message, const char* functionName, CryoSymbolTable *table);
void getNextToken                           (Lexer *lexer);
void error                                  (char* message, char* functionName, CryoSymbolTable *table);
void debugCurrentToken                      (void);
Token peekNextUnconsumedToken               (Lexer *lexer);

/* @DataType_Management */
CryoDataType getCryoDataType                (const char* typeStr);
CryoDataType parseType                      (Lexer* lexer, ParsingContext* context, CryoSymbolTable *table);
int getOperatorPrecedence                   (CryoTokenType type);

/* @Parser_Management */
// ASTNode* createProgramNode               (void);     !! Move to AST
void addStatementToProgram                  (ASTNode* program, CryoSymbolTable *table, ASTNode* statement);

/* @ASTNode_Parsing - Expressions & Statements*/
ASTNode* parseStatement                     (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parsePrimaryExpression             (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseExpression                    (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseExpressionStatement           (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseBinaryExpression              (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, ASTNode* left, int precedence);
ASTNode* parseUnaryExpression               (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parsePublicDeclaration             (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);

/* @ASTNode_Parsing - Blocks*/
ASTNode* parseBlock                         (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseFunctionBlock                 (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);

/* @ASTNode_Parsing - Variables*/
ASTNode* parseVarDeclaration                (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);

/* @ASTNode_Parsing - Functions*/
ASTNode* parseFunctionDeclaration           (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility);
ASTNode* parseExternFunctionDeclaration     (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseFunctionCall                  (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char* functionName);
ASTNode* parseReturnStatement               (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
/* @ASTNode_Parsing - Parameters */
ASTNode* parseParameter                     (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode** parseParameterList                (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseArguments                     (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseArgumentList                  (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseArgumentsWithExpectedType     (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoDataType expectedType);
void addParameterToList                     (CryoSymbolTable *table, ASTNode* paramListNode, ASTNode* param);
void addArgumentToList                      (CryoSymbolTable *table, ASTNode* argListNode, ASTNode* arg);
void addArgumentToFunctionCall              (CryoSymbolTable *table, ASTNode* functionCall, ASTNode* arg);
void addParameterToExternDecl               (CryoSymbolTable *table, ASTNode* externDeclNode, ASTNode* param);

/* @ASTNode_Parsing - Modules & Externals */
ASTNode* parseImport                        (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseExtern                        (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);

/* @ASTNode_Parsing - Conditionals */
ASTNode* parseIfStatement                   (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseForLoop                       (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
ASTNode* parseWhileStatement                (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);

/* @ASTNode_Parsing - Arrays */
ASTNode* parseArrayLiteral                  (Lexer *lexer, CryoSymbolTable *table, ParsingContext *context);
void addElementToArrayLiteral               (CryoSymbolTable *table, ASTNode* arrayLiteral, ASTNode* element);

/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */
int parser(int argc, char* argv[]);


#endif // PARSER_H
