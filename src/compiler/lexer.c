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
void initLexer(Lexer *lexer, const char *source)
{
    printf("Starting Lexer Initialization...\n");
    lexer->start = source;
    lexer->current = source;
    lexer->end = source + strlen(source);
    lexer->line = 1;
    lexer->column = 0;
    lexer->hasPeeked = false;

    printf("{lexer} -------------- <Input Source Code> --------------\n\n");
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

// <advance>
char advance(Lexer *lexer)
{
    // printf("[Lexer] Advancing to next character: %c\n", lexer->current[0]);
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}
// </advance>

// <isAtEnd>
bool isAtEnd(Lexer *lexer)
{
    return *lexer->current == '\0' || *lexer->current == NULL || lexer->current >= lexer->end;
}
// </isAtEnd>

// <peek>
char peek(Lexer *lexer)
{
    return *lexer->current;
}
// </peek>c

// <peekNext>
char peekNext(Lexer *lexer)
{
    if (isAtEnd(lexer))
        return '\0';
    printf("[Lexer] Peeking next character: %c\n", lexer->current[1]);
    return (char)&lexer->current[1];
}
// </peekNext>

// <peekNextUnconsumedLexerToken>
char peekNextUnconsumedLexerToken(Lexer *lexer)
{
    Lexer tempLexer = *lexer;
    return peekNextToken(&tempLexer).type;
}
// </peekNextUnconsumedLexerToken>

// <currentChar>
char currentChar(Lexer *lexer)
{
    return *lexer->current;
}
// </currentChar>

// <matchToken>
bool matchToken(Lexer *lexer, CryoTokenType type)
{
    if (peekToken(lexer).type == type)
    {
        nextToken(lexer, &lexer->currentToken);
        return true;
    }
    return false;
}
// </matchToken>

// <skipWhitespace>
void skipWhitespace(Lexer *lexer)
{
    for (;;)
    {
        char c = peek(lexer);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(lexer);
            break;
        case '\n':
            lexer->line++;
            lexer->column = 0;
            advance(lexer);
            break;
        default:
            return;
        }
    }
    return;
}
// </skipWhitespace>

void skipComment(Lexer *lexer)
{
    char c = currentChar(lexer);
    logMessage("INFO", __LINE__, "Lexer", "Skipping comment... Current character: %c", c);
    if (c == '/')
    {
        logMessage("INFO", __LINE__, "Lexer", "Skipping single line comment...");
        if (peek(lexer) == '/')
        {
            logMessage("INFO", __LINE__, "Lexer", "Skipping comment...");
            while (currentChar(lexer) != '\n' && !isAtEnd(lexer))
            {
                logMessage("INFO", __LINE__, "Lexer", "Skipping comment... Current character: %c", currentChar(lexer));
                advance(lexer);
            }
        }
        else
        {
            return;
        }
    }

    if (c == '*')
    {
        logMessage("INFO", __LINE__, "Lexer", "Skipping multi-line comment...");
        while (currentChar(lexer) != '*' && peekNext(lexer) != '/' && !isAtEnd(lexer))
        {
            logMessage("INFO", __LINE__, "Lexer", "Skipping comment... Current character: %c", currentChar(lexer));
            advance(lexer);
        }
    }

    return;
}

/* =========================================================== */
/* @Lexer_Functions */

// <nextToken>
Token nextToken(Lexer *lexer, Token *token)
{
    skipWhitespace(lexer);
    skipComment(lexer);
    skipWhitespace(lexer);

    lexer->start = lexer->current;

    if (isAtEnd(lexer))
    {
        logMessage("INFO", __LINE__, "Lexer", "Creating EOF token");
        *token = makeToken(lexer, TOKEN_EOF);
        return *token;
    }

    char c = advance(lexer);

    logMessage("INFO", __LINE__, "Lexer", "Current character: %c", c);

    if (isAlpha(c))
    {
        // logMessage("INFO", __LINE__, "Lexer", "Creating identifier token");
        *token = checkKeyword(lexer);
        logMessage("INFO", __LINE__, "Lexer", "Keyword token created");
        // printf("[Lexer] Identifier token created: %.*s\n", token->length, token->start);
        return *token;
    }

    if (isDigit(c))
    {
        // logMessage("INFO", __LINE__, "Lexer", "Creating number token");
        *token = number(lexer);
        logMessage("INFO", __LINE__, "Lexer", "Number token created");
        return *token;
    }

    if (c == '"')
    {
        // logMessage("INFO", __LINE__, "Lexer", "Creating string token");
        *token = string(lexer);
        logMessage("INFO", __LINE__, "Lexer", "String token created");
        return *token;
    }

    if (c == '&')
    {
        // logMessage("INFO", __LINE__, "Lexer", "Creating ampersand token");
        *token = makeToken(lexer, TOKEN_AMPERSAND);
        logMessage("INFO", __LINE__, "Lexer", "Ampersand token created");
        return *token;
    }
    // logMessage("INFO", __LINE__, "Lexer", "Creating symbol token");
    Token symToken = symbolChar(lexer, c);
    if (symToken.type != TOKEN_UNKNOWN)
    {
        *token = symToken;
        logMessage("INFO", __LINE__, "Lexer", "Symbol token created");
        return *token;
    }

    // printf("\n\n\n\n\nCurrent Token: %s \n\n\n\n\n", CryoTokenToString(peekToken(lexer).type));

    logMessage("ERROR", __LINE__, "Lexer", "Unexpected character: %c", c);
    return identifier(lexer);
}
// </nextToken>

