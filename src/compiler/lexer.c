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


//#################################//
//       Keyword Dictionary.       //
//#################################//
KeywordToken keywords[] = {
    {"public",      TOKEN_KW_PUBLIC             },
    {"function",    TOKEN_KW_FN                 },
    {"if",          TOKEN_KW_IF                 },
    {"else",        TOKEN_KW_ELSE               },
    {"while",       TOKEN_KW_WHILE              },
    {"for",         TOKEN_KW_FOR                },
    {"return",      TOKEN_KW_RETURN             },
    {"const",       TOKEN_KW_CONST              },
    {"mut",         TOKEN_KW_MUT                },
    {"true",        TOKEN_BOOLEAN_LITERAL       },
    {"false",       TOKEN_BOOLEAN_LITERAL       },
    {"int",         TOKEN_KW_INT                },
    {"string",      TOKEN_KW_STRING             },
    {"boolean",     TOKEN_KW_BOOL               },
    {"expression",  TOKEN_KW_EXPRESSION         },
    {"void",        TOKEN_KW_VOID               },
    {"null",        TOKEN_KW_NULL               },
    {"struct",      TOKEN_KW_STRUCT             },
    {"enum",        TOKEN_KW_ENUM               },
    {"break",       TOKEN_KW_BREAK              },
    {"extern",      TOKEN_KW_EXTERN             },
    {"continue",    TOKEN_KW_CONTINUE           },
    {"import",      TOKEN_KW_IMPORT             },
    {NULL,          TOKEN_UNKNOWN               }  // Sentinel value
};


//#################################//
//       DataType Dictionary.      //
//#################################//
DataTypeToken dataTypes[] = {
    {"int",         TOKEN_TYPE_INT              },
    {"string",      TOKEN_TYPE_STRING           },
    {"bool",        TOKEN_TYPE_BOOLEAN          },
    {"void",        TOKEN_TYPE_VOID             },
    {"int[]",       TOKEN_TYPE_INT_ARRAY        },
    {"string[]",    TOKEN_TYPE_STRING_ARRAY     },
    {"boolean[]",   TOKEN_TYPE_BOOLEAN_ARRAY    },
    {NULL,          TOKEN_UNKNOWN               }  // Sentinel value
};

/* =========================================================== */
/* @Util_Functions */

// <my_strndup>
char* my_strndup(const char* src, size_t len) {
    char* dest = (char*)malloc(len + 1);
    if (dest) {
        strncpy(dest, src, len);
        dest[len] = '\0';
    } else {
        fprintf(stderr, "[Lexer] Error: Memory allocation failed in my_strndup\n");
    }
    return dest;
}
// </my_strndup>



/* =========================================================== */
/* @Lexer */

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


// <freeLexer>
void freeLexer(Lexer* lexer) {
    free(lexer);
}
// </freeLexer>



/* =========================================================== */
/* @Lexer_Utils */


// <advance>
char* advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->current[-1];
}
// </advance>


// <isAtEnd>
bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}
// </isAtEnd>


// <peek>
char* peek(Lexer* lexer) {
    return *lexer->current;
}
// </peek>


// <peekNext>
char* peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}
// </peekNext>


