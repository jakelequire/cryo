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

    const char* filePath = argv[1];
    char* source = readFile(filePath);
    if (source == NULL) return 1;

    Lexer lexer;
    init_lexer(&lexer, source);

    Token token;

    do {
        token = get_next_token(&lexer); // Correct function call
        printf("Token type: %d, value: %s at line %d, column %d\n", token.type, token.value.stringValue, lexer.line, lexer.column);
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
    if (!generate_code(ir)) {
        fprintf(stderr, "Code generation failed\n");
        return 1;
    }

    return 0;
}


char* readFile(const char* path) {
    FILE* file;
    errno_t err = fopen_s(&file, path, "rb");  // Open the file in binary mode to avoid transformations
    if (err != 0) {
        perror("Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);

    size_t fileSize = length;
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead < fileSize) {
        perror("Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}