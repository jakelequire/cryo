#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <string.h>
#include <stdbool.h>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/token.h"



void initLexer(Lexer* lexer, const char* source) {
    lexer->start = lexer->current = (char*)source;
    lexer->line = 1;
    lexer->column = 1;
}

void nextToken(Lexer* lexer, Token* token) {
    while (*lexer->current != '\0') {
        lexer->start = lexer->current;

        // Handle spaces
        if (isspace(*lexer->current)) {
            do {
                if (*lexer->current == '\n') {
                    lexer->line++;
                    lexer->column = 1; // Reset column number at the start of a new line
                } else {
                    lexer->column++;
                }
                lexer->current++;
            } while (isspace(*lexer->current));
            // printf("Skipped whitespace, next character: '%c' (%d) at line %d, column %d\n", *lexer->current, (int)*lexer->current, lexer->line, lexer->column);
            continue;
        }

        // Reset token length at the start of token detection
        token->length = 0;
        token->line = lexer->line;
        token->column = lexer->column;

        // Handle newlines
        if (*lexer->current == '\n') {
            lexer->line++;
            lexer->column = 1; // Reset column number at the start of a new line
            token->type = TOKEN_NEWLINE;
            token->length = 1;
            lexer->current++;
            // printf("Detected newline at line %d, column %d\n", lexer->line, lexer->column);
            return;
        }

        // Handling identifiers
        if (isalpha(*lexer->current) || *lexer->current == '_') {
            while (isalpha(*lexer->current) || isdigit(*lexer->current) || *lexer->current == '_') {
                lexer->current++;
                lexer->column++;
            }
            token->type = TOKEN_IDENTIFIER;
            token->length = lexer->current - lexer->start;
            // printf("Detected identifier: '%.*s'\n", token->length, lexer->start);

            // Check for keywords
            if (strncmp(lexer->start, "public", token->length) == 0) token->type = TOKEN_KW_PUBLIC;
            else if (strncmp(lexer->start, "fn", token->length) == 0) token->type = TOKEN_KW_FN;
            else if (strncmp(lexer->start, "return", token->length) == 0) token->type = TOKEN_KW_RETURN;
            else if (strncmp(lexer->start, "Result", token->length) == 0) token->type = TOKEN_KW_RESULT;
            else if (strncmp(lexer->start, "break", token->length) == 0) token->type =  TOKEN_KW_BREAK;
            else if (strncmp(lexer->start, "case", token->length) == 0) token->type = TOKEN_KW_CASE;
            else if (strncmp(lexer->start, "class", token->length) == 0) token->type = TOKEN_KW_CLASS;
            else if (strncmp(lexer->start, "const", token->length) == 0) token->type = TOKEN_KW_CONST;
            else if (strncmp(lexer->start, "continue", token->length) == 0) token->type = TOKEN_KW_CONTINUE;
            else if (strncmp(lexer->start, "default", token->length) == 0) token->type = TOKEN_KW_DEFAULT;
            else if (strncmp(lexer->start, "else", token->length) == 0) token->type = TOKEN_KW_ELSE;
            else if (strncmp(lexer->start, "extends", token->length) == 0) token->type = TOKEN_KW_EXTENDS;
            else if (strncmp(lexer->start, "false", token->length) == 0) token->type = TOKEN_KW_FALSE;
            else if (strncmp(lexer->start, "for", token->length) == 0) token->type = TOKEN_KW_FOR;
            else if (strncmp(lexer->start, "if", token->length) == 0) token->type = TOKEN_KW_IF;
            else if (strncmp(lexer->start, "import", token->length) == 0) token->type = TOKEN_KW_IMPORT;
            else if (strncmp(lexer->start, "in", token->length) == 0) token->type = TOKEN_KW_IN;
            else if (strncmp(lexer->start, "instanceof", token->length) == 0) token->type = TOKEN_KW_INSTANCEOF;
            else if (strncmp(lexer->start, "new", token->length) == 0) token->type = TOKEN_KW_NEW;
            else if (strncmp(lexer->start, "null", token->length) == 0) token->type = TOKEN_KW_NULL;
            else if (strncmp(lexer->start, "protected", token->length) == 0) token->type = TOKEN_KW_PROTECTED;
            else if (strncmp(lexer->start, "static", token->length) == 0) token->type = TOKEN_KW_STATIC;
            else if (strncmp(lexer->start, "true", token->length) == 0) token->type = TOKEN_KW_TRUE;
            else if (strncmp(lexer->start, "while", token->length) == 0) token->type = TOKEN_KW_WHILE;
            else if (strncmp(lexer->start, "mut", token->length) == 0) token->type = TOKEN_KW_MUT;
            else if (strncmp(lexer->start, "protected", token->length) == 0) token->type = TOKEN_KW_PROTECTED;
            else if (strncmp(lexer->start, "private", token->length) == 0) token->type = TOKEN_KW_PRIVATE;
            else if (strncmp(lexer->start, "struct", token->length) == 0) token->type = TOKEN_KW_STRUCT;
            else if (strncmp(lexer->start, "enum", token->length) == 0) token->type = TOKEN_KW_ENUM;
            else if (strncmp(lexer->start, "trait", token->length) == 0) token->type = TOKEN_KW_TRAIT;
            else if (strncmp(lexer->start, "impl", token->length) == 0) token->type = TOKEN_KW_IMPL;
            else if (strncmp(lexer->start, "use", token->length) == 0) token->type = TOKEN_KW_USE;
            else if (strncmp(lexer->start, "as", token->length) == 0) token->type = TOKEN_KW_AS;
            else if (strncmp(lexer->start, "module", token->length) == 0) token->type = TOKEN_KW_MODULE;
            else if (strncmp(lexer->start, "namespace", token->length) == 0) token->type = TOKEN_KW_NAMESPACE;
            else if (strncmp(lexer->start, "type", token->length) == 0) token->type = TOKEN_KW_TYPE;
            else if (strncmp(lexer->start, "cast", token->length) == 0) token->type = TOKEN_KW_CAST;
            else if (strncmp(lexer->start, "size", token->length) == 0) token->type = TOKEN_KW_SIZE;
            else if (strncmp(lexer->start, "byte", token->length) == 0) token->type = TOKEN_KW_BYTE;
            else if (strncmp(lexer->start, "char", token->length) == 0) token->type = TOKEN_KW_CHAR;
            else if (strncmp(lexer->start, "double", token->length) == 0) token->type = TOKEN_KW_DOUBLE;
            else if (strncmp(lexer->start, "float", token->length) == 0) token->type = TOKEN_KW_FLOAT;
            else if (strncmp(lexer->start, "int", token->length) == 0) token->type = TOKEN_KW_INT;
            else if (strncmp(lexer->start, "long", token->length) == 0) token->type = TOKEN_KW_LONG;
            else if (strncmp(lexer->start, "native", token->length) == 0) token->type = TOKEN_KW_NATIVE;
            else if (strncmp(lexer->start, "short", token->length) == 0) token->type = TOKEN_KW_SHORT;
            else if (strncmp(lexer->start, "synchronized", token->length) == 0) token->type = TOKEN_KW_SYNCHRONIZED;
            else if (strncmp(lexer->start, "volatile", token->length) == 0) token->type = TOKEN_KW_VOLATILE;
            else if (strncmp(lexer->start, "void", token->length) == 0) token->type = TOKEN_KW_VOID;
            else if (strncmp(lexer->start, "interface", token->length) == 0) token->type = TOKEN_KW_INTERFACE;
            else if (strncmp(lexer->start, "boolean", token->length) == 0) token->type = TOKEN_KW_BOOLEAN;
            else if (strncmp(lexer->start, "typeof", token->length) == 0) token->type = TOKEN_KW_TYPEOF;
            else if (strncmp(lexer->start, "await", token->length) == 0) token->type = TOKEN_KW_AWAIT;
            else if (strncmp(lexer->start, "enum", token->length) == 0) token->type = TOKEN_KW_ENUM;

            return;
        }

        // Handling numeric literals
        if (isdigit(*lexer->current)) {
            while (isdigit(*lexer->current)) {
                lexer->current++;
                lexer->column++;
            }
            token->type = TOKEN_INT;
            token->length = lexer->current - lexer->start;
            return;
        }

        // Handling strings
        if (*lexer->current == '"') {
            lexer->current++;  // Skip the initial double quote
            lexer->column++;
            while (*lexer->current != '"' && *lexer->current != '\0') {
                if (*lexer->current == '\\' && *(lexer->current + 1) == '"') {
                    lexer->current += 2;  // Skip the escape character and the escaped quote
                    lexer->column += 2;
                } else {
                    lexer->current++;
                    lexer->column++;
                }
            }

            if (*lexer->current == '"') {
                lexer->current++;  // Skip the closing double quote
                lexer->column++;
            }
            token->type = TOKEN_STRING;
            token->length = lexer->current - lexer->start;
            // printf("Detected string: '%.*s'\n", token->length, lexer->start);
            return;
        }

        // Handling single character tokens
        if (ispunct(*lexer->current)) {
            token->type = *lexer->current;  // Assign the ASCII value of the character as the token type
            token->length = 1;
            token->line = lexer->line;
            token->column = lexer->column;
            lexer->current++;
            lexer->column++;
            // printf("Detected punctuation: '%c' at line %d, column %d\n", *lexer->start, lexer->line, token->column);
            return;
        }

        if (*lexer->current == '-') {
            if (*(lexer->current + 1) == '>') {
                // It's a '->', handle as a single token
                lexer->current += 2; // Advance past '->'
                lexer->column += 2;
                token->type = TOKEN_RESULT_ARROW;
                token->length = 2;
                // printf("Detected result arrow: '->' at line %d, column %d\n", lexer->line, token->column);
                return;
            }
        }

        // Handle unexpected characters
        fprintf(stderr, "Error: Unexpected character '%c' at line %d\n", *lexer->current, lexer->line);
        lexer->current++;
        lexer->column++;
    }

    token->type = TOKEN_EOF;
    token->line = lexer->line;
    token->column = lexer->column;
    token->length = 0;
    printf("Detected EOF\n");
    exit(0);
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
        printf("Returning peeked token\n");
        return lexer->lookahead;
    } else {
        nextToken(lexer, &lexer->currentToken);
        printf("Returning new token\n");
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


void freeLexer(Lexer* lexer) {
    free(lexer->start);
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

    freeLexer(&lexer);
    free(source);
    return 0;
}
