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
#include "compiler/lexer.h"
/*---------<end>---------*/

//#################################//
//       Keyword Dictionary.       //
//#################################//
KeywordToken keywords[] = {
    {"public",      TOKEN_KW_PUBLIC         },
    {"function",    TOKEN_KW_FN             },
    {"if",          TOKEN_KW_IF             },
    {"else",        TOKEN_KW_ELSE           },
    {"while",       TOKEN_KW_WHILE          },
    {"for",         TOKEN_KW_FOR            },
    {"return",      TOKEN_KW_RETURN         },
    {"const",       TOKEN_KW_CONST          },
    {"mut",         TOKEN_KW_MUT            },
    {"true",        TOKEN_BOOLEAN_LITERAL   },
    {"false",       TOKEN_BOOLEAN_LITERAL   },
    {"int",         TOKEN_KW_INT            },
    {"string",      TOKEN_KW_STRING         },
    {"boolean",     TOKEN_KW_BOOL           },
    {"expression",  TOKEN_KW_EXPRESSION     },
    {"void",        TOKEN_KW_VOID           },
    {"null",        TOKEN_KW_NULL           },
    {"struct",      TOKEN_KW_STRUCT         },
    {"enum",        TOKEN_KW_ENUM           },
    {"break",       TOKEN_KW_BREAK          },
    {"continue",    TOKEN_KW_CONTINUE       },
    {NULL,          TOKEN_UNKNOWN           }  // Sentinel value
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
        fprintf(stderr, "[Lexer] Error: Memory allocation failed in my_strndup\n");
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
    while (true) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 0; // Reset column at new line
                advance(lexer);
                break;
            case '/':
                if (peekNext(lexer) == '/') {
                    // Skip line comments
                    while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                } else if (peekNext(lexer) == '*') {
                    // Skip block comments
                    advance(lexer); // Skip '/'
                    advance(lexer); // Skip '*'
                    while (!(peek(lexer) == '*' && peekNext(lexer) == '/') && !isAtEnd(lexer)) {
                        if (peek(lexer) == '\n') lexer->line++;
                        advance(lexer);
                    }
                    if (!isAtEnd(lexer)) {
                        advance(lexer); // Skip '*'
                        advance(lexer); // Skip '/'
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



// <peekNextToken>
Token peekNextToken(Lexer* lexer) {
    Lexer tempLexer = *lexer;  // Copy the current lexer state
    Token tempNextToken = nextToken(&tempLexer, &tempLexer.currentToken);  // Get the next token from the copied state
    printf("[Lexer - DEBUG] Peeked Next Token: %.*s\n", tempNextToken.length, tempNextToken.start);
    return tempNextToken;
}
// </peekNextToken>


// <makeToken>
Token makeToken(Lexer* lexer, CryoTokenType type) {
    // Create the token with the given type and return it
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column;

    if(token.length > 1) {
        printf("[Lexer] Created token: %.*s (Type: %d, Line: %d, Column: %d)\n",
           token.length, token.start, token.type, token.line, token.column);
    }
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
CryoTokenType checkKeyword(Lexer *lexer, const char *keyword, CryoTokenType type) {
    const char *start = lexer->start;
    while (*keyword && tolower(*start) == tolower(*keyword)) {
        start++;
        keyword++;
    }
    if (*keyword == '\0' && !isalnum(*start)) {
        lexer->current = start;  // Update the lexer->current to the end of the matched keyword
        return type;
    }

    printf("[Lexer] Keyword Identified: %.*s\n", (int)(lexer->current - lexer->start), lexer->start);

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


// <string>
Token string(Lexer* lexer) {
    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer)) {
        return errorToken(lexer, "Unterminated string.");
    }

    // The closing quote.
    advance(lexer);
    return makeToken(lexer, TOKEN_STRING_LITERAL);
}
// </string>


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


// <boolean>
Token boolean(Lexer* lexer) {
    // TOKEN_BOOLEAN_LITERAL
    if (peek(lexer) == 't') {
        advance(lexer);
        if (peek(lexer) == 'r') {
            advance(lexer);
            if (peek(lexer) == 'u') {
                advance(lexer);
                if (peek(lexer) == 'e') {
                    advance(lexer);
                    return makeToken(lexer, TOKEN_BOOLEAN_LITERAL);
                }
            }
        }
    } else if (peek(lexer) == 'f') {
        advance(lexer);
        if (peek(lexer) == 'a') {
            advance(lexer);
            if (peek(lexer) == 'l') {
                advance(lexer);
                if (peek(lexer) == 's') {
                    advance(lexer);
                    if (peek(lexer) == 'e') {
                        advance(lexer);
                        return makeToken(lexer, TOKEN_BOOLEAN_LITERAL);
                    }
                }
            }
        }
    }
    return errorToken(lexer, "Invalid boolean literal.");
}
// </boolean>


// <get_next_token>
Token get_next_token(Lexer *lexer) {
    nextToken(lexer, &lexer->currentToken);
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

    printf("[Lexer] Processing Token | Type: %d, Line: %d, Column: %d\n",
        token->type, token->line, token->column);

    char c = advance(lexer);


    // This is to check if it's an identifier.
    if (isAlpha(c)) {
        CryoTokenType type;
        switch (tolower(c)) {
            case 'c':
                type = checkKeyword(lexer, "const", TOKEN_KW_CONST);
                break;
            case 'm':
                type = checkKeyword(lexer, "mut", TOKEN_KW_MUT);
                break;
            case 'i':
                type = checkKeyword(lexer, "if", TOKEN_KW_IF);
                break;
            case 'e':
                type = checkKeyword(lexer, "else", TOKEN_KW_ELSE);
                break;
            case 'w':
                type = checkKeyword(lexer, "while", TOKEN_KW_WHILE);
                break;
            case 'r':
                type = checkKeyword(lexer, "return", TOKEN_KW_RETURN);
                break;
            case 'p':
                type = checkKeyword(lexer, "public", TOKEN_KW_PUBLIC);
                break;
            case 'f':
                if (peek(lexer) == 'u') {
                    type = checkKeyword(lexer, "function", TOKEN_KW_FN);
                } else if (peek(lexer) == 'o') {
                    type = checkKeyword(lexer, "for", TOKEN_KW_FOR);
                } else {
                    type = checkKeyword(lexer, "false", TOKEN_BOOLEAN_LITERAL);
                }
                break;
            case 't':
                type = checkKeyword(lexer, "true", TOKEN_BOOLEAN_LITERAL);
                break;
            default:
                type = TOKEN_IDENTIFIER;
                break;
        }
        *token = makeToken(lexer, type);

        // If it's still an identifier, process the entire identifier
        if (type == TOKEN_IDENTIFIER) {
            *token = identifier(lexer);
        }
        return *token;
    }

    if (isDigit(c)) {
        *token = number(lexer);
        return *token;
    }

    if (c == '"') {
        *token = string(lexer);
        return *token;
    }

    switch (c) {
        case '{': *token = makeToken(lexer, TOKEN_LBRACE); break;
        case '}': *token = makeToken(lexer, TOKEN_RBRACE); break;
        case '(': *token = makeToken(lexer, TOKEN_LPAREN); break;
        case ')': *token = makeToken(lexer, TOKEN_RPAREN); break;
        case '+': *token = makeToken(lexer, TOKEN_OP_PLUS); break;
        case '*': *token = makeToken(lexer, TOKEN_OP_STAR); break;
        case '/': *token = makeToken(lexer, TOKEN_OP_SLASH); break;
        case '=': *token = makeToken(lexer, TOKEN_ASSIGN); break;
        case ';': *token = makeToken(lexer, TOKEN_SEMICOLON); break;
        case ',': *token = makeToken(lexer, TOKEN_COMMA); break;
        case ':': *token = makeToken(lexer, TOKEN_COLON); break;
        case '<': *token = makeToken(lexer, TOKEN_OP_LT); break;
        case '>': *token = makeToken(lexer, TOKEN_OP_GT); break;
        case '!':
            if (peek(lexer) == '=') {
                advance(lexer);
                *token = makeToken(lexer, TOKEN_OP_NEQ);
            } else {
                *token = makeToken(lexer, TOKEN_OP_NOT);
            }
            break;
        case '&':
            if (peek(lexer) == '&') {
                advance(lexer);
                *token = makeToken(lexer, TOKEN_OP_AND);
            } else {
                *token = errorToken(lexer, "Unexpected character.");
            }
            break;
        case '|':
            if (peek(lexer) == '|') {
                advance(lexer);
                *token = makeToken(lexer, TOKEN_OP_OR);
            } else {
                *token = errorToken(lexer, "Unexpected character.");
            }
            break;
        case '-':
            if (peek(lexer) == '>') {
                advance(lexer); // consume '>'
                *token = makeToken(lexer, TOKEN_RESULT_ARROW);
            } else {
                *token = makeToken(lexer, TOKEN_OP_MINUS);
            }
            break;
        default:
            *token = errorToken(lexer, "Unexpected character.");
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
    } while (token.type != TOKEN_EOF);

    freeLexer(&lexer);
    free(source);
    return 0;
}
// </lexer>
