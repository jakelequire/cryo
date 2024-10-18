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
#include "compiler/lexer.h"

// #################################//
//        Keyword Dictionary.       //
// #################################//
KeywordToken keywords[] = {
    {"public", TOKEN_KW_PUBLIC},
    {"function", TOKEN_KW_FN},
    {"if", TOKEN_KW_IF},
    {"else", TOKEN_KW_ELSE},
    {"while", TOKEN_KW_WHILE},
    {"for", TOKEN_KW_FOR},
    {"return", TOKEN_KW_RETURN},
    {"const", TOKEN_KW_CONST},
    {"mut", TOKEN_KW_MUT},
    {"true", TOKEN_BOOLEAN_LITERAL},
    {"false", TOKEN_BOOLEAN_LITERAL},
    {"int", TOKEN_KW_INT},
    {"string", TOKEN_KW_STRING},
    {"boolean", TOKEN_KW_BOOL},
    {"expression", TOKEN_KW_EXPRESSION},
    {"void", TOKEN_KW_VOID},
    {"null", TOKEN_KW_NULL},
    {"struct", TOKEN_KW_STRUCT},
    {"enum", TOKEN_KW_ENUM},
    {"break", TOKEN_KW_BREAK},
    {"extern", TOKEN_KW_EXTERN},
    {"continue", TOKEN_KW_CONTINUE},
    {"import", TOKEN_KW_IMPORT},
    {"namespace", TOKEN_KW_NAMESPACE},
    {"debugger", TOKEN_KW_DEBUGGER},
    {"struct", TOKEN_KW_STRUCT},
    {"declare", TOKEN_KW_DECLARE},
    {NULL, TOKEN_UNKNOWN} // Sentinel value
};

// #################################//
//        DataType Dictionary.      //
// #################################//
DataTypeToken dataTypes[] = {
    {"int", TOKEN_TYPE_INT},
    {"string", TOKEN_TYPE_STRING},
    {"boolean", TOKEN_TYPE_BOOLEAN},
    {"void", TOKEN_TYPE_VOID},
    {"int[]", TOKEN_TYPE_INT_ARRAY},
    {"string[]", TOKEN_TYPE_STRING_ARRAY},
    {"boolean[]", TOKEN_TYPE_BOOLEAN_ARRAY},
    {NULL, TOKEN_UNKNOWN} // Sentinel value
};

/* =========================================================== */
/* @Util_Functions */

// <my_strndup>
char *my_strndup(const char *src, size_t len)
{
    char *dest = (char *)malloc(len + 1);
    if (dest)
    {
        strncpy(dest, src, len);
        dest[len] = '\0';
    }
    else
    {
        fprintf(stderr, "[Lexer] Error: Memory allocation failed in my_strndup\n");
    }
    return dest;
}
// </my_strndup>

/* =========================================================== */
/* @Lexer */

// <initLexer>
void initLexer(Lexer *lexer, const char *source, const char *fileName, CompilerState *state)
{
    printf("Starting Lexer Initialization...\n");
    lexer->start = source;
    lexer->current = source;
    lexer->end = source + strlen(source);
    lexer->line = 1;
    lexer->column = 0;
    lexer->hasPeeked = false;
    lexer->fileName = fileName;

    printf("{lexer} -------------- <Input Source Code> --------------\n\n");
    printf("\n{lexer} File Name: %s\n", fileName);
    printf("\n{lexer} Lexer initialized. \nStart: %p \nCurrent: %p \n\nSource:\n-------\n%s\n\n", lexer->start, lexer->current, source);
    printf("\n{lexer} -------------------- <END> ----------------------\n\n");
}
// </initLexer>

// <freeLexer>
void freeLexer(Lexer *lexer)
{
    free(lexer);
}
// </freeLexer>

/* =========================================================== */
/* @Lexer_Utils */

Lexer *freezeLexer(Lexer *lexer)
{
    Lexer *frozenLexer = (Lexer *)malloc(sizeof(Lexer));
    frozenLexer->start = lexer->start;
    frozenLexer->current = lexer->current;
    frozenLexer->end = lexer->end;
    frozenLexer->line = lexer->line;
    frozenLexer->column = lexer->column;
    frozenLexer->hasPeeked = lexer->hasPeeked;
    frozenLexer->fileName = lexer->fileName;
    return frozenLexer;
}

// <getLPos>
int getLPos(Lexer *lexer)
{
    return lexer->current - lexer->start;
}
// </getLPos>

