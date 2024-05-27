#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>



char* my_strndup(const char* src, size_t len) {
    char* dest = (char*)malloc(len + 1);
    if (dest) {
        strncpy(dest, src, len);
        dest[len] = '\0';
    } else {
        fprintf(stderr, "{lexer} Error: Memory allocation failed in my_strndup\n");
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

    printf("{lexer} -------------- <Input Source Code> --------------\n\n");
    printf("\n{lexer} Lexer initialized. \nStart: %p \nCurrent: %p \n\nSource:\n-------\n%s\n\n", lexer->start, lexer->current, source);
    printf("\n{lexer} -------------------- <END> ----------------------\n\n");
}

CryoTokenType checkKeyword(const char* identifier) {
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

static bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}

static void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token makeToken(Lexer* lexer, CryoTokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

static Token identifier(Lexer* lexer) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
    return makeToken(lexer, TOKEN_IDENTIFIER);
}

static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) advance(lexer);
    return makeToken(lexer, TOKEN_INT);
}

// <nextToken>
Token nextToken(Lexer* lexer, Token* token) {
    skipWhitespace(lexer);

    lexer->start = lexer->current;

    if (isAtEnd(lexer)) {
        return makeToken(lexer, TOKEN_EOF);
    }
    printf("{lexer} Processing char: '%c' at line %d, column %d\n", *lexer->current, lexer->line, lexer->column);
    
    token->length = 0;
    token->line = lexer->line;
    token->column = lexer->column;
    
    char c = advance(lexer);

    if (isalpha(c)) return identifier(lexer);
    if (isdigit(c)) return number(lexer);

    switch (c) {
        case '(': return makeToken(lexer, TOKEN_LPAREN);
        case ')': return makeToken(lexer, TOKEN_RPAREN);
        case '{': return makeToken(lexer, TOKEN_LBRACE);
        case '}': return makeToken(lexer, TOKEN_RBRACE);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case '+': return makeToken(lexer, TOKEN_OP_PLUS);
        case '-': return makeToken(lexer, TOKEN_OP_MINUS);
        case '*': return makeToken(lexer, TOKEN_OP_STAR);
        case '/': return makeToken(lexer, TOKEN_OP_SLASH);
        case '=': return makeToken(lexer, TOKEN_ASSIGN);
        default:
            return errorToken(lexer, "Unexpected character.");
    }
    
    if(token->type == TOKEN_ERROR) {
        printf("{lexer} Error: Unknown character found: '%c'\n", *lexer->current);
    }
}
// </nextToken>

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
        perror("{lexer} Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        perror("{lexer} File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        perror("{lexer} Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length) {
        perror("{lexer} Failed to read the full file");
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
        fprintf(stderr, "{lexer} Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;
    do {
        nextToken(&lexer, &token);
        printf("{lexer} [!DEBUG!] Parsing token: %.*s, Type: %d\n", token.length, token.start, token.type);
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
