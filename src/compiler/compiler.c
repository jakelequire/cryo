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
#include "compiler/compiler.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"
#include "frontend/frontendSymbolTable.h"

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
    INIT_SUBSYSTEMS(buildDir, rootDirectory, fileName, source, settings, linker, arena, lexer, state);

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, arena, state);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
        return 1;
    }

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Outputs the SymTable into a file in the build directory.
    initASTDebugOutput(programCopy, settings);

    CompilationUnitDir dir = createCompilationUnitDir(filePath, buildDir, CRYO_MAIN);
    dir.print(dir);

    CompilationUnit *unit = createNewCompilationUnit(programNode, dir);
    if (!unit)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
        CONDITION_FAILED;
        return 1;
    }
    if (unit->verify(unit) != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
        CONDITION_FAILED;
        return 1;
    }
    if (generateIRFromAST(unit, state, linker) != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
        CONDITION_FAILED;
        return 1;
    }

    DEBUG_PRINT_FILTER({
        END_COMPILATION_MESSAGE;
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
    char *filePath = (char *)malloc(strlen(settings->projectDir) + 64);
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
    const char *fileName = trimFilePath(strdup(filePath));

    const char *fileDirectory = settings->inputFilePath;
    const char *rootDirectory = settings->compilerRootPath;
    const char *buildDir = settings->buildDir;
    if (!rootDirectory)
    {
        fprintf(stderr, "Error: Root directory not set\n");
        return 1;
    }

    CryoLinker *linker;
    Arena *arena;
    Lexer lexer;
    CompilerState *state;
    INIT_SUBSYSTEMS(buildDir, settings->compilerRootPath, fileName, source, settings, linker, arena, lexer, state);
    state->setFilePath(state, strdup(filePath));
    initFrontendSymbolTable();

    logCompilerSettings(settings);

    // Initialize the Type Definitions before parsing
    DTM->initDefinitions();

    CryoLinker_InitCryoCore(linker, settings->compilerRootPath, buildDir, state);

    state->setFilePath(state, strdup(filePath));
    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, arena, state);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
        return 1;
    }

    DEBUG_PRINT_FILTER({
        printf("Printing Stack Trace...\n");
        GDM->printStackTrace(GDM);
    });

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
    programNode->print(programNode);

    // ==========================================
    // Generate code (The C++ backend process)

    CompilationUnitDir dir = createCompilationUnitDir(filePath, buildDir, CRYO_MAIN);
    dir.print(dir);

    CompilationUnit *unit = createNewCompilationUnit(programNode, dir);
    if (!unit)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
        CONDITION_FAILED;
        return 1;
    }
    if (unit->verify(unit) != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
        CONDITION_FAILED;
        return 1;
    }

    if (generateIRFromAST(unit, state, linker) != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
        CONDITION_FAILED;
        return 1;
    }

    LINK_ALL_MODULES(linker);

    DEBUG_PRINT_FILTER({
        FEST->printTable(FEST);
    });

    DEBUG_PRINT_FILTER({
        END_COMPILATION_MESSAGE;
        programNode->print(programNode);
    });

    return 0;
}

// ============================================================================= //
//                           Compile for AST Node                                //
// ============================================================================= //

ASTNode *compileForASTNode(const char *filePath, CompilerState *state)
{
    __STACK_FRAME__
    const char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return NULL;
    }
    const char *fileName = trimFilePath(filePath);

    GDM->frontendState->setSourceCode(GDM->frontendState, source);
    GDM->frontendState->setCurrentFile(GDM->frontendState, strdup(fileName));

    state->setFilePath(state, filePath);

    Lexer lexer;
    initLexer(&lexer, source, fileName, state);

    Arena *arena;
    arena = createArena(ARENA_SIZE, ALIGNMENT);

    ASTNode *programNode = parseProgram(&lexer, arena, state);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
    }

    return programNode;
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
