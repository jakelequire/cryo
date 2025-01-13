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

int sourceTextCompiler(char *sourceBuffer, CompilerSettings *settings)
{
    const char *fileName = "sourceText";
    if (!sourceBuffer)
    {
        fprintf(stderr, "Error: No source text specified\n");
        fprintf(stderr, "Received: %s\n", sourceBuffer);
        return 1;
    }

    const char *source = handleTextBuffer(sourceBuffer);

    const char *rootDirectory = settings->compilerRootPath;
    if (!rootDirectory)
    {
        fprintf(stderr, "Error: Root directory not set\n");
        return 1;
    }
    const char *buildDir = settings->buildDir;

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

    CryoLinker_LogState(linker);

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Import the runtime definitions and initialize the global dependencies
    boostrapRuntimeDefinitions(typeTable, globalSymbolTable);
    CryoLinker_LogState(linker);

    printGlobalSymbolTable(globalSymbolTable);

    // Update the global symbol table to be the primary table.
    setPrimaryTableStatus(globalSymbolTable, true);

    // Initialize the lexer
    Lexer lex;
    CompilerState *state = initCompilerState(arena, &lex, fileName);
    initLexer(&lex, source, fileName, state);
    state->settings = settings;

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lex, arena, state, typeTable, globalSymbolTable);

    if (programNode == NULL)
    {
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
        CONDITION_FAILED;
        return 1;
    }

    DEBUG_PRINT_FILTER({ END_COMPILATION_MESSAGE; });

    printGlobalSymbolTable(globalSymbolTable);
    logASTNodeDebugView(programCopy);

    return 0;
}

const char *handleTextBuffer(char *source)
{
    if (!source)
    {
        fprintf(stderr, "Error: No source text specified\n");
        return NULL;
    }

    // Check if the source text is empty
    if (strlen(source) == 0)
    {
        fprintf(stderr, "Error: Source text is empty\n");
        return NULL;
    }

    return (const char *)source;
}
