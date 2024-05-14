#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include "lexer.h"

// #ifdef DEBUG
//     printf("File Contents Begin:\n%s\nFile Contents End.\n", buffer);
//     printf("Buffer content:\n%s\n", buffer);
// #endif

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = lexer->current = (char*)source;
    lexer->line = 1;
}

void nextToken(Lexer* lexer, Token* token) {
    while (*lexer->current != '\0') {
        lexer->start = lexer->current;

        printf("Processing character: '%c' (%d)\n", *lexer->current, (int)*lexer->current);

        if (isspace(*lexer->current)) {
            while (isspace(*lexer->current)) {
                lexer->current++;
            }
            continue;  // Correctly loop back to handle the next character
        }

        // Reset token length at the start of token detection
        token->length = 0;

        if (isdigit(*lexer->current)) {
            while (isdigit(*lexer->current)) lexer->current++;
            token->type = TOKEN_INT;
            token->length = lexer->current - lexer->start;
            return;
        }

        if (isalpha(*lexer->current) || *lexer->current == '_') {
            while (isalpha(*lexer->current) || isdigit(*lexer->current) || *lexer->current == '_') lexer->current++;
            token->type = TOKEN_IDENTIFIER;
            token->length = lexer->current - lexer->start;
            return;
        }

        switch (*lexer->current) {
            case '+': 
            case '-': 
            case '*': 
            case '/': 
            case '=':
                token->type = *lexer->current;
                token->length = 1;
                lexer->current++;
                break;
            default:
                printf("Unknown or unsupported character: '%c' (%d, 0x%X)\n", *lexer->current, (int)*lexer->current, (unsigned char)*lexer->current);
                token->type = TOKEN_ERROR;
                token->length = 1;
                lexer->current++;  // Skip this character
        }
        
        // printf("Processing: '%c' (%d, 0x%X)\n", *lexer->current, (int)*lexer->current, (unsigned char)*lexer->current);

        printf("Token generated: Type=%d, Length=%d, Text='%.*s'\n", token->type, token->length, token->length, lexer->start);
        printf("Continuing from character: '%c' (%d)\n", *lexer->current, (int)*lexer->current);

        if (*lexer->current != '\0' && isspace(*lexer->current)) {
            do {
                lexer->current++;
            } while (isspace(*lexer->current) && *lexer->current != '\0');
        }
    }

    if (*lexer->current != '\0' && isspace(*lexer->current)) {
        do {
            lexer->current++;
        } while (isspace(*lexer->current) && *lexer->current != '\0');
    }

    if (*lexer->current == '\0') {
        printf("End of input reached. Final character processed: '%c'\n", *(lexer->current - 1));
        token->type = TOKEN_EOF;
        token->length = 0;
        return;
    }
}

char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");  // Open the file in binary mode to avoid transformations
    if (file == NULL) {
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

    buffer[bytesRead] = '\0';  // Null-terminate the string
    fclose(file);

    // Print the contents of the file to the console
    // printf("File Contents Begin:\n%s\nFile Contents End.\n", buffer);
    // printf("Buffer content:\n%s\n", buffer);

    return buffer;
}


int lexer(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;
    do {
        nextToken(&lexer, &token);
        printf("Token: %.*s (Type: %d)\n", token.length, token.start, token.type);
    } while (token.type != TOKEN_EOF);

    free(source);
    return 0;
}
