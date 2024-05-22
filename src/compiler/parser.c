#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"
#include <stdio.h>  // Include for printf

Token currentToken;

void getNextToken(Lexer* lexer) {
    currentToken = getToken(lexer);
    // printf("getNextToken: Token type: %d at line %d\n", currentToken.type, lexer->line);
}

void error(const char* message) {
    fprintf(stderr, "Error: %s at line %d column %d \n", message, currentToken.line, currentToken.column);
}

ASTNode* parsePrimaryExpression(Lexer* lexer);
ASTNode* parseUnaryExpression(Lexer* lexer);
ASTNode* parseBinaryExpression(Lexer* lexer, int precedence);
int getOperatorPrecedence(TokenType type);

ASTNode* parsePrimaryExpression(Lexer* lexer) {
    switch (currentToken.type) {
        case TOKEN_INT: {
            int value = currentToken.value.intValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(value);
        }
        case TOKEN_FLOAT: {
            float value = currentToken.value.floatValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr((int)value);
        }
        case TOKEN_STRING:
        case TOKEN_STRING_LITERAL: {
            char* value = currentToken.value.stringValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr((int)value);
        }
        case TOKEN_KW_TRUE:
        case TOKEN_KW_FALSE: {
            int value = currentToken.type == TOKEN_KW_TRUE;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(value);
        }
        case TOKEN_IDENTIFIER: {
            char* name = currentToken.value.stringValue;
            getNextToken(lexer);  // Consume the identifier
            if (currentToken.type == TOKEN_LPAREN) {
                // Function call
                return parseFunctionCall(name);
            }
            return createVariableExpr(name);
        }
        case TOKEN_LPAREN: {
            getNextToken(lexer);  // Consume '('
            ASTNode* expr = parseExpression(lexer);
            if (currentToken.type != TOKEN_RPAREN) {
                error("Expected ')'");
                return NULL;
            }
            getNextToken(lexer);  // Consume ')'
            return expr;
        }
        default:
            error("Expected an expression");
            return NULL;
    }
}

ASTNode* parseUnaryExpression(Lexer* lexer) {
    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG) {
        TokenType operator = currentToken.type;
        getNextToken(lexer);  // Consume the operator
        ASTNode* operand = parseUnaryExpression(lexer);
        return createUnaryExpr(operator, operand);
    }
    return parsePrimaryExpression(lexer);
}

ASTNode* parseBinaryExpression(Lexer* lexer, int precedence) {
    ASTNode* left = parseUnaryExpression(lexer);

    while (1) {
        int currentPrecedence = getOperatorPrecedence(currentToken.type);
        if (currentPrecedence < precedence) {
            return left;
        }

        TokenType operator = currentToken.type;
        getNextToken(lexer);  // Consume the operator
        ASTNode* right = parseBinaryExpression(lexer, currentPrecedence + 1);

        left = createBinaryExpr(left, right, operator);
    }
}

int getOperatorPrecedence(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 1;
        case TOKEN_STAR:
        case TOKEN_SLASH:
            return 2;
        default:
            return 0;
    }
}

ASTNode* parseExpression(Lexer* lexer) {
    return parseBinaryExpression(lexer, 0);
}

ASTNode* parseStatement(Lexer* lexer) {
    switch (currentToken.type) {
        case TOKEN_KW_IF:
            return parseIfStatement();
        case TOKEN_KW_RETURN:
            return parseReturnStatement();
        case TOKEN_KW_WHILE:
            return parseWhileStatement(lexer);
        case TOKEN_KW_FOR:
            return parseForStatement(lexer);
        case TOKEN_KW_CONST:
            return parseVarDeclaration(lexer);
        default:
            return parseExpressionStatement();
    }
}

ASTNode* parseWhileStatement(Lexer* lexer) {
    // Placeholder implementation
    getNextToken(lexer);  // Consume 'while'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' after while condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    // ASTNode* body = parseStatement(lexer); // Commented out unused variable
    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' after while body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    // Construct and return the while statement node
    return condition; // Placeholder, replace with actual while statement node creation
}

ASTNode* parseForStatement(Lexer* lexer) {
    // Placeholder implementation
    getNextToken(lexer);  // Consume 'for'
    // ASTNode* initializer = parseStatement(lexer); // Commented out unused variable
    // ASTNode* condition = parseExpression(lexer); // Commented out unused variable
    // ASTNode* increment = parseStatement(lexer); // Commented out unused variable
    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' after for condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    // ASTNode* body = parseStatement(lexer); // Commented out unused variable
    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' after for body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    // Construct and return the for statement node
    return NULL; // Placeholder, replace with actual for statement node creation
}

ASTNode* parseVarDeclaration(Lexer* lexer) {
    // Placeholder implementation
    getNextToken(lexer);  // Consume 'const' or 'var'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected variable name");
        return NULL;
    }
    char* varName = strdup(currentToken.value.stringValue);
    getNextToken(lexer);  // Consume variable name
    if (currentToken.type != TOKEN_ASSIGN) { // Ensure TOKEN_ASSIGN is defined in token.h
        error("Expected '=' after variable name");
        return NULL;
    }
    getNextToken(lexer);  // Consume '='
    // ASTNode* initializer = parseExpression(lexer); // Commented out unused variable
    // Construct and return the variable declaration node
    return createVariableExpr(varName); // Placeholder, replace with actual variable declaration node creation
}

ASTNode* parseFunctionDeclaration(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'fn'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
        return NULL;
    }
    const char* functionName = getTokenStringValue(&currentToken);
    printf("parseFunctionDeclaration: Function name: %s\n", functionName);
    getNextToken(lexer);  // Consume the function name

    if (currentToken.type != TOKEN_LPAREN) {
        error("Expected '(' after function name");
        return NULL;
    }
    getNextToken(lexer);

    // Parse parameters
    while (currentToken.type != TOKEN_RPAREN) {
        if (currentToken.type != TOKEN_IDENTIFIER) {
            error("Expected parameter name");
            return NULL;
        }
        getNextToken(lexer);  // Consume parameter name

        if (currentToken.type != TOKEN_COMMA && currentToken.type != TOKEN_RPAREN) {
            error("Expected ',' or ')' after parameter name");
            return NULL;
        }
        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);  // Consume ','
        }
        // Store the parameter in the function node (not shown here)
    }
    getNextToken(lexer);  // Consume ')'

    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' to start function body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'

    // Parse function body (statements)
    ASTNode* body = parseStatement(lexer);

    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' to end function body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'

    return createFunctionNode(functionName, body);
}

void parseProgram(Lexer* lexer) {
    getNextToken(lexer);  // Initialize token stream
    while (currentToken.type != TOKEN_EOF) {
        // printf("Parsing token: %d (Type: %d) at line %d\n", currentToken.type, currentToken.type, lexer->line);
        if (currentToken.type == TOKEN_KW_PUBLIC) {
            getNextToken(lexer);  // Consume 'public'
            printf("After public: Token: %d (Type: %d) at line %d\n", currentToken.type, currentToken.type, lexer->line);
            if (currentToken.type == TOKEN_KW_FN) {
                getNextToken(lexer);  // Consume 'fn'
                printf("After fn: Token: %d (Type: %d) at line %d\n", currentToken.type, currentToken.type, lexer->line);
                ASTNode* function = parseFunctionDeclaration(lexer);
                if (function == NULL) return;  // Handle error
                // Add function to the program's AST
                printf("Parsed function declaration.\n");
            } else {
                error("Expected 'fn' after 'public'");
            }
        }
        // Handle other top-level constructs
        // ...
        getNextToken(lexer);
    }
}

