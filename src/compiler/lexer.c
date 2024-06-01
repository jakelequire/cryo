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
/*------ <includes> ------*/
#include "lexer.h"
/*---------<end>---------*/

//#################################
// Keyword Dictionary. 
//#################################
KeywordToken keywords[] = {
    {"public", TOKEN_KW_PUBLIC},
    {"fn", TOKEN_KW_FN},
    {"if", TOKEN_KW_IF},
    {"else", TOKEN_KW_ELSE},
    {"while", TOKEN_KW_WHILE},
    {"for", TOKEN_KW_FOR},
    {"return", TOKEN_KW_RETURN},
    {"const", TOKEN_KW_CONST},
    {"true", TOKEN_KW_TRUE},
    {"false", TOKEN_KW_FALSE},
    {NULL, TOKEN_UNKNOWN} // Sentinel value
};


// <isAlpha>
bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
// </isAlpha>


// <isDigit>
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}
// </isDigit>


// <my_strndup>
char* my_strndup(const char* src, size_t len) {
    char* dest = (char*)malloc(len + 1);
    if (dest) {
        strncpy(dest, src, len);
        dest[len] = '\0';
    } else {
        fprintf(stderr, "{lexer} Error: Memory allocation failed in my_strndup\n");
    }
    return dest;
}
// </my_strndup>


// <initLexer>
void initLexer(Lexer* lexer, const char* source) {
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


// <isAtEnd>
bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}
// </isAtEnd>


// <advance>
char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}
// </advance>


// <peek>
char peek(Lexer* lexer) {
    return *lexer->current;
}
// </peek>


// <peekNext>
char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}
// </peekNext>


// <matchToken>
bool matchToken(Lexer* lexer, CryoTokenType type) {
    if (peekToken(lexer).type == type) {
        nextToken(lexer, &lexer->currentToken);
        return true;
    }
    return false;
}
// </matchToken>


// <skipWhitespace>
void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 1;
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}
// </skipWhitespace>


// <makeToken>
Token makeToken(Lexer* lexer, CryoTokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column;
    printf("{lexer} [DEBUG] Created token: %.*s (Type: %d, Line: %d, Column: %d)\n",
           token.length, token.start, token.type, token.line, token.column);
    return token;
}
// </makeToken>


// <errorToken>
Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}
// </errorToken>


// <checkKeyword>
CryoTokenType checkKeyword(const char* start, int length) {
    for (int i = 0; keywords[i].keyword != NULL; ++i) {
        if (strncmp(start, keywords[i].keyword, length) == 0 && keywords[i].keyword[length] == '\0') {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}
// </checkKeyword>


// <identifier>
Token identifier(Lexer* lexer) {
    while (isAlpha(*lexer->current) || isDigit(*lexer->current)) {
        advance(lexer);
    }
    return makeToken(lexer, TOKEN_IDENTIFIER);
}
// </identifier>


// <number>
Token number(Lexer* lexer) {
    while (isDigit(*lexer->current)) {
        advance(lexer);
    }
    return makeToken(lexer, TOKEN_TYPE_INT);
}
// </number>


// <get_next_token>
Token get_next_token(Lexer *lexer) {
    nextToken(lexer, &lexer->currentToken);
    //printf("{lexer} [DEBUG] Next token: %.*s, Type: %d, Line: %d, Column: %d\n",
    //        lexer->currentToken.length,
    //        lexer->currentToken.start,
    //        lexer->currentToken.type,
    //        lexer->currentToken.line,
    //        lexer->currentToken.column
    //    );
    return lexer->currentToken;
}
// </get_next_token>


// <nextToken>
Token nextToken(Lexer* lexer, Token* token) {
    skipWhitespace(lexer);

    lexer->start = lexer->current;

    if (isAtEnd(lexer)) {
        *token = makeToken(lexer, TOKEN_EOF);
        return *token;
    }
    printf("{lexer} Processing char: '%c' at line %d, column %d\n", *lexer->current, lexer->line, lexer->column);

    char c = advance(lexer);

    if (isAlpha(c)) {
        *token = identifier(lexer);
        return *token;
    }

    if (isDigit(c)) {
        *token = number(lexer);
        return *token;
    }

    switch (c) {
        case '(': *token = makeToken(lexer, TOKEN_LPAREN); break;
        case ')': *token = makeToken(lexer, TOKEN_RPAREN); break;
        case '+': *token = makeToken(lexer, TOKEN_OP_PLUS); break;
        case '-':
            if (peek(lexer) == '>') {
                advance(lexer); // consume '>'
                *token = makeToken(lexer, TOKEN_RESULT_ARROW);
            } else {
                *token = makeToken(lexer, TOKEN_OP_MINUS);
            }
            break;
        case '*': *token = makeToken(lexer, TOKEN_OP_STAR); break;
        case '/': *token = makeToken(lexer, TOKEN_OP_SLASH); break;
        case '=': *token = makeToken(lexer, TOKEN_ASSIGN); break;
        case ';': *token = makeToken(lexer, TOKEN_SEMICOLON); break;
        case ',': *token = makeToken(lexer, TOKEN_COMMA); break;
        case ':': *token = makeToken(lexer, TOKEN_COLON); break;
        case '{': *token = makeToken(lexer, TOKEN_LBRACE); break;
        case '}': *token = makeToken(lexer, TOKEN_RBRACE); break;
        default:
            *token = makeToken(lexer, TOKEN_ERROR);
            return *token;
    }

    return *token;
}
// </nextToken>


// <getToken>
Token getToken(Lexer* lexer) {
    if (lexer->hasPeeked) {
        lexer->hasPeeked = false;
        return lexer->lookahead;
    } else {
        nextToken(lexer, &lexer->currentToken);
        return lexer->currentToken;
    }
}
// </getToken>


// <peekToken>
Token peekToken(Lexer* lexer) {
    if (!lexer->hasPeeked) {
        nextToken(lexer, &lexer->lookahead);
        lexer->hasPeeked = true;
    }
    return lexer->lookahead;
}
// </peekToken>


// <readFile>
char* readFile(const char* path) {
    FILE* file;
    errno_t err = fopen_s(&file, path, "rb");  // Open the file in binary mode to avoid transformations
    if (err != 0) {
        perror("{lexer} Could not open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    // handle fseek error
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        perror("{lexer} File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)malloc(length + 1);
    if (buffer == NULL) {
        perror("{lexer} Not enough memory to read file");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, length, file);
    if (bytesRead < length) {
        perror("{lexer} Failed to read the full file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';  // Null-terminate the buffer
    fclose(file);
    return buffer;
}
// </readFile>


// <freeLexer>
void freeLexer(Lexer* lexer) {
    free(lexer);
}
// </freeLexer>


// <lexer>
int lexer(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "{lexer} Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Token token;
    do {
        nextToken(&lexer, &token);
        printf("{lexer} [!DEBUG!] Parsing token: %.*s, Type: %d\n", token.length, token.start, token.type);
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
// </lexer>
