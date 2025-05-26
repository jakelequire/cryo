/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#ifndef GLOBAL_DIAGNOSTICS_MANAGER_H
#define GLOBAL_DIAGNOSTICS_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "errorCodes.h"
#include "tools/utils/cTypes.h"
#include "tools/utils/cWrappers.h"
#include "tools/utils/attributes.h"
#include "tools/macros/consoleColors.h"

#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/AST.h"

typedef struct ASTNode ASTNode;
typedef struct Lexer Lexer;

#define STACK_TRACE_CAPACITY 16
#define ERROR_CAPACITY 100

// =============================================================================
// Structures & Enums

typedef struct GlobalDiagnosticsManager GlobalDiagnosticsManager;
typedef struct DiagnosticEntry DiagnosticEntry;
typedef struct CompilerInternalError CompilerInternalError;
typedef struct CryoErrorInfo CryoErrorInfo;
typedef struct StackFrame StackFrame;
typedef struct StackTrace StackTrace;

// The global diagnostics manager
extern GlobalDiagnosticsManager *g_diagnosticsManager;

// Add to diagnostics.h
typedef enum DiagnosticSeverity
{
    DIAG_NOTE,
    DIAG_WARNING,
    DIAG_ERROR,
    DIAG_FATAL
} DiagnosticSeverity;

typedef enum CompilationStage
{
    COMPILATION_STAGE_NONE,
    COMPILATION_STAGE_FRONTEND,
    COMPILATION_STAGE_SEMANTIC_ANALYSIS,
    COMPILATION_STAGE_CODEGEN,
    COMPILATION_STAGE_LINKING,
    COMPILATION_STAGE_EXECUTION
} CompilationStage;

typedef struct FrontendState_t
{
    Lexer *lexer;    // The current lexer
    bool isLexerSet; // Flag to check if the lexer is set

    const char *sourceCode;  // The source code being parsed
    const char *currentFile; // The current file being parsed
    void (*setSourceCode)(struct FrontendState_t *self, const char *sourceCode);
    void (*setCurrentFile)(struct FrontendState_t *self, const char *currentFile);

    // The current position in the source code
    size_t currentLine;
    size_t currentColumn;
    size_t currentOffset;

    void (*setLexer)(struct FrontendState_t *self, Lexer *lexer);
    void (*clearLexer)(struct FrontendState_t *self);

    void (*incrementLine)(struct FrontendState_t *self, size_t amount);
    void (*incrementColumn)(struct FrontendState_t *self, size_t amount);
    void (*incrementOffset)(struct FrontendState_t *self, size_t amount);

    void (*printErrorScreen)(struct FrontendState_t *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                             const char *message, const char *compiler_file, const char *compiler_function, int compiler_line);

    struct FrontendState_t *(*takeSnapshot)(struct FrontendState_t *self);
} FrontendState;

typedef struct DiagnosticEntry
{
    CryoErrorCode err;
    CompilerInternalError *internalErr;
    CryoErrorInfo *cryoErrInfo;
    CryoErrorSeverity severity;
    bool isInternalError;
    bool isCryoError;
} DiagnosticEntry;

typedef struct CompilerInternalError
{
    char *message;
    char *filename;
    char *function;
    int line;
} CompilerInternalError;

typedef struct CryoErrorInfo
{
    char *message;
    char *filename;
    int line;
    int column;
} CryoErrorInfo;

typedef struct StackFrame
{
    char *functionName;
    char *filename;
    int line;
} StackFrame;

typedef struct StackTrace
{
    StackFrame **frames;
    size_t frameCount;
    size_t frameCapacity;

    // `void push(*self, StackFrame *frame)` - Push a new stack frame onto the stack trace
    _NEW_METHOD(void, push, StackFrame *frame);
} StackTrace;

typedef struct ModuleFileCache_t
{
    const char *moduleName;
    const char *fileName;
    const char *fileContents;
} ModuleFileCache;

typedef struct GlobalDiagnosticsManager
{
    // The current compilation stage
    CompilationStage stage;
    void (*setCompilationStage)(GlobalDiagnosticsManager *self, CompilationStage stage);

    DiagnosticEntry **errors; // Array of error entries
    size_t errorCount;        // Number of errors
    size_t errorCapacity;     // Capacity of the error array

    ModuleFileCache **moduleFileCache; // Array of module file caches
    size_t moduleFileCacheCount;       // Number of module file caches
    size_t moduleFileCacheCapacity;    // Capacity of the module file cache array
    void (*addModuleFileCache)(GlobalDiagnosticsManager *self, const char *moduleName,
                               const char *fileName, const char *fileContents);
    ModuleFileCache *(*getModuleFileCache)(GlobalDiagnosticsManager *self, const char *moduleName);
    void (*printModuleFileCache)(GlobalDiagnosticsManager *self);

    void (*addError)(GlobalDiagnosticsManager *self, DiagnosticEntry *entry);

    StackTrace *stackTrace; // The current stack trace

    FrontendState *frontendState; // The current frontend state
    void (*initFrontendState)(GlobalDiagnosticsManager *self);
    void (*debugPrintCurrentState)(GlobalDiagnosticsManager *self);

    bool (*hasFatalErrors)(GlobalDiagnosticsManager *self);
    void (*printDiagnostics)(GlobalDiagnosticsManager *self);
    void (*createStackFrame)(GlobalDiagnosticsManager *self, char *functionName, char *filename, int line);
    void (*printStackTrace)(GlobalDiagnosticsManager *self);

    void (*reportDiagnostic)(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                             const char *message, const char *function, const char *file, int line);
    void (*reportInternalError)(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                const char *message, const char *function, const char *file, int line);
    void (*reportBackendDiagnostic)(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                    const char *moduleName, ASTNode *failedNode,
                                    const char *message, const char *function, const char *file, int line);
    void (*reportConditionFailed)(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                  const char *message, const char *function, const char *file, int line);
    void (*reportDebugBreakpoint)(GlobalDiagnosticsManager *self,
                                  const char *message, const char *function, const char *file, int line,
                                  ...);
} GlobalDiagnosticsManager;

