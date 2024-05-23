#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Initialize the lexer with the input source code
void initLexer(Lexer* lexer, const char* source) {
    lexer->start = lexer->current = (char*)source;
    lexer->line = 1;
    lexer->column = 1;
    printf("Lexer initialized.\n");
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
            printf("Token: NEWLINE at line %d, column %d\n", token->line, token->column);
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
            printf("Token: IDENTIFIER '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
            // Check for keywords
            // (Add your keyword checks here)

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
            printf("Token: INTEGER '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
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
            printf("Token: STRING '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
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
            printf("Token: PUNCTUATION '%c' at line %d, column %d\n", token->type, token->line, token->column);
            return;
        }

        if (*lexer->current == '-') {
            if (*(lexer->current + 1) == '>') {
                // It's a '->', handle as a single token
                lexer->current += 2; // Advance past '->'
                lexer->column += 2;
                token->type = TOKEN_RESULT_ARROW;
                token->length = 2;
                printf("Token: RESULT_ARROW '->' at line %d, column %d\n", token->line, token->column);
                return;
            }
        }

        // Handle unexpected characters
        fprintf(stderr, "Error: Unexpected character '%c' (ASCII %d) at line %d, column %d\n", *lexer->current, (int)*lexer->current, lexer->line, lexer->column);
        lexer->current++;
        lexer->column++;
    }

    token->type = TOKEN_EOF;
    token->line = lexer->line;
    token->column = lexer->column;
    token->length = 0;
    printf("############ <End of File> ############\n");
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

Token get_next_token(Lexer *lexer) {
    return getToken(lexer);
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

char* getTokenStringValue(Token* token) {
    char* buffer = malloc(token->length + 1);
    strncpy_s(buffer, token->length + 1, token->start, token->length);
    buffer[token->length] = '\0';
    return buffer;
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

void freeLexer(Lexer* lexer) {
    free(lexer);
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
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
