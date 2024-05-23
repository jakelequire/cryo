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
    lexer->hasPeeked = false;
    printf("-------------- <Input Source Code> --------------\n\n");
    printf("\nLexer initialized. \nStart: %p \nCurrent: %p \n\nSource:\n-------\n%s\n\n", lexer->start, lexer->current, source);
    printf("\n-------------------- <END> ----------------------\n\n");
}


void nextToken(Lexer* lexer, Token* token) {
    while (*lexer->current != '\0') {
        lexer->start = lexer->current;
        //printf("Processing: %c <lexer.c> (ASCII %d) at line %d, column %d\n", *lexer->current, (int)*lexer->current, lexer->line, lexer->column);

        if (isspace(*lexer->current) || *lexer->current == '\r') {
            do {
                if (*lexer->current == '\n') {
                    printf("Newline found at line %d, column %d\n", lexer->line, lexer->column);
                    token->type = TOKEN_NEWLINE;
                    lexer->line++;
                    lexer->column = 1;
                    lexer->current++;
                } else if (*lexer->current == '\r') {
                    printf("Carriage return found at line %d, column %d\n", lexer->line, lexer->column);
                    lexer->current++;
                    if (*lexer->current == '\n') {
                        lexer->current++;
                    }
                    lexer->line++;
                    lexer->column = 1;
                } else {
                    printf("Whitespace found at line %d, column %d\n", lexer->line, lexer->column);
                    lexer->column++;
                    lexer->current++;
                }
            } while (isspace(*lexer->current) || *lexer->current == '\r');
            continue;
        }

        token->length = 0;
        token->line = lexer->line;
        token->column = lexer->column;

        if (isalpha(*lexer->current) || *lexer->current == '_') {
            while (isalpha(*lexer->current) || isdigit(*lexer->current) || *lexer->current == '_') {
                lexer->current++;
                lexer->column++;
            }

            token->type = TOKEN_IDENTIFIER;
            token->length = lexer->current - lexer->start;
            printf("Token: IDENTIFIER '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
            //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer->current, lexer->start, lexer->line, lexer->column);
            return;
        }

        if (isdigit(*lexer->current)) {
            while (isdigit(*lexer->current)) {
                lexer->current++;
                lexer->column++;
            }
            token->type = TOKEN_INT;
            token->length = lexer->current - lexer->start;
            printf("Token: INTEGER '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
            //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer->current, lexer->start, lexer->line, lexer->column);
            return;
        }

        if (*lexer->current == '"') {
            lexer->current++;
            lexer->column++;
            while (*lexer->current != '"' && *lexer->current != '\0') {
                if (*lexer->current == '\\' && *(lexer->current + 1) == '"') {
                    lexer->current += 2;
                    lexer->column += 2;
                } else {
                    lexer->current++;
                    lexer->column++;
                }
            }
            if (*lexer->current == '"') {
                lexer->current++;
                lexer->column++;
            }
            token->type = TOKEN_STRING;
            token->length = lexer->current - lexer->start;
            printf("Token: STRING '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
            //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer->current, lexer->start, lexer->line, lexer->column);
            return;
        }

        switch (*lexer->current) {
            case '(': token->type = TOKEN_LPAREN; printf("Left Paren Found\n"); break;
            case ')': token->type = TOKEN_RPAREN; printf("Right Paren Found\n"); break;
            case '{': token->type = TOKEN_LBRACE; printf("Left Brace Found\n"); break;
            case '}': token->type = TOKEN_RBRACE; printf("Right Brace Found\n"); break;
            case '-':
                lexer->current++;
                lexer->column++;
                if (*lexer->current == '>') {
                    token->type = TOKEN_RESULT_ARROW;
                    token->length = 2;
                    lexer->current++;
                    lexer->column++;
                    printf("Found '->'\n");
                } else {
                    token->type = TOKEN_ERROR;
                    printf("Found '-'\n");
                }
                break;
            default:
                token->type = TOKEN_ERROR;
                printf("Unknown character found: '%c'\n", *lexer->current);
                break;
        }

        if (token->type == TOKEN_ERROR) {
            token->length = 1;
            lexer->current++;
            lexer->column++;
            printf("Token: ERROR '%c' at line %d, column %d\n", *lexer->start, token->line, token->column);
            return;
        }

        token->length = 1;
        token->line = lexer->line;
        token->column = lexer->column;
        lexer->current++;
        lexer->column++;
        printf("Token: PUNCTUATION '%c' at line %d, column %d\n", *lexer->start, token->line, token->column);
        //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer->current, lexer->start, lexer->line, lexer->column);
        return;
    }

    token->type = TOKEN_EOF;
    token->length = 0;
    lexer->current++;
    printf("############ <End of File> ############\n");
    //printf("Lexer State -> Current: %p, Start: %p, Line: %d, Column: %d\n", lexer->current, lexer->start, lexer->line, lexer->column);
    exit(0);
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
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        perror("File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        perror("Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length) {
        perror("Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';  // Null-terminate the buffer
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
