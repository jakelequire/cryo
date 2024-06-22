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
    TOKEN_TYPE_INT,             // `1`, `2`, `3`, etc. integers                     // 
    TOKEN_TYPE_FLOAT,           // `1.0`, `2.0`, `3.0`, etc. floats                 // 
    TOKEN_TYPE_STRING,          // `"Hello, World!"`, `"Hello, " + "World!"`, etc.  // 
    TOKEN_TYPE_STRING_LITERAL,  // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc   // 
    TOKEN_TYPE_BOOLEAN,         // `true`, `false`                                  // 
    TOKEN_TYPE_VOID,            // `void`                                           // 
    TOKEN_RESULT_ARROW,         // `->` token                                       // 

    TOKEN_INT_LITERAL,          // `123`         // 
    TOKEN_FLOAT_LITERAL,        // `123.0`       //
    TOKEN_STRING_LITERAL,       // `"Hello, World!"`
    TOKEN_BOOLEAN_LITERAL,      // `true`, `false`
    TOKEN_VOID_LITERAL,         // `void`
    TOKEN_NULL_LITERAL,         // `null`
    TOKEN_BIN_OP_LITERAL,       // `2 + 2`, `3 * 3`, etc.

    // Operators
    TOKEN_OP_PLUS,              // `+` token        // 
    TOKEN_OP_MINUS,             // `-` token        // 
    TOKEN_OP_STAR,              // `*` token        // 
    TOKEN_OP_SLASH,             // `/` token        // 
    TOKEN_OP_MOD,               // `%` token        // 
    TOKEN_OP_AND,               // `&` token        // 
    TOKEN_OP_OR,                // `|` token        // 
    TOKEN_OP_XOR,               // `^` token        // 
    TOKEN_OP_NOT,               // `!` token        // 
    TOKEN_OP_LSHIFT,            // `<<` token       // 
    TOKEN_OP_RSHIFT,            // `>>` token       // 
    TOKEN_OP_LT,                // `<` token        // 
    TOKEN_OP_GT,                // `>` token        // 
    TOKEN_OP_LTE,               // `<=` token       // 
    TOKEN_OP_GTE,               // `>=` token       // 
    TOKEN_OP_EQ,                // `==` token       // 
    TOKEN_OP_NEQ,               // `!=` token       // 
    TOKEN_OP_ASSIGN,            // `=` token        // 
    TOKEN_OP_ADD_ASSIGN,        // `+=` token       // 
    TOKEN_OP_SUB_ASSIGN,        // `-=` token       // 
    TOKEN_OP_MUL_ASSIGN,        // `*=` token       // 
    TOKEN_OP_DIV_ASSIGN,        // `/=` token       // 
    TOKEN_OP_MOD_ASSIGN,        // `%=` token       // 
    TOKEN_OP_AND_ASSIGN,        // `&=` token       // 
    TOKEN_OP_OR_ASSIGN,         // `|=` token       // 
    TOKEN_OP_XOR_ASSIGN,        // `^=` token       // 

    // Keywords
    TOKEN_KW_IF,                // `if`             // 
    TOKEN_KW_ELSE,              // `else`           // 
    TOKEN_KW_FOR,               // `for`            // 
    TOKEN_KW_WHILE,             // `while`          // 
    TOKEN_KW_BREAK,             // `break`          // 
    TOKEN_KW_CONTINUE,          // `continue`       // 
    TOKEN_KW_RETURN,            // `return`         // 
    TOKEN_KW_FN,                // `fn`             // 
    TOKEN_KW_CONST,             // `const`          // 
    TOKEN_KW_PUBLIC,            // `public`         // 
    TOKEN_KW_PRIVATE,           // `private`        // 
    TOKEN_KW_STRUCT,            // `struct`         // 
    TOKEN_KW_ENUM,              // `enum`           // 
    TOKEN_KW_TRAIT,             // `trait`          // 
    TOKEN_KW_IMPL,              // `impl`           // 
    TOKEN_KW_USE,               // `use`            // 
    TOKEN_KW_AS,                // `as`             // 
    TOKEN_KW_MODULE,            // `module`         // 
    TOKEN_KW_NAMESPACE,         // `namespace`      // 
    TOKEN_KW_TYPE,              // `type`           // 
    TOKEN_KW_CAST,              // `cast`           // 
    TOKEN_KW_SIZE,              // `size`           // 
    TOKEN_KW_CASE,              // `case`           // 
    TOKEN_KW_CLASS,             // `class`          // 
    TOKEN_KW_DEFAULT,           // `default`        // 
    TOKEN_KW_EXTENDS,           // `extends`        // 
    TOKEN_KW_IMPORT,            // `import`         // 
    TOKEN_KW_IN,                // `in`             // 
    TOKEN_KW_INSTANCEOF,        // `instanceof`     // 
    TOKEN_KW_NEW,               // `new`            // 
    TOKEN_KW_NULL,              // `null`           // 
    TOKEN_KW_PROTECTED,         // `protected`      // 
    TOKEN_KW_STATIC,            // `static`         // 
    TOKEN_KW_RESULT,            // `result`         // 
    TOKEN_KW_TRUE,              // `true`           // 
    TOKEN_KW_FALSE,             // `false`          // 
    TOKEN_KW_MUT,               // `mut`            // 
    TOKEN_KW_BYTE,              // `byte`           // 
    TOKEN_KW_SHORT,             // `short`          // 
    TOKEN_KW_INT,               // `int`            // 
    TOKEN_KW_LONG,              // `long`           // 
    TOKEN_KW_FLOAT,             // `float`          // 
    TOKEN_KW_DOUBLE,            // `double`         // 
    TOKEN_KW_CHAR,              // `char`           // 
    TOKEN_KW_BOOL,              // `bool`           // 
    TOKEN_KW_VOID,              // `void`           // 
    TOKEN_KW_ANY,               // `any`            // 
    TOKEN_KW_STRING,            // `string`         // 
    TOKEN_KW_ARRAY,             // `array`          // 
    TOKEN_KW_MAP,               // `map`            // 
    TOKEN_KW_SET,               // `set`            // 
    TOKEN_KW_NATIVE,            // `native`         // 
    TOKEN_KW_SYNCHRONIZED,      // `synchronized`   // 
    TOKEN_KW_VOLATILE,          // `volatile`       // 
    TOKEN_KW_INTERFACE,         // `interface`      // 
    TOKEN_KW_BOOLEAN,           // `boolean`        // 
    TOKEN_KW_TYPEOF,            // `typeof`         // 
    TOKEN_KW_AWAIT,             // `await`          // 
    TOKEN_KW_ASYNC,             // `async`          //
    TOKEN_KW_EXPRESSION,        // `expression`     //
    // Add more keywords as needed


    // Operators and punctuations
    TOKEN_BANG,                 // `!`              // 
    TOKEN_PLUS,                 // `+`              // 
    TOKEN_MINUS,                // `-`              // 
    TOKEN_STAR,                 // `*`              // 
    TOKEN_SLASH,                // `/`              // 
    TOKEN_PIPE,                 // `|`              // 
    TOKEN_EXCLAMATION,          // `!`              // 
    TOKEN_QUESTION,             // `?`              // 
    TOKEN_COLON,                // `:`              // 
    TOKEN_LESS,                 // `<`              // 
    TOKEN_GREATER,              // `>`              // 
    TOKEN_EQUAL,                // `=`              // 
    TOKEN_DOT,                  // `.`              // 
    TOKEN_COMMA,                // `,`              // 
    TOKEN_HASH,                 // `#`              // 
    TOKEN_DOLLAR,               // `$`              // 
    TOKEN_AT,                   // `@`              // 
    TOKEN_BACKSLASH,            // `\`              // 
    TOKEN_UNDERSCORE,           // `_`              // 
    TOKEN_BACKTICK,             // '`'              // 
    TOKEN_QUOTE,                // `'`              // 
    TOKEN_APOSTROPHE,           // `'`              // 
    TOKEN_SEMICOLON,            // `;`              // 
    TOKEN_TILDE,                // `~`              // 
    TOKEN_CARET,                // `^`              // 
    TOKEN_AMPERSAND,            // `&`              // 
    TOKEN_PERCENT,              // `%`              // 
    TOKEN_LPAREN,               // `(`              // 
    TOKEN_RPAREN,               // `)`              // 
    TOKEN_LBRACKET,             // `[`              // 
    TOKEN_RBRACKET,             // `]`              // 
    TOKEN_LBRACE,               // `{`              // 
    TOKEN_RBRACE,               // `}`              // 
    TOKEN_LANGLE,               // `<`              // 
    TOKEN_RANGLE,               // `>`              // 
    TOKEN_LCURLY,               // `{`              //          
    TOKEN_RCURLY,               // `}`              // 
    TOKEN_LSQUARE,              // `[`              // 
    TOKEN_RSQUARE,              // `]`              // 
    TOKEN_LANGLE_BRACKET,       // `<<`             // 
    TOKEN_RANGLE_BRACKET,       // `>>`             // 
    TOKEN_LCURLY_BRACKET,       // `{{`             // 
    TOKEN_RCURLY_BRACKET,       // `}}`             // 
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


#ifdef __cplusplus
}
#endif

#endif // TOKEN_H
