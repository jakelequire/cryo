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

int compileProject(CompilerSettings *settings)
{
    printf("\nStarting compilation...\n");
    String *filePath = Str(settings->projectDir);
    filePath->append(filePath, "/src/main.cryo");

    if (!filePath)
    {
        fprintf(stderr, "Error: Failed to allocate memory for file path\n");
        return 1;
    }

    filePath->print(filePath);

    const char *source = readFile(filePath->c_str(filePath));
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return 1;
    }
    const char *fileName = trimFilePath(filePath->c_str(filePath));

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
    CryoLinker *linker = CreateCryoLinker(buildDir);
    printf("Linker created\n");

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Import the runtime definitions and initialize the global dependencies
    boostrapRuntimeDefinitions(typeTable, globalSymbolTable, linker);

    printGlobalSymbolTable(globalSymbolTable);

    // Update the global symbol table to be the primary table.
    setPrimaryTableStatus(globalSymbolTable, true);

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

    printf("Printing Stack Trace...\n");
    GDM->printStackTrace(GDM);

    ASTNode *programCopy = (ASTNode *)malloc(sizeof(ASTNode));
    memcpy(programCopy, programNode, sizeof(ASTNode));

    // Outputs the SymTable into a file in the build directory.
    initASTDebugOutput(programNode, settings);
    printTypeTable(typeTable);

    // Generate code (The C++ backend process)
    int result = generateCodeWrapper(programNode, state, linker);
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
