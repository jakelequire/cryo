#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include "token.h"
#include "ast.h"


typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
} TokenValue;

typedef struct {
    CryoTokenType type;         // Type of the token
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
    const char* end;
    const char *current;
    int line;
    int column;          
    Token currentToken;  
    Token lookahead;     
    bool hasPeeked;
} Lexer;
bool matchToken(Lexer* lexer, CryoTokenType type); // Add this line

void initLexer(Lexer* lexer, const char* source);
int lexer(int argc, char* argv[]);
char* readFile(const char* path);
char* my_strndup(const char* src, size_t n);
bool isAtEnd(Lexer* lexer);
static char advance(Lexer* lexer);
static char peek(Lexer* lexer);
static char peekNext(Lexer* lexer);
static void skipWhitespace(Lexer* lexer);
static Token makeToken(Lexer* lexer, CryoTokenType type);
static Token errorToken(Lexer* lexer, const char* message);
Token identifier(Lexer* lexer);
Token number(Lexer* lexer);

bool isAlpha(char c);
bool isDigit(char c);
CryoTokenType checkKeyword(const char* start, int length);
// Lexer functions
Token get_next_token(Lexer *lexer);
Token getToken(Lexer *lexer);
Token peekToken(Lexer* lexer);
Token nextToken(Lexer* lexer, Token* token);


#endif // LEXER_H