// <matchToken>
bool* matchToken(Lexer* lexer, CryoTokenType type) {
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
                lexer->column = 0;
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                } else if (peekNext(lexer) == '*') {
                    advance(lexer);
                    advance(lexer);
                    while (!(peek(lexer) == '*' && peekNext(lexer) == '/') && !isAtEnd(lexer)) {
                        if (peek(lexer) == '\n') lexer->line++;
                        advance(lexer);
                    }
                    if (!isAtEnd(lexer)) {
                        advance(lexer);
                        advance(lexer);
                    }
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



/* =========================================================== */
/* @Lexer_Functions */

// <nextToken>
Token nextToken(Lexer* lexer, Token* token) {
    skipWhitespace(lexer);

    lexer->start = lexer->current;

    if (isAtEnd(lexer)) {
        return makeToken(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    if (isAlpha(c)) {
        return identifier(lexer);
    }

    if (isDigit(c)) {
        return number(lexer);
    }

    switch(c) {
        case '"':
            return string(lexer);
    }

    return symbolChar(lexer);
}
// </nextToken>


// <get_next_token>
Token get_next_token(Lexer* lexer) {
    Token token = nextToken(lexer, &lexer->lookahead);
    lexer->hasPeeked = true;
    return token;
}
// </get_next_token>


// <getToken>
Token getToken(Lexer* lexer) {
    if (lexer->hasPeeked) {
        lexer->hasPeeked = false;
        return lexer->lookahead;
    } else {
        return get_next_token(lexer);
    }
}
// </getToken>


// <peekToken>
Token peekToken(Lexer* lexer) {
    if (lexer->hasPeeked) {
        return lexer->lookahead;
    } else {
        lexer->lookahead = get_next_token(lexer);
        lexer->hasPeeked = true;
        return lexer->lookahead;
    }
}
// </peekToken>


// <peekNextToken>
Token peekNextToken(Lexer* lexer) {
    if (lexer->hasPeeked) {
        return lexer->lookahead;
    } else {
        return get_next_token(lexer);
    }
}
// </peekNextToken>



/* =========================================================== */
/* @Token_Creation */

// <makeToken>
Token makeToken(Lexer* lexer, CryoTokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column;

    if(token.length > 1) {
        printf("[Lexer] Created token: %.*s (Type: %d, Line: %d, Column: %d)\n",
                token.length, token.start, token.type, token.line, token.column
            );
    }
    return token;
}
// </makeToken>


// <errorToken>
Token errorToken(Lexer* lexer, char* message) {
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
Token number(Lexer* lexer) {
    while (isDigit(*lexer->current)) {
        advance(lexer);
    }
    Token token = makeToken(lexer, TOKEN_INT_LITERAL);
    printf("[Lexer] Number token: %.*s\n", token.length, token.start);
    return token;
}
// </number>


// <string>
Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer)) return errorToken(lexer, "Unterminated string.");

    advance(lexer);
    Token token = makeToken(lexer, TOKEN_STRING_LITERAL);
    printf("[Lexer] String token: %.*s\n", token.length, token.start);
    return token;
}
// </string>


// <boolean>
Token boolean(Lexer* lexer) {
    if (peek(lexer) == 't' && peekNext(lexer) == 'r' && peekNext(lexer) == 'u' && peekNext(lexer) == 'e') {
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
    } else if (peek(lexer) == 'f' && peekNext(lexer) == 'a' && peekNext(lexer) == 'l' && peekNext(lexer) == 's' && peekNext(lexer) == 'e') {
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
        advance(lexer);
    } else {
        return errorToken(lexer, "Invalid boolean literal.");
    }

    Token token = makeToken(lexer, TOKEN_BOOLEAN_LITERAL);
    printf("[Lexer] Boolean token: %.*s\n", token.length, token.start);
    return token;
}
// </boolean>


// <symbolChar>
Token symbolChar(Lexer* lexer) {
    switch (*lexer->current) {
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
        case '.':
            return makeToken(lexer, TOKEN_DOT);
        case '-':
            if(peek(lexer) == '>' && !isAtEnd(lexer)) {
                advance(lexer);
                return makeToken(lexer, TOKEN_RESULT_ARROW);
            }
            return makeToken(lexer, TOKEN_MINUS);
        case '+':
            return makeToken(lexer, TOKEN_PLUS);
        case ';':
            return makeToken(lexer, TOKEN_SEMICOLON);
        case '*':
            return makeToken(lexer, TOKEN_STAR);
        case '/':
            return makeToken(lexer, TOKEN_SLASH);
        case '%':
            return makeToken(lexer, TOKEN_PERCENT);
        case '!':
            return makeToken(lexer, TOKEN_BANG);
        case '=':
            return makeToken(lexer, TOKEN_EQUAL);
        case '<':
            return makeToken(lexer, TOKEN_LESS);
        case '>':
            return makeToken(lexer, TOKEN_GREATER);
        case '&':
            return makeToken(lexer, TOKEN_AMPERSAND);
        case '|':
            return makeToken(lexer, TOKEN_PIPE);
        case '^':
            return makeToken(lexer, TOKEN_CARET);
        case '~':
            return makeToken(lexer, TOKEN_TILDE);
        case '?':
            return makeToken(lexer, TOKEN_QUESTION);
        case ':':
            return makeToken(lexer, TOKEN_COLON);
        default:
            return errorToken(lexer, "Unexpected character.");
    }
}
// </symbolChar>


// <identifier>
Token identifier(Lexer* lexer) {
    char* currentToken = *lexer->current;

    while (
        isAlpha(*lexer->current) ||
        isDigit(*lexer->current) ||
        isType(*lexer->current)
    ) {
        advance(lexer);
    }

    Token token = makeToken(lexer, TOKEN_IDENTIFIER);
}
// </identifier>



/* =========================================================== */
/* @Data_Types */

// <checkKeyword>
CryoTokenType checkKeyword(Lexer* lexer, char* keyword, CryoTokenType type) {
    int i = 0;
    while (keywords[i].keyword != NULL) {
        if (strcmp(keywords[i].keyword, keyword) == 0) {
            return keywords[i].type;
        }
        i++;
    }
    return type;
}
// </checkKeyword>


// <checkDataType>
CryoTokenType checkDataType(Lexer* lexer, char* dataType, CryoTokenType type) {
    int i = 0;
    while (dataTypes[i].dataType != NULL) {
        if (strcmp(dataTypes[i].dataType, dataType) == 0) {
            return dataTypes[i].baseType;
        }
        i++;
    }
    return type;
}
// </checkDataType>



/* =========================================================== */
/* @DataType_Evaluation */

bool isAlpha(char* c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char* c) {
    return c >= '0' && c <= '9';
}

bool isType(char* c) {
    return c == '[' || c == ']';
}


/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */

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
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
