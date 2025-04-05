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
#include "tools/utils/fs.h"
#include "tools/utils/c_logger.h"
#include "tools/utils/utility.h"
#include "tools/arena/arena.h"
#include "common/common.h"
#include "compiler/compiler.h"
#include "settings/compilerSettings.h"
#include "tools/macros/consoleColors.h"
#include "dataTypes/dataTypes.h"
#include "dataTypes/dataTypeManager.h"

#define INITIAL_STATEMENT_CAPACITY 512
#define INITIAL_PARAM_CAPACITY 16
#define MAX_ARGUMENTS 255
#define MAX_USING_MODULES 16
#define MAX_FILES 1024

typedef struct Lexer Lexer;
typedef struct ASTNode ASTNode;
typedef struct Token Token;
typedef struct CompilerState CompilerState;
typedef struct Position Position;
typedef struct ConstructorMetaData ConstructorMetaData;

typedef struct DataType_t DataType;
typedef struct GenericType GenericType;
typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

typedef struct ScopeParsingContext
{
    const char *name;
    const char *scopeID;
    int level;
    bool isStatic;
    CryoNodeType nodeType;
    // Parent Scope for classes
    struct ScopeParsingContext *parent;
} ScopeParsingContext;

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
    int scopeLevel;
    const char *currentNamespace;
    const char *namespaceScopeID;
    const char *functionName;

    ASTNode *programNodePtr;

    ThisContext *thisContext;

    // An array of the last 16 tokens
    Token lastTokens[16];
    int lastTokenCount;

    // The current scope context
    ScopeParsingContext *scopeContext;

    // Context Flags
    bool isParsingModuleFile;
    bool isParsingIfCondition;
    bool inGenericContext;

    GenericType **currentGenericParams; // Current generic parameters in scope
    int currentGenericParamCount;       // Number of current generic parameter
    int currentGenericParamCapacity;    // Capacity of the current generic parameter array
    void (*addGenericParam)(ParsingContext *context, const char *name, GenericType *param);
} ParsingContext;

void ParsingContext_addGenericParam(ParsingContext *context, const char *name, GenericType *param);

/* =========================================================== */
/* @Function_Prototypes                                        */

Position getPosition(Lexer *lexer);
void printPosition(Position position);

void printLine(const char *source, int line, Arena *arena, CompilerState *state);

/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */
ASTNode *parseProgram(Lexer *lexer, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseAnnotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @Helper_Functions | Debugging, Errors, Walkers */
void consume(int line, Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, Arena *arena, CompilerState *state, ParsingContext *context);
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state);
void parsingError(char *message, char *functionName, Arena *arena, CompilerState *state, Lexer *lexer, const char *source, CryoGlobalSymbolTable *globalTable);
void debugCurrentToken(Lexer *lexer, Arena *arena, CompilerState *state);
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state);
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state);
void printHorizontalLine(int width);
bool isOperator(CryoTokenType type);

/* @DataType_Management */
DataType *getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state, Lexer *lexer, CryoGlobalSymbolTable *globalTable);
DataType *parseType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseGenericDataTypeInstantiation(DataType *type, Lexer *lexer, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state);

