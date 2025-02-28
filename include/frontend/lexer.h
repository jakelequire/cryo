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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>

#include "frontend/tokens.h"
#include "tools/utils/fs.h"
#include "tools/utils/utility.h"
#include "tools/utils/c_logger.h"
#include "common/common.h"
#include "tools/utils/cWrappers.h"
#include "tools/utils/cTypes.h"

typedef struct CompilerState CompilerState;

typedef struct Lexer
{
    const char *source;
    const char *start;
    const char *end;
    const char *current;
    int line;
    int column;
    Token currentToken;
    Token nextToken;
    Token lookahead;
    bool hasPeeked;
    const char *fileName;

    _NEW_METHOD(int, getLPos, Lexer *self); // Get the current line position
    _NEW_METHOD(int, getCPos, Lexer *self); // Get the current column position
} Lexer;

/* @Util_Functions */
char *my_strndup(const char *src, size_t len);

/* @Lexer */
void initLexer(Lexer *lexer, const char *source, const char *fileName, CompilerState *state);
void freeLexer(Lexer *lexer);

/* @Lexer_Utils */
Lexer *freezeLexer(Lexer *lexer);
int getLPos(Lexer *lexer);
int getCPos(Lexer *lexer);
char advance(Lexer *lexer, CompilerState *state);
bool isAtEnd(Lexer *lexer, CompilerState *state);
char peek(Lexer *lexer, CompilerState *state);
char peekNext(Lexer *lexer, CompilerState *state);
bool matchToken(Lexer *lexer, CryoTokenType type, CompilerState *state);
void skipWhitespace(Lexer *lexer, CompilerState *state);
bool skipComment(Lexer *lexer, CompilerState *state);
char peekNextUnconsumedLexerToken(Lexer *lexer, CompilerState *state);
char currentChar(Lexer *lexer, CompilerState *state);

const char *getCurrentFileLocationFromLexer(Lexer *lexer);

/* @Lexer_Functions */
Token nextToken(Lexer *lexer, Token *token, CompilerState *state);
Token get_next_token(Lexer *lexer, CompilerState *state);
Token getToken(Lexer *lexer, CompilerState *state);
Token peekToken(Lexer *lexer, CompilerState *state);
Token peekNextToken(Lexer *lexer, CompilerState *state);

/* @Token_Creation */
Token makeToken(Lexer *lexer, CryoTokenType type, CompilerState *state);
Token errorToken(Lexer *lexer, const char *message, CompilerState *state);
Token number(Lexer *lexer, CompilerState *state, bool isNegative);
Token string(Lexer *lexer, CompilerState *state);
Token boolean(Lexer *lexer, CompilerState *state);
Token symbolChar(Lexer *lexer, char symbol, CompilerState *state);
Token identifier(Lexer *lexer, CompilerState *state);

/* @Data_Types */
Token checkKeyword(Lexer *lexer, CompilerState *state);
CryoTokenType checkDataType(Lexer *lexer, const char *dataType, CryoTokenType type, CompilerState *state);
Token handleTypeIdentifier(Lexer *lexer, CompilerState *state);

/* @DataType_Evaluation */
bool isAlpha(char c);
bool isAlphaNumeric(char c);
bool isDigit(char c);
bool isType(char c);
bool isOperatorToken(CryoTokenType type);

#endif // LEXER_H
