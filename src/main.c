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
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    // Parse the source code
    ASTNode* program = parseProgram(&lexer); // Change to parseProgram

    if (!program) {
        fprintf(stderr, "Failed to parse program.\n");
        return 1;
    }

    // Code generation
    for (int i = 0; i < program->data.program.stmtCount; ++i) {
        ASTNode* function = program->data.program.statements[i];
        codegen_function(function); // Generate code for each function
    }

    // Finalize LLVM codegen (output file produced here)
    finalize_codegen();

    // Clean up
    free(source);
    return 0;
}
