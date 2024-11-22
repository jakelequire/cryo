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

// This is being hard coded for now, but will be replaced with a more dynamic solution later.
char *runtimeFilePaths[] = {
    "/home/phock/Programming/apps/cryo/cryo/runtime.cryo"};

// This function will take the Symbol Table and Type Table from the compiler and bootstrap the runtime definitions
// into the primary compiler state. This will produce an AST Node of the runtime definitions that can be used to
// compile the runtime into the program.
void boostrapRuntimeDefinitions(CryoSymbolTable *table, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Bootstrap", "Bootstrapping runtime definitions...");

    char *runtimePath = runtimeFilePaths[0];
    Bootstrapper *bootstrap = initBootstrapper(runtimePath);

    logMessage("INFO", __LINE__, "Bootstrap", "Bootstrapper initialized");

    // Update the bootstrap status
    updateBootstrapStatus(bootstrap, BOOTSTRAP_IN_PROGRESS);

    // Compile the runtime file
    ASTNode *runtimeNode = compileForRuntimeNode(bootstrap, runtimePath);

    if (!runtimeNode)
    {
        fprintf(stderr, "Error: Failed to compile runtime node\n");
        updateBootstrapStatus(bootstrap, BOOTSTRAP_FAILED);
        return;
    }

    logMessage("INFO", __LINE__, "Bootstrap", "Runtime node compiled successfully");

    // Add the runtime definitions to the symbol table
    for (int i = 0; i < runtimeNode->data.program->statementCount; i++)
    {
        ASTNode *node = runtimeNode->data.program->statements[i];
        addDefinitionToSymbolTable(table, node, bootstrap->arena);
    }

    logMessage("INFO", __LINE__, "Bootstrap", "Runtime definitions added to symbol table");

    // Add the runtime definitions to the type table
    importTypesFromRootNode(typeTable, runtimeNode);

    logMessage("INFO", __LINE__, "Bootstrap", "Runtime definitions added to type table");

    // Update the bootstrap status
    updateBootstrapStatus(bootstrap, BOOTSTRAP_SUCCESS);

    logMessage("INFO", __LINE__, "Bootstrap", "Runtime definitions bootstrapped successfully");

    // Free the bootstrap state
    free(bootstrap);

    logMessage("INFO", __LINE__, "Bootstrap", "Bootstrapper freed");

    return;
}

Bootstrapper *initBootstrapper(const char *filePath)
{
    Bootstrapper *bootstrapper = (Bootstrapper *)malloc(sizeof(Bootstrapper));

    // Init the bootstrapper state
    bootstrapper->status = BOOTSTRAP_IDLE;

    // Initialize the Arena
    bootstrapper->arena = createArena(ARENA_SIZE, ALIGNMENT);

    // Initialize the symbol table
    bootstrapper->table = createSymbolTable(bootstrapper->arena);

    bootstrapper->typeTable = initTypeTable();

    // Set the program node to null, we will parse it later.
    bootstrapper->programNode = NULL;

    return bootstrapper;
}

ASTNode *compileForRuntimeNode(Bootstrapper *bootstrap, const char *filePath)
{
    logMessage("INFO", __LINE__, "Bootstrap", "@compileForRuntimeNode Reading file: %s", filePath);
    // This needs to create a whole separate compiler state & arena for each program node
    // This is because the program node is the root of the AST and needs to be compiled separately
    char *source = readFile(filePath);
    if (!source)
    {
        fprintf(stderr, "Error: Failed to read file: %s\n", filePath);
        return NULL;
    }

    printf("Source: %s\n", source);

    // Initialize the lexer
    Lexer lexer;
    CompilerState *state = initCompilerState(bootstrap->arena, &lexer, bootstrap->table, filePath);

    logMessage("INFO", __LINE__, "Bootstrap", "Compiler state initialized");

    initLexer(&lexer, source, filePath, state);

    logMessage("INFO", __LINE__, "Bootstrap", "Lexer initialized");

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, bootstrap->table, bootstrap->arena, state, bootstrap->typeTable);

    if (programNode == NULL)
    {
        fprintf(stderr, "Error: Failed to parse program node\n");
        return NULL;
    }

    logMessage("INFO", __LINE__, "Bootstrap", "Program node parsed successfully");

    return programNode;
}

void compileRuntimeObjectFile(ASTNode *runtimeNode, CompilerState *state)
{
    // Generate code
    int result = generateCodeWrapper(runtimeNode, state);
    if (result != 0)
    {
        CONDITION_FAILED;
        return;
    }

    return;
}

void updateBootstrapStatus(Bootstrapper *bootstrapper, enum BootstrapStatus status)
{
    bootstrapper->status = status;
}
