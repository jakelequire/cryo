#ifndef TOKEN_H
#define TOKEN_H


typedef enum {
    // Special tokens
    TOKEN_UNKNOWN,              // Unknown token        // -1
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
    TOKEN_INT,                  // `1`, `2`, `3`, etc. integers                     // 11
    TOKEN_FLOAT,                // `1.0`, `2.0`, `3.0`, etc. floats                 // 12
    TOKEN_STRING,               // `"Hello, World!"`, `"Hello, " + "World!"`, etc.  // 13
    TOKEN_STRING_LITERAL,       // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc   // 14
    TOKEN_BOOLEAN,              // `true`, `false`                                  // 15

    // Special Tokens
    TOKEN_RESULT_ARROW,         // `->` token       // 16

    // Operators
    TOKEN_OP_PLUS,              // `+` token        // 17
    TOKEN_OP_MINUS,             // `-` token        // 18
    TOKEN_OP_STAR,              // `*` token        // 19
    TOKEN_OP_SLASH,             // `/` token        // 20
    TOKEN_OP_MOD,               // `%` token        // 21
    TOKEN_OP_AND,               // `&` token        // 22
    TOKEN_OP_OR,                // `|` token        // 23
    TOKEN_OP_XOR,               // `^` token        // 24
    TOKEN_OP_NOT,               // `!` token        // 25
    TOKEN_OP_LSHIFT,            // `<<` token       // 26
    TOKEN_OP_RSHIFT,            // `>>` token       // 27
    TOKEN_OP_LT,                // `<` token        // 28
    TOKEN_OP_GT,                // `>` token        // 29
    TOKEN_OP_LTE,               // `<=` token       // 30
    TOKEN_OP_GTE,               // `>=` token       // 31
    TOKEN_OP_EQ,                // `==` token       // 32
    TOKEN_OP_NEQ,               // `!=` token       // 33
    TOKEN_OP_ASSIGN,            // `=` token        // 34
    TOKEN_OP_ADD_ASSIGN,        // `+=` token       // 35
    TOKEN_OP_SUB_ASSIGN,        // `-=` token       // 36
    TOKEN_OP_MUL_ASSIGN,        // `*=` token       // 37
    TOKEN_OP_DIV_ASSIGN,        // `/=` token       // 38
    TOKEN_OP_MOD_ASSIGN,        // `%=` token       // 39
    TOKEN_OP_AND_ASSIGN,        // `&=` token       // 40
    TOKEN_OP_OR_ASSIGN,         // `|=` token       // 41
    TOKEN_OP_XOR_ASSIGN,        // `^=` token       // 42

    // Keywords
    TOKEN_KW_IF,                // `if`             // 43
    TOKEN_KW_ELSE,              // `else`           // 44
    TOKEN_KW_FOR,               // `for`            // 45
    TOKEN_KW_WHILE,             // `while`          // 46
    TOKEN_KW_BREAK,             // `break`          // 47 
    TOKEN_KW_CONTINUE,          // `continue`       // 48
    TOKEN_KW_RETURN,            // `return`         // 49
    TOKEN_KW_FN,                // `fn`             // 50
    TOKEN_KW_CONST,             // `const`          // 51
    TOKEN_KW_LET,               // `let`            // 52
    TOKEN_KW_PUBLIC,            // `public`         // 53
    TOKEN_KW_PRIVATE,           // `private`        // 54
    TOKEN_KW_STRUCT,            // `struct`         // 55
    TOKEN_KW_ENUM,              // `enum`           // 56
    TOKEN_KW_TRAIT,             // `trait`          // 57
    TOKEN_KW_IMPL,              // `impl`           // 58
    TOKEN_KW_USE,               // `use`            // 59
    TOKEN_KW_AS,                // `as`             // 60
    TOKEN_KW_MODULE,            // `module`         // 61
    TOKEN_KW_NAMESPACE,         // `namespace`      // 62
    TOKEN_KW_TYPE,              // `type`           // 63
    TOKEN_KW_CAST,              // `cast`           // 64
    TOKEN_KW_SIZE,              // `size`           // 65
    TOKEN_KW_CASE,              // `case`           // 66
    TOKEN_KW_CLASS,             // `class`          // 67
    TOKEN_KW_DEFAULT,           // `default`        // 68
    TOKEN_KW_EXTENDS,           // `extends`        // 69
    TOKEN_KW_IMPORT,            // `import`         // 70
    TOKEN_KW_IN,                // `in`             // 71
    TOKEN_KW_INSTANCEOF,        // `instanceof`     // 72
    TOKEN_KW_NEW,               // `new`            // 73
    TOKEN_KW_NULL,              // `null`           // 74
    TOKEN_KW_PROTECTED,         // `protected`      // 75
    TOKEN_KW_STATIC,            // `static`         // 76
    TOKEN_KW_RESULT,            // `result`         // 77
    TOKEN_KW_TRUE,              // `true`           // 78
    TOKEN_KW_FALSE,             // `false`          // 79
    TOKEN_KW_MUT,               // `mut`            // 80
    TOKEN_KW_BYTE,              // `byte`           // 81
    TOKEN_KW_SHORT,             // `short`          // 82
    TOKEN_KW_INT,               // `int`            // 83
    TOKEN_KW_LONG,              // `long`           // 84
    TOKEN_KW_FLOAT,             // `float`          // 85
    TOKEN_KW_DOUBLE,            // `double`         // 86
    TOKEN_KW_CHAR,              // `char`           // 87
    TOKEN_KW_BOOL,              // `bool`           // 88
    TOKEN_KW_VOID,              // `void`           // 89
    TOKEN_KW_ANY,               // `any`            // 90
    TOKEN_KW_STRING,            // `string`         // 91
    TOKEN_KW_ARRAY,             // `array`          // 92
    TOKEN_KW_MAP,               // `map`            // 93
    TOKEN_KW_SET,               // `set`            // 94
    TOKEN_KW_NATIVE,            // `native`         // 95
    TOKEN_KW_SYNCHRONIZED,      // `synchronized`   // 96
    TOKEN_KW_VOLATILE,          // `volatile`       // 97
    TOKEN_KW_INTERFACE,         // `interface`      // 98
    TOKEN_KW_BOOLEAN,           // `boolean`        // 99
    TOKEN_KW_TYPEOF,            // `typeof`         // 100
    TOKEN_KW_AWAIT,             // `await`          // 101
    // Add more keywords as needed


    // Operators and punctuations
    TOKEN_BANG,                 // `!`              // 102
    TOKEN_PLUS,                 // `+`              // 103
    TOKEN_MINUS,                // `-`              // 104
    TOKEN_STAR,                 // `*`              // 105
    TOKEN_SLASH,                // `/`              // 106
    TOKEN_PIPE,                 // `|`              // 107
    TOKEN_EXCLAMATION,          // `!`              // 108
    TOKEN_QUESTION,             // `?`              // 109
    TOKEN_COLON,                // `:`              // 110
    TOKEN_LESS,                 // `<`              // 111
    TOKEN_GREATER,              // `>`              // 112
    TOKEN_EQUAL,                // `=`              // 113
    TOKEN_DOT,                  // `.`              // 114
    TOKEN_COMMA,                // `,`              // 115
    TOKEN_HASH,                 // `#`              // 116
    TOKEN_DOLLAR,               // `$`              // 117
    TOKEN_AT,                   // `@`              // 118
    TOKEN_BACKSLASH,            // `\`              // 119
    TOKEN_UNDERSCORE,           // `_`              // 120
    TOKEN_BACKTICK,             // '`'              // 121
    TOKEN_QUOTE,                // `'`              // 122
    TOKEN_APOSTROPHE,           // `'`              // 123
    TOKEN_SEMICOLON,            // `;`              // 124
    TOKEN_TILDE,                // `~`              // 125
    TOKEN_CARET,                // `^`              // 126
    TOKEN_AMPERSAND,            // `&`              // 127
    TOKEN_PERCENT,              // `%`              // 128
    TOKEN_LPAREN,               // `(`              // 129
    TOKEN_RPAREN,               // `)`              // 130
    TOKEN_LBRACKET,             // `[`              // 131
    TOKEN_RBRACKET,             // `]`              // 132
    TOKEN_LBRACE,               // `{`              // 133
    TOKEN_RBRACE,               // `}`              // 134
    TOKEN_LANGLE,               // `<`              // 135
    TOKEN_RANGLE,               // `>`              // 136
    TOKEN_LCURLY,               // `{`              // 137            
    TOKEN_RCURLY,               // `}`              // 138
    TOKEN_LSQUARE,              // `[`              // 139
    TOKEN_RSQUARE,              // `]`              // 140
    TOKEN_LANGLE_BRACKET,       // `<<`             // 141
    TOKEN_RANGLE_BRACKET,       // `>>`             // 142
    TOKEN_LCURLY_BRACKET,       // `{{`             // 143
    TOKEN_RCURLY_BRACKET,       // `}}`             // 144
} CryoTokenType;

#endif