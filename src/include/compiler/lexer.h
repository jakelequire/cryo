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
#include "logger.h"
#include "utils/fs.h"
/*---------<end>---------*/

typedef struct {
    char* keyword;
    CryoTokenType type;
} KeywordToken;

typedef struct {
    char* baseType;
    CryoTokenType dataType;
} DataTypeToken;

typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
} TokenValue;

typedef struct Token {
    CryoTokenType type;         // Type of the token
    const char* start;          // Pointer to the beginning of the token in the source code
    int length;                 // Length of the token
    int line;                   // Line number where the token is found
    int column;                 // Column number where the token is found
    const char* style;          // Style of the token
    TokenValue value;           // Token value
} Token;

typedef struct Lexer {
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


/* @Util_Functions */
char* my_strndup                (const char* src, size_t len);

/* @Lexer */
void initLexer                  (Lexer* lexer, const char* source);
void freeLexer                  (Lexer* lexer);

/* @Lexer_Utils */
char advance                    (Lexer* lexer);
bool isAtEnd                    (Lexer* lexer);
char peek                       (Lexer* lexer);
char peekNext                   (Lexer* lexer);
bool matchToken                 (Lexer* lexer, CryoTokenType type);
void skipWhitespace             (Lexer* lexer);

/* @Lexer_Functions */
Token nextToken                 (Lexer* lexer, Token* token);
Token get_next_token            (Lexer* lexer);
Token getToken                  (Lexer* lexer);
Token peekToken                 (Lexer* lexer);
Token peekNextToken             (Lexer* lexer);

/* @Token_Creation */
Token makeToken                 (Lexer* lexer, CryoTokenType type);
Token errorToken                (Lexer* lexer, const char* message);
Token number                    (Lexer* lexer);
Token string                    (Lexer* lexer);
Token boolean                   (Lexer* lexer);
Token symbolChar                (Lexer* lexer, char symbol);
Token identifier                (Lexer* lexer);

/* @Data_Types */
Token checkKeyword              (Lexer* lexer);
CryoTokenType checkDataType     (Lexer* lexer, const char* dataType, CryoTokenType type);

/* @DataType_Evaluation */
bool isAlpha                    (char c);
bool isDigit                    (char c);
bool isType                     (char c);

/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */
int lexer                       (int argc, char* argv[]);

#endif // LEXER_H
