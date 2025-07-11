/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "frontend/lexer.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

// #################################//
//        Keyword Dictionary.       //
// #################################//
KeywordToken keywords[] = {
    {"public", TOKEN_KW_PUBLIC},
    {"private", TOKEN_KW_PRIVATE},
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
    {"default", TOKEN_KW_DEFAULT},
    {"constructor", TOKEN_KW_CONSTRUCTOR},
    {"this", TOKEN_KW_THIS},
    {"new", TOKEN_KW_NEW},
    {"extends", TOKEN_KW_EXTENDS},
    {"static", TOKEN_KW_STATIC},
    {"protected", TOKEN_KW_PROTECTED},
    {"class", TOKEN_KW_CLASS},
    {"using", TOKEN_KW_USING},
    {"extends", TOKEN_KW_EXTENDS},
    {"typeof", TOKEN_KW_TYPEOF},
    {"module", TOKEN_KW_MODULE},
    {"declare", TOKEN_KW_DECLARE},
    {"any", TOKEN_KW_ANY},
    {"null", TOKEN_KW_NULL},
    {"i8", TOKEN_TYPE_I8},
    {"i16", TOKEN_TYPE_I16},
    {"i32", TOKEN_TYPE_I32},
    {"i64", TOKEN_TYPE_I64},
    {"i128", TOKEN_TYPE_I128},
    {"float", TOKEN_TYPE_FLOAT},
    {"auto", TOKEN_KW_AUTO},
    {"undefined", TOKEN_KW_UNDEFINED},
    {"pragma", TOKEN_KW_PRAGMA},
    {"type", TOKEN_KW_TYPE},
    {"implement", TOKEN_KW_IMPLEMENT},
    {NULL, TOKEN_UNKNOWN} // Sentinel value
};

#define X(str, type) DECLARE_TOKEN(str, type),
DataTypeToken dataTypes[] = {
    DATA_TYPE_TABLE
        DECLARE_TOKEN(NULL, TOKEN_UNKNOWN) // Sentinel value
};
#undef X

/* =========================================================== */
/* @Lexer */

