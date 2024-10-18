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

#include "settings.h"

typedef struct CryoSymbolTable CryoSymbolTable;
typedef struct CompilerSettings CompilerSettings;
typedef struct Lexer Lexer;
typedef struct Token Token;
typedef struct Arena Arena;
typedef struct ASTNode ASTNode;

/*================================*/

typedef struct CompiledFile
{
    const char *fileName;
    const char *filePath;
    const char *outputPath;
} CompiledFile;

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
    bool isActiveBuild;
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
    bool isActiveBuild;
    CompilerSettings *settings;
    int errorCount;
    CompilerError **errors;
    // Functions for debugging
    void (*errorReport)(struct CompilerState);
    void (*logCompilerError)(CompilerError *);
    void (*dumpCompilerState)(struct CompilerState);
} CompilerState;

#define GET_SOURCE_INFO \
    lexer->line, lexer->column, lexer->fileName

#define NEW_COMPILER_ERROR(state, type, message, detail) \
    createError(CAPTURE_INTERNAL_DEBUG, state, type, message, detail, GET_SOURCE_INFO)

CompilerState *initCompilerState(Arena *arena, Lexer *lexer, CryoSymbolTable *table, const char *fileName);
void updateCompilerLineNumber(Lexer *lexer, CompilerState *state);
void updateCompilerColumnNumber(Lexer *lexer, CompilerState *state);
CompilerState addProgramNodeToState(CompilerState state, ASTNode *programNode);

InternalDebug captureInternalDebug(const char *functionName, const char *fileName, int lineNumber);
CompilerError initNewError(InternalDebug debug);
CompilerError createError(InternalDebug internals, CompilerState *state, const char *type, const char *message, const char *detail, int lineNumber, int column, const char *fileName);

void errorReport(CompilerState state);
void logCompilerError(CompilerError *error);
char *getErrorTypeString(ErrorType type);

void dumpCompilerState(CompilerState state);
void dumpSymbolTable(CompilerState state);

// Function Prototypes for C++ Calls
#ifdef __cplusplus
extern "C"
{
    struct CompiledFile;
    void dumpCompilerStateCXX(CompilerState state);
    void dumpSymbolTableCXX(CompilerState state);
    CompiledFile compileFileCXX(const char *filePath, const char *compilerFlags);
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    CompiledFile compileFile(const char *filePath, const char *compilerFlags);

#ifdef __cplusplus
}
#endif

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

#define START_COMPILATION_MESSAGE                                                              \
    printf("\n\n\n");                                                                          \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("<!> ### - - - - - - - - - - - Start of Compilation - - - - - - - - -  ### <!>\n"); \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("\n\n\n");

#define END_COMPILATION_MESSAGE                                                                \
    printf("\n\n\n");                                                                          \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("<!> ### - - - - - - - - - - - End of Compilation - - - - - - - - - -  ### <!>\n"); \
    printf("<!> ### ============================================================= ### <!>\n"); \
    printf("\n\n\n");

#define PRINT_AST_START                                                                \
    printf("\n\n\n");                                                                  \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("<> ~~~                     AST Tree Output                     ~~~ <>\n"); \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("\n");

#define PRINT_AST_END                                                                  \
    printf("\n");                                                                      \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("<> ~~~                 End of AST Tree Output                  ~~~ <>\n"); \
    printf("<> ~~~ ======================================================= ~~~ <>\n"); \
    printf("\n");
#endif // COMMON_H