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
#include "diagnostics/diagnostics.h"
#include "tools/logger/logger_config.h"

GlobalDiagnosticsManager *g_diagnosticsManager = NULL;

void FrontendState_setLexer(FrontendState *self, Lexer *lexer)
{
    if (self)
    {
        self->lexer = lexer;
        self->isLexerSet = true;
    }
}

void FrontendState_clearLexer(FrontendState *self)
{
    if (self)
    {
        self->lexer = NULL;
        self->isLexerSet = false;
    }
}

void FrontendState_setSourceCode(FrontendState *self, const char *sourceCode)
{
    if (self)
    {
        self->sourceCode = sourceCode;
    }
}

void FrontendState_setCurrentFile(FrontendState *self, const char *currentFile)
{
    if (self)
    {
        self->currentFile = currentFile;
    }
}

FrontendState *FrontendState_takeSnapshot(FrontendState *self)
{
    FrontendState *snapshot = (FrontendState *)malloc(sizeof(FrontendState));
    if (!snapshot)
    {
        logMessage(LMI, "ERROR", "Diagnostics", "Failed to allocate memory for FrontendState snapshot");
        return NULL;
    }
    snapshot->lexer = self->lexer;
    snapshot->isLexerSet = self->isLexerSet;

    snapshot->sourceCode = self->sourceCode;
    snapshot->currentFile = self->currentFile;

    snapshot->currentLine = self->currentLine;
    snapshot->currentColumn = self->currentColumn;
    snapshot->currentOffset = self->currentOffset;

    return snapshot;
}

FrontendState *newFrontendState(void)
{
    FrontendState *state = (FrontendState *)malloc(sizeof(FrontendState));
    if (!state)
    {
        logMessage(LMI, "ERROR", "Diagnostics", "Failed to allocate memory for FrontendState");
        return NULL;
    }
    state->lexer = NULL;
    state->isLexerSet = false;

    state->sourceCode = NULL;
    state->currentFile = NULL;

    state->setSourceCode = FrontendState_setSourceCode;
    state->setCurrentFile = FrontendState_setCurrentFile;

    state->currentLine = 0;
    state->currentColumn = 0;
    state->currentOffset = 0;

    state->setLexer = FrontendState_setLexer;
    state->clearLexer = FrontendState_clearLexer;

    state->takeSnapshot = FrontendState_takeSnapshot;

    return state;
}

void GlobalDiagnosticsManager_initFrontendState(GlobalDiagnosticsManager *self)
{
    if (self->frontendState == NULL)
    {
        self->frontendState = (FrontendState *)malloc(sizeof(FrontendState));
        self->frontendState->lexer = NULL;
        self->frontendState->isLexerSet = false;
    }
}

CompilerInternalError *newCompilerInternalError(char *function, char *filename, int line, char *message)
{
    CompilerInternalError *error = (CompilerInternalError *)malloc(sizeof(CompilerInternalError));
    if (!error)
    {
        logMessage(LMI, "ERROR", "Diagnostics", "Failed to allocate memory for CompilerInternalError");
        return NULL;
    }
    error->function = function;
    error->filename = filename;
    error->line = line;
    error->message = message;

    return error;
}

void GlobalDiagnosticsManager_debugPrintCurrentState(GlobalDiagnosticsManager *self)
{
    if (self->frontendState != NULL)
    {
        logMessage(LMI, "DEBUG", "Diagnostics", "Current Frontend State: Lexer: %p, IsLexerSet: %d",
                   self->frontendState->lexer, self->frontendState->isLexerSet);
    }
    else
    {
        logMessage(LMI, "DEBUG", "Diagnostics", "Frontend State is NULL");
    }
}