/* @Parser_Management */
void addStatementToProgram(ASTNode *program, ASTNode *statement, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Expressions & Statements*/
ASTNode *parseStaticKeyword(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseIdentifierExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseScopeCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseScopedFunctionCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName, CryoGlobalSymbolTable *globalTable);
ASTNode *parseNamespace(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parsePrimaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseExpressionStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseBinaryExpression(Lexer *lexer, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseUnaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parsePublicDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parsePrivateDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

void parseDebugger(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Blocks*/
ASTNode *parseBlock(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseFunctionBlock(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Variables*/
ASTNode *parseVarDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Functions*/
ASTNode *parseFunctionDeclaration(Lexer *lexer, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseFunctionCall(Lexer *lexer, ParsingContext *context, char *functionName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseReturnStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Parameters */
ASTNode *parseParameter(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode **parseParameterList(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseArguments(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseArgumentList(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, ParsingContext *context, DataType *expectedType, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseExpectedTypeArgWithThisKW(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

void addParameterToList(ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void addArgumentToList(ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void addArgumentToFunctionCall(ASTNode *functionCall, ASTNode *arg, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void addParameterToExternDecl(ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void validateParameterList(ASTNode **params, int paramCount, Arena *arena, CompilerState *state);

/* @ASTNode_Parsing - Modules & Externals */
void importTypeDefinitions(const char *module, const char *subModule, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseExtern(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Conditionals */
ASTNode *parseIfStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseIfCondition(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseForLoop(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseWhileStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

/* @ASTNode_Parsing - Arrays */
ASTNode *parseArrayLiteral(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void addElementToArrayLiteral(ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseArrayIndexing(Lexer *lexer, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseAssignment(Lexer *lexer, ParsingContext *context, char *varName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseThisContext(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseDotNotationWithType(ASTNode *object, DataType *typeOfNode, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseLHSIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, DataType *typeOfNode, CryoGlobalSymbolTable *globalTable);
ASTNode *parseIdentifierDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseForThisValueProperty(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// # ============================================================ #
// # Context Management
// # ============================================================ #

ParsingContext *createParsingContext(void);
void setDefaultThisContext(const char *currentNamespace, ParsingContext *context);
void setThisContext(ParsingContext *context, const char *nodeName, CryoNodeType nodeType);
void clearThisContext(ParsingContext *context);
void addPropertyToThisContext(ParsingContext *context, ASTNode *propertyNode);
void addMethodToThisContext(ParsingContext *context, ASTNode *methodNode);

ASTNode *getPropertyByName(ParsingContext *context, const char *name);
ASTNode *getMethodByName(ParsingContext *context, const char *name);
ASTNode *parseMethodCall(ASTNode *accessorObj, char *methodName, DataType *instanceType,
                         Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

void addTokenToContext(ParsingContext *context, Token token);
void addStaticIdentifierToContext(ParsingContext *context, bool value);

void setCurrentFunction(ParsingContext *context, const char *functionName, const char *namespaceScopeID);
void setCurrentMethod(ParsingContext *context, const char *methodName, const char *className);
void resetCurrentFunction(ParsingContext *context);
void resetCurrentMethod(ParsingContext *context);
const char *getCurrentScopeID(ParsingContext *context);
const char *getNamespaceScopeID(ParsingContext *context);

void setModuleFileParsingFlag(ParsingContext *context, bool value);

// Scope Parsing Context Functions

ScopeParsingContext *createScopeParsingContext(const char *name, int level, bool isStatic, CryoNodeType nodeType);
ScopeParsingContext *createClassScopeContext(const char *className, int level, bool isStatic);
ScopeParsingContext *createMethodScopeContext(const char *methodName, int level, bool isStatic, ScopeParsingContext *parent);
ScopeParsingContext *createFunctionScopeContext(const char *functionName, int level, ScopeParsingContext *parent);
ScopeParsingContext *createNamespaceScopeContext(const char *namespaceName);

void createNamespaceScope(ParsingContext *context, const char *namespaceName);
void createFunctionScope(ParsingContext *context, const char *functionName, const char *namespaceScopeID);
void createClassScope(ParsingContext *context, const char *className);
void createMethodScope(ParsingContext *context, const char *methodName, const char *className);
void clearScopeContext(ParsingContext *context);

// This is a helper function to generate a unique scope ID
const char *getScopeID(const char *name);

// Debugging Functions

void logThisContext(ParsingContext *context);
void logTokenArray(ParsingContext *context);
void logParsingContext(ParsingContext *context);
void logScopeInformation(ParsingContext *context);

// # =========================================================================== #
// # Struct & Type Parsing
// # (typeParsing.c)
// # =========================================================================== #

ASTNode *parseStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseGenericStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, const char *parentNamespaceNameID);
ASTNode *parseNonGenericStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, const char *parentNamespaceNameID);
bool parseStructFieldOrMethod(DataType *dataType, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, ASTNode **properties, int *propertyCount, ASTNode **methods, int *methodCount, bool *hasDefaultProperty, int *defaultPropertyCount, bool *hasConstructor, ASTNode **constructorNode, DataType **ctorArgs, int *ctorArgCount);
ASTNode *finalizeStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, ASTNode **properties, int propertyCount, ASTNode **methods, int methodCount, bool hasDefaultProperty, bool hasConstructor, ASTNode *constructorNode, DataType **ctorArgs, int ctorArgCount);

ASTNode *parseNewStructObject(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *parseStructField(const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseConstructor(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, ConstructorMetaData *metaData, CryoGlobalSymbolTable *globalTable);
ASTNode *parseMethodDeclaration(bool isStatic, const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

bool parsePropertyForDefaultFlag(ASTNode *propertyNode);

ASTNode *parseGenericDecl(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseStructInstance(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ConstructorMetaData *createConstructorMetaData(const char *parentName, CryoNodeType parentNodeType, bool hasDefaultFlag);

ASTNode *parseTypeDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

DataType *parseTypeDefinition(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseForPrimitive(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseFunctionType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseStructType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseClassType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
DataType *parseObjectType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// # =========================================================================== #
// # Class Parsing
// # =========================================================================== #

ASTNode *parseClassDeclaration(bool isStatic, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *parseClassBody(ASTNode *classNode, const char *className, bool isStatic,
                        Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

void addConstructorToClass(ASTNode *classNode, ASTNode *constructorNode, Arena *arena, CompilerState *state);
void addMethodToClass(ASTNode *classNode, ASTNode *methodNode, CryoVisibilityType visibility,
                      Arena *arena, CompilerState *state, ParsingContext *context, CryoGlobalSymbolTable *globalTable);
void addPropertyToClass(ASTNode *classNode, ASTNode *propNode, CryoVisibilityType visibility,
                        Arena *arena, CompilerState *state, ParsingContext *context, CryoGlobalSymbolTable *globalTable);

void addPrivateMethod(ASTNode *classNode, ASTNode *methodNode,
                      Arena *arena, CompilerState *state, ParsingContext *context);
void addPublicMethod(ASTNode *classNode, ASTNode *methodNode,
                     Arena *arena, CompilerState *state, ParsingContext *context);
void addProtectedMethod(ASTNode *classNode, ASTNode *methodNode,
                        Arena *arena, CompilerState *state, ParsingContext *context);
void addPublicProperty(ASTNode *classNode, ASTNode *propNode,
                       Arena *arena, CompilerState *state, ParsingContext *context);
void addPrivateProperty(ASTNode *classNode, ASTNode *propNode,
                        Arena *arena, CompilerState *state, ParsingContext *context);
void addProtectedProperty(ASTNode *classNode, ASTNode *propNode,
                          Arena *arena, CompilerState *state, ParsingContext *context);

static void ensureCapacity(ASTNode **array, int *capacity, int count, int increment);

ASTNode *parseMethodScopeResolution(const char *scopeName,
                                    Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

ASTNode *createClassPropertyAccessNode(ASTNode *object, ASTNode *property, const char *propName, DataType *typeOfNode,
                                       Arena *arena, CompilerState *state, Lexer *lexer);
// New Keyword Parsing
ASTNode *parseNewExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// Null Expression Parsing
ASTNode *parseNullExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// `typeof` keyword
ASTNode *parseTypeofIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// # =========================================================================== #
// # `implement` Keyword Parsing
// # =========================================================================== #

ASTNode *parseImplementation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// # =========================================================================== #
// # `Using` Keyword Parsing
// # =========================================================================== #

#define MAX_MODULE_CHAIN 16

ASTNode *parseUsingKeyword(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void importUsingModule(const char *primaryModule, const char *moduleChain[], size_t moduleCount, CompilerState *state, CryoGlobalSymbolTable *globalTable);
const char *getSTDLibraryModulePath(const char *moduleName, CompilerState *state);
const char **getFilesInModuleDir(const char *modulePath);
const char *findRegularFile(const char **moduleFiles, size_t moduleCount, const char *fileName);
const char *findModuleFile(const char **moduleFiles, size_t moduleCount, const char *moduleName);
ASTNode *compileModuleFileDefinitions(const char *modulePath, CryoGlobalSymbolTable *globalTable, CompilerState *state);
int compileAndImportModuleToCurrentScope(const char *modulePath, CompilerState *state, CryoGlobalSymbolTable *globalTable);
void importSpecificNamespaces(const char *primaryModule, const char *namespaces[], size_t namespaceCount,
                              CompilerState *state, CryoGlobalSymbolTable *globalTable);

static void cleanupModuleChain(char **names, size_t length);
static void parseModuleChain(Lexer *lexer, struct ModuleChainEntry *moduleChain, size_t *chainLength,
                             ParsingContext *context, Arena *arena,
                             CompilerState *state);
static void parseTypeList(Lexer *lexer, const char *lastModule,
                          ParsingContext *context, Arena *arena, CompilerState *state,
                          CryoGlobalSymbolTable *globalTable);

bool nonCryoFileCheck(const char *fullPath);
// Structures
struct ModuleChainEntry
{
    char *name;
    size_t length;
};

struct TypeEntry
{
    char *name;
    size_t length;
};

// # =========================================================================== #
// # `module` Keyword Parsing
// # =========================================================================== #

ASTNode *parseModuleDeclaration(CryoVisibilityType visibility,
                                Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
int handleModuleParsing(const char *moduleSrcPath, CompilerState *state, CryoGlobalSymbolTable *globalTable, Arena *arena);

const char *getModuleFile(const char **dirList, const char *moduleName);
const char **getDirFileList(const char *dir);
bool isValidCryoFile(const char *fileName);

// # =========================================================================== #
// # `import` Keyword Parsing
// # =========================================================================== #

ASTNode *handleImportParsing(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);
ASTNode *handleRelativeImport(const char *modulePath,
                              Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable);

// # =========================================================================== #
// # Generic Type Parsing
// # =========================================================================== #

GenericType **parseGenericTypeParams(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, int *paramCount);

#endif // PARSER_H