// <getCPos>
int getCPos(Lexer *lexer)
{
    return lexer->column;
}
// </getCPos>

// <advance>
char advance(Lexer *lexer, CompilerState *state)
{
    // printf("[Lexer] Advancing to next character: %c\n", lexer->current[0]);
    lexer->current++;
    lexer->column++;
    updateCompilerColumnNumber(lexer, state);
    return lexer->current[-1];
}
// </advance>

// <isAtEnd>
bool isAtEnd(Lexer *lexer, CompilerState *state)
{
    return *lexer->current == '\0' || lexer->current >= lexer->end;
}
// </isAtEnd>

// <peek>
char peek(Lexer *lexer, CompilerState *state)
{
    return *lexer->current;
}
// </peek>

// <peekNext>
char peekNext(Lexer *lexer, CompilerState *state)
{
    if (isAtEnd(lexer, state))
        return '\0';
    char c = (char)&lexer->current[0];
    printf("[Lexer] <!> Peeked next character: %c\n", c);
    return c;
}
// </peekNext>

// <peekNextUnconsumedLexerToken>
char peekNextUnconsumedLexerToken(Lexer *lexer, CompilerState *state)
{
    Lexer tempLexer = *lexer;
    char c = advance(&tempLexer, state);
    return c;
}
// </peekNextUnconsumedLexerToken>

// <currentChar>
char currentChar(Lexer *lexer, CompilerState *state)
{
    return *lexer->current;
}
// </currentChar>

// <matchToken>
bool matchToken(Lexer *lexer, CryoTokenType type, CompilerState *state)
{
    if (peekToken(lexer, state).type == type)
    {
        nextToken(lexer, &lexer->currentToken, state);
        return true;
    }
    return false;
}
// </matchToken>

// <skipWhitespace>
void skipWhitespace(Lexer *lexer, CompilerState *state)
{
    for (;;)
    {
        char c = peek(lexer, state);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(lexer, state);
            break;
        case '\n':
            lexer->line++;
            lexer->column = 0;
            updateCompilerLineNumber(lexer, state);
            updateCompilerColumnNumber(lexer, state);
            advance(lexer, state);
            break;
        case '/':
            printf("[Lexer] Found comment character: %c\n", c);
            advance(lexer, state);
            char next = peekNextUnconsumedLexerToken(lexer, state);
            printf("[Lexer] Next Unconsumed character: %c\n", next);
            if (next == '/')
            {
                printf("[Lexer] ! Found single line comment character: %c\n", c);
                while (peek(lexer, state) != '\n' && !isAtEnd(lexer, state))
                    advance(lexer, state);
            }
            else if (peekNext(lexer, state) == '*')
            {
                advance(lexer, state);
                advance(lexer, state);
                while (peek(lexer, state) != '*' && peekNext(lexer, state) != '/' && !isAtEnd(lexer, state))
                {
                    if (peek(lexer, state) == '\n')
                    {
                        lexer->line++;
                        lexer->column = 0;
                    }
                    advance(lexer, state);
                }
                if (isAtEnd(lexer, state))
                {
                    logMessage("ERROR", __LINE__, "Lexer", "Unterminated block comment.");
                    return;
                }
                advance(lexer, state);
                advance(lexer, state);
            }
            else
            {
                lexer->current--;
                lexer->column--;
                return;
            }
            break;
        default:
            return;
        }
    }
}
// </skipWhitespace>

/* =========================================================== */
/* @Lexer_Functions */

// <nextToken>
Token nextToken(Lexer *lexer, Token *token, CompilerState *state)
{
    skipWhitespace(lexer, state);

    lexer->start = lexer->current;

    if (isAtEnd(lexer, state))
    {
        logMessage("INFO", __LINE__, "Lexer", "Creating EOF token");
        *token = makeToken(lexer, TOKEN_EOF, state);
        return *token;
    }

    char c = advance(lexer, state);
    logMessage("INFO", __LINE__, "Lexer", "Current character: %c", c);

    if (isAlpha(c))
    {
        *token = checkKeyword(lexer, state);
        logMessage("INFO", __LINE__, "Lexer", "Keyword token created");
        return *token;
    }

    if (isDigit(c))
    {
        *token = number(lexer, state);
        logMessage("INFO", __LINE__, "Lexer", "Number token created");
        return *token;
    }

    if (c == '"')
    {
        *token = string(lexer, state);
        logMessage("INFO", __LINE__, "Lexer", "String token created");
        return *token;
    }

    if (c == '&')
    {
        *token = makeToken(lexer, TOKEN_AMPERSAND, state);
        logMessage("INFO", __LINE__, "Lexer", "Ampersand token created");
        return *token;
    }
    Token symToken = symbolChar(lexer, c, state);
    if (symToken.type != TOKEN_UNKNOWN)
    {
        *token = symToken;
        logMessage("INFO", __LINE__, "Lexer", "Symbol token created");
        return *token;
    }

    logMessage("ERROR", __LINE__, "Lexer", "Unexpected character: %c", c);
    return identifier(lexer, state);
}
// </nextToken>

