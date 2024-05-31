/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *  
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          * 
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#ifndef LEXER_H
#define LEXER_H
/*------ <includes> ------*/
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
/*---<custom_includes>---*/
#include "token.h"
#include "ast.h"
/*---------<end>---------*/
/*-------<structure_defs>-------*/
typedef struct {
    const char* keyword;
    CryoTokenType type;
} KeywordToken;

typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
} TokenValue;

typedef struct {
    CryoTokenType type;         // Type of the token
    const char* start;          // Pointer to the beginning of the token in the source code
    int length;                 // Length of the token
    int line;                   // Line number where the token is found
    int column;                 // Column number where the token is found
    const char* style;          // Style of the token
    union {                     // Union to handle different types of token values
        int intValue;           // Use if the token is an integer
        float floatValue;       // Use if the token is a float
        char* stringValue;      // Use if the token is a string
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
/*-------<end_defs>-------*/

/*-----<function_prototypes>-----*/
// Helper Functions
bool isAlpha(char c);
bool isDigit(char c);
char* my_strndup(const char* src, size_t len);
// Lexer Function
void initLexer(Lexer* lexer, const char* source);
bool isAtEnd(Lexer* lexer);
char advance(Lexer* lexer);
char peek(Lexer* lexer);
char peekNext(Lexer* lexer);
bool matchToken(Lexer* lexer, CryoTokenType type);
void skipWhitespace(Lexer* lexer);
// Token Creation
Token makeToken(Lexer* lexer, CryoTokenType type);
Token errorToken(Lexer* lexer, const char* message);
CryoTokenType checkKeyword(const char* start, int length);
Token identifier(Lexer* lexer);
Token number(Lexer* lexer);
// Lexer Walking Functions
Token get_next_token(Lexer *lexer);
Token nextToken(Lexer* lexer, Token* token);
Token getToken(Lexer* lexer);
Token peekToken(Lexer* lexer);
// Final Functions
/*<!> Moving to utils.h*/ char* readFile(const char* path); /*<!>*/
void freeLexer(Lexer* lexer);
int lexer(int argc, char* argv[]);
/*-----<end_prototypes>-----*/

// Note to self, add more documentation for the structure
// definitions across the whole program.


#endif // LEXER_H