// <get_next_token>
Token get_next_token(Lexer *lexer)
{
    // printf("[Lexer] Getting next token...\n");
    nextToken(lexer, &lexer->currentToken);
    return lexer->currentToken;
}
// </get_next_token>

// <getToken>
Token getToken(Lexer *lexer)
{
    if (lexer->hasPeeked)
    {
        lexer->hasPeeked = false;
        return lexer->lookahead;
    }
    else
    {
        nextToken(lexer, &lexer->currentToken);
        return lexer->currentToken;
    }
}
// </getToken>

// <peekToken>
Token peekToken(Lexer *lexer)
{
    if (!lexer->hasPeeked)
    {
        nextToken(lexer, &lexer->lookahead);
        lexer->hasPeeked = true;
    }
    return lexer->lookahead;
}
// </peekToken>

// <peekNextToken>
Token peekNextToken(Lexer *lexer)
{
    Lexer tempLexer = *lexer;                    // Copy the current lexer state
    Token tempNextToken = peekToken(&tempLexer); // Get the next token from the copied state
    return tempNextToken;
}
// </peekNextToken>

/* =========================================================== */
/* @Token_Creation */

// <makeToken>
Token makeToken(Lexer *lexer, CryoTokenType type)
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
Token errorToken(Lexer *lexer, const char *message)
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
Token number(Lexer *lexer)
{
    while (isDigit(peek(lexer)))
    {
        advance(lexer);
    }
    Token token = makeToken(lexer, TOKEN_INT_LITERAL);
    // printf("[Lexer] Number token: %.*s\n", token.length, token.start);
    return token;
}
// </number>

// <string>
Token string(Lexer *lexer)
{
    while (peek(lexer) != '"' && !isAtEnd(lexer))
    {
        if (peek(lexer) == '\n')
            lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer))
        return errorToken(lexer, "Unterminated string.");

    advance(lexer);
    Token token = makeToken(lexer, TOKEN_STRING_LITERAL);
    // printf("[Lexer] String token: %.*s\n", token.length, token.start);
    return token;
}
// </string>

// <boolean>
Token boolean(Lexer *lexer)
{
    if (peek(lexer) == 't' && peekNext(lexer) == 'r' && peekNext(lexer) == 'u' && peekNext(lexer) == 'e')
    {
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
    }
    else if (peek(lexer) == 'f' && peekNext(lexer) == 'a' && peekNext(lexer) == 'l' && peekNext(lexer) == 's' && peekNext(lexer) == 'e')
    {
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
    }
    else
    {
        return errorToken(lexer, "Invalid boolean literal.");
    }

    Token token = makeToken(lexer, TOKEN_BOOLEAN_LITERAL);
    // printf("[Lexer] Boolean token: %.*s\n", token.length, token.start);
    return token;
}
// </boolean>

// <symbolChar>
Token symbolChar(Lexer *lexer, char symbol)
{
    switch (symbol)
    {
    case '(':
        return makeToken(lexer, TOKEN_LPAREN);
    case ')':
        return makeToken(lexer, TOKEN_RPAREN);
    case '{':
        return makeToken(lexer, TOKEN_LBRACE);
    case '}':
        return makeToken(lexer, TOKEN_RBRACE);
    case '[':
        return makeToken(lexer, TOKEN_LBRACKET);
    case ']':
        return makeToken(lexer, TOKEN_RBRACKET);
    case ',':
        return makeToken(lexer, TOKEN_COMMA);
    case '$':
        return makeToken(lexer, TOKEN_DOLLAR);
    case '/':
        return makeToken(lexer, TOKEN_SLASH);
    case '.':
    {
        if (peek(lexer) == '.')
        {
            if (peekNext(lexer) == '.')
            {
                advance(lexer);
                advance(lexer);
                return makeToken(lexer, TOKEN_ELLIPSIS);
            }
        }
        else
        {
            return makeToken(lexer, TOKEN_DOT);
        }
    }
    case ':':
        return makeToken(lexer, TOKEN_COLON);
    case '-':
        if (peek(lexer) == '>')
        {
            advance(lexer);
            return makeToken(lexer, TOKEN_RESULT_ARROW);
        }
        return makeToken(lexer, TOKEN_MINUS);
    case '+':
        if (peek(lexer) == '+')
        {
            advance(lexer);
            return makeToken(lexer, TOKEN_INCREMENT);
        }
        return makeToken(lexer, TOKEN_PLUS);
    case ';':
        return makeToken(lexer, TOKEN_SEMICOLON);
    case '*':
        return makeToken(lexer, TOKEN_STAR);
    case '%':
        return makeToken(lexer, TOKEN_PERCENT);
    case '!':
    {
        if (peek(lexer) == '=')
        {
            advance(lexer);
            return makeToken(lexer, TOKEN_NOT_EQUAL);
        }
        return makeToken(lexer, TOKEN_BANG);
    }
    case '=':
    {
        if (peek(lexer) == '=')
        {
            advance(lexer);
            return makeToken(lexer, TOKEN_STRICT_EQUAL);
        }
        return makeToken(lexer, TOKEN_EQUAL);
    }
    case '<':
        return makeToken(lexer, TOKEN_LESS);
    case '>':
        return makeToken(lexer, TOKEN_GREATER);
    case '|':
        return makeToken(lexer, TOKEN_PIPE);
    case '^':
        return makeToken(lexer, TOKEN_CARET);
    case '~':
        return makeToken(lexer, TOKEN_TILDE);
    case '?':
        return makeToken(lexer, TOKEN_QUESTION);
    default:
        return errorToken(lexer, "Unexpected character.");
    }

    return errorToken(lexer, "Unexpected character.");
}
// </symbolChar>

