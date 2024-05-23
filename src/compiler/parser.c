#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"
#include <stdio.h>
#include <stdlib.h>

Token currentToken;

void getNextToken(Lexer *lexer) {
    currentToken = get_next_token(lexer);
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
            return createLiteralExpr((int)value); // Change this line to correct casting
        }
        case TOKEN_STRING:
        case TOKEN_STRING_LITERAL: {
            char* value = currentToken.value.stringValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr((int)value); // Change this line to correct casting
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
            return parseIfStatement(lexer);
        case TOKEN_KW_RETURN:
            return parseReturnStatement(lexer);
        case TOKEN_KW_WHILE:
            return parseWhileStatement(lexer);
        case TOKEN_KW_FOR:
            return parseForStatement(lexer);
        case TOKEN_KW_CONST:
            return parseVarDeclaration(lexer);
        default:
            return parseExpressionStatement(lexer);
    }
}

ASTNode* parseIfStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'if'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' after if condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* thenBranch = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' after if body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    ASTNode* elseBranch = NULL;
    if (currentToken.type == TOKEN_KW_ELSE) {
        getNextToken(lexer);  // Consume 'else'
        if (currentToken.type == TOKEN_LBRACE) {
            getNextToken(lexer);  // Consume '{'
            elseBranch = parseStatement(lexer);
            if (currentToken.type != TOKEN_RBRACE) {
                error("Expected '}' after else body");
                return NULL;
            }
            getNextToken(lexer);  // Consume '}'
        } else {
            elseBranch = parseStatement(lexer);
        }
    }
    return createIfStatement(condition, thenBranch, elseBranch);
}

ASTNode* parseReturnStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'return'
    ASTNode* expr = NULL;
    if (currentToken.type != TOKEN_SEMICOLON) {
        expr = parseExpression(lexer);
    }
    if (currentToken.type != TOKEN_SEMICOLON) {
        error("Expected ';' after return value");
        return NULL;
    }
    getNextToken(lexer);  // Consume ';'
    return createReturnStatement(expr);
}

ASTNode* parseExpressionStatement(Lexer* lexer) {
    ASTNode* expr = parseExpression(lexer);
    if (currentToken.type != TOKEN_SEMICOLON) {
        error("Expected ';' after expression");
        return NULL;
    }
    getNextToken(lexer);  // Consume ';'
    return createExpressionStatement(expr);
}

ASTNode* parseWhileStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'while'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' after while condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' after while body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    return createWhileStatement(condition, body);
}

ASTNode* parseForStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'for'
    ASTNode* initializer = parseStatement(lexer);
    ASTNode* condition = parseExpression(lexer);
    ASTNode* increment = parseStatement(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("Expected '{' after for condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("Expected '}' after for body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    return createForStatement(initializer, condition, increment, body);
}

ASTNode* parseVarDeclaration(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'const' or 'var'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected variable name");
        return NULL;
    }
    char* varName = _strdup(currentToken.value.stringValue);
    getNextToken(lexer);  // Consume variable name
    if (currentToken.type != TOKEN_ASSIGN) {
        error("Expected '=' after variable name");
        return NULL;
    }
    getNextToken(lexer);  // Consume '='
    ASTNode* initializer = parseExpression(lexer);
    return createVarDeclarationNode(varName, initializer);
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
    getNextToken(lexer);  // Consume '('

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
        if (currentToken.type == TOKEN_KW_PUBLIC) {
            getNextToken(lexer);  // Consume 'public'
            if (currentToken.type == TOKEN_KW_FN) {
                getNextToken(lexer);  // Consume 'fn'
                ASTNode* function = parseFunctionDeclaration(lexer);
                if (function == NULL) return;  // Handle error
                // Add function to the program's AST
            } else {
                error("Expected 'fn' after 'public'");
            }
        } else {
            error("Unexpected token at top level");
        }
        getNextToken(lexer);
    }
}
