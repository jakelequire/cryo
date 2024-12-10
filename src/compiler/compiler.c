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

int cryoCompiler(const char *filePath, CompilerSettings *settings)
{
    START_COMPILATION_MESSAGE;

    const char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return 1;
    }
    const char *fileName = trimFilePath(filePath);

    const char *rootDirectory = settings->rootDir;
    if (!rootDirectory)
    {
        fprintf(stderr, "Error: Root directory not set\n");
        return 1;
    }
    const char *buildDir = appendStrings(rootDirectory, "/build");

    // Initialize the new Symbol Table
    CryoGlobalSymbolTable globalSymbolTable = CryoGlobalSymbolTable_Create();
    if (!globalSymbolTable)
    {
        fprintf(stderr, "Error: Failed to create global symbol table\n");
        return 1;
    }

    CryoGlobalSymbolTable_PrintGlobalTable(globalSymbolTable);

    // Create and initialize linker
    CryoLinker linker = CryoLinker_Create();
    CryoLinker_SetBuildSrcDirectory(linker, buildDir);

    CryoLinker_LogState(linker);

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Import the runtime definitions and initialize the global dependencies
    boostrapRuntimeDefinitions(table, typeTable);
    CryoLinker_LogState(linker);

    // Initialize the lexer
    Lexer lex;
    CompilerState *state = initCompilerState(arena, &lex, table, fileName);
    initLexer(&lex, source, fileName, state);
    state->settings = settings;

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lex, table, arena, state, typeTable);

    if (programNode == NULL)
    {
        CONDITION_FAILED;
        return 1;
    }

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Outputs the SymTable into a file in the build directory.
    outputSymTable(table, settings);
    initASTDebugOutput(programCopy, settings);
    printTypeTable(typeTable);
    logASTNodeDebugView(programCopy);

    // Generate code (The C++ backend process)
    int result = generateCodeWrapper(programNode, state, linker);
    if (result != 0)
    {
        CONDITION_FAILED;
        return 1;
    }

    // Before returning, add cleanup
    // CryoLinker_Destroy(linker); <- This was causing a problem at the end of compilation, unsure why

    END_COMPILATION_MESSAGE;

    logASTNodeDebugView(programCopy);

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

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(arena, &lexer, table, filePath);
    state->settings = settings;
    initLexer(&lexer, source, filePath, state);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena, state, typeTable);

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

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(arena, &lexer, table, filePath);
    state->settings = &settings;
    initLexer(&lexer, source, filePath, state);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena, state, typeTable);

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
