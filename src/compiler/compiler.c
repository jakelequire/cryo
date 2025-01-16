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
#include "linker/linkerv2.hpp"
#include "symbolTable/globalSymtable.hpp"
#include "compiler/compiler.h"
#include "tools/logger/logger_config.h"

int cryoCompiler(const char *filePath, CompilerSettings *settings)
{
    DEBUG_PRINT_FILTER({ START_COMPILATION_MESSAGE; });

    // ========================================
    // Handle Flags

    bool isSource = settings->isSource;
    if (isSource)
    {
        char *sourceText = settings->sourceText;
        return sourceTextCompiler(sourceText, settings);
    }

    bool isLSP = settings->isLSP;
    if (isLSP)
    {
        const char *inputFilePath = settings->inputFile;
        fprintf(stdout, "File Path: %s\n", inputFilePath);
        return lspCompiler(inputFilePath, settings);
    }

    bool isSingleFile = settings->isSingleFile;

    // ========================================
    // Compile the file (default behavior)

    printf("\nStarting compilation...\n");

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
    // **New global symbol table**
    // Initialize the new Symbol Table
    CryoGlobalSymbolTable *globalSymbolTable = CryoGlobalSymbolTable_Create(buildDir);
    if (!globalSymbolTable)
    {
        fprintf(stderr, "Error: Failed to create global symbol table\n");
        return 1;
    }
    // Debug print the global symbol table
    printGlobalSymbolTable(globalSymbolTable);

    // Create and initialize linker
    CryoLinker linker = CryoLinker_Create();
    CryoLinker_SetBuildSrcDirectory(linker, buildDir);

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Import the runtime definitions and initialize the global dependencies
    boostrapRuntimeDefinitions(typeTable, globalSymbolTable);

    printGlobalSymbolTable(globalSymbolTable);

    // Update the global symbol table to be the primary table.
    setPrimaryTableStatus(globalSymbolTable, true);

    // Initialize the lexerCryoLinker_SetBuildSrcDirectory
    Lexer lex;
    CompilerState *state = initCompilerState(arena, &lex, fileName);
    setGlobalSymbolTable(state, globalSymbolTable);
    initLexer(&lex, source, fileName, state);
    state->settings = settings;

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lex, arena, state, typeTable, globalSymbolTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        CONDITION_FAILED;
        return 1;
    }
    TableFinished(globalSymbolTable); // Finish the global symbol table

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Outputs the SymTable into a file in the build directory.
    initASTDebugOutput(programCopy, settings);
    printTypeTable(typeTable);

    // Generate code (The C++ backend process)
    int result = generateCodeWrapper(programNode, state, linker);
    if (result != 0)
    {
        logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate code");
        CONDITION_FAILED;
        return 1;
    }

    // After the compilation is complete, we need to link up everything.
    CryoLinker_CompleteCompilationAndLink(linker, buildDir);

    DEBUG_PRINT_FILTER({
        END_COMPILATION_MESSAGE;

        printGlobalSymbolTable(globalSymbolTable);
        logASTNodeDebugView(programCopy);
    });

    return 0;
}

int compileImportFile(const char *filePath, CompilerSettings *settings)
{
    // This needs to create a whole separate compiler state & arena for each program node
    // This is because the program node is the root of the AST and needs to be compiled separately
    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return 1;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(arena, &lexer, filePath);
    state->settings = settings;
    initLexer(&lexer, source, filePath, state);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, arena, state, typeTable, NULL);

    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return 1;
    }

    // Generate code
    int result = generateCodeWrapper(programNode, state, NULL);
    if (result != 0)
    {
        CONDITION_FAILED;
        return 1;
    }

    return 0;
}

ASTNode *compileForProgramNode(const char *filePath)
{
    // This needs to create a whole separate compiler state & arena for each program node
    // This is because the program node is the root of the AST and needs to be compiled separately
    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return NULL;
    }

    CompilerSettings settings = createCompilerSettings();
    settings.inputFile = trimFilePath(filePath);
    settings.inputFilePath = filePath;

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(arena, &lexer, filePath);
    state->settings = &settings;
    initLexer(&lexer, source, filePath, state);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, arena, state, typeTable, NULL);

    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    return programNode;
}

int compileImportFileCXX(const char *filePath, CompilerSettings *settings)
{
    return compileImportFile(filePath, settings);
}
