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
#include "common/common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int generateCodeWrapper(ASTNode *node, CompilerState *state);

#ifdef __cplusplus
}
#endif

// -------------------------------------------------------------------
// @Compiler Errors

CompilerState *initCompilerState(Arena *arena, Lexer *lexer, CryoSymbolTable *table, const char *fileName)
{
    CompilerState *state = (CompilerState *)malloc(sizeof(CompilerState));
    state->arena = arena;
    state->lexer = lexer;
    state->table = table;
    state->programNode = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));
    state->currentNode = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));
    state->fileName = fileName;
    state->lineNumber = 0;
    state->columnNumber = 0;
    state->isActiveBuild = false;
    state->errorCount = 0;
    state->settings = (CompilerSettings *)malloc(sizeof(CompilerSettings));
    state->errors = (CompilerError **)malloc(sizeof(CompilerError *));

    logMessage("INFO", __LINE__, "CompilerState", "Compiler state initialized");
    return state;
}

void updateCompilerLineNumber(Lexer *lexer, CompilerState *state)
{
    state->lineNumber = lexer->line;
    state->columnNumber = lexer->column;
}

void updateCompilerColumnNumber(Lexer *lexer, CompilerState *state)
{
    state->columnNumber = lexer->column;
}

CompilerState addProgramNodeToState(CompilerState state, ASTNode *programNode)
{
    state.programNode = programNode;
    return state;
}

InternalDebug captureInternalDebug(const char *functionName, const char *fileName, int lineNumber)
{
    InternalDebug debug;
    debug.functionName = functionName;
    debug.fileName = fileName;
    debug.lineNumber = lineNumber;
    return debug;
}

CompilerError initNewError(InternalDebug debug)
{
    CompilerError error;
    error.type = UNKNOWN;
    error.debug = debug;
    error.message = NULL;
    error.detail = NULL;
    error.lineNumber = 0;
    error.column = 0;
    error.fileName = NULL;
    error.functionName = NULL;
    error.isActiveBuild = false;
    return error;
}

CompilerError createError(InternalDebug internals, CompilerState *state, const char *type, const char *message, const char *detail, int lineNumber, int column, const char *fileName)
{
    CompilerError error;
    if (strcmp(type, "ERROR") == 0)
    {
        error.type = ERROR;
    }
    else if (strcmp(type, "WARNING") == 0)
    {
        error.type = WARNING;
    }
    else if (strcmp(type, "INFO") == 0)
    {
        error.type = INFO;
    }
    else
    {
        error.type = UNKNOWN;
    }
    error.debug = internals;
    error.message = (char *)message;
    error.detail = (char *)detail;
    error.lineNumber = lineNumber;
    error.column = column;
    error.fileName = (char *)fileName;
    error.functionName = (char *)internals.functionName;
    error.isActiveBuild = state->isActiveBuild;
    state->errorCount++;
    state->errors = (CompilerError **)realloc(state->errors, sizeof(CompilerError *) * state->errorCount);
    state->errors[state->errorCount - 1] = &error;
    return error;
}

// -------------------------------------------------------------------

void errorReport(CompilerState state)
{
    if (state.errorCount == 0)
    {
        logMessage("INFO", __LINE__, "CompilerState", "No errors found.");
        return;
    }

    for (int i = 0; i < state.errorCount; ++i)
    {
        logCompilerError(state.errors[i]);
    }
}

void logCompilerError(CompilerError *error)
{
    char *type = getErrorTypeString(error->type);
    fprintf(stderr, "\n#COMPILATION_ERROR\n\n");
    fprintf(stderr, "File: %s\n", error->fileName);
    fprintf(stderr, "Type: [%s]\n", type);
    fprintf(stderr, "Message: %s\n", error->message);
    if (error->detail != NULL)
    {
        fprintf(stderr, "Details: %s\n", error->detail);
    }
    fprintf(stderr, "Location: %s:%d:%d\n", error->fileName, error->lineNumber, error->column);
    fprintf(stderr, "Line: %d\n", error->lineNumber);
    fprintf(stderr, "Column: %d\n", error->column);
    fprintf(stderr, "Function: %s\n", error->functionName);
    fprintf(stderr, "Is Active Build: %s\n", error->isActiveBuild ? "true" : "false");
    fprintf(stderr, "\n#END_COMPILATION_ERROR\n\n");
}

