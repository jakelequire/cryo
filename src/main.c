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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void generateCodeWrapper(ASTNode *node);

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    const char *fileName;
    char *source;
    if (strcmp(argv[1], "-s") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Error: No source text provided after -s flag\n");
            return 1;
        }
        source = argv[2];
    }
    else
    {
        const char *filePath = argv[1];
        // Set the filename to the path but trim everything up until the filename

        fileName = strrchr(filePath, '/');
        if (fileName == NULL)
        {
            fileName = filePath;
        }
        else
        {
            fileName++;
        }

        printf("[Main] Reading source file: %s\n", fileName);

        source = readFile(filePath);
        if (source == NULL)
        {
            fprintf(stderr, "Failed to read source file.\n");
            return 1;
        }
    }

    // Initialize the call stack
    initCallStack(&callStack, 10);

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Initialize the lexer
    Lexer lexer;
    CompilerState state = initCompilerState(arena, &lexer, table, fileName);
    initLexer(&lexer, source, fileName, &state);
    logMessage("INFO", __LINE__, "Main", "Lexer Initialized... ");

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena, &state);

    if (programNode != NULL)
    {
        dumpCompilerState(state);
        int size = programNode->data.program->statementCount;
        ASTNode *nodeCpy = (ASTNode *)malloc(sizeof(ASTNode) * size);
        memcpy(nodeCpy, programNode, sizeof(ASTNode));

        printSymbolTable(table, arena);
        printAST(nodeCpy, 0, arena);
        DEBUG_ARENA_PRINT(arena);

        printf("[Main] Generating IR code...\n");
        generateCodeWrapper(nodeCpy); // <- The C++ wrapper function
        printf(">===------------- CPP End Code Generation -------------===<\n");
        printf("[Main] IR code generated, freeing AST.\n");

        // Free the Arena
        freeArena(arena);
    }
    else
    {
        fprintf(stderr, "[Main] Failed to parse program.\n");
        freeArena(arena);
    }

    printf("[DEBUG] Program parsed\n");

    free(source);
    return 0;
}
