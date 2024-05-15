#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    TokenType type;
    const char* start;  // Pointer to the beginning of the token in the source code
    int length;         // Length of the token
    int line;           // Line number where the token is found
    union {             // Union to handle different types of token values
        int intValue;       // Use if the token is an integer
        float floatValue;   // Use if the token is a float
        char* stringValue;  // Use if the token is a string
    } value;
} Token;

typedef struct {
    char* start;
    char* current;
    int line;
    Token currentToken;  // Store the current token
    Token lookahead;     // Store the next token for peeking
    bool hasPeeked;      // Flag to check if lookahead is filled
} Lexer;

void initLexer(Lexer* lexer, const char* source);
void nextToken(Lexer* lexer, Token* token);

// New utility functions
Token getToken(Lexer* lexer);
Token peekToken(Lexer* lexer);
void unreadToken(Lexer* lexer, Token token);

// Value extraction utilities (optional)
int getTokenIntegerValue(Token* token);
float getTokenFloatValue(Token* token);
const char* getTokenStringValue(Token* token);


#endif