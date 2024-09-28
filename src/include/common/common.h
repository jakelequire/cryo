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
#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>

#include "compiler/ast.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/symtable.h"
#include "utils/utility.h"
#include "utils/arena.h"
#include "utils/fs.h"

typedef struct CryoSymbolTable CryoSymbolTable;
typedef struct Lexer Lexer;
typedef struct Token Token;
typedef struct Arena Arena;
typedef struct ASTNode ASTNode;

/*================================*/

typedef enum ErrorType
{
    ERROR,
    WARNING,
    INFO,
    UNKNOWN
} ErrorType;

typedef struct InternalDebug
{
    const char *functionName;
    const char *fileName;
    int lineNumber;
} InternalDebug;

#define INTERAL_DEBUG_INFO \
    (InternalDebug) { __FUNCTION__, __FILE__, __LINE__ }

#define CAPTURE_INTERNAL_DEBUG \
    captureInternalDebug(__FUNCTION__, __FILE__, __LINE__)

typedef struct CompilerError
{
    ErrorType type;
    InternalDebug debug;
    char *message;
    char *detail;
    int lineNumber;
    int column;
    char *fileName;
    char *functionName;
} CompilerError;

typedef struct CompilerState
{
    struct Arena *arena;
    struct Lexer *lexer;
    struct CryoSymbolTable *table;
    struct ASTNode *programNode;
    struct ASTNode *currentNode;
    const char *fileName;
    int lineNumber;
    int columnNumber;
    int errorCount;
    CompilerError **errors;
} CompilerState;

#define GET_SOURCE_INFO \
    lexer->line, lexer->column, lexer->fileName

#define NEW_COMPILER_ERROR(type, message, detail) \
    createError(CAPTURE_INTERNAL_DEBUG, type, message, detail, GET_SOURCE_INFO)

CompilerState initCompilerState(Arena *arena, Lexer *lexer, CryoSymbolTable *table, const char *fileName);
void updateCompilerLineNumber(Lexer *lexer, CompilerState *state);
void updateCompilerColumnNumber(Lexer *lexer, CompilerState *state);
CompilerState addProgramNodeToState(CompilerState state, ASTNode *programNode);

InternalDebug captureInternalDebug(const char *functionName, const char *fileName, int lineNumber);
CompilerError initNewError(InternalDebug debug);
CompilerError createError(InternalDebug internals, const char *type, const char *message, const char *detail, int lineNumber, int column, const char *fileName);

void errorReport(CompilerState state);
void logCompilerError(CompilerError *error);
char *getErrorTypeString(ErrorType type);

void dumpCompilerState(CompilerState state);
/*================================*/
// Macros

/// @brief A macro to break the program at a specific point for debugging.
#define DEBUG_BREAKPOINT                                                                        \
    printf("\n#========================================================================#\n");   \
    printf("\n<!> Debug Breakpoint! Exiting...");                                               \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                             \
    printf("\n<!> File: %s\n", __FILE__);                                                       \
    printf("\n#========================================================================#\n\n"); \
    exit(0)

#define CONDITION_FAILED                                                                      \
    printf("\n#========================================================================#\n"); \
    printf("\n<!> Condition Failed! Exiting...");                                             \
    printf("\n<!> Line: %i, Function: %s", __LINE__, __FUNCTION__);                           \
    printf("\n<!> File: %s\n", __FILE__);                                                     \
    printf("\n#========================================================================#\n"); \
    exit(1)

/*================================*/
// Function Prototypes

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct ASTNode ASTNode;

    int globalCompiler(const char *source);
    ASTNode *getProgramNode(const char *source);

#ifdef __cplusplus
}
#endif

typedef struct ASTNode ASTNode;

int globalCompiler(const char *source);
ASTNode *getProgramNode(const char *source);

#endif // COMMON_H
