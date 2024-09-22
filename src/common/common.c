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

// Export the `globalCompiler` function to be used in C++

int globalCompiler(const char *source)
{
    if (source == NULL || strlen(source) == 0)
    {
        fprintf(stderr, "Source code is empty.\n");
        return 1;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the call stack
    initCallStack(&callStack, 10);

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source);
    logMessage("INFO", __LINE__, "Main", "Lexer Initialized... ");

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena);
    if (programNode == NULL)
    {
        fprintf(stderr, "Failed to parse source code.\n");
        return 1;
    }

    return 0;
}

ASTNode *getProgramNode(const char *source)
{
    if (source == NULL || strlen(source) == 0)
    {
        fprintf(stderr, "Source code is empty.\n");
        return NULL;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the call stack
    initCallStack(&callStack, 10);

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source);
    logMessage("INFO", __LINE__, "Main", "Lexer Initialized... ");

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena);
    if (programNode == NULL)
    {
        fprintf(stderr, "Failed to parse source code.\n");
        return NULL;
    }

    return programNode;
}
