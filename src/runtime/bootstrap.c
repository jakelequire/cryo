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
#include "runtime/bootstrap.h"

Bootstrapper *initBootstrapper(const char *filePath, CompilerSettings *settings)
{
    Bootstrapper *bootstrapper = (Bootstrapper *)malloc(sizeof(Bootstrapper));

    // Init the bootstrapper state
    bootstrapper->status = BOOTSTRAP_IDLE;

    // Initialize the Arena
    bootstrapper->arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the symbol table
    bootstrapper->table = createSymbolTable(bootstrapper->arena);

    bootstrapper->typeTable = initTypeTable();

    // Initialize the lexer
    CompilerState *state = initCompilerState(bootstrapper->arena, &bootstrapper->lexer, bootstrapper->table, filePath);
    state->settings = settings;
    initLexer(&bootstrapper->lexer, filePath, filePath, state);

    // Set the program node to null, we will parse it later.
    bootstrapper->programNode = NULL;

    return bootstrapper;
}

void updateBootstrapStatus(Bootstrapper *bootstrapper, enum BootstrapStatus status)
{
    bootstrapper->status = status;
}

ASTNode *compileForRuntimeNode(Bootstrapper *bootstrap, const char *filePath)
{
    // This needs to create a whole separate compiler state & arena for each program node
    // This is because the program node is the root of the AST and needs to be compiled separately
    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return NULL;
    }

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(bootstrap->arena, &lexer, bootstrap->table, filePath);
    state->settings = bootstrap->state->settings;
    initLexer(&lexer, source, filePath, state);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, bootstrap->table, bootstrap->arena, state, bootstrap->typeTable);

    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    return programNode;
}