// <get_next_token>
Token get_next_token(Lexer *lexer, CompilerState *state)
{
    // printf("[Lexer] Getting next token...\n");
    nextToken(lexer, &lexer->currentToken, state);
    return lexer->currentToken;
}
// </get_next_token>

// <getToken>
Token getToken(Lexer *lexer, CompilerState *state)
{
    if (lexer->hasPeeked)
    {
        lexer->hasPeeked = false;
        return lexer->lookahead;
    }
    else
    {
        nextToken(lexer, &lexer->currentToken, state);
        return lexer->currentToken;
    }
}
// </getToken>

// <peekToken>
Token peekToken(Lexer *lexer, CompilerState *state)
{
    if (!lexer->hasPeeked)
    {
        nextToken(lexer, &lexer->lookahead, state);
        lexer->hasPeeked = true;
    }
    return lexer->lookahead;
}
// </peekToken>

// <peekNextToken>
Token peekNextToken(Lexer *lexer, CompilerState *state)
{
    Lexer tempLexer = *lexer;                           // Copy the current lexer state
    Token tempNextToken = peekToken(&tempLexer, state); // Get the next token from the copied state
    return tempNextToken;
}
// </peekNextToken>

/* =========================================================== */
/* @Token_Creation */

// <makeToken>
Token makeToken(Lexer *lexer, CryoTokenType type, CompilerState *state)
{
    // printf("\n\nCreating Token: %s\n\n", CryoTokenToString(type));
    // current token
    char *currentToken = my_strndup(lexer->start, lexer->current - lexer->start);
    // printf("Current Token: %s\n", currentToken);

    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column;

    // printf("[Lexer] Created token: %.*s (Type: %d, Line: %d, Column: %d)\n",
    //        token.length, token.start, token.type, token.line, token.column);
    logMessage("INFO", __LINE__, "Lexer", "Token created: Type: %s, Line: %d, Column: %d", CryoTokenToString(token.type), token.line, token.column);
    return token;
}
// </makeToken>

// <errorToken>
Token errorToken(Lexer *lexer, const char *message, CompilerState *state)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}
// </errorToken>

// <number>
Token number(Lexer *lexer, CompilerState *state)
{
    while (isDigit(peek(lexer, state)))
    {
        advance(lexer, state);
    }
    Token token = makeToken(lexer, TOKEN_INT_LITERAL, state);
    // printf("[Lexer] Number token: %.*s\n", token.length, token.start);
    return token;
}
// </number>

// <string>
Token string(Lexer *lexer, CompilerState *state)
{
    while (peek(lexer, state) != '"' && !isAtEnd(lexer, state))
    {
        if (peek(lexer, state) == '\n')
            lexer->line++;
        advance(lexer, state);
    }

    if (isAtEnd(lexer, state))
        return errorToken(lexer, "Unterminated string.", state);

    advance(lexer, state);
    Token token = makeToken(lexer, TOKEN_STRING_LITERAL, state);
    // printf("[Lexer] String token: %.*s\n", token.length, token.start);
    return token;
}
// </string>

// <boolean>
Token boolean(Lexer *lexer, CompilerState *state)
{
    if (peek(lexer, state) == 't' && peekNext(lexer, state) == 'r' && peekNext(lexer, state) == 'u' && peekNext(lexer, state) == 'e')
    {
        advance(lexer, state);
        advance(lexer, state);
        advance(lexer, state);
        advance(lexer, state);
    }
    else if (peek(lexer, state) == 'f' && peekNext(lexer, state) == 'a' && peekNext(lexer, state) == 'l' && peekNext(lexer, state) == 's' && peekNext(lexer, state) == 'e')
    {
        advance(lexer, state);
        advance(lexer, state);
        advance(lexer, state);
        advance(lexer, state);
        advance(lexer, state);
    }
    else
    {
        return errorToken(lexer, "Invalid boolean literal.", state);
    }

    Token token = makeToken(lexer, TOKEN_BOOLEAN_LITERAL, state);
    // printf("[Lexer] Boolean token: %.*s\n", token.length, token.start);
    return token;
}
// </boolean>

