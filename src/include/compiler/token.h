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
#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Special tokens
    TOKEN_UNKNOWN = -1,         // Unknown token        // -1
    TOKEN_EOF,                  // End of file token    // 0
    TOKEN_IDENTIFIER,           // `identifier` token   // 1
    TOKEN_ERROR,                // Error token          // 2
    TOKEN_SINGLE_COMMENT,       // `//` token           // 3
    TOKEN_MULTI_COMMENT,        // `/* */` token        // 4    
    TOKEN_WHITESPACE,           // ` ` token            // 5
    TOKEN_ASSIGN,               // `=` token            // 6
    TOKEN_NEWLINE,              // `\n` token           // 7
    TOKEN_TAB,                  // `\t` token           // 8
    TOKEN_INDENT,               // Indent token         // 9
    TOKEN_DEDENT,               // Dedent token         // 10

    // Data types
    TOKEN_TYPE_INT,             // `1`, `2`, `3`, etc. integers                     // 11
    TOKEN_TYPE_FLOAT,           // `1.0`, `2.0`, `3.0`, etc. floats                 // 12
    TOKEN_TYPE_STRING,          // `"Hello, World!"`, `"Hello, " + "World!"`, etc.  // 13
    TOKEN_TYPE_STRING_LITERAL,  // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc   // 14
    TOKEN_TYPE_BOOLEAN,         // `true`, `false`                                  // 15
    TOKEN_TYPE_VOID,            // `void`                                           // 16
    TOKEN_RESULT_ARROW,         // `->` token                                       // 17

    TOKEN_INT_LITERAL,          // `123`         // 18

    // Operators
    TOKEN_OP_PLUS,              // `+` token        // 18
    TOKEN_OP_MINUS,             // `-` token        // 19
    TOKEN_OP_STAR,              // `*` token        // 20
    TOKEN_OP_SLASH,             // `/` token        // 21
    TOKEN_OP_MOD,               // `%` token        // 22
    TOKEN_OP_AND,               // `&` token        // 23
    TOKEN_OP_OR,                // `|` token        // 24
    TOKEN_OP_XOR,               // `^` token        // 25
    TOKEN_OP_NOT,               // `!` token        // 26
    TOKEN_OP_LSHIFT,            // `<<` token       // 27
    TOKEN_OP_RSHIFT,            // `>>` token       // 28
    TOKEN_OP_LT,                // `<` token        // 29
    TOKEN_OP_GT,                // `>` token        // 30
    TOKEN_OP_LTE,               // `<=` token       // 31
    TOKEN_OP_GTE,               // `>=` token       // 32
    TOKEN_OP_EQ,                // `==` token       // 33
    TOKEN_OP_NEQ,               // `!=` token       // 34
    TOKEN_OP_ASSIGN,            // `=` token        // 35
    TOKEN_OP_ADD_ASSIGN,        // `+=` token       // 36
    TOKEN_OP_SUB_ASSIGN,        // `-=` token       // 37
    TOKEN_OP_MUL_ASSIGN,        // `*=` token       // 38
    TOKEN_OP_DIV_ASSIGN,        // `/=` token       // 39
    TOKEN_OP_MOD_ASSIGN,        // `%=` token       // 40
    TOKEN_OP_AND_ASSIGN,        // `&=` token       // 41
    TOKEN_OP_OR_ASSIGN,         // `|=` token       // 42
    TOKEN_OP_XOR_ASSIGN,        // `^=` token       // 43

    // Keywords
    TOKEN_KW_IF,                // `if`             // 44
    TOKEN_KW_ELSE,              // `else`           // 45
    TOKEN_KW_FOR,               // `for`            // 46
    TOKEN_KW_WHILE,             // `while`          // 47
    TOKEN_KW_BREAK,             // `break`          // 48 
    TOKEN_KW_CONTINUE,          // `continue`       // 49
    TOKEN_KW_RETURN,            // `return`         // 50
    TOKEN_KW_FN,                // `fn`             // 51
    TOKEN_KW_CONST,             // `const`          // 52
    TOKEN_KW_PUBLIC,            // `public`         // 54
    TOKEN_KW_PRIVATE,           // `private`        // 55
    TOKEN_KW_STRUCT,            // `struct`         // 56
    TOKEN_KW_ENUM,              // `enum`           // 57
    TOKEN_KW_TRAIT,             // `trait`          // 58
    TOKEN_KW_IMPL,              // `impl`           // 59
    TOKEN_KW_USE,               // `use`            // 60
    TOKEN_KW_AS,                // `as`             // 61
    TOKEN_KW_MODULE,            // `module`         // 62
    TOKEN_KW_NAMESPACE,         // `namespace`      // 63
    TOKEN_KW_TYPE,              // `type`           // 64
    TOKEN_KW_CAST,              // `cast`           // 65
    TOKEN_KW_SIZE,              // `size`           // 66
    TOKEN_KW_CASE,              // `case`           // 67
    TOKEN_KW_CLASS,             // `class`          // 68
    TOKEN_KW_DEFAULT,           // `default`        // 69
    TOKEN_KW_EXTENDS,           // `extends`        // 70
    TOKEN_KW_IMPORT,            // `import`         // 71
    TOKEN_KW_IN,                // `in`             // 72
    TOKEN_KW_INSTANCEOF,        // `instanceof`     // 73
    TOKEN_KW_NEW,               // `new`            // 74
    TOKEN_KW_NULL,              // `null`           // 75
    TOKEN_KW_PROTECTED,         // `protected`      // 76
    TOKEN_KW_STATIC,            // `static`         // 77
    TOKEN_KW_RESULT,            // `result`         // 78
    TOKEN_KW_TRUE,              // `true`           // 79
    TOKEN_KW_FALSE,             // `false`          // 80
    TOKEN_KW_MUT,               // `mut`            // 81
    TOKEN_KW_BYTE,              // `byte`           // 82
    TOKEN_KW_SHORT,             // `short`          // 83
    TOKEN_KW_INT,               // `int`            // 84
    TOKEN_KW_LONG,              // `long`           // 85
    TOKEN_KW_FLOAT,             // `float`          // 86
    TOKEN_KW_DOUBLE,            // `double`         // 87
    TOKEN_KW_CHAR,              // `char`           // 88
    TOKEN_KW_BOOL,              // `bool`           // 89
    TOKEN_KW_VOID,              // `void`           // 90
    TOKEN_KW_ANY,               // `any`            // 91
    TOKEN_KW_STRING,            // `string`         // 92
    TOKEN_KW_ARRAY,             // `array`          // 93
    TOKEN_KW_MAP,               // `map`            // 94
    TOKEN_KW_SET,               // `set`            // 95
    TOKEN_KW_NATIVE,            // `native`         // 96
    TOKEN_KW_SYNCHRONIZED,      // `synchronized`   // 97
    TOKEN_KW_VOLATILE,          // `volatile`       // 98
    TOKEN_KW_INTERFACE,         // `interface`      // 99
    TOKEN_KW_BOOLEAN,           // `boolean`        // 100
    TOKEN_KW_TYPEOF,            // `typeof`         // 101
    TOKEN_KW_AWAIT,             // `await`          // 102
    // Add more keywords as needed


    // Operators and punctuations
    TOKEN_BANG,                 // `!`              // 103
    TOKEN_PLUS,                 // `+`              // 104
    TOKEN_MINUS,                // `-`              // 105
    TOKEN_STAR,                 // `*`              // 106
    TOKEN_SLASH,                // `/`              // 107
    TOKEN_PIPE,                 // `|`              // 108
    TOKEN_EXCLAMATION,          // `!`              // 109
    TOKEN_QUESTION,             // `?`              // 110
    TOKEN_COLON,                // `:`              // 111
    TOKEN_LESS,                 // `<`              // 112
    TOKEN_GREATER,              // `>`              // 113
    TOKEN_EQUAL,                // `=`              // 114
    TOKEN_DOT,                  // `.`              // 115
    TOKEN_COMMA,                // `,`              // 116
    TOKEN_HASH,                 // `#`              // 117
    TOKEN_DOLLAR,               // `$`              // 118
    TOKEN_AT,                   // `@`              // 119
    TOKEN_BACKSLASH,            // `\`              // 120
    TOKEN_UNDERSCORE,           // `_`              // 121
    TOKEN_BACKTICK,             // '`'              // 122
    TOKEN_QUOTE,                // `'`              // 123
    TOKEN_APOSTROPHE,           // `'`              // 124
    TOKEN_SEMICOLON,            // `;`              // 125
    TOKEN_TILDE,                // `~`              // 126
    TOKEN_CARET,                // `^`              // 127
    TOKEN_AMPERSAND,            // `&`              // 128
    TOKEN_PERCENT,              // `%`              // 129
    TOKEN_LPAREN,               // `(`              // 130
    TOKEN_RPAREN,               // `)`              // 131
    TOKEN_LBRACKET,             // `[`              // 132
    TOKEN_RBRACKET,             // `]`              // 133
    TOKEN_LBRACE,               // `{`              // 134
    TOKEN_RBRACE,               // `}`              // 135
    TOKEN_LANGLE,               // `<`              // 136
    TOKEN_RANGLE,               // `>`              // 137
    TOKEN_LCURLY,               // `{`              // 138            
    TOKEN_RCURLY,               // `}`              // 139
    TOKEN_LSQUARE,              // `[`              // 140
    TOKEN_RSQUARE,              // `]`              // 141
    TOKEN_LANGLE_BRACKET,       // `<<`             // 142
    TOKEN_RANGLE_BRACKET,       // `>>`             // 143
    TOKEN_LCURLY_BRACKET,       // `{{`             // 144
    TOKEN_RCURLY_BRACKET,       // `}}`             // 145
} CryoTokenType;

typedef enum CryoNodeType {
    NODE_PROGRAM,                   // 0
    NODE_FUNCTION_DECLARATION,      // 1
    NODE_VAR_DECLARATION,           // 2
    NODE_STATEMENT,                 // 3
    NODE_EXPRESSION,                // 4
    NODE_BINARY_EXPR,               // 5
    NODE_UNARY_EXPR,                // 6
    NODE_LITERAL_EXPR,              // 7
    NODE_VAR_NAME,                  // 8
    NODE_FUNCTION_CALL,             // 9
    NODE_IF_STATEMENT,              // 10
    NODE_WHILE_STATEMENT,           // 11
    NODE_FOR_STATEMENT,             // 12
    NODE_RETURN_STATEMENT,          // 13
    NODE_BLOCK,                     // 14
    NODE_EXPRESSION_STATEMENT,      // 15
    NODE_ASSIGN,                    // 16
    NODE_PARAM_LIST,                // 17
    NODE_TYPE,                      // 18
    NODE_UNKNOWN,                   // 19
} CryoNodeType;


#endif // TOKEN_H
