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

#include "include/main.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }
    
    // Example Implementation of new logging Macro.
    // included in the `main.h` file as `#include "include/utils.h"`
    VERBOSE_LOG("[Main]", "Starting Task...\n", "TASK_START");
    //            *custom msg*       *token example*

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) {
        fprintf(stderr, "Failed to read source file.\n");
        return 1;
    }

    Lexer lexer;
    initLexer(&lexer, source);
    printf("[DEBUG] Lexer initialized\n");

    // Parse the source code
    ASTNode* program = parseProgram(&lexer);
    if (!program) {
        fprintf(stderr, "Failed to parse program.\n");
        free(source);
        return 1;
    }
    printf("[DEBUG] Program parsed\n");

    // Print AST structure
    printAST(program, 0);
    
    // Clean up
    free(source);
    freeAST(program);
    return 0;
}
