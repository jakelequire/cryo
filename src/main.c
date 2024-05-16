#include <stdio.h>
#include <stdlib.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"

char* readFile(const char* path); 

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;

    do {
        nextToken(&lexer, &token);
        parseProgram(&lexer);
    } while (token.type != TOKEN_EOF);

    ASTNode* ast = parseExpression(&lexer);
    

    if (ast == NULL) {
        fprintf(stderr, "Failed to parse expression.\n");
        free(source);
        return 1;
    }

    printAST(ast);  // Print the AST
    freeAST(ast);   // Free the AST
    free(source);   // Free the source buffer
    return 0;
}