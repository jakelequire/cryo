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
#ifndef TOKEN_H
#define TOKEN_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/utils/c_logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        // Special tokens
        TOKEN_UNKNOWN = -1,    // Unknown token
        TOKEN_EOF,             // End of file token
        TOKEN_IDENTIFIER,      // `identifier` token
        TOKEN_TYPE_IDENTIFIER, // `: {identifier}` token
        TOKEN_ERROR,           // Error token
        TOKEN_SINGLE_COMMENT,  // `//` token
        TOKEN_MULTI_COMMENT,   // `/* */` token
        TOKEN_WHITESPACE,      // ` ` token
        TOKEN_ASSIGN,          // `=` token
        TOKEN_NEWLINE,         // `\n` token
        TOKEN_TAB,             // `\t` token
        TOKEN_INDENT,          // Indent token
        TOKEN_DEDENT,          // Dedent token

        // Data types
        TOKEN_TYPE_INT,            // `1`, `2`, `3`, etc. integers
        TOKEN_TYPE_FLOAT,          // `1.0`, `2.0`, `3.0`, etc. floats
        TOKEN_TYPE_STRING,         // `"Hello, World!"`, `"Hello, " + "World!"`, etc.
        TOKEN_TYPE_STRING_LITERAL, // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc
        TOKEN_TYPE_BOOLEAN,        // `true`, `false`
        TOKEN_TYPE_VOID,           // `void`
        TOKEN_TYPE_NULL,           // `null`
        TOKEN_TYPE_ANY,            // `any`
        TOKEN_TYPE_CHAR,           // `char`
        TOKEN_TYPE_INT_ARRAY,      // `int[]`
        TOKEN_TYPE_FLOAT_ARRAY,    // `float[]`
        TOKEN_TYPE_STRING_ARRAY,   // `string[]`
        TOKEN_TYPE_BOOLEAN_ARRAY,  // `boolean[]`

        TOKEN_TYPE_I8,   // `i8`
        TOKEN_TYPE_I16,  // `i16`
        TOKEN_TYPE_I32,  // `i32`
        TOKEN_TYPE_I64,  // `i64`
        TOKEN_TYPE_I128, // `i128`

        TOKEN_RESULT_ARROW, // `->` token

        TOKEN_INT_LITERAL,     // `123`
        TOKEN_FLOAT_LITERAL,   // `123.0`
        TOKEN_STRING_LITERAL,  // `"Hello, World!"`
        TOKEN_BOOLEAN_LITERAL, // `true`, `false`
        TOKEN_VOID_LITERAL,    // `void`
        TOKEN_NULL_LITERAL,    // `null`
        TOKEN_BIN_OP_LITERAL,  // `2 + 2`, `3 * 3`, etc.

        // Operators
        TOKEN_OP_PLUS,       // `+` token
        TOKEN_OP_MINUS,      // `-` token
        TOKEN_OP_STAR,       // `*` token
        TOKEN_OP_SLASH,      // `/` token
        TOKEN_OP_MOD,        // `%` token
        TOKEN_OP_AND,        // `&` token
        TOKEN_OP_OR,         // `|` token
        TOKEN_OP_XOR,        // `^` token
        TOKEN_OP_NOT,        // `!` token
        TOKEN_OP_LSHIFT,     // `<<` token
        TOKEN_OP_RSHIFT,     // `>>` token
        TOKEN_OP_LT,         // `<` token
        TOKEN_OP_GT,         // `>` token
        TOKEN_OP_LTE,        // `<=` token
        TOKEN_OP_GTE,        // `>=` token
        TOKEN_OP_EQ,         // `==` token
        TOKEN_OP_NEQ,        // `!=` token
        TOKEN_OP_ASSIGN,     // `=` token
        TOKEN_OP_ADD_ASSIGN, // `+=` token
        TOKEN_OP_SUB_ASSIGN, // `-=` token
        TOKEN_OP_MUL_ASSIGN, // `*=` token
        TOKEN_OP_DIV_ASSIGN, // `/=` token
        TOKEN_OP_MOD_ASSIGN, // `%=` token
        TOKEN_OP_AND_ASSIGN, // `&=` token
        TOKEN_OP_OR_ASSIGN,  // `|=` token
        TOKEN_OP_XOR_ASSIGN, // `^=` token
        TOKEN_OP_INCREMENT,  // `++` token

        // Keywords
        TOKEN_KW_IF,           // `if`
        TOKEN_KW_ELSE,         // `else`
        TOKEN_KW_FOR,          // `for`
        TOKEN_KW_WHILE,        // `while`
        TOKEN_KW_BREAK,        // `break`
        TOKEN_KW_CONTINUE,     // `continue`
        TOKEN_KW_RETURN,       // `return`
        TOKEN_KW_FN,           // `function`
        TOKEN_KW_MODULE,       // `module`
        TOKEN_KW_DECLARE,      // `declare`
        TOKEN_KW_CONST,        // `const`
        TOKEN_KW_PUBLIC,       // `public`
        TOKEN_KW_PRIVATE,      // `private`
        TOKEN_KW_STRUCT,       // `struct`
        TOKEN_KW_IMPLEMENT,    // `implementation`
        TOKEN_KW_ENUM,         // `enum`
        TOKEN_KW_TRAIT,        // `trait`
        TOKEN_KW_AS,           // `as`
        TOKEN_KW_NAMESPACE,    // `namespace`
        TOKEN_KW_TYPE,         // `type`
        TOKEN_KW_CAST,         // `cast`
        TOKEN_KW_SIZE,         // `size`
        TOKEN_KW_CASE,         // `case`
        TOKEN_KW_CLASS,        // `class`
        TOKEN_KW_DEFAULT,      // `default`
        TOKEN_KW_EXTENDS,      // `extends`
        TOKEN_KW_IMPORT,       // `import`
        TOKEN_KW_IN,           // `in`
        TOKEN_KW_INSTANCEOF,   // `instanceof`
        TOKEN_KW_NEW,          // `new`
        TOKEN_KW_NULL,         // `null`
        TOKEN_KW_PROTECTED,    // `protected`
        TOKEN_KW_STATIC,       // `static`
        TOKEN_KW_RESULT,       // `result`
        TOKEN_KW_TRUE,         // `true`
        TOKEN_KW_FALSE,        // `false`
        TOKEN_KW_MUT,          // `mut`
        TOKEN_KW_BYTE,         // `byte`
        TOKEN_KW_SHORT,        // `short`
        TOKEN_KW_INT,          // `int`
        TOKEN_KW_LONG,         // `long`
        TOKEN_KW_FLOAT,        // `float`
        TOKEN_KW_DOUBLE,       // `double`
        TOKEN_KW_CHAR,         // `char`
        TOKEN_KW_BOOL,         // `bool`
        TOKEN_KW_VOID,         // `void`
        TOKEN_KW_ANY,          // `any`
        TOKEN_KW_STRING,       // `string`
        TOKEN_KW_ARRAY,        // `array`
        TOKEN_KW_MAP,          // `map`
        TOKEN_KW_SET,          // `set`
        TOKEN_KW_NATIVE,       // `native`
        TOKEN_KW_SYNCHRONIZED, // `synchronized`
        TOKEN_KW_VOLATILE,     // `volatile`
        TOKEN_KW_INTERFACE,    // `interface`
        TOKEN_KW_BOOLEAN,      // `boolean`
        TOKEN_KW_TYPEOF,       // `typeof`
        TOKEN_KW_AWAIT,        // `await`
        TOKEN_KW_ASYNC,        // `async`
        TOKEN_KW_EXPRESSION,   // `expression`
        TOKEN_KW_EXTERN,       // `extern`
        TOKEN_KW_DEBUGGER,     // `debugger`
        TOKEN_KW_CONSTRUCTOR,  // `constructor`
        TOKEN_KW_THIS,         // `this`
        TOKEN_KW_USING,        // `using`
        TOKEN_KW_AUTO,         // `auto`
        TOKEN_KW_UNDEFINED,    // `undefined`
        TOKEN_KW_PRAGMA,       // `pragma`

        // Operators and punctuations
        TOKEN_BANG,           // `!`
        TOKEN_PLUS,           // `+`
        TOKEN_MINUS,          // `-`
        TOKEN_STAR,           // `*`
        TOKEN_SLASH,          // `/`
        TOKEN_PIPE,           // `|`
        TOKEN_EXCLAMATION,    // `!`
        TOKEN_QUESTION,       // `?`
        TOKEN_COLON,          // `:`
        TOKEN_LESS,           // `<`
        TOKEN_LESS_EQUAL,     // `<=`
        TOKEN_GREATER,        // `>`
        TOKEN_GREATER_EQUAL,  // `>=`
        TOKEN_EQUAL,          // `=`
        TOKEN_STRICT_EQUAL,   // `==`
        TOKEN_NOT_EQUAL,      // `!=`
        TOKEN_DOT,            // `.`
        TOKEN_COMMA,          // `,`
        TOKEN_HASH,           // `#`
        TOKEN_DOLLAR,         // `$`
        TOKEN_AT,             // `@`
        TOKEN_BACKSLASH,      // `\`
        TOKEN_UNDERSCORE,     // `_`
        TOKEN_BACKTICK,       // '`'
        TOKEN_QUOTE,          // `'`
        TOKEN_APOSTROPHE,     // `'`
        TOKEN_SEMICOLON,      // `;`
        TOKEN_TILDE,          // `~`
        TOKEN_CARET,          // `^`
        TOKEN_AMPERSAND,      // `&`
        TOKEN_PERCENT,        // `%`
        TOKEN_LPAREN,         // `(`
        TOKEN_RPAREN,         // `)`
        TOKEN_LBRACKET,       // `[`
        TOKEN_RBRACKET,       // `]`
        TOKEN_LBRACE,         // `{`
        TOKEN_RBRACE,         // `}`
        TOKEN_LANGLE,         // `<`
        TOKEN_RANGLE,         // `>`
        TOKEN_LANGLE_BRACKET, // `<<`
        TOKEN_RANGLE_BRACKET, // `>>`
        TOKEN_LCURLY_BRACKET, // `{{`
        TOKEN_RCURLY_BRACKET, // `}}`
        TOKEN_ELLIPSIS,       // `...`
        TOKEN_DOUBLE_COLON,   // `::`
        TOKEN_INCREMENT,      // `++`
        TOKEN_DECREMENT,      // `--`

        TOKEN_ITER_VAR,    // `$`
        TOKEN_ITER_STEP,   // `..`
        TOKEN_EMPTY_ARRAY, // `[]`

        TOKEN_ADDRESS_OF,  // `&`
        TOKEN_DEREFERENCE, // `*`
    } CryoTokenType;

    typedef enum
    {
        OPERATOR_UNKNOWN = -1, // `<UNKNOWN>`       -1
        OPERATOR_ADD,          // `+`               0
        OPERATOR_SUB,          // `-`               1
        OPERATOR_MUL,          // `*`               2
        OPERATOR_DIV,          // `/`               3
        OPERATOR_MOD,          // `%`               4
        OPERATOR_AND,          // `&`               5
        OPERATOR_OR,           // `|`               6
        OPERATOR_XOR,          // `^`               7
        OPERATOR_NOT,          // `!`               8
        OPERATOR_LSHIFT,       // `<<`              9
        OPERATOR_RSHIFT,       // `>>`              10
        OPERATOR_LT,           // `<`               11
        OPERATOR_GT,           // `>`               12
        OPERATOR_LTE,          // `<=`              13
        OPERATOR_GTE,          // `>=`              14
        OPERATOR_EQ,           // `==`              15
        OPERATOR_NEQ,          // `!=`              16
        OPERATOR_ASSIGN,       // `=`               17
        OPERATOR_ADD_ASSIGN,   // `+=`              18
        OPERATOR_SUB_ASSIGN,   // `-=`              19
        OPERATOR_MUL_ASSIGN,   // `*=`              20
        OPERATOR_DIV_ASSIGN,   // `/=`              21
        OPERATOR_MOD_ASSIGN,   // `%=`              22
        OPERATOR_AND_ASSIGN,   // `&=`              23
        OPERATOR_OR_ASSIGN,    // `|=`              24
        OPERATOR_XOR_ASSIGN,   // `^=`              25
        OPERATOR_INCREMENT,    // `++`              26
        OPERATOR_DECREMENT,    // `--`              27
        OPERATOR_NA,           // `<UNKNOWN>`       28
    } CryoOperatorType;

    typedef enum CryoNodeType
    {
        NODE_PROGRAM,              // 0
        NODE_FUNCTION_DECLARATION, // 1
        NODE_VAR_DECLARATION,      // 2
        NODE_STATEMENT,            // 3
        NODE_EXPRESSION,           // 4
        NODE_BINARY_EXPR,          // 5
        NODE_UNARY_EXPR,           // 6
        NODE_LITERAL_EXPR,         // 7
        NODE_VAR_NAME,             // 8
        NODE_FUNCTION_CALL,        // 9
        NODE_IF_STATEMENT,         // 10
        NODE_WHILE_STATEMENT,      // 11
        NODE_FOR_STATEMENT,        // 12
        NODE_RETURN_STATEMENT,     // 13
        NODE_BLOCK,                // 14
        NODE_FUNCTION_BLOCK,       // 15
        NODE_EXPRESSION_STATEMENT, // 16
        NODE_ASSIGN,               // 17
        NODE_PARAM_LIST,           // 18
        NODE_PARAM,                // 19
        NODE_TYPE,                 // 20
        NODE_STRING_LITERAL,       // 21
        NODE_STRING_EXPRESSION,    // 22
        NODE_BOOLEAN_LITERAL,      // 23
        NODE_ARRAY_LITERAL,        // 24
        NODE_IMPORT_STATEMENT,     // 25
        NODE_EXTERN_STATEMENT,     // 26
        NODE_EXTERN_FUNCTION,      // 27
        NODE_ARG_LIST,             // 28
        NODE_NAMESPACE,            // 29
        NODE_INDEX_EXPR,           // 30
        NODE_VAR_REASSIGN,         // 31
        NODE_STRUCT_DECLARATION,   // 32
        NODE_PROPERTY,             // 33
        NODE_CUSTOM_TYPE,          // 34
        NODE_SCOPED_FUNCTION_CALL, // 35
        NODE_EXTERNAL_SYMBOL,      // 36
        NODE_STRUCT_CONSTRUCTOR,   // 37
        NODE_PROPERTY_ACCESS,      // 38
        NODE_THIS,                 // 39
        NODE_THIS_ASSIGNMENT,      // 40
        NODE_PROPERTY_REASSIGN,    // 41
        NODE_METHOD,               // 42
        NODE_IDENTIFIER,           // 43
        NODE_METHOD_CALL,          // 44
        NODE_ENUM,                 // 45
        NODE_GENERIC_DECL,         // 46
        NODE_GENERIC_INST,         // 47
        NODE_CLASS,                // 48
        NODE_CLASS_CONSTRUCTOR,    // 49
        NODE_OBJECT_INST,          // 50
        NODE_NULL_LITERAL,         // 51
        NODE_TYPEOF,               // 52
        NODE_USING,                // 53
        NODE_MODULE,               // 54
        NODE_ANNOTATION,           // 55
        NODE_TYPE_CAST,            // 56
        NODE_DISCARD,              // 57
        NODE_IMPLEMENTATION,       // 58
        NODE_BREAK,                // 59
        NODE_CONTINUE,             // 60
        NODE_UNKNOWN,              // 59
    } CryoNodeType;

    typedef enum CryoVisibilityType
    {
        VISIBILITY_PUBLIC,    // `public`         0
        VISIBILITY_PRIVATE,   // `private`        1
        VISIBILITY_PROTECTED, // `protected`      2
        VISIBILITY_EXTERN,    // `extern`         3
        VISIBILITY_UNKNOWN,   // `<UNKNOWN>`      4
    } CryoVisibilityType;

    typedef struct KeywordToken
    {
        char *keyword;
        CryoTokenType type;
    } KeywordToken;

    typedef enum TagType
    {
        TAG_TYPE_UNKNOWN = -1,
        TAG_TYPE_INTERNAL,
        TAG_TYPE_EXTERNAL,
    } TagType;

    typedef struct TagToken
    {
        char *tag;
        TagType type;
    } TagToken;

    typedef struct DataTypeToken
    {
        char *baseType;
        CryoTokenType dataType;
    } DataTypeToken;

    typedef union TokenValue
    {
        int intValue;
        float floatValue;
        char *stringValue;
    } TokenValue;

    typedef struct Token
    {
        CryoTokenType type; // Type of the token
        const char *lexeme; // Lexeme of the token
        const char *start;  // Pointer to the beginning of the token in the source code
        int length;         // Length of the token
        int line;           // Line number where the token is found
        int column;         // Column number where the token is found
        const char *style;  // Style of the token
        TokenValue value;   // Token value
        bool isOperator;    // Whether the token is an operator
    } Token;

    //
    // Function prototypes for the token module
    //

    char *operatorToString(CryoOperatorType op);
    char *CryoNodeTypeToString(CryoNodeType node);
    char *CryoVisibilityTypeToString(CryoVisibilityType visibility);
    char *CryoTokenToString(CryoTokenType node);
    char *CryoOperatorTypeToString(CryoOperatorType op);
    char *CryoOperatorToString(CryoOperatorType op);
    char *TokenToString(Token token);

    CryoVisibilityType stringToVisibilityType(const char *visibility);
    CryoOperatorType CryoTokenToOperator(CryoTokenType token);

#ifdef __cplusplus
}
#endif

#endif // TOKEN_H