// <symbolChar>
Token symbolChar(Lexer *lexer, char symbol, CompilerState *state)
{
    switch (symbol)
    {
    case '(':
        return makeToken(lexer, TOKEN_LPAREN, state);
    case ')':
        return makeToken(lexer, TOKEN_RPAREN, state);
    case '{':
        return makeToken(lexer, TOKEN_LBRACE, state);
    case '}':
        return makeToken(lexer, TOKEN_RBRACE, state);
    case '[':
        return makeToken(lexer, TOKEN_LBRACKET, state);
    case ']':
        return makeToken(lexer, TOKEN_RBRACKET, state);
    case ',':
        return makeToken(lexer, TOKEN_COMMA, state);
    case '$':
        return makeToken(lexer, TOKEN_DOLLAR, state);
    case '/':
        return makeToken(lexer, TOKEN_SLASH, state);
    case '.':
    {
        if (peek(lexer, state) == '.')
        {
            if (peekNext(lexer, state) == '.')
            {
                advance(lexer, state);
                advance(lexer, state);
                return makeToken(lexer, TOKEN_ELLIPSIS, state);
            }
        }
        else
        {
            return makeToken(lexer, TOKEN_DOT, state);
        }
    }
    case ':':
    {
        if (peek(lexer, state) == ':')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_DOUBLE_COLON, state);
        }
        return makeToken(lexer, TOKEN_COLON, state);
    }
    case '-':
        if (peek(lexer, state) == '>')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_RESULT_ARROW, state);
        }
        return makeToken(lexer, TOKEN_MINUS, state);
    case '+':
        if (peek(lexer, state) == '+')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_INCREMENT, state);
        }
        return makeToken(lexer, TOKEN_PLUS, state);
    case ';':
        return makeToken(lexer, TOKEN_SEMICOLON, state);
    case '*':
        return makeToken(lexer, TOKEN_STAR, state);
    case '%':
        return makeToken(lexer, TOKEN_PERCENT, state);
    case '!':
    {
        if (peek(lexer, state) == '=')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_NOT_EQUAL, state);
        }
        return makeToken(lexer, TOKEN_BANG, state);
    }
    case '=':
    {
        if (peek(lexer, state) == '=')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_STRICT_EQUAL, state);
        }
        return makeToken(lexer, TOKEN_EQUAL, state);
    }
    case '<':
        if (peek(lexer, state) == '=')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_LESS_EQUAL, state);
        }
        return makeToken(lexer, TOKEN_LESS, state);
    case '>':
        if (peek(lexer, state) == '=')
        {
            advance(lexer, state);
            return makeToken(lexer, TOKEN_GREATER_EQUAL, state);
        }
        return makeToken(lexer, TOKEN_GREATER, state);
    case '|':
        return makeToken(lexer, TOKEN_PIPE, state);
    case '^':
        return makeToken(lexer, TOKEN_CARET, state);
    case '~':
        return makeToken(lexer, TOKEN_TILDE, state);
    case '?':
        return makeToken(lexer, TOKEN_QUESTION, state);
    default:
        return errorToken(lexer, "Unexpected character.", state);
    }

    return errorToken(lexer, "Unexpected character.", state);
}
// </symbolChar>

// <identifier>
Token identifier(Lexer *lexer, CompilerState *state)
{
    char *cur_token = my_strndup(lexer->start, lexer->current - lexer->start);
    // printf("\n!![Lexer] Current token: %s\n", cur_token);
    while (isAlpha(peek(lexer, state)) || isDigit(peek(lexer, state)))
    {
        // printf("\n!![Lexer] Current token: %s\n", cur_token);
        advance(lexer, state);
    }
    if (peek(lexer, state) == '[')
    {
        // Peek next character for a number specificly for array indexing
        if (isDigit(peekNext(lexer, state)))
        {
            // printf("\n!![Lexer] Current token: %s\n", cur_token);
            advance(lexer, state);
            while (isDigit(peek(lexer, state)))
            {
                // printf("\n!![Lexer] Current token: %s\n", cur_token);
                advance(lexer, state);
            }
        }
        else
        {
            // printf("\n!![Lexer] Current token: %s\n", cur_token);
            exit(0);
        }
    }
    // printf("\n!![Lexer] Current token: %s\n", cur_token);
    Token token = makeToken(lexer, TOKEN_IDENTIFIER, state);
    // printf("[Lexer] Identifier token: %.*s\n", token.length, token.start);
    return token;
}
// </identifier>

