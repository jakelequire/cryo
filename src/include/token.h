

typedef enum {
    // Special tokens
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_ERROR,
    TOKEN_SINGLE_COMMENT,
    TOKEN_MULTI_COMMENT,
    TOKEN_WHITESPACE,

    // Data types
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_STRING_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,

    // Special Tokens
    TOKEN_RESULT_ARROW,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_FN,


    // Operators and punctuations
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

