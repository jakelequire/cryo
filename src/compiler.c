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
#include "compiler.h"

int standardCryoCompiler(CompilerSettings settings, CompilerState *state, Arena *arena)
{

    char *source = readFile(settings.inputFilePath);
    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, state->fileName, state);

    // Set the lexer in the compiler state
    state->lexer = &lexer;

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, state->table, arena, state);

    if (programNode != NULL)
    {
        dumpCompilerState(*state);
        int size = programNode->data.program->statementCount;
        ASTNode *nodeCpy = (ASTNode *)malloc(sizeof(ASTNode) * size);
        memcpy(nodeCpy, programNode, sizeof(ASTNode));

        printSymbolTable(state->table);
        printAST(nodeCpy, 0, arena);

        printf("[Main] Generating IR code...\n");
        if (generateCodeWrapper(nodeCpy, state) == 0)
        {
            printf("Compilation completed successfully.\n");
        }
        else
        {
            printf("Compilation failed.\n");
        }
    }

    return 0;
}

int compileExternalFile(const char *fileName, const char *rootDir, CompilerState *state)
{
    char *source = readFile(fileName);

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source, fileName, state);

    // Set the lexer in the compiler state
    state->lexer = &lexer;

    // Initialize the parser
    ASTNode *programNode = parseProgram(&lexer, state->table, state->arena, state);

    // Need to add support to compile external files
}