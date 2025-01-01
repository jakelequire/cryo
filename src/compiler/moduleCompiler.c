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

ASTNode *compileModuleFileToProgramNode(const char *filePath, const char *outputPath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    logMessage(LMI, "INFO", "Compiler", "Compiling module file: %s", filePath);

    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        CONDITION_FAILED;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Initialize the type table
    TypeTable *typeTable = initTypeTable();

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, filePath, state);

    ASTNode *programNode = parseProgram(&lexer, table, arena, state, typeTable, globalTable);
    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    logMessage(LMI, "INFO", "Compiler", "MODULE COMPILER: Program node parsed successfully");

    return programNode;
}

void processNodeToIRObject(ASTNode *node, CompilerState *state, const char *outputPath)
{
    logMessage(LMI, "INFO", "Compiler", "Processing node to IR object...");

    // Generate code
    int result = preprocessRuntimeIR(node, state, outputPath);
    if (result != 0)
    {
        CONDITION_FAILED;
        return;
    }

    return;
}
