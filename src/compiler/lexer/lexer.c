#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <string.h>
#include <stdbool.h>

#include "include/lexer.h"


// #ifdef DEBUG
//     printf("File Contents Begin:\n%s\nFile Contents End.\n", buffer);
//     printf("Buffer content:\n%s\n", buffer);
// #endif

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = lexer->current = (char*)source;
    lexer->line = 1;
}

void nextToken(Lexer* lexer, Token* token) {
    // printf("Processing line: %d\n", lexer->line);
    // printf("Current character: '%c' (%d)\n", *lexer->current, (int)*lexer->current);
    while (*lexer->current != '\0') {
        lexer->start = lexer->current;

        // printf("Processing character: '%c' (%d)\n", *lexer->current, (int)*lexer->current);

        // Handle spaces
        if (isspace(*lexer->current)) {
            do {
                if (*lexer->current == '\n') lexer->line++;
                lexer->current++;
            } while (isspace(*lexer->current));
            // printf("Skipped whitespace, next character: '%c' (%d)\n", *lexer->current, (int)*lexer->current);
            continue;
        }

        // Reset token length at the start of token detection
        token->length = 0;

        // Handling numeric literals
        if (isdigit(*lexer->current)) {
            while (isdigit(*lexer->current)) lexer->current++;
            token->type = TOKEN_INT;
            token->length = lexer->current - lexer->start;
            printf("Detected integer: '%.*s'\n", token->length, lexer->start);
            return;
        }

        // Handling identifiers
        if (isalpha(*lexer->current) || *lexer->current == '_') {
            while (isalpha(*lexer->current) || isdigit(*lexer->current) || *lexer->current == '_') lexer->current++;
            token->type = TOKEN_IDENTIFIER;
            token->length = lexer->current - lexer->start;
            printf("Detected identifier: '%.*s'\n", token->length, lexer->start);
            return;
        }


        if (*lexer->current == '-') {
            if (*(lexer->current + 1) == '>') {
                // It's a '->', handle as a single token
                lexer->current += 2; // Advance past '->'
                token->type = TOKEN_RESULT_ARROW;
                token->length = 2;
                printf("Detected result arrow: '->'\n");
                return;
            }
        }

        // Handling strings
        if (*lexer->current == '"') {
            lexer->current++;  // Skip the initial double quote
            while (*lexer->current != '"' && *lexer->current != '\0') {
                if (*lexer->current == '\\' && *(lexer->current + 1) == '"') {
                    lexer->current += 2;  // Skip the escape character and the escaped quote
                } else {
                    lexer->current++;
                }
            }

            if (*lexer->current == '"') lexer->current++;  // Skip the closing double quote
            token->type = TOKEN_STRING;
            token->length = lexer->current - lexer->start;
            printf("Detected string: '%.*s'\n", token->length, lexer->start);
            return;
        }

        // Handling single character tokens
        if (ispunct(*lexer->current)) {
            token->type = *lexer->current;  // Assign the ASCII value of the character as the token type
            token->length = 1;
            lexer->current++;
            printf("Detected punctuation: '%c'\n", *lexer->start);
            return;
        }



        // Handle unexpected characters
        fprintf(stderr, "Error: Unexpected character '%c' (%d) at line %d\n", *lexer->current, (int)*lexer->current, lexer->line);
        lexer->current++;  // Skip the unexpected character
        token->type = TOKEN_ERROR;
        token->length = 1;
        return;
    }

    // Explicitly setting TOKEN_EOF
    token->type = TOKEN_EOF;
    token->length = 0;
    lexer->current++;
    printf("Reached EOF\n");
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
    printf("--------- <Reading File> ---------\n");
    printf("\n----- Begin -----\n%s\n----- End ------\n\n", buffer);
    //printf("Buffer content:\n%s\n", buffer);

    return buffer;
}

Token getToken(Lexer* lexer) {
    if (lexer->hasPeeked) {
        lexer->hasPeeked = false;
        return lexer->lookahead;
    } else {
        nextToken(lexer, &lexer->currentToken);
        return lexer->currentToken;
    }
}

Token peekToken(Lexer* lexer) {
    if (!lexer->hasPeeked) {
        nextToken(lexer, &lexer->lookahead);
        lexer->hasPeeked = true;
    }
    return lexer->lookahead;
}

void unreadToken(Lexer* lexer, Token token) {
    lexer->lookahead = token;
    lexer->hasPeeked = true;
}

int getTokenIntegerValue(Token* token) {
    return strtol(token->start, NULL, 10);
}

float getTokenFloatValue(Token* token) {
    return strtof(token->start, NULL);
}

const char* getTokenStringValue(Token* token) {
    char* buffer = malloc(token->length + 1);
    strncpy(buffer, token->start, token->length);
    buffer[token->length] = '\0';
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
