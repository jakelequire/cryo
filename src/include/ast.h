#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VAR_DECL,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL_EXPR,
    NODE_VARIABLE_EXPR,
    NODE_FUNCTION_CALL,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    // Add other node types as needed
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line;  // Line number for error reporting
    union {
        struct {
            struct ASTNode** statements;
            int stmtCount;
        } program;

        struct {
            char* name;
            struct ASTNode* body;
        } functionDecl;

        struct {
            char* name;
        } varDecl;

        struct {
            struct ASTNode* stmt;
        } stmt;

        struct {
            struct ASTNode* expr;
        } expr;

        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            TokenType operator;
            char* operatorText;  // Descriptive text for the operator
        } bin_op;

        struct {
            TokenType operator;
            struct ASTNode* operand;
        } unary_op;

        int value;  // For literal number nodes

        struct {
            char* varName;
        } varName;

        struct {
            char* name;
        } functionCall;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* thenBranch;
            struct ASTNode* elseBranch;
        } ifStmt;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } whileStmt;
        
        struct {
            struct ASTNode* initializer;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } forStmt;

        struct {
            struct ASTNode* returnValue;
        } returnStmt;
        
    } data;
} ASTNode;

void printAST(ASTNode* node);
void printASTIndented(ASTNode* node, int indent);
void freeAST(ASTNode* node);

ASTNode* createLiteralExpr(int value);
ASTNode* createVariableExpr(const char* name);
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, TokenType op);
ASTNode* createUnaryExpr(TokenType operator, ASTNode* operand);
ASTNode* parseFunctionCall(const char* name);
ASTNode* parseIfStatement();
ASTNode* parseReturnStatement();
ASTNode* parseExpressionStatement();
ASTNode* createFunctionDecl(const char* name, ASTNode* body);

#endif // AST_H
