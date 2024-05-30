#ifndef AST_H
#define AST_H

#include "token.h"
#include "lexer.h"

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECLARATION,
    NODE_VAR_DECLARATION,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_LITERAL,
    NODE_VAR_NAME,
    NODE_FUNCTION_CALL,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_BLOCK,
    NODE_EXPRESSION_STATEMENT,
    NODE_ASSIGN,
    NODE_PARAM_LIST,
    NODE_TYPE,
    NODE_UNKNOWN,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line;  // Line number for error reporting
    struct ASTNode* firstChild;  // First child node (for linked list structure)
    struct ASTNode* nextSibling; // Next sibling node (for linked list structure)
    union {
        struct {
            struct ASTNode** statements;
            int stmtCount;
            int stmtCapacity;
        } program;

        struct {
            char* name;
            struct ASTNode* params;
            struct ASTNode* returnType;
            struct ASTNode* body;
        } functionDecl;

        struct {
            char* name;
            struct ASTNode* initializer;
        } varDecl;

        struct {
            struct ASTNode** statements;
            int stmtCount;
            int stmtCapacity;
        } block;

        struct {
            struct ASTNode* stmt;
        } stmt;

        struct {
            struct ASTNode* expr;
        } expr;

        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            CryoTokenType operator;
            char* operatorText;  // Descriptive text for the operator
        } bin_op;

        struct {
            CryoTokenType operator;
            struct ASTNode* operand;
        } unary_op; // For unary operators, e.g (-5, !true, etc.)

        int value;  // For literal number nodes

        struct {
            char* varName;
        } varName;

        struct {
            char* name;
            struct ASTNode** args;
            int argCount;
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

        struct {
            struct ASTNode** params;
            int paramCount;
        } paramList;
        
    } data;
} ASTNode;

// Function prototypes for creating AST nodes
ASTNode* createASTNode(NodeType type);
ASTNode* createTypeNode(CryoTokenType type);
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body);
ASTNode* createParamNode(const char* name, ASTNode* type);
ASTNode* createLiteralExpr(int value);
ASTNode* createVariableExpr(const char* name);
ASTNode* createBinaryExpr(ASTNode* left, ASTNode* right, CryoTokenType operator);
ASTNode* createUnaryExpr(CryoTokenType operator, ASTNode* operand);
ASTNode* createFunctionNode(const char* function_name, ASTNode* function_body);
ASTNode* createReturnStatement(ASTNode* return_val);
ASTNode* createBlock();
ASTNode* createIfStatement(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* createWhileStatement(ASTNode* condition, ASTNode* body);
ASTNode* createForStatement(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body);
ASTNode* createVarDeclarationNode(const char* var_name, ASTNode* initializer);
ASTNode* createExpressionStatement(ASTNode* expression);
ASTNode* createFunctionCallNode(const char* name, ASTNode** args, int argCount);

void addStatementToBlock(ASTNode* block, ASTNode* statement);
void addFunctionToProgram(ASTNode* program, ASTNode* function);

void freeAST(ASTNode* node);
void printAST(ASTNode* node, int indent);

#endif // AST_H