// <initLexer>
void initLexer(Lexer *lexer, const char *source, const char *fileName, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Lexer", "Initializing lexer...");
    lexer->start = source;
    lexer->current = source;
    lexer->end = source + strlen(source);
    lexer->line = 1;
    lexer->column = 0;
    lexer->hasPeeked = false;
    lexer->fileName = fileName;
    lexer->nextToken = peekNextToken(lexer, state);
    lexer->source = source;

    lexer->getLPos = getLPos;
    lexer->getCPos = getCPos;

    GDM->initFrontendState(GDM);
    GDM->frontendState->setLexer(GDM->frontendState, lexer);

    DEBUG_PRINT_FILTER({
        printf("{lexer} -------------- <Input Source Code> --------------\n\n");
        printf("\n{lexer} File Name: %s\n", fileName);
        printf("\n{lexer} Lexer initialized. \nStart: %p \nCurrent: %p \n\nSource:\n-------\n%s\n\n", lexer->start, lexer->current, source);
        printf("\n{lexer} -------------------- <END> ----------------------\n\n");
    });
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
    __STACK_FRAME__
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

bool isModuleFile(Lexer *lexer)
{
    __STACK_FRAME__
    // Check if the file is a module file ({FILENAME}.mod.cryo)
    const char *needle = ".mod.cryo";
    if (strstr(lexer->fileName, needle) != NULL)
    {
        return true;
    }
    return false;
}

const char *getCurrentFileLocationFromLexer(Lexer *lexer)
{
    __STACK_FRAME__
    return lexer->fileName;
}

// <getLPos>
int getLPos(Lexer *lexer)
{
    __STACK_FRAME__
    return lexer->line;
}
// </getLPos>

// <getCPos>
int getCPos(Lexer *lexer)
{
    __STACK_FRAME__
    return lexer->column;
}
// </getCPos>

// <advance>
char advance(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
    return *lexer->current == '\0' || lexer->current >= lexer->end;
}
// </isAtEnd>

// <peek>
char peek(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    return *lexer->current;
}
// </peek>

// <peekNext>
char peekNext(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    if (isAtEnd(lexer, state))
        return '\0';
    char c = *lexer->current;
    logMessage(LMI, "INFO", "Lexer", "Peeking next character: %c", c);
    return c;
}
// </peekNext>

// <peekNextUnconsumedLexerToken>
char peekNextUnconsumedLexerToken(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    Lexer tempLexer = *lexer;
    char c = advance(&tempLexer, state);
    return c;
}
// </peekNextUnconsumedLexerToken>

// <currentChar>
char currentChar(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    return *lexer->current;
}
// </currentChar>

// <matchToken>
bool matchToken(Lexer *lexer, CryoTokenType type, CompilerState *state)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
        {
            advance(lexer, state);
            char next = peekNextUnconsumedLexerToken(lexer, state);
            if (next == '/')
            {
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
                    logMessage(LMI, "ERROR", "Lexer", "Unterminated block comment.");
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
        }
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
    __STACK_FRAME__
    skipWhitespace(lexer, state);

    lexer->start = lexer->current;

    if (isAtEnd(lexer, state))
    {
        logMessage(LMI, "INFO", "Lexer", "Creating EOF token");
        *token = makeToken(lexer, TOKEN_EOF, state);
        return *token;
    }

    char c = advance(lexer, state);

    if (isAlpha(c))
    {
        *token = checkKeyword(lexer, state);
        return *token;
    }

    if (isDigit(c))
    {
        *token = number(lexer, state, false);
        return *token;
    }

    if (c == '-')
    {
        // For negative numbers
        if (isDigit(peek(lexer, state)))
        {
            *token = number(lexer, state, true);
            return *token;
        }
        // For arrow operator
        else if (peek(lexer, state) == '>')
        {
            advance(lexer, state);
            *token = makeToken(lexer, TOKEN_RESULT_ARROW, state);
            return *token;
        }
        // For minus operator
        else
        {
            *token = makeToken(lexer, TOKEN_MINUS, state);
            return *token;
        }
    }

    if (c == '"')
    {
        *token = string(lexer, state);
        return *token;
    }

    if (c == '&')
    {
        *token = makeToken(lexer, TOKEN_AMPERSAND, state);
        return *token;
    }
    Token symToken = symbolChar(lexer, c, state);
    if (symToken.type != TOKEN_UNKNOWN)
    {
        *token = symToken;
        return *token;
    }

    logMessage(LMI, "ERROR", "Lexer", "Unexpected character: %c", c);
    return identifier(lexer, state);
}
// </nextToken>

// <get_next_token>
Token get_next_token(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    // printf("[Lexer] Getting next token...\n");
    nextToken(lexer, &lexer->currentToken, state);

    lexer->nextToken = peekNextToken(lexer, state);

    return lexer->currentToken;
}
// </get_next_token>

// <getToken>
Token getToken(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    // printf("\n\nCreating Token: %s\n\n", CryoTokenToString(type));
    // current token
    char *currentToken = strndup(lexer->start, lexer->current - lexer->start);
    // printf("Current Token: %s\n", currentToken);

    int tokLen = (int)(lexer->current - lexer->start);
    int col = lexer->column - tokLen;

    Token token;
    token.lexeme = currentToken;
    token.type = type;
    token.start = lexer->start;
    token.length = tokLen;
    token.line = lexer->line;
    token.column = col;
    token.isOperator = isOperatorToken(type);

    // logMessage(LMI, "INFO", "Lexer", "Token created: Type: %s, Line: %d, Column: %d", CryoTokenToString(token.type), token.line, token.column);
    return token;
}
// </makeToken>

// <errorToken>
Token errorToken(Lexer *lexer, const char *message, CompilerState *state)
{
    __STACK_FRAME__
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
Token number(Lexer *lexer, CompilerState *state, bool isNegative)
{
    __STACK_FRAME__
    if (isNegative)
    {
        advance(lexer, state); // Consume the minus sign
    }

    while (isDigit(peek(lexer, state)))
    {
        advance(lexer, state);
    }

    Token token = makeToken(lexer, TOKEN_INT_LITERAL, state);
    if (isNegative)
    {
        // Prepend the minus sign to the lexeme
        char *negativeLexeme = (char *)malloc(token.length + 2); // +1 for '-' and +1 for '\0'
        negativeLexeme[0] = '-';
        strncpy(negativeLexeme + 1, token.lexeme, token.length);
        negativeLexeme[token.length + 1] = '\0';

        token.lexeme = negativeLexeme;
        token.length += 1;
    }

    return token;
}
// </number>

// <string>
Token string(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    case '@':
        return makeToken(lexer, TOKEN_AT, state);
    case '#':
        return makeToken(lexer, TOKEN_HASH, state);
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
    __STACK_FRAME__
    char *cur_token = strndup(lexer->start, lexer->current - lexer->start);
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
    __STACK_FRAME__
    while (isAlpha(peek(lexer, state)) || isDigit(peek(lexer, state)))
        advance(lexer, state);

    int length = (int)(lexer->current - lexer->start);
    const char *keyword = strndup(lexer->start, length);
    // printf("[Lexer] Checking keyword: %s\n", keyword);
    char *nextChar = strndup(lexer->current, 1);
    char *nextnextChar = strndup(lexer->current + 1, 1);
    // printf("[Lexer] Next character: %s\n", nextChar);
    // printf("[Lexer] Next next character: %s\n", nextnextChar);

    char *typeStr = strndup(lexer->start, length);

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
    __STACK_FRAME__
    // Check if the next token is the `[` character to determine if it is an array type
    if (peek(lexer, state) == '[')
    {
        if (peekNext(lexer, state) == ']')
        {
            logMessage(LMI, "INFO", "Lexer", "Parsing array type...");
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
            logMessage(LMI, "INFO", "Lexer", "Parsing array index...");
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

Token handleTypeIdentifier(Lexer *lexer, CompilerState *state)
{
    __STACK_FRAME__
    // Skip the initial ':'
    advance(lexer, state);
    skipWhitespace(lexer, state);

    // Parse the type identifier
    if (!isAlpha(peek(lexer, state)))
    {
        return errorToken(lexer, "Expected type identifier", state);
    }

    while (isAlpha(peek(lexer, state)) || isDigit(peek(lexer, state)))
    {
        advance(lexer, state);
    }

    // Check for array syntax
    if (peek(lexer, state) == '[')
    {
        while (peek(lexer, state) == '[')
        {
            advance(lexer, state); // Consume '['

            // Allow optional size
            while (isDigit(peek(lexer, state)))
            {
                advance(lexer, state);
            }

            if (peek(lexer, state) != ']')
            {
                return errorToken(lexer, "Expected ']'", state);
            }
            advance(lexer, state); // Consume ']'
        }
    }

    return makeToken(lexer, TOKEN_TYPE_IDENTIFIER, state);
}

/* =========================================================== */
/* @DataType_Evaluation */

bool isAlpha(char c)
{
    __STACK_FRAME__
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c)
{
    __STACK_FRAME__
    return c >= '0' && c <= '9';
}

bool isType(char c)
{
    __STACK_FRAME__
    return c == '[' || c == ']';
}

bool isOperatorToken(CryoTokenType type)
{
    __STACK_FRAME__
    switch (type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_STAR:
    case TOKEN_SLASH:
    case TOKEN_PERCENT:
    case TOKEN_AMPERSAND:
    case TOKEN_PIPE:
    case TOKEN_CARET:
    case TOKEN_TILDE:
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
    case TOKEN_LESS:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER:
    case TOKEN_GREATER_EQUAL:
    case TOKEN_BANG:
    case TOKEN_RESULT_ARROW:
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
    case TOKEN_DOUBLE_COLON:
    case TOKEN_ELLIPSIS:
        return true;
    default:
        return false;
    }
}
