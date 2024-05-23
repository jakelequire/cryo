#ifndef TOKEN_H
#define TOKEN_H


typedef enum {
    // Special tokens
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_ERROR,
    TOKEN_SINGLE_COMMENT,
    TOKEN_MULTI_COMMENT,
    TOKEN_WHITESPACE,
    TOKEN_ASSIGN,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,

    // Data types
    TOKEN_INT,                  // `1`, `2`, `3`, etc. integers
    TOKEN_FLOAT,                // `1.0`, `2.0`, `3.0`, etc. floats
    TOKEN_STRING,               // `"Hello, World!"`, `"Hello, " + "World!"`, etc.
    TOKEN_STRING_LITERAL,       // `'Hello, World!'`, `'Hello, ' + 'World!'`, etc
    TOKEN_BOOLEAN,              // `true`, `false`

    // Special Tokens
    TOKEN_RESULT_ARROW,         // `->` token

    // Operators
    TOKEN_OP_PLUS,                 // `+` token
    TOKEN_OP_MINUS,                // `-` token
    TOKEN_OP_STAR,                 // `*` token
    TOKEN_OP_SLASH,                // `/` token
    TOKEN_OP_MOD,                  // `%` token
    TOKEN_OP_AND,                  // `&` token
    TOKEN_OP_OR,                   // `|` token
    TOKEN_OP_XOR,                  // `^` token
    TOKEN_OP_NOT,                  // `!` token
    TOKEN_OP_LSHIFT,               // `<<` token
    TOKEN_OP_RSHIFT,               // `>>` token
    TOKEN_OP_LT,                   // `<` token
    TOKEN_OP_GT,                   // `>` token
    TOKEN_OP_LTE,                  // `<=` token
    TOKEN_OP_GTE,                  // `>=` token
    TOKEN_OP_EQ,                   // `==` token
    TOKEN_OP_NEQ,                  // `!=` token
    TOKEN_OP_ASSIGN,               // `=` token
    TOKEN_OP_ADD_ASSIGN,           // `+=` token
    TOKEN_OP_SUB_ASSIGN,           // `-=` token
    TOKEN_OP_MUL_ASSIGN,           // `*=` token
    TOKEN_OP_DIV_ASSIGN,           // `/=` token
    TOKEN_OP_MOD_ASSIGN,           // `%=` token
    TOKEN_OP_AND_ASSIGN,           // `&=` token
    TOKEN_OP_OR_ASSIGN,            // `|=` token
    TOKEN_OP_XOR_ASSIGN,           // `^=` token

    // Keywords
    TOKEN_KW_IF,                // `if` keyword
    TOKEN_KW_ELSE,              // `else` keyword
    TOKEN_KW_FOR,               // `for` keyword
    TOKEN_KW_WHILE,             // `while` keyword
    TOKEN_KW_BREAK,             // `break` keyword
    TOKEN_KW_CONTINUE,
    TOKEN_KW_RETURN,
    TOKEN_KW_FN,
    TOKEN_KW_CONST,
    TOKEN_KW_LET,
    TOKEN_KW_PUBLIC,
    TOKEN_KW_PRIVATE,
    TOKEN_KW_STRUCT,
    TOKEN_KW_ENUM,
    TOKEN_KW_TRAIT,
    TOKEN_KW_IMPL,
    TOKEN_KW_USE,
    TOKEN_KW_AS,
    TOKEN_KW_MODULE,
    TOKEN_KW_NAMESPACE,
    TOKEN_KW_TYPE,
    TOKEN_KW_CAST,
    TOKEN_KW_SIZE,
    TOKEN_KW_CASE,
    TOKEN_KW_CLASS,
    TOKEN_KW_DEFAULT,
    TOKEN_KW_EXTENDS,
    TOKEN_KW_IMPORT,
    TOKEN_KW_IN,
    TOKEN_KW_INSTANCEOF,
    TOKEN_KW_NEW,
    TOKEN_KW_NULL,
    TOKEN_KW_PROTECTED,
    TOKEN_KW_STATIC,
    TOKEN_KW_RESULT,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
    TOKEN_KW_MUT,
    TOKEN_KW_BYTE,
    TOKEN_KW_SHORT,
    TOKEN_KW_INT,
    TOKEN_KW_LONG,
    TOKEN_KW_FLOAT,
    TOKEN_KW_DOUBLE,
    TOKEN_KW_CHAR,
    TOKEN_KW_BOOL,
    TOKEN_KW_VOID,
    TOKEN_KW_ANY,
    TOKEN_KW_STRING,
    TOKEN_KW_ARRAY,
    TOKEN_KW_MAP,
    TOKEN_KW_SET,
    TOKEN_KW_NATIVE,
    TOKEN_KW_SYNCHRONIZED,
    TOKEN_KW_VOLATILE,
    TOKEN_KW_INTERFACE,
    TOKEN_KW_BOOLEAN,
    TOKEN_KW_TYPEOF,
    TOKEN_KW_AWAIT,
    // Add more keywords as needed


    // Operators and punctuations
    TOKEN_BANG,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PIPE,
    TOKEN_EXCLAMATION,
    TOKEN_QUESTION,
    TOKEN_COLON,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_EQUAL,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_HASH,
    TOKEN_DOLLAR,
    TOKEN_AT,
    TOKEN_BACKSLASH,
    TOKEN_UNDERSCORE,
    TOKEN_BACKTICK,
    TOKEN_QUOTE,
    TOKEN_APOSTROPHE,
    TOKEN_SEMICOLON,
    TOKEN_TILDE,
    TOKEN_CARET,
    TOKEN_AMPERSAND,
    TOKEN_PERCENT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LANGLE,
    TOKEN_RANGLE,
    TOKEN_LCURLY,
    TOKEN_RCURLY,
    TOKEN_LSQUARE,
    TOKEN_RSQUARE,
    TOKEN_LANGLE_BRACKET,
    TOKEN_RANGLE_BRACKET,
    TOKEN_LCURLY_BRACKET,
    TOKEN_RCURLY_BRACKET,
} TokenType;

#endif