// <identifier>
Token identifier(Lexer *lexer)
{
    char *cur_token = my_strndup(lexer->start, lexer->current - lexer->start);
    // printf("\n!![Lexer] Current token: %s\n", cur_token);
    while (isAlpha(peek(lexer)) || isDigit(peek(lexer)))
    {
        // printf("\n!![Lexer] Current token: %s\n", cur_token);
        advance(lexer);
    }
    if (peek(lexer) == '[')
    {
        // Peek next character for a number specificly for array indexing
        if (isDigit(peekNext(lexer)))
        {
            // printf("\n!![Lexer] Current token: %s\n", cur_token);
            advance(lexer);
            while (isDigit(peek(lexer)))
            {
                // printf("\n!![Lexer] Current token: %s\n", cur_token);
                advance(lexer);
            }
        }
        else
        {
            // printf("\n!![Lexer] Current token: %s\n", cur_token);
            exit(0);
        }
    }
    // printf("\n!![Lexer] Current token: %s\n", cur_token);
    Token token = makeToken(lexer, TOKEN_IDENTIFIER);
    // printf("[Lexer] Identifier token: %.*s\n", token.length, token.start);
    return token;
}
// </identifier>

/* =========================================================== */
/* @Data_Types */

// <checkKeyword>
Token checkKeyword(Lexer *lexer)
{
    while (isAlpha(peek(lexer)))
        advance(lexer);

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

        advance(lexer);
        advance(lexer);
    }
    else
    {
        // printf("[Lexer] Next character is not an array type\n");
    }

    // printf("\n\n\nType String: %s\n\n\n", typeStr);

    // Check if the keyword is a data type
    CryoTokenType type = checkDataType(lexer, typeStr, TOKEN_IDENTIFIER);
    if (type != TOKEN_IDENTIFIER)
    {
        free((char *)keyword); // Free allocated memory
        return makeToken(lexer, type);
    }

    for (int i = 0; keywords[i].keyword != NULL; i++)
    {
        if (strcmp(keywords[i].keyword, keyword) == 0)
        {
            free((char *)keyword); // Free allocated memory
            return makeToken(lexer, keywords[i].type);
        }
    }

    free((char *)keyword); // Free allocated memory
    return makeToken(lexer, TOKEN_IDENTIFIER);
}
// </checkKeyword>

// <checkDataType>
CryoTokenType checkDataType(Lexer *lexer, const char *dataType, CryoTokenType type)
{
    // printf("[Lexer] Checking data type: %s\n", dataType);
    // Check if the next token is the `[` character to determine if it is an array type
    if (peek(lexer) == '[')
    {
        if (peekNext(lexer) == ']')
        {
            logMessage("INFO", __LINE__, "Lexer", "Parsing array type...");
            advance(lexer);
            // append the `[]` to the data type
            char *arrayType = (char *)malloc(strlen(dataType) + 2);
            strcpy(arrayType, dataType);
            strcat(arrayType, "[]");
            // Consuming the `]` character
            advance(lexer);
            return checkDataType(lexer, arrayType, type);
        }
        if (isDigit(peekNext(lexer)))
        {
            logMessage("INFO", __LINE__, "Lexer", "Parsing array index...");
            while (isDigit(peek(lexer)))
            {
                advance(lexer);
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

/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */

int lexer(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "{lexer} Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char *source = readFile(argv[1]);
    if (source == NULL)
        return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;
    do
    {
        nextToken(&lexer, &token);
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
