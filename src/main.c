#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    init_codegen();

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

    // Code generation
    for (int i = 0; i < program->data.program.stmtCount; ++i) 
    {
        ASTNode* function = program->data.program.statements[i];
        if (function) {
            codegen_function(function); // Generate code for each function
            printf("[DEBUG] Function code generated\n");
        } else {
            printf("[DEBUG] Null function encountered at index %d\n", i);
        }
    }

    // Finalize LLVM codegen (output file produced here)
    finalize_codegen();
    printf("[DEBUG] Codegen finalized\n");

    // Clean up
    free(source);
    return 0;
}
