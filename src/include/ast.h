#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "token.h"

void printAST(ASTNode* node);

void printASTIndented(ASTNode* node, int indent);

// Node types for different AST nodes
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VAR_DECL,
    NODE_STATEMENT,
    NODE_EXPRESSION
} NodeType;

// Expression types
typedef enum {
    EXPR_ASSIGNMENT,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_FUNCTION_CALL
} ExprType;

// Statement types
typedef enum {
    STMT_EXPRESSION,
    STMT_RETURN,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_BREAK,
    STMT_CONTINUE
} StmtType;

// Data type for literals
typedef enum {
    DATA_INT,
    DATA_FLOAT,
    DATA_BOOL,
    DATA_STRING,
    DATA_NULL
} DataType;

// Operator types for expressions
typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT
} OperatorType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef struct {
    Expr* left;
    OperatorType op;
    Expr* right;
} BinaryExpr;

typedef struct {
    OperatorType op;
    Expr* operand;
} UnaryExpr;

typedef struct {
    DataType type;
    union {
        int intValue;
        float floatValue;
        bool boolValue;
        char* stringValue;
    } value;
} LiteralExpr;

typedef struct {
    char* name;
} VariableExpr;

typedef struct {
    char* functionName;
    Expr** arguments;
    int argCount;
} FunctionCallExpr;

struct Expr {
    ExprType type;
    union {
        BinaryExpr binary;
        UnaryExpr unary;
        LiteralExpr literal;
        VariableExpr variable;
        FunctionCallExpr functionCall;
    } data;
};

typedef struct {
    Expr* expression;
} ExpressionStmt;

typedef struct {
    char* variableName;
    DataType varType;
    Expr* initialValue;
    bool isConst;
} VarDeclStmt;

typedef struct {
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;
} IfStmt;

typedef struct {
    Expr* condition;
    Stmt* body;
} WhileStmt;

typedef struct {
    Stmt* initialization;
    Expr* condition;
    Expr* increment;
    Stmt* body;
} ForStmt;

typedef struct {
    Expr* returnValue;
} ReturnStmt;

struct Stmt {
    StmtType type;
    union {
        ExpressionStmt exprStmt;
        VarDeclStmt varDecl;
        IfStmt ifStmt;
        WhileStmt whileStmt;
        ForStmt forStmt;
        ReturnStmt returnStmt;
    } data;
    Stmt* next;  // For linking statements in a block
};

typedef struct {
    char* name;
    VarDeclStmt* parameters;
    int paramCount;
    DataType returnType;
    Stmt* body;
} FunctionDecl;

typedef struct {
    FunctionDecl** functions;
    int functionCount;
} Program;