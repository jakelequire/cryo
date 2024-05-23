#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
#include "include/semantics.h"
#include "include/ir.h"
#include "include/codegen.h"

char* readFile(const char* path); 

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    printf("\n!! Generating code...\n");
    int result = generate_code();
    printf("\n!! Result from generate_code: %d\n", result);

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;

    do {
        token = get_next_token(&lexer);
        //printf("Token type: %d, value: %.*s at line %d, column %d\n", token.type, token.length, lexer.start, lexer.line, lexer.column);
        //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer.current, lexer.start, lexer.line, lexer.column);
    } while (token.type != TOKEN_EOF);



    // Parse the source code
    ASTNode* ast = parseExpression(&lexer);
    free(source);   // Free the source buffer

    if (!ast) {
        fprintf(stderr, "Failed to parse expression.\n");
        return 1;
    }

    // Perform semantic analysis
    if (!analyze(ast)) {
        fprintf(stderr, "Semantic analysis failed\n");
        return 1;
    }

    // Generate IR
    IRInstruction* ir = generate_ir(ast);
    if (!ir) {
        fprintf(stderr, "IR generation failed\n");
        return 1;
    }

    // Generate code from IR


    free(source);
    return 0;
}