void GlobalDiagnosticsManager_reportDiagnostic(GlobalDiagnosticsManager *self,
                                               DiagnosticSeverity severity,
                                               const char *message,
                                               const char *function,
                                               const char *file,
                                               int line)
{
    // Create a diagnostic entry
    DiagnosticEntry *entry = NULL;

    if (severity == DIAG_FATAL)
    {
        // For fatal, create an internal error that will trigger immediate exit
        CompilerInternalError *internalErr = newCompilerInternalError(
            (char *)function, (char *)file, line, (char *)message);

        entry = newDiagnosticEntry(CRYO_ERROR_UNKNOWN, internalErr, NULL);
        entry->isInternalError = true;
        entry->isCryoError = false;
    }
    else
    {
        // For non-fatal, create a CryoErrorInfo
        CryoErrorInfo *errorInfo = newCryoErrorInfo(
            (char *)file, line, 0, (char *)message);

        // Create error code based on severity
        CryoError *err = NULL;
        CryoErrorCode errCode = CRYO_ERROR_UNKNOWN;
        switch (severity)
        {
        case DIAG_NOTE:
            err = newCryoError(CRYO_ERROR_TYPE_GENERAL, CRYO_SEVERITY_NOTE, CRYO_ERROR_UNKNOWN);
            break;
        case DIAG_WARNING:
            err = newCryoError(CRYO_ERROR_TYPE_GENERAL, CRYO_SEVERITY_WARNING, CRYO_ERROR_UNKNOWN);
            break;
        case DIAG_ERROR:
            err = newCryoError(CRYO_ERROR_TYPE_GENERAL, CRYO_SEVERITY_ERROR, CRYO_ERROR_UNKNOWN);
            break;
        default:
            break;
        }

        entry = newDiagnosticEntry(errCode, NULL, errorInfo);
        entry->isInternalError = false;
        entry->isCryoError = true;
    }

    // Add error to the list
    if (self->errors == NULL)
    {
        self->errorCapacity = ERROR_CAPACITY;
        self->errors = (DiagnosticEntry **)malloc(sizeof(DiagnosticEntry *) * self->errorCapacity);
    }
    else if (self->errorCount >= self->errorCapacity)
    {
        // Double capacity if needed
        self->errorCapacity *= 2;
        self->errors = (DiagnosticEntry **)realloc(self->errors,
                                                   sizeof(DiagnosticEntry *) * self->errorCapacity);
    }

    self->errors[self->errorCount++] = entry;

    // For fatal errors, print the stack trace and exit immediately
    if (severity == DIAG_FATAL)
    {
        self->printStackTrace(self);

        // Print the error message
        printf(BOLD RED "\n#========================================================================#\n" COLOR_RESET);
        printf("\n<!> Fatal Error! Exiting...");
        printf("\n<!> %s", message);
        printf("\n<!> Line: %i, Function: %s", line, function);
        printf("\n<!> File: %s\n", file);
        printf(BOLD RED "\n#========================================================================#\n\n" COLOR_RESET);

        exit(1);
    }
}

void GlobalDiagnosticsManager_reportInternalError(GlobalDiagnosticsManager *self,
                                                  const char *message,
                                                  const char *function,
                                                  const char *file,
                                                  int line)
{
    // Internal errors are always fatal
    GlobalDiagnosticsManager_reportDiagnostic(self, DIAG_FATAL, message, function, file, line);
}

bool GlobalDiagnosticsManager_hasFatalErrors(GlobalDiagnosticsManager *self)
{
    for (size_t i = 0; i < self->errorCount; i++)
    {
        DiagnosticEntry *entry = self->errors[i];
        if (entry->isInternalError ||
            (entry->isCryoError && entry->severity == CRYO_SEVERITY_ERROR))
        {
            return true;
        }
    }
    return false;
}

void GlobalDiagnosticsManager_printDiagnostics(GlobalDiagnosticsManager *self)
{
    if (self->errorCount == 0)
    {
        return;
    }

    printf("\n#======== Compiler Diagnostics ========#\n");

    for (size_t i = 0; i < self->errorCount; i++)
    {
        DiagnosticEntry *entry = self->errors[i];

        if (entry->isInternalError)
        {
            printf(BOLD RED "INTERNAL ERROR: %s\n" COLOR_RESET, entry->internalErr->message);
            printf("  at %s:%d in function %s\n",
                   entry->internalErr->filename,
                   entry->internalErr->line,
                   entry->internalErr->function);
        }
        else if (entry->isCryoError)
        {
            const char *severity = "NOTE";
            const char *color = YELLOW;

            if (entry->severity == CRYO_SEVERITY_WARNING)
            {
                severity = "WARNING";
                color = YELLOW;
            }
            else if (entry->severity == CRYO_SEVERITY_ERROR)
            {
                severity = "ERROR";
                color = RED;
            }
            else if (entry->severity == CRYO_SEVERITY_FATAL)
            {
                severity = "FATAL";
                color = BOLD RED;
            }

            printf("%s%s: %s\n" COLOR_RESET, color, severity, entry->cryoErrInfo->message);
            printf("  at %s:%d\n",
                   entry->cryoErrInfo->filename,
                   entry->cryoErrInfo->line);
        }
    }

    printf("\n#=======================================#\n");
}

__C_CONSTRUCTOR__
void initGlobalDiagnosticsManager(void)
{
    if (g_diagnosticsManager == NULL)
    {
        g_diagnosticsManager = (GlobalDiagnosticsManager *)malloc(sizeof(GlobalDiagnosticsManager));
        GDM->errorCount = 0;
        GDM->errors = NULL;
        GDM->stackTrace = newStackTrace();

        GDM->frontendState = newFrontendState();
        GDM->initFrontendState = GlobalDiagnosticsManager_initFrontendState;
        GDM->debugPrintCurrentState = GlobalDiagnosticsManager_debugPrintCurrentState;

        GDM->printStackTrace = print_stack_trace;
        GDM->createStackFrame = create_stack_frame;
    }
}
