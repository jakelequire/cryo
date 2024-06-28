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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // Special tokens
    TOKEN_UNKNOWN = -1,         // Unknown token        -1
    TOKEN_EOF,                  // End of file token    0
    TOKEN_IDENTIFIER,           // `identifier` token   1
    TOKEN_ERROR,                // Error token          2
    TOKEN_SINGLE_COMMENT,       // `//` token           3
    TOKEN_MULTI_COMMENT,        // `/* */` token        4
    TOKEN_WHITESPACE,           // ` ` token            5
    TOKEN_ASSIGN,               // `=` token            6
    TOKEN_NEWLINE,              // `\n` token           7
    TOKEN_TAB,                  // `\t` token           8
    TOKEN_INDENT,               // Indent token         9
    TOKEN_DEDENT,               // Dedent token         10

    // Data types
    TOKEN_TYPE_INT,             // `1`, `2`, `3`, etc. integers                         11
    TOKEN_TYPE_FLOAT,           // `1.0`, `2.0`, `3.0`, etc. floats                     12
    TOKEN_TYPE_STRING,          // `"Hello, World!"`, `"Hello, " + "World!"`, etc.      13
    TOKEN_TYPE_STRING_LITERAL,  // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc       14
    TOKEN_TYPE_BOOLEAN,         // `true`, `false`                                      15
    TOKEN_TYPE_VOID,            // `void`                                               16
    TOKEN_RESULT_ARROW,         // `->` token                                           17

    TOKEN_INT_LITERAL,          // `123`                        18
    TOKEN_FLOAT_LITERAL,        // `123.0`                      19
    TOKEN_STRING_LITERAL,       // `"Hello, World!"`            20
    TOKEN_BOOLEAN_LITERAL,      // `true`, `false`              21
    TOKEN_VOID_LITERAL,         // `void`                       22
    TOKEN_NULL_LITERAL,         // `null`                       23
    TOKEN_BIN_OP_LITERAL,       // `2 + 2`, `3 * 3`, etc.       24

    // Operators
    TOKEN_OP_PLUS,              // `+` token        25
    TOKEN_OP_MINUS,             // `-` token        26
    TOKEN_OP_STAR,              // `*` token        27
    TOKEN_OP_SLASH,             // `/` token        28
    TOKEN_OP_MOD,               // `%` token        29
    TOKEN_OP_AND,               // `&` token        30
    TOKEN_OP_OR,                // `|` token        31
    TOKEN_OP_XOR,               // `^` token        32
    TOKEN_OP_NOT,               // `!` token        33
    TOKEN_OP_LSHIFT,            // `<<` token       34
    TOKEN_OP_RSHIFT,            // `>>` token       35
    TOKEN_OP_LT,                // `<` token        36
    TOKEN_OP_GT,                // `>` token        37
    TOKEN_OP_LTE,               // `<=` token       38
    TOKEN_OP_GTE,               // `>=` token       39
    TOKEN_OP_EQ,                // `==` token       40
    TOKEN_OP_NEQ,               // `!=` token       41
    TOKEN_OP_ASSIGN,            // `=` token        42
    TOKEN_OP_ADD_ASSIGN,        // `+=` token       43
    TOKEN_OP_SUB_ASSIGN,        // `-=` token       44
    TOKEN_OP_MUL_ASSIGN,        // `*=` token       45
    TOKEN_OP_DIV_ASSIGN,        // `/=` token       46
    TOKEN_OP_MOD_ASSIGN,        // `%=` token       47
    TOKEN_OP_AND_ASSIGN,        // `&=` token       48
    TOKEN_OP_OR_ASSIGN,         // `|=` token       49
    TOKEN_OP_XOR_ASSIGN,        // `^=` token       50

    // Keywords
    TOKEN_KW_IF,                // `if`             51     
    TOKEN_KW_ELSE,              // `else`           52
    TOKEN_KW_FOR,               // `for`            53
    TOKEN_KW_WHILE,             // `while`          54
    TOKEN_KW_BREAK,             // `break`          55
    TOKEN_KW_CONTINUE,          // `continue`       56
    TOKEN_KW_RETURN,            // `return`         57
    TOKEN_KW_FN,                // `fn`             58
    TOKEN_KW_CONST,             // `const`          59
    TOKEN_KW_PUBLIC,            // `public`         60  
    TOKEN_KW_PRIVATE,           // `private`        61   
    TOKEN_KW_STRUCT,            // `struct`         62   
    TOKEN_KW_ENUM,              // `enum`           63
    TOKEN_KW_TRAIT,             // `trait`          64
    TOKEN_KW_IMPL,              // `impl`           65
    TOKEN_KW_USE,               // `use`            66
    TOKEN_KW_AS,                // `as`             67
    TOKEN_KW_MODULE,            // `module`         68
    TOKEN_KW_NAMESPACE,         // `namespace`      69
    TOKEN_KW_TYPE,              // `type`           70
    TOKEN_KW_CAST,              // `cast`           71
    TOKEN_KW_SIZE,              // `size`           72
    TOKEN_KW_CASE,              // `case`           73
    TOKEN_KW_CLASS,             // `class`          74
    TOKEN_KW_DEFAULT,           // `default`        75
    TOKEN_KW_EXTENDS,           // `extends`        76
    TOKEN_KW_IMPORT,            // `import`         77
    TOKEN_KW_IN,                // `in`             78
    TOKEN_KW_INSTANCEOF,        // `instanceof`     79
    TOKEN_KW_NEW,               // `new`            80 
    TOKEN_KW_NULL,              // `null`           81
    TOKEN_KW_PROTECTED,         // `protected`      82
    TOKEN_KW_STATIC,            // `static`         83
    TOKEN_KW_RESULT,            // `result`         84
    TOKEN_KW_TRUE,              // `true`           85
    TOKEN_KW_FALSE,             // `false`          86
    TOKEN_KW_MUT,               // `mut`            87
    TOKEN_KW_BYTE,              // `byte`           88
    TOKEN_KW_SHORT,             // `short`          89
    TOKEN_KW_INT,               // `int`            90
    TOKEN_KW_LONG,              // `long`           91
    TOKEN_KW_FLOAT,             // `float`          92
    TOKEN_KW_DOUBLE,            // `double`         93
    TOKEN_KW_CHAR,              // `char`           94
    TOKEN_KW_BOOL,              // `bool`           95
    TOKEN_KW_VOID,              // `void`           96
    TOKEN_KW_ANY,               // `any`            97
    TOKEN_KW_STRING,            // `string`         98
    TOKEN_KW_ARRAY,             // `array`          99
    TOKEN_KW_MAP,               // `map`            100
    TOKEN_KW_SET,               // `set`            101
    TOKEN_KW_NATIVE,            // `native`         102
    TOKEN_KW_SYNCHRONIZED,      // `synchronized`   103
    TOKEN_KW_VOLATILE,          // `volatile`       104
    TOKEN_KW_INTERFACE,         // `interface`      105
    TOKEN_KW_BOOLEAN,           // `boolean`        106
    TOKEN_KW_TYPEOF,            // `typeof`         107
    TOKEN_KW_AWAIT,             // `await`          108
    TOKEN_KW_ASYNC,             // `async`          109
    TOKEN_KW_EXPRESSION,        // `expression`     110
    // Add more keywords as needed


    // Operators and punctuations
    TOKEN_BANG,                 // `!`          111
    TOKEN_PLUS,                 // `+`          112
    TOKEN_MINUS,                // `-`          113
    TOKEN_STAR,                 // `*`          114
    TOKEN_SLASH,                // `/`          115
    TOKEN_PIPE,                 // `|`          116
    TOKEN_EXCLAMATION,          // `!`          117
    TOKEN_QUESTION,             // `?`          118
    TOKEN_COLON,                // `:`          119
    TOKEN_LESS,                 // `<`          120
    TOKEN_GREATER,              // `>`          121
    TOKEN_EQUAL,                // `=`          122
    TOKEN_DOT,                  // `.`          123
    TOKEN_COMMA,                // `,`          124
    TOKEN_HASH,                 // `#`          125
    TOKEN_DOLLAR,               // `$`          126
    TOKEN_AT,                   // `@`          127
    TOKEN_BACKSLASH,            // `\`          128
    TOKEN_UNDERSCORE,           // `_`          129
    TOKEN_BACKTICK,             // '`'          130
    TOKEN_QUOTE,                // `'`          131
    TOKEN_APOSTROPHE,           // `'`          132
    TOKEN_SEMICOLON,            // `;`          133
    TOKEN_TILDE,                // `~`          134
    TOKEN_CARET,                // `^`          135
    TOKEN_AMPERSAND,            // `&`          136
    TOKEN_PERCENT,              // `%`          137
    TOKEN_LPAREN,               // `(`          138
    TOKEN_RPAREN,               // `)`          139
    TOKEN_LBRACKET,             // `[`          140
    TOKEN_RBRACKET,             // `]`          141
    TOKEN_LBRACE,               // `{`          142
    TOKEN_RBRACE,               // `}`          143
    TOKEN_LANGLE,               // `<`          144
    TOKEN_RANGLE,               // `>`          145
    TOKEN_LCURLY,               // `{`          146
    TOKEN_RCURLY,               // `}`          147
    TOKEN_LSQUARE,              // `[`          148
    TOKEN_RSQUARE,              // `]`          149
    TOKEN_LANGLE_BRACKET,       // `<<`         150
    TOKEN_RANGLE_BRACKET,       // `>>`         151
    TOKEN_LCURLY_BRACKET,       // `{{`         152
    TOKEN_RCURLY_BRACKET,       // `}}`         153
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
    NODE_STRING_LITERAL,            // 19
    NODE_BOOLEAN_LITERAL,           // 20
    NODE_UNKNOWN,                   // 21
} CryoNodeType;

typedef enum CryoDataType {
    DATA_TYPE_INT,       // `int`       0
    DATA_TYPE_FLOAT,     // `float`     1
    DATA_TYPE_STRING,    // `string`    2
    DATA_TYPE_BOOLEAN,   // `boolean`   3
    DATA_TYPE_VOID,      // `void`      4
    DATA_TYPE_NULL,      // `null`      5
    DATA_TYPE_UNKNOWN,   // `<UNKNOWN>` 6
} CryoDataType;

#ifdef __cplusplus
}
#endif

#endif // TOKEN_H
