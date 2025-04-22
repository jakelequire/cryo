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

/*
Error Code Template:
    X0-000000
    X = Error Type
        E = General Error
        F = Frontend Error
        S = Semantic Error
        L = Linker Error
        I = Internal Error
    0 = Error Severity
        0 = Note
        1 = Warning
        2 = Error
        3 = Fatal
    000000 = Error Number
*/

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

    struct FrontendState_t *(*takeSnapshot)(struct FrontendState_t *self);
} FrontendState;

typedef struct GlobalDiagnosticsManager
{
    // -----------------------------------
    // Public Properties

    DiagnosticEntry **errors; // Array of error entries
    size_t errorCount;        // Number of errors
    size_t errorCapacity;     // Capacity of the error array

    StackTrace *stackTrace; // The current stack trace

    FrontendState *frontendState; // The current frontend state
    void (*initFrontendState)(GlobalDiagnosticsManager *self);
    void (*debugPrintCurrentState)(GlobalDiagnosticsManager *self);

    // Add to GlobalDiagnosticsManager struct
    void (*reportDiagnostic)(GlobalDiagnosticsManager *self, DiagnosticSeverity severity,
                             const char *message, const char *function, const char *file, int line);
    void (*reportInternalError)(GlobalDiagnosticsManager *self, const char *message,
                                const char *function, const char *file, int line);
    bool (*hasFatalErrors)(GlobalDiagnosticsManager *self);
    void (*printDiagnostics)(GlobalDiagnosticsManager *self);

    // -----------------------------------
    // Public Methods

    void (*createStackFrame)(GlobalDiagnosticsManager *self, char *functionName, char *filename, int line);

    // `void printStackTrace(*self)`
    void (*printStackTrace)(GlobalDiagnosticsManager *self);
} GlobalDiagnosticsManager;

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

// =============================================================================
// Implementation Methods

void addLexer(GlobalDiagnosticsManager *self, Lexer *lexer);
void create_stack_frame(GlobalDiagnosticsManager *self, char *functionName, char *filename, int line);
void print_stack_trace(GlobalDiagnosticsManager *self);

// =============================================================================
// Helper Functions

void dyn_stackframe_push(StackFrame *frame);

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

#endif // GLOBAL_DIAGNOSTICS_MANAGER_H
// =============================================================================