// =============================================================================
// Initialization & Cleanup

__C_CONSTRUCTOR__
void initGlobalDiagnosticsManager(void);

CryoError *newCryoError(CryoErrorType type, CryoErrorSeverity severity, CryoErrorCode code);
DiagnosticEntry *newDiagnosticEntry(CryoErrorCode err, CompilerInternalError *internalErr, CryoErrorInfo *cryoErrInfo);
CompilerInternalError *newCompilerInternalError(char *function, char *filename, int line, char *message);
CryoErrorInfo *newCryoErrorInfo(char *filename, int line, int column, char *message);
FrontendState *newFrontendState(void);

StackFrame *newStackFrame(char *functionName, char *filename, int line);
StackTrace *newStackTrace(void);

void FrontendState_printErrorScreen(struct FrontendState_t *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                    const char *message, const char *compiler_file, const char *compiler_function, int compiler_line);

void GlobalDiagnosticsManager_printASTErrorScreen(GlobalDiagnosticsManager *self,
                                                  CryoErrorCode errorCode, CryoErrorSeverity severity,
                                                  const char *moduleName, ASTNode *failedNode,
                                                  const char *message,
                                                  const char *compiler_file, const char *compiler_function, int compiler_line);
// =============================================================================
// Implementation Methods

void addLexer(GlobalDiagnosticsManager *self, Lexer *lexer);
void create_stack_frame(GlobalDiagnosticsManager *self, char *functionName, char *filename, int line);
void print_stack_trace(GlobalDiagnosticsManager *self);

// =============================================================================
// Helper Functions

void dyn_stackframe_push(StackFrame *frame);
const char *CryoErrorCodeToString(CryoErrorCode errorCode);
const char *CryoErrorSeverityToString(CryoErrorSeverity severity);

// =============================================================================
// Macros

// GDM - Global Diagnostics Manager.
// This macro is used to initialize the global diagnostics manager.
#define INIT_GDM() initGlobalDiagnosticsManager();
// GDM - Global Diagnostics Manager.
#define GDM g_diagnosticsManager

// _FL_ - File, Line
#define _FL_ __FILE__, __LINE__
// _FFL_ - Function, File, Line
#define _FFL_ (char *)__func__, (char *)__FILE__, (int)__LINE__
// ------------------------------------------------------------------

#define NEW_STACK_FRAME(FN, F, L) \
    GDM->createStackFrame(GDM, FN, F, L);
#define __STACK_FRAME__ GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
#define __DUMP_STACK_TRACE__ GDM->printStackTrace(GDM);

// GDM: Global Diagnostics Manager
// ERR: Error Code
// SEV: Error Severity
// MSG: Error Message
// LN: Line Number (COMPILER)
// FL: File Name (COMPILER)
// FN: Function Name (COMPILER)
#define NEW_ERROR(GDM, ERR, SEV, MSG, LN, FL, FN) \
    GDM->reportDiagnostic(GDM, ERR, SEV, MSG, FN, FL, LN);

// GDM: Global Diagnostics Manager
// ERR: Error Code
// SEV: Error Severity
// MSG: Error Message
// NODE: AST Node
// LN: Line Number (COMPILER)
// FL: File Name (COMPILER)
// FN: Function Name (COMPILER)
#define NEW_BACKEND_ERROR(GDM, ERR, SEV, MSG, NODE, LN, FL, FN) \
    GDM->reportBackendDiagnostic(GDM, ERR, SEV, MSG, NODE, FN, FL, LN);

// GDM: Global Diagnostics Manager
// ERR: Error Code
// SEV: Error Severity
// MSG: Error Message
// LN: Line Number (COMPILER)
// FL: File Name (COMPILER)
// FN: Function Name (COMPILER)
#define DIAG_CONDITION_FAILED(GDM, ERR, SEV, MSG, LN, FL, FN) \
    GDM->reportConditionFailed(GDM, ERR, SEV, MSG, FN, FL, LN);

// FN: Function Name (COMPILER)
// FL: File Name (COMPILER)
// LN: Line Number (COMPILER)
// GDM: Global Diagnostics Manager
// MSG: Error Message
#define _P_DIAG_DEBUG_BREAKPOINT(FN, FL, LN, GDM, MSG, ...) \
    GDM->reportDebugBreakpoint(GDM, MSG, FN, FL, LN, ##__VA_ARGS__);

///
/// @brief This is created as a function to to allow for the use of the `_FFL_` macro for shorter syntax.
///
/// Example usage:
/// ```c
///     DIAG_DEBUG_BREAKPOINT(__FFL__, GDM, "Debug Breakpoint Hit!");
/// ```
///
void DIAG_DEBUG_BREAKPOINT(char *function, char *file, int line, GlobalDiagnosticsManager *self, const char *message);

#endif // GLOBAL_DIAGNOSTICS_MANAGER_H
// =============================================================================
