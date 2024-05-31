#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }
    
    // Example Implementation of new logging Macro.
    // included in the `main.h` file as `#include "include/utils.h"`
    VERBOSE_LOG("Starting Task...\n", "TASK_START");
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