/* =========================================================== */
/* @Data_Types */

// <checkKeyword>
Token checkKeyword(Lexer *lexer, CompilerState *state)
{
    while (isAlpha(peek(lexer, state)))
        advance(lexer, state);

    int length = (int)(lexer->current - lexer->start);
    const char *keyword = my_strndup(lexer->start, length);
    // printf("[Lexer] Checking keyword: %s\n", keyword);
    char *nextChar = my_strndup(lexer->current, 1);
    char *nextnextChar = my_strndup(lexer->current + 1, 1);
    // printf("[Lexer] Next character: %s\n", nextChar);
    // printf("[Lexer] Next next character: %s\n", nextnextChar);

    char *typeStr = my_strndup(lexer->start, length);

    // check if after the keyword is a `[` character and a `]` character immediately after
    if (strcmp(nextChar, "[") == 0 && strcmp(nextnextChar, "]") == 0)
    {
        // printf("[Lexer] Next character is an array type\n");
        // append `[]` to the typeStr to indicate an array type
        char *arrayType = (char *)malloc(strlen(typeStr) + 3); // +3 for '[]' and null terminator
        if (arrayType == NULL)
        {
            // Handle memory allocation failure
            fprintf(stderr, "Memory allocation failed\n");
            free(typeStr);
            exit(1);
        }
        strcpy(arrayType, typeStr);
        strcat(arrayType, "[]");
        // Consuming the `]` character
        // printf("[Lexer] Array type: %s\n", arrayType);

        // Free the old typeStr and update it with the new array type
        free(typeStr);
        typeStr = arrayType;

        advance(lexer, state);
        advance(lexer, state);
    }
    else
    {
        // printf("[Lexer] Next character is not an array type\n");
    }

    // printf("\n\n\nType String: %s\n\n\n", typeStr);

    // Check if the keyword is a data type
    CryoTokenType type = checkDataType(lexer, typeStr, TOKEN_IDENTIFIER, state);
    if (type != TOKEN_IDENTIFIER)
    {
        free((char *)keyword); // Free allocated memory
        return makeToken(lexer, type, state);
    }

    for (int i = 0; keywords[i].keyword != NULL; i++)
    {
        if (strcmp(keywords[i].keyword, keyword) == 0)
        {
            free((char *)keyword); // Free allocated memory
            return makeToken(lexer, keywords[i].type, state);
        }
    }

    free((char *)keyword); // Free allocated memory
    return makeToken(lexer, TOKEN_IDENTIFIER, state);
}
// </checkKeyword>

// <checkDataType>
CryoTokenType checkDataType(Lexer *lexer, const char *dataType, CryoTokenType type, CompilerState *state)
{
    // printf("[Lexer] Checking data type: %s\n", dataType);
    // Check if the next token is the `[` character to determine if it is an array type
    if (peek(lexer, state) == '[')
    {
        if (peekNext(lexer, state) == ']')
        {
            logMessage("INFO", __LINE__, "Lexer", "Parsing array type...");
            advance(lexer, state);
            // append the `[]` to the data type
            char *arrayType = (char *)malloc(strlen(dataType) + 2);
            strcpy(arrayType, dataType);
            strcat(arrayType, "[]");
            // Consuming the `]` character
            advance(lexer, state);
            return checkDataType(lexer, arrayType, type, state);
        }
        if (isDigit(peekNext(lexer, state)))
        {
            logMessage("INFO", __LINE__, "Lexer", "Parsing array index...");
            while (isDigit(peek(lexer, state)))
            {
                advance(lexer, state);
            }
        }
    }
    int i = 0;
    while (dataTypes[i].dataType != NULL)
    {
        const char *currentDataType = CryoTokenToString(dataTypes[i].dataType);
        if (strcmp(currentDataType, dataType) == 0)
        {
            return dataTypes[i].dataType;
        }
        i++;
    }
    return type;
}
// </checkDataType>

/* =========================================================== */
/* @DataType_Evaluation */

bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool isType(char c)
{
    return c == '[' || c == ']';
}
