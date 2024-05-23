#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
    NODE_PROGRAM,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_VAR_DECLARATION,
    NODE_FUNCTION_DECLARATION,
    NODE_FUNCTION_CALL,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL,
    NODE_IDENTIFIER
} NodeType;

typedef struct Node {
    NodeType type;
    struct Node *left;
    struct Node *right;
    char *value;
} Node;

Node *create_node(NodeType type);
void free_node(Node *node);

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
    struct ASTNode* next;  // Next node in the linked list
} ASTNode;

ASTNode* createLiteralExpr(int value);
ASTNode* createVariableExpr(const char* name);
ASTNode* createUnaryExpr(TokenType operator, ASTNode* operand);
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, TokenType operator);
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body);
ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* createVarDeclarationNode(const char* name, ASTNode* initializer);
ASTNode* createFunctionNode(const char* name, ASTNode* body);
ASTNode* parseFunctionCall(const char* name);
ASTNode* createIfStatement(ASTNode* condition, ASTNode* thenBranch, ASTNode* elseBranch);
ASTNode* createReturnStatement(ASTNode* expr);
ASTNode* createExpressionStatement(ASTNode* expr);


#endif // AST_H
