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

typedef struct GlobalDiagnosticsManager
{
    // -----------------------------------
    // Public Properties

    DiagnosticEntry **errors;
    size_t errorCount;
    size_t errorCapacity;

    StackTrace *stackTrace;

    // -----------------------------------
    // Public Methods

} GlobalDiagnosticsManager;

typedef struct DiagnosticEntry
{
    CryoErrorCode *err;
    CompilerInternalError *internalErr;
    CryoErrorInfo *cryoErrInfo;
    bool isInternalError;
    bool isCryoError;
} DiagnosticEntry;

typedef struct CompilerInternalError
{
    char *message;
    char *filename;
    int line;
    int column;
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
    int column;
} StackFrame;

typedef struct StackTrace
{
    StackFrame **frames;
    size_t frameCount;
    size_t frameCapacity;
} StackTrace;

// =============================================================================
// Initialization & Cleanup

void initGlobalDiagnosticsManager(void);
CryoError *newCryoError(CryoErrorType type, CryoErrorSeverity severity, CryoErrorCode code);
DiagnosticEntry *newDiagnosticEntry(CryoErrorCode *err, CompilerInternalError *internalErr, CryoErrorInfo *cryoErrInfo);
CompilerInternalError *newCompilerInternalError(char *filename, int line, int column, char *message);
CryoErrorInfo *newCryoErrorInfo(char *filename, int line, int column, char *message);

StackFrame *newStackFrame(char *functionName, char *filename, int line, int column);
StackTrace *newStackTrace(void);

// =============================================================================
// Macros

// GDM - Global Diagnostics Manager.
// This macro is used to initialize the global diagnostics manager.
#define INIT_GDM() initGlobalDiagnosticsManager();
// GDM - Global Diagnostics Manager.
#define GDM g_diagnosticsManager
// FLC - File, Line, Column
#define FLC __FILE__, __LINE__, __COLUMN__
// FFLC - Function, File, Line, Column
#define FFLC __FUNCTION__, __FILE__, __LINE__, __COLUMN__

#endif // GLOBAL_DIAGNOSTICS_MANAGER_H
// =============================================================================
// =============================================================================

/*
namespace Cryo
{

    // Forward declarations
    class DiagnosticEntry;

    class GlobalDiagnosticsManager
    {
    public:
        GlobalDiagnosticsManager(const char *buildDir);
        ~GlobalDiagnosticsManager();

        // Error reporting methods
        void reportError(CryoErrorCode code,
                         CryoErrorSeverity severity,
                         const std::string &message,
                         const std::string &filename = "",
                         int line = -1,
                         int column = -1);

        // Error callback registration
        using ErrorCallback = std::function<void(const DiagnosticEntry &)>;
        void registerErrorCallback(ErrorCallback callback);

        // Query methods
        bool hasErrors() const;
        size_t getErrorCount() const;
        void clearErrors();

        // Format and output methods
        std::string formatError(const DiagnosticEntry &entry) const;
        void printErrors() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl; // PIMPL idiom to hide implementation details

        // Disable copy and assignment
        GlobalDiagnosticsManager(const GlobalDiagnosticsManager &) = delete;
        GlobalDiagnosticsManager &operator=(const GlobalDiagnosticsManager &) = delete;
    };

    // Represents a single diagnostic entry
    class DiagnosticEntry
    {
    public:
        DiagnosticEntry(CryoErrorCode code,
                        CryoErrorSeverity severity,
                        std::string message,
                        std::string filename = "",
                        int line = -1,
                        int column = -1);

        CryoErrorCode getCode() const { return code; }
        CryoErrorSeverity getSeverity() const { return severity; }
        const std::string &getMessage() const { return message; }
        const std::string &getFilename() const { return filename; }
        int getLine() const { return line; }
        int getColumn() const { return column; }

    private:
        CryoErrorCode code;
        CryoErrorSeverity severity;
        std::string message;
        std::string filename;
        int line;
        int column;
    };

} // namespace Cryo

*/
