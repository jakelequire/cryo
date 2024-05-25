#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "token.h"

typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
} TokenValue;

typedef struct {
    TokenType type;         // Type of the token
    const char* start;      // Pointer to the beginning of the token in the source code
    int length;             // Length of the token
    int line;               // Line number where the token is found
    int column;             // Column number where the token is found
    const char* style;      // Style of the token
    union {                 // Union to handle different types of token values
        int intValue;       // Use if the token is an integer
        float floatValue;   // Use if the token is a float
        char* stringValue;  // Use if the token is a string
    } value;
} Token;

typedef struct {
    const char* source;
    const char *start;
    const char *current;
    int line;
    int column;          
    Token currentToken;  
    Token lookahead;     
    bool hasPeeked;
} Lexer;


char* my_strndup(const char* src, size_t n);

// Lexer functions
Token get_next_token(Lexer *lexer);
Token getToken(Lexer *lexer);
Token lex_identifier_or_keyword(Lexer *lexer);
Token lex_number(Lexer *lexer);
Token create_token(TokenType type, const char *value);
Token peekToken(Lexer* lexer);
void nextToken(Lexer* lexer, Token* token);
void initLexer(Lexer* lexer, const char* source);
char* getTokenStringValue(Token *token);
int getTokenIntegerValue(Token* token);
float getTokenFloatValue(Token* token);
void unreadToken(Lexer* lexer, Token token);
int lexer(int argc, char* argv[]);
char advance(Lexer *lexer);
void skip_whitespace(Lexer *lexer);
void skip_comment(Lexer *lexer);
char* readFile(const char* path);

#endif // LEXER_H