char *getErrorTypeString(ErrorType type)
{
    switch (type)
    {
    case ERROR:
        return "ERROR";
    case WARNING:
        return "WARNING";
    case INFO:
        return "INFO";
    default:
        return "UNKNOWN";
    }
}

// -------------------------------------------------------------------

void dumpCompilerState(CompilerState state)
{
    printf("\n\n!# ==================== Compiler State ==================== #!\n");
    fprintf(stderr, "  - Arena: %p\n", state.arena);
    fprintf(stderr, "  - Lexer: %p\n", state.lexer);
    fprintf(stderr, "  - Symbol Table: %p\n", state.table);
    fprintf(stderr, "  - Program Node: %p\n", state.programNode);
    fprintf(stderr, "  - Current Node: %p\n", state.currentNode);
    fprintf(stderr, "  - Line Number: %d\n", state.lineNumber);
    fprintf(stderr, "  - Column Number: %d\n", state.columnNumber);
    fprintf(stderr, "  - Active Build: %s\n", state.isActiveBuild ? "true" : "false");
    fprintf(stdout, "  - File Name: %s\n", state.fileName);
    fprintf(stderr, "  - Error Count: %d\n", state.errorCount);
    for (int i = 0; i < state.errorCount; ++i)
    {
        logCompilerError(state.errors[i]);
    }
    printf("!# ========================================================= #!\n\n");
}

void dumpSymbolTable(CompilerState state)
{
    printf("\n\n!# ==================== Symbol Table ==================== #!\n");
    printSymbolTable(state.table);
    printf("!# ======================================================== #!\n\n");
}

void dumpCompilerStateCXX(CompilerState state)
{
    dumpCompilerState(state);
}

void dumpSymbolTableCXX(CompilerState state)
{
    dumpSymbolTable(state);
}

// -------------------------------------------------------------------

CompiledFile compileFile(const char *filePath, const char *compilerFlags)
{
    logMessage("INFO", __LINE__, "Common", "Compiling file...");
    // Get the cryo ENV variable
    const char *cryoEnv = getenv("CRYO_PATH");
    if (cryoEnv == NULL)
    {
        fprintf(stderr, "Error: CRYO_PATH environment variable not set.\n");
        exit(1);
    }

    // Get the executable path (CRYO_PATH/src/bin/main)
    char *executablePath = (char *)malloc(strlen(cryoEnv) + 32);
    strcpy(executablePath, cryoEnv);
    strcat(executablePath, "/src/bin/main");

    // Check if the file exists
    FILE *file = fopen(filePath, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: File not found: %s\n", filePath);
        exit(1);
    }
    // Close the file
    fclose(file);

    // Create the command string
    char *command = (char *)malloc(strlen(executablePath) + strlen(filePath) + 32);
    strcpy(command, executablePath);
    strcat(command, " -f ");
    strcat(command, filePath);

    // Check if the compiler flags are set
    if (compilerFlags != NULL)
    {
        logMessage("INFO", __LINE__, "Common", "Compiler flags detected: %s", strdup(compilerFlags));
        strcat(command, " ");
        strcat(command, strdup(compilerFlags));
    }

    printf("\n\n Executing command: %s\n\n", command);

    // Execute the command
    system(command);

    logMessage("INFO", __LINE__, "Common", "File compiled successfully.");

    // Free the memory
    free(executablePath);
    free(command);

    char *fileName = (char *)malloc(strlen(filePath) + 1);
    // Trim the file path
    char *trimmedFileName = strrchr(filePath, '/');
    if (trimmedFileName == NULL)
    {
        // If there's no '/' in the path, use the whole fileName
        trimmedFileName = (char *)filePath;
    }
    else
    {
        // Skip the '/' character
        trimmedFileName++;
    }
    strcpy(fileName, trimmedFileName);

    // Return the compiled file
    CompiledFile compiledFile;
    compiledFile.fileName = fileName;
    compiledFile.filePath = filePath;
    compiledFile.outputPath = "unknown";

    logMessage("INFO", __LINE__, "Common", "File compiled successfully.");
    return compiledFile;
}

CompiledFile compileFileCXX(const char *filePath, const char *compilerFlags)
{
    return compileFile(filePath, compilerFlags);
}

// -------------------------------------------------------------------
