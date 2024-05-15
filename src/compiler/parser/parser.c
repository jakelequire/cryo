#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"

// Assume getToken(), peekToken() and other lexer utilities are implemented in lexer.h

static Token currentToken;

static void getNextToken() {
    currentToken = getToken();
}

static void error(const char* message) {
    printf("Error: %s at line %d\n", message, currentToken.lineNumber);
}

static Expr* parseExpression();
static Stmt* parseStatement();
static FunctionDecl* parseFunctionDeclaration();

static Expr* parsePrimaryExpression() {
    switch (currentToken.type) {
        case TOKEN_INT_LITERAL:
        case TOKEN_FLOAT_LITERAL:
        case TOKEN_STRING_LITERAL:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            {
                LiteralExpr value = { .type = currentToken.type, .value = currentToken.value };
                getNextToken();  // Consume the literal
                return createLiteralExpr(value);
            }
        case TOKEN_IDENTIFIER:
            {
                char* name = currentToken.value.asString;
                getNextToken();  // Consume the identifier
                if (currentToken.type == TOKEN_LPAREN) {
                    // Function call
                    return parseFunctionCall(name);
                }
                return createVariableExpr(name);
            }
        default:
            error("Expected an expression");
            return NULL;
    }
}

static Expr* parseExpression() {
    Expr* expr = parsePrimaryExpression();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        OperatorType op = currentToken.type == TOKEN_PLUS ? OP_PLUS : OP_MINUS;
        getNextToken();  // Consume operator
        Expr* right = parsePrimaryExpression();
        expr = createBinaryExpr(expr, right, op);
    }
    return expr;
}

static Stmt* parseStatement() {
    switch (currentToken.type) {
        case TOKEN_IF:
            return parseIfStatement();
        case TOKEN_RETURN:
            return parseReturnStatement();
        default:
            return parseExpressionStatement();
    }
}

static FunctionDecl* parseFunctionDeclaration() {
    getNextToken();  // Consume 'fn'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
        return NULL;
    }
    char* functionName = currentToken.value.asString;
    getNextToken();  // Consume function name

    // Parameters and body would be parsed here

    FunctionDecl* func = createFunctionDecl(functionName, NULL, 0, NULL);
    return func;
}

// Placeholder for the main entry point or other supporting functions
void parseProgram() {
    getNextToken();  // Initialize token stream
    while (currentToken.type != TOKEN_EOF) {
        FunctionDecl* func = parseFunctionDeclaration();
        if (func) {
            // Do something with the parsed function
        }
    }
}
