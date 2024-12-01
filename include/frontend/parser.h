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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "frontend/lexer.h"
#include "frontend/AST.h"
#include "frontend/tokens.h"
#include "frontend/symTable.h"
#include "tools/utils/fs.h"
#include "tools/utils/c_logger.h"
#include "tools/utils/utility.h"
#include "tools/arena/arena.h"
#include "common/common.h"
#include "compiler/compiler.h"
#include "settings/compilerSettings.h"
#include "tools/macros/consoleColors.h"
#include "frontend/dataTypes.h"

#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 16
#define MAX_ARGUMENTS 255

typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct CryoSymbolTable CryoSymbolTable;
typedef struct CompilerState CompilerState;
typedef struct Position Position;
typedef struct ConstructorMetaData ConstructorMetaData;
typedef struct TypeTable TypeTable;
typedef struct DataType DataType;

typedef struct ThisContext
{
    CryoNodeType nodeType;
    const char *nodeName;
    ASTNode **properties;
    int propertyCount;
    ASTNode **methods;
    int methodCount;
    bool isStatic;
} ThisContext;

/**
 * @brief The ParsingContext struct is used to manage the state of the parser.
 *
 * ``` c
 *
 *  bool isParsingIfCondition;
 *
 *  int scopeLevel;
 *
 *  const char *currentNamespace;
 *
 *  ThisContext *thisContext;
 *
 * ```
 *
 */
typedef struct ParsingContext
{
    bool isParsingIfCondition;
    int scopeLevel;
    const char *currentNamespace;

    /// #### The ThisContext struct is used to manage the state of the 'this' keyword.
    /// ---
    ///``` c
    /// CryoNodeType nodeType;
    /// const char *nodeName;
    /// ASTNode **properties;
    /// int propertyCount;
    /// ASTNode **methods;
    /// int methodCount;
    /// bool isStatic;
    ///```
    ThisContext *thisContext;

    // An array of the last 16 tokens
    Token lastTokens[16];
    int lastTokenCount;

    // Add other context flags as needed
} ParsingContext;

/* =========================================================== */
/* @Function_Prototypes                                        */

Position getPosition(Lexer *lexer);
void printPosition(Position position);

void printLine(const char *source, int line, Arena *arena, CompilerState *state);

/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */
ASTNode *parseProgram(Lexer *lexer, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @Helper_Functions | Debugging, Errors, Walkers */
void consume(int line, Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable);
void parsingError(char *message, char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state, Lexer *lexer, const char *source, TypeTable *typeTable);
void debugCurrentToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable);
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable);
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable);
void printHorizontalLine(int width);

/* @DataType_Management */
DataType *getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state, Lexer *lexer, TypeTable *typeTable);
DataType *parseType(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable);
int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @Parser_Management */
void addStatementToProgram(ASTNode *program, CryoSymbolTable *table, ASTNode *statement, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Expressions & Statements*/
ASTNode *parseStaticKeyword(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseIdentifierExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseScopeCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseScopedFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName, TypeTable *typeTable);
ASTNode *parseNamespace(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parsePrivateDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

void parseDebugger(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Blocks*/
ASTNode *parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Variables*/
ASTNode *parseVarDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Functions*/
ASTNode *parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *functionName, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Parameters */
ASTNode *parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, TypeTable *typeTable);
ASTNode **parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, TypeTable *typeTable);
ASTNode *parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, DataType *expectedType, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseExpectedTypeArgWithThisKW(Lexer *lexer, DataType *expectedType, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

void addParameterToList(CryoSymbolTable *table, ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state, TypeTable *typeTable);
void addArgumentToList(CryoSymbolTable *table, ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state, TypeTable *typeTable);
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode *functionCall, ASTNode *arg, Arena *arena, CompilerState *state, TypeTable *typeTable);
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Modules & Externals */
ASTNode *parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
void importTypeDefinitions(const char *module, const char *subModule, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Conditionals */
ASTNode *parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseIfCondition(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

/* @ASTNode_Parsing - Arrays */
ASTNode *parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseArrayIndexing(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state, TypeTable *typeTable);

ASTNode *parseAssignment(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *varName, Arena *arena, CompilerState *state, TypeTable *typeTable);

ASTNode *parseThisContext(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseDotNotation(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseDotNotationWithType(ASTNode *object, DataType *typeOfNode, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseLHSIdentifier(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, DataType *typeOfNode);
ASTNode *parseIdentifierDotNotation(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

ASTNode *parseForThisValueProperty(Lexer *lexer, DataType *expectedType, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

// # ============================================================ #

void setDefaultThisContext(const char *currentNamespace, ParsingContext *context, TypeTable *typeTable);
void setThisContext(ParsingContext *context, const char *nodeName, CryoNodeType nodeType, TypeTable *typeTable);
void clearThisContext(ParsingContext *context, TypeTable *typeTable);
void addPropertyToThisContext(ParsingContext *context, ASTNode *propertyNode, TypeTable *typeTable);
void addMethodToThisContext(ParsingContext *context, ASTNode *methodNode, TypeTable *typeTable);

ASTNode *getPropertyByName(ParsingContext *context, const char *name, TypeTable *typeTable);
ASTNode *getMethodByName(ParsingContext *context, const char *name, TypeTable *typeTable);
ASTNode *parseMethodCall(ASTNode *accessorObj, char *methodName, DataType *instanceType,
                         Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

void addTokenToContext(ParsingContext *context, Token token);
void addStaticIdentifierToContext(ParsingContext *context, bool value);

void logThisContext(ParsingContext *context);
void logTokenArray(ParsingContext *context);
void logParsingContext(ParsingContext *context);

// # =========================================================================== #
// # Struct & Type Parsing
// # (typeParsing.c)
// # =========================================================================== #

ASTNode *parseStructDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseStructField(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseConstructor(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, ConstructorMetaData *metaData, TypeTable *typeTable);
ASTNode *parseMethodDeclaration(bool isStatic, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

bool parsePropertyForDefaultFlag(ASTNode *propertyNode);

ASTNode *parseGenericDecl(const char *typeName, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseStructInstance(const char *structName, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

ConstructorMetaData *createConstructorMetaData(const char *parentName, CryoNodeType parentNodeType, bool hasDefaultFlag);

// # =========================================================================== #
// # Class Parsing
// # =========================================================================== #

ASTNode *parseClassDeclaration(bool isStatic, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);
ASTNode *parseClassBody(ASTNode *classNode, const char *className, bool isStatic,
                        Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

void addConstructorToClass(ASTNode *classNode, ASTNode *constructorNode, Arena *arena, CompilerState *state, TypeTable *typeTable);
void addMethodToClass(ASTNode *classNode, ASTNode *methodNode, CryoVisibilityType visibility,
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addPropertyToClass(ASTNode *classNode, ASTNode *propNode, CryoVisibilityType visibility,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);

void addPrivateMethod(ASTNode *classNode, ASTNode *methodNode,
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addPublicMethod(ASTNode *classNode, ASTNode *methodNode,
                     Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addProtectedMethod(ASTNode *classNode, ASTNode *methodNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addPublicProperty(ASTNode *classNode, ASTNode *propNode,
                       Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addPrivateProperty(ASTNode *classNode, ASTNode *propNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);
void addProtectedProperty(ASTNode *classNode, ASTNode *propNode,
                          Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context);

static void ensureCapacity(ASTNode **array, int *capacity, int count, int increment);
void addDataTypeToClassNode(ASTNode *classNode, DataType *type);

ASTNode *parseMethodScopeResolution(const char *scopeName,
                                    Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable);

#endif // PARSER_H
