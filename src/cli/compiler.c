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
#include "cli/compiler.h"

// <cryoCompiler>
int cryoCompiler(const char *source)
{
    if (!source)
    {
        fprintf(stderr, "\n[Main] Error in reading source.\n");
        return 1;
    }

    // Initialize the Arena
    Arena *arena = createArena(ARENA_SIZE, ALIGNMENT);

    initCallStack(&callStack, 10);

    Lexer lexer;
    initLexer(&lexer, source);
    printf("\n[DEBUG] Lexer initialized\n\n");

    // Initialize the symbol table
    CryoSymbolTable *table = createSymbolTable(arena);

    // Parse the source code
    ASTNode *programNode = parseProgram(&lexer, table, arena);

    if (programNode != NULL)
    {
        printf("\n\n>===------- AST Tree -------===<\n\n");
        int size = programNode->data.program->statementCount;
        ASTNode *nodeCpy = (ASTNode *)malloc(sizeof(ASTNode) * size);
        memcpy(nodeCpy, programNode, sizeof(ASTNode));

        printSymbolTable(table, arena);

        printAST(nodeCpy, 0, arena);

        printf("\n>===------- End Tree ------===<\n\n");

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
// <cryoCompiler>
