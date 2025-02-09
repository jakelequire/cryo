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
#include "linker/linker.hpp"
#include "symbolTable/globalSymtable.hpp"
#include "compiler/compiler.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

enum CryoBuildType
{
    BT_ERROR = -1,
    BT_SOURCE,
    BT_LSP,
    BT_PROJECT,
    BT_SINGLE_FILE
} CryoBuildType;

enum CryoBuildType getBuildType(CompilerSettings *settings)
{
    if (settings->isSource)
    {
        return BT_SOURCE;
    }
    else if (settings->isLSP)
    {
        return BT_LSP;
    }
    else if (settings->isProject)
    {
        return BT_PROJECT;
    }
    else if (settings->isSingleFile)
    {
        return BT_SINGLE_FILE;
    }
    else
    {
        return BT_ERROR;
    }
}

/// @brief The main compiler function for the Cryo programming language.
/// This function is the entry point for the compiler and is responsible for
/// managing the compilation process.
int cryoCompile(CompilerSettings *settings)
{
    __STACK_FRAME__
    DEBUG_PRINT_FILTER({ START_COMPILATION_MESSAGE; });

    enum CryoBuildType buildType = getBuildType(settings);

    switch (buildType)
    {
    case BT_SOURCE:
        return exe_source_build(settings);
    case BT_LSP:
        return exe_lsp_build(settings);
    case BT_PROJECT:
        return exe_project_build(settings);
    case BT_SINGLE_FILE:
        return exe_single_file_build(settings);
    case BT_ERROR:
        fprintf(stderr, "Error: Invalid build type\n");
        return 1;
    default:
        fprintf(stderr, "Error: Unknown build type\n");
        return 1;
    }

    return 0;
}

// ============================================================================= //
//                            Single File Build                                  //
// ============================================================================= //

int exe_single_file_build(CompilerSettings *settings)
{
    __STACK_FRAME__
    const char *filePath = settings->inputFilePath;
    const char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return 1;
    }
    const char *fileName = trimFilePath(filePath);

    const char *fileDirectory = settings->inputFilePath;
    const char *rootDirectory = settings->compilerRootPath;
    const char *buildDir = settings->buildDir;
    if (!rootDirectory)
    {
        fprintf(stderr, "Error: Root directory not set\n");
        return 1;
    }

    logMessage(LMI, "INFO", "Compiler", "Build directory: %s", buildDir);

    CryoGlobalSymbolTable *globalSymbolTable;
    CryoLinker *linker;
    Arena *arena;
    Lexer lexer;
    CompilerState *state;
    INIT_SUBSYSTEMS(buildDir, fileName, source, settings, globalSymbolTable, linker, arena, lexer, state);

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, arena, state, globalSymbolTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
        return 1;
    }
    TableFinished(globalSymbolTable); // Finish the global symbol table

    printf("Printing Stack Trace...\n");
    GDM->printStackTrace(GDM);

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Outputs the SymTable into a file in the build directory.
    initASTDebugOutput(programCopy, settings);

    CompilationUnitDir dir = createCompilationUnitDir(filePath, buildDir, CRYO_MODULE);
    dir.print(dir);

    // Generate code (The C++ backend process)
    int result = generateCodeWrapper(programNode, state, linker, globalSymbolTable);
    if (result != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate code");
        CONDITION_FAILED;
        return 1;
    }

    DEBUG_PRINT_FILTER({
        END_COMPILATION_MESSAGE;

        printGlobalSymbolTable(globalSymbolTable);
        logASTNodeDebugView(programCopy);
    });

    return 0;
}

// ============================================================================= //
//                              Project Build                                    //
// ============================================================================= //

int exe_project_build(CompilerSettings *settings)
{
    __STACK_FRAME__
    char *filePath = (char *)malloc(strlen(settings->projectDir) + 16);
    strcpy(filePath, settings->projectDir);
    strcat(filePath, "/src/main.cryo");
    if (!filePath)
    {
        fprintf(stderr, "Error: Failed to allocate memory for file path\n");
        return 1;
    }
    const char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return 1;
    }
    const char *fileName = trimFilePath(filePath);

    const char *fileDirectory = settings->inputFilePath;
    const char *rootDirectory = settings->compilerRootPath;
    const char *buildDir = settings->buildDir;
    if (!rootDirectory)
    {
        fprintf(stderr, "Error: Root directory not set\n");
        return 1;
    }

    CryoGlobalSymbolTable *globalSymbolTable;
    CryoLinker *linker;
    Arena *arena;
    Lexer lexer;
    CompilerState *state;
    INIT_SUBSYSTEMS(buildDir, fileName, source, settings, globalSymbolTable, linker, arena, lexer, state);

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, arena, state, globalSymbolTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
        return 1;
    }
    TableFinished(globalSymbolTable); // Finish the global symbol table

    printf("Printing Stack Trace...\n");
    GDM->printStackTrace(GDM);

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Analyze the AST
    int analysisResult = initSemanticAnalysis(programCopy);
    if (analysisResult != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to analyze the AST");
        CONDITION_FAILED;
        return 1;
    }

    // Outputs the SymTable into a file in the build directory.
    initASTDebugOutput(programNode, settings);

    printGlobalSymbolTable(globalSymbolTable);

    CompilationUnitDir dir = createCompilationUnitDir(filePath, buildDir, CRYO_MAIN);
    dir.print(dir);

    // Generate code (The C++ backend process)
    int result = generateCodeWrapper(programNode, state, linker, globalSymbolTable);
    if (result != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate code");
        CONDITION_FAILED;
        return 1;
    }

    DEBUG_PRINT_FILTER({
        END_COMPILATION_MESSAGE;

        printGlobalSymbolTable(globalSymbolTable);
        logASTNodeDebugView(programCopy);
    });

    return 0;
}

// ============================================================================= //
//                             Source Text Build                                 //
// ============================================================================= //

int exe_source_build(CompilerSettings *settings)
{
    __STACK_FRAME__
    DEBUG_BREAKPOINT;
}

// ============================================================================= //
//                                 LSP Build                                     //
// ============================================================================= //

int exe_lsp_build(CompilerSettings *settings)
{
    __STACK_FRAME__
    DEBUG_BREAKPOINT;
}
