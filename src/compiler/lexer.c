#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* my_strndup(const char* src, size_t len) {
    char* dest = malloc(len + 1);
    if (dest) {
        strncpy_s(dest, len + 1, src, len);
        dest[len] = '\0';
    }
    return dest;
}

// Initialize the lexer with the input source code
void initLexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->start = lexer->current = (char*)source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->hasPeeked = false;
    printf("-------------- <Input Source Code> --------------\n\n");
    printf("\nLexer initialized. \nStart: %p \nCurrent: %p \n\nSource:\n-------\n%s\n\n", lexer->start, lexer->current, source);
    printf("\n-------------------- <END> ----------------------\n\n");
}

TokenType checkKeyword(const char* identifier) {
    if (strcmp(identifier, "public") == 0) return TOKEN_KW_PUBLIC;
    if (strcmp(identifier, "fn") == 0) return TOKEN_KW_FN;
    if (strcmp(identifier, "int") == 0) return TOKEN_KW_INT;
    if (strcmp(identifier, "float") == 0) return TOKEN_KW_FLOAT;
    if (strcmp(identifier, "string") == 0) return TOKEN_KW_STRING;
    if (strcmp(identifier, "return") == 0) return TOKEN_KW_RETURN;
    if (strcmp(identifier, "if") == 0) return TOKEN_KW_IF;
    if (strcmp(identifier, "else") == 0) return TOKEN_KW_ELSE;
    if (strcmp(identifier, "for") == 0) return TOKEN_KW_FOR;
    if (strcmp(identifier, "while") == 0) return TOKEN_KW_WHILE;
    if (strcmp(identifier, "break") == 0) return TOKEN_KW_BREAK;
    if (strcmp(identifier, "continue") == 0) return TOKEN_KW_CONTINUE;
    // Add other keywords as needed
    return TOKEN_IDENTIFIER;
}

void nextToken(Lexer* lexer, Token* token) {
    while (*lexer->current != '\0') {
        lexer->start = lexer->current;

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

        char c = *lexer->current;
        if (isalpha(c)) {  // Start of an identifier or keyword
            int start = lexer->current - lexer->source;
            while (isalnum(*lexer->current)) {
                lexer->current++;
            }
            int length = lexer->current - (lexer->source + start);
            char* identifier = my_strndup(lexer->source + start, length);

            token->type = checkKeyword(identifier);  // Check if it's a keyword
            token->value.stringValue = identifier;
            token->line = lexer->line;
            token->column = lexer->column;

            printf("Generated Token: %s, Type: %d\n", identifier, token->type); // Debug print

            return;
        }

        if (isdigit(*lexer->current)) {
            while (isdigit(*lexer->current)) {
                lexer->current++;
                lexer->column++;
            }
            token->type = TOKEN_INT;
            token->length = lexer->current - lexer->start;
            printf("Generated Token: INTEGER '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
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
            printf("Generated Token: STRING '%.*s' at line %d, column %d\n", token->length, lexer->start, token->line, token->column);
            return;
        }

        switch (*lexer->current) {
            case '(': token->type = TOKEN_LPAREN; printf("Generated Token: '('\n"); break;
            case ')': token->type = TOKEN_RPAREN; printf("Generated Token: ')'\n"); break;
            case '{': token->type = TOKEN_LBRACE; printf("Generated Token: '{'\n"); break;
            case '}': token->type = TOKEN_RBRACE; printf("Generated Token: '}'\n"); break;
            case '-':
                lexer->current++;
                lexer->column++;
                if (*lexer->current == '>') {
                    token->type = TOKEN_RESULT_ARROW;
                    token->length = 2;
                    lexer->current++;
                    lexer->column++;
                    printf("Generated Token: '->'\n");
                } else {
                    token->type = TOKEN_ERROR;
                    printf("Generated Token: '-'\n");
                }
                return;
            default:
                token->type = TOKEN_ERROR;
                printf("Unknown character found: '%c'\n", *lexer->current);
                return;
        }

        token->length = 1;
        token->line = lexer->line;
        token->column = lexer->column;
        lexer->current++;
        lexer->column++;
        printf("Generated Token: PUNCTUATION '%c' at line %d, column %d\n", *lexer->start, token->line, token->column);
        return;
    }

    token->type = TOKEN_EOF;
    token->length = 0;
    lexer->current++;
    printf("Generated Token: <EOF>\n");
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
    strncpy(buffer, token->start, token->length);
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
