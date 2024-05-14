#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    char* start;
    char* current;
    int line;
} Lexer;

void initLexer(Lexer* lexer, const char* source);
void nextToken(Lexer* lexer, Token* token);

#endif