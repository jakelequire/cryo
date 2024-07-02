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
extern "C" {
#endif

void generateCodeWrapper(ASTNode* node);

#ifdef __cplusplus
}
#endif


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) {
        fprintf(stderr, "Failed to read source file.\n");
        return 1;
    }

    Lexer lexer;
    initLexer(&lexer, source);
    printf("\n[DEBUG] Lexer initialized\n\n");

    // Parse the source code
    ASTNode* programNode = parseProgram(&lexer);

    if(programNode != NULL) {
        printf("\n\n>===------- AST Tree -------===<\n\n");
        printAST(programNode, 0);
        printf("\n>===------- End Tree ------===<\n\n");

        // printf("\n>===------- Semantic Analysis -------===<\n\n");
        // analyzeNode(programNode);
        // printf("\n\n>===------- End Analysis ------===<\n\n");
    }

    if (programNode) {
        printf("[Main] Generating IR code...\n");
        generateCodeWrapper(programNode); // <- The C++ wrapper function
        printf(">===------------- CPP End Code Generation -------------===<\n");
        printf("[Main] IR code generated, freeing AST.\n");
        freeAST(programNode);
    }

    printf("[DEBUG] Program parsed\n");

    free(source);
    return 0;
}
