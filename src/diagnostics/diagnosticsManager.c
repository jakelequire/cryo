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

ModuleFileCache *newModuleFileCache(const char *moduleName, const char *fileName, const char *fileContents)
{
    ModuleFileCache *cache = (ModuleFileCache *)malloc(sizeof(ModuleFileCache));
    if (!cache)
    {
        logMessage(LMI, "ERROR", "Diagnostics", "Failed to allocate memory for ModuleFileCache");
        return NULL;
    }
    cache->moduleName = moduleName;
    cache->fileName = fileName;
    cache->fileContents = fileContents;

    return cache;
}

void GlobalDiagnosticsManager_addModuleFileCache(GlobalDiagnosticsManager *self, const char *moduleName,
                                                 const char *fileName, const char *fileContents)
{
    if (self->moduleFileCacheCount >= self->moduleFileCacheCapacity)
    {
        self->moduleFileCacheCapacity *= 2;
        self->moduleFileCache = (ModuleFileCache **)realloc(self->moduleFileCache, sizeof(ModuleFileCache *) * self->moduleFileCacheCapacity);
    }
    ModuleFileCache *cache = newModuleFileCache(moduleName, fileName, fileContents);
    if (!cache)
    {
        logMessage(LMI, "ERROR", "Diagnostics", "Failed to create ModuleFileCache");
        return;
    }
    self->moduleFileCache[self->moduleFileCacheCount++] = cache;
}

ModuleFileCache *GlobalDiagnosticsManager_getModuleFileCache(GlobalDiagnosticsManager *self, const char *moduleName)
{
    for (size_t i = 0; i < self->moduleFileCacheCount; i++)
    {
        if (strcmp(self->moduleFileCache[i]->moduleName, moduleName) == 0)
        {
            return self->moduleFileCache[i];
        }
    }
    return NULL;
}

