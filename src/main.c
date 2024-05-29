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

    // Perform semantic analysis
    if (!analyzeNode(program)) {
        fprintf(stderr, "Semantic analysis failed.\n");
        free(source);
        return 1;
    }
    printf("[DEBUG] Semantic analysis completed\n");

    // Print AST structure
    printAST(program, 0);
    
    // Initialize LLVM
    initializeLLVM();

    // Code generation
    generateCodeFromAST(program);
    printf("[DEBUG] Code generation completed\n");

    // Finalize LLVM codegen (output file produced here)
    finalizeLLVM();
    printf("[DEBUG] Codegen finalized\n");

    // Clean up
    free(source);
    freeAST(program);
    return 0;
}