void GlobalDiagnosticsManager_printModuleFileCache(GlobalDiagnosticsManager *self)
{
    printf("------------------- [ Module File Cache ] -------------------\n");
    for (size_t i = 0; i < self->moduleFileCacheCount; i++)
    {
        ModuleFileCache *cache = self->moduleFileCache[i];
        printf("Module Name: %s\n", cache->moduleName);
        printf("File Name: %s\n", cache->fileName);
        printf("---------------------------------------------------------------\n");
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

void FrontendState_setLexer(FrontendState *self, Lexer *lexer)
{
    if (self)
    {
        if (self->lexer)
        {
            self->clearLexer(self);
        }
        self->lexer = lexer;
        self->isLexerSet = true;
    }
}

void FrontendState_incrementLine(FrontendState *self, size_t amount)
{
    if (self)
    {
        self->currentLine = amount;
    }
}
void FrontendState_incrementColumn(FrontendState *self, size_t amount)
{
    if (self)
    {
        self->currentColumn = amount;
    }
}
void FrontendState_incrementOffset(FrontendState *self, size_t amount)
{
    if (self)
    {
        self->currentOffset += amount;
    }
}

void FrontendState_setSourceCode(FrontendState *self, const char *sourceCode)
{
    if (self)
    {
        if (self->sourceCode)
        {
            self->sourceCode = NULL;
        }
        self->sourceCode = sourceCode;
    }
}

void FrontendState_setCurrentFile(FrontendState *self, const char *currentFile)
{
    if (self)
    {
        if (self->currentFile)
        {
            self->currentFile = NULL;
        }
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

    state->incrementLine = FrontendState_incrementLine;
    state->incrementColumn = FrontendState_incrementColumn;
    state->incrementOffset = FrontendState_incrementOffset;

    state->printErrorScreen = FrontendState_printErrorScreen;

    state->takeSnapshot = FrontendState_takeSnapshot;

    return state;
}

void GlobalDiagnosticsManager_initFrontendState(GlobalDiagnosticsManager *self)
{
    if (self->frontendState == NULL)
    {
        self->frontendState = newFrontendState();
        if (self->frontendState == NULL)
        {
            logMessage(LMI, "ERROR", "Diagnostics", "Failed to initialize FrontendState");
            return;
        }
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

void GlobalDiagnosticsManager_reportInternalError(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                                  const char *message, const char *function, const char *file, int line)
{
    // Internal errors are always fatal
    CompilerInternalError *internalErr = newCompilerInternalError(
        (char *)function, (char *)file, line, (char *)message);
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

void GlobalDiagnosticsManager_reportDiagnostic(GlobalDiagnosticsManager *self, CryoErrorCode errorCode, CryoErrorSeverity severity,
                                               const char *message, const char *function, const char *file, int line)
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
    self->addError(self, entry);

    // Print the `FrontendState_printErrorScreen`
    if (self->frontendState != NULL)
    {
        self->frontendState->printErrorScreen(self->frontendState, errorCode, severity,
                                              message, file, function, line);
    }
    else
    {
        printf("Error: FrontendState is NULL\n");
    }

    exit(EXIT_FAILURE);
}

void GlobalDiagnosticsManager_reportBackendDiagnostic(GlobalDiagnosticsManager *self,
                                                      CryoErrorCode errorCode, CryoErrorSeverity severity,
                                                      const char *moduleName, ASTNode *failedNode,
                                                      const char *message, const char *function, const char *file, int line)
{
    // TODO: Implement this function
    DEBUG_BREAKPOINT;
}

void GlobalDiagnosticsManager_reportConditionFailed(GlobalDiagnosticsManager *self,
                                                    CryoErrorCode errorCode, CryoErrorSeverity severity,
                                                    const char *message, const char *function, const char *file, int line)
{
    DiagnosticEntry *entry = newDiagnosticEntry(errorCode, NULL, NULL);
    entry->isInternalError = true;
    entry->isCryoError = false;
    entry->internalErr = newCompilerInternalError(
        (char *)function, (char *)file, line, (char *)message);
    entry->severity = CRYO_SEVERITY_INTERNAL;
    // Add error to the list
    self->addError(self, entry);
    // Print the stack trace
    self->printStackTrace(self);
    // Print the error message
    if (self->frontendState != NULL)
    {
        self->frontendState->printErrorScreen(self->frontendState, errorCode, severity,
                                              message, file, function, line);
    }
    else
    {
        printf("Error: FrontendState is NULL\n");
    }
    // Exit the program
    exit(EXIT_FAILURE);
}

void GlobalDiagnosticsManager_reportDebugBreakpoint(GlobalDiagnosticsManager *self,
                                                    const char *message, const char *function, const char *file, int line,
                                                    ...)
{
    int MESSAGE_SIZE = 5120;
    char formattedMessage[MESSAGE_SIZE];
    va_list args;
    va_start(args, line);
    if (message == NULL)
    {
        snprintf(formattedMessage, MESSAGE_SIZE, "<DEBUG BREAKPOINT> Internal Compiler Breakpoint.");
    }
    else
    {
        snprintf(formattedMessage, MESSAGE_SIZE, "<DEBUG BREAKPOINT> %s", message);
    }
    va_end(args);

    DiagnosticEntry *entry = newDiagnosticEntry(CRYO_ERROR_DEBUG_BREAKPOINT, NULL, NULL);
    entry->isInternalError = true;
    entry->isCryoError = false;
    entry->internalErr = newCompilerInternalError(
        (char *)function, (char *)file, line, (char *)message);
    entry->severity = CRYO_SEVERITY_INTERNAL;

    // Add error to the list
    self->addError(self, entry);

    // Print the stack trace
    self->printStackTrace(self);

    // Print the error message
    if (self->frontendState != NULL)
    {
        self->frontendState->printErrorScreen(self->frontendState, CRYO_ERROR_DEBUG_BREAKPOINT,
                                              CRYO_SEVERITY_INTERNAL, formattedMessage, file, function, line);
    }
    else
    {
        printf("Error: FrontendState is NULL\n");
    }

    // Exit the program
    exit(EXIT_FAILURE);
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

const char *CompilationStageToString(CompilationStage stage)
{
    switch (stage)
    {
    case COMPILATION_STAGE_NONE:
        return "None";
    case COMPILATION_STAGE_FRONTEND:
        return "Frontend";
    case COMPILATION_STAGE_SEMANTIC_ANALYSIS:
        return "Semantic Analysis";
    case COMPILATION_STAGE_CODEGEN:
        return "Code Generation";
    case COMPILATION_STAGE_LINKING:
        return "Linking";
    case COMPILATION_STAGE_EXECUTION:
        return "Execution";
    default:
        return "Unknown";
    }
}

void GlobalDiagnosticsManager_setCompilationStage(GlobalDiagnosticsManager *self, CompilationStage stage)
{
    if (self)
    {
        self->stage = stage;
    }
}

void GlobalDiagnosticsManager_addError(GlobalDiagnosticsManager *self, DiagnosticEntry *entry)
{
    if (self->errorCount >= self->errorCapacity)
    {
        // Double capacity if needed
        self->errorCapacity *= 2;
        self->errors = (DiagnosticEntry **)realloc(self->errors,
                                                   sizeof(DiagnosticEntry *) * self->errorCapacity);
    }

    self->errors[self->errorCount++] = entry;
}

__C_CONSTRUCTOR__
void initGlobalDiagnosticsManager(void)
{
    if (g_diagnosticsManager == NULL)
    {
        g_diagnosticsManager = (GlobalDiagnosticsManager *)malloc(sizeof(GlobalDiagnosticsManager));
        GDM->errorCount = 0;
        GDM->errors = NULL;
        GDM->errorCapacity = 0;
        GDM->addError = GlobalDiagnosticsManager_addError;
        GDM->stackTrace = newStackTrace();

        GDM->moduleFileCacheCount = 0;
        GDM->moduleFileCacheCapacity = 16;
        GDM->moduleFileCache = (ModuleFileCache **)malloc(sizeof(ModuleFileCache *) * GDM->moduleFileCacheCapacity);
        GDM->addModuleFileCache = GlobalDiagnosticsManager_addModuleFileCache;
        GDM->getModuleFileCache = GlobalDiagnosticsManager_getModuleFileCache;
        GDM->printModuleFileCache = GlobalDiagnosticsManager_printModuleFileCache;

        GDM->stage = COMPILATION_STAGE_NONE;
        GDM->setCompilationStage = GlobalDiagnosticsManager_setCompilationStage;

        GDM->frontendState = newFrontendState();
        GDM->initFrontendState = GlobalDiagnosticsManager_initFrontendState;
        GDM->debugPrintCurrentState = GlobalDiagnosticsManager_debugPrintCurrentState;

        GDM->printStackTrace = print_stack_trace;
        GDM->createStackFrame = create_stack_frame;

        GDM->reportDiagnostic = GlobalDiagnosticsManager_reportDiagnostic;
        GDM->reportInternalError = GlobalDiagnosticsManager_reportInternalError;
        GDM->reportBackendDiagnostic = GlobalDiagnosticsManager_reportBackendDiagnostic;
        GDM->reportConditionFailed = GlobalDiagnosticsManager_reportConditionFailed;
        GDM->reportDebugBreakpoint = GlobalDiagnosticsManager_reportDebugBreakpoint;

        GDM->hasFatalErrors = GlobalDiagnosticsManager_hasFatalErrors;
        GDM->printDiagnostics = GlobalDiagnosticsManager_printDiagnostics;
    }
}

// ==========================================================================
// ==========================================================================

void DIAG_DEBUG_BREAKPOINT(char *function, char *file, int line, GlobalDiagnosticsManager *self, const char *message)
{
    _P_DIAG_DEBUG_BREAKPOINT(function, file, line, self, message);
}
