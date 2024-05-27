#include "include/parser.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef HAVE_STRNDUP
char* strndup(const char* s, size_t n) {
    size_t len = strnlen(s, n);
    char* new_str = (char*)malloc(len + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memcpy(new_str, s, len);
    new_str[len] = '\0';
    return new_str;
}
#endif

Token currentToken;

void getNextToken(Lexer *lexer) {
    // Ensure currentToken.start is valid before using it
    if (currentToken.start == NULL) {
        printf("{parser} currentToken.start is NULL in getNextToken, getting\n");
        return;
    }
    currentToken = get_next_token(lexer);

    printf("{lexer} [DEBUG] Next token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);
}



void error(const char* message) {
    fprintf(stderr, "{parser} Error: %s at line %d, column %d\n", message, currentToken.line, currentToken.column);
    exit(1); // Exit on error to prevent further processing
}


void consume(Lexer *lexer, CryoTokenType type, const char *message) {
    printf("{parser} [DEBUG] Consuming token: expected: %d, actual: %d\n", type, currentToken.type);
    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        fprintf(stderr, "{parser} Critical error: Panic\n");
        error(message);
        exit(1);
    }
}


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
            return createLiteralExpr(*(int*)&value); // Correct casting
        }
        case TOKEN_STRING_LITERAL: {
            char* value = currentToken.value.stringValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(*(int*)&value); // Correct casting
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
                error("{parser} Expected ')'");
                return NULL;
            }
            getNextToken(lexer);  // Consume ')'
            return expr;
        }
        default:
            error("{parser} Expected an expression");
            return NULL;
    }
}

ASTNode* parseUnaryExpression(Lexer* lexer) {
    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG) {
        CryoTokenType operator = currentToken.type;
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

        CryoTokenType operator = currentToken.type;
        getNextToken(lexer);  // Consume the operator
        ASTNode* right = parseBinaryExpression(lexer, currentPrecedence + 1);

        left = createBinaryExpr(left, right, operator);
    }
}

int getOperatorPrecedence(CryoTokenType type) {
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
        error("{parser} Expected '{' after if condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* thenBranch = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("{parser} Expected '}' after if body");
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
                error("{parser} Expected '}' after else body");
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
        error("{parser} Expected ';' after return value");
        return NULL;
    }
    getNextToken(lexer);  // Consume ';'
    return createReturnStatement(expr);
}

ASTNode* parseExpressionStatement(Lexer* lexer) {
    ASTNode* expr = parseExpression(lexer);
    if (currentToken.type != TOKEN_SEMICOLON) {
        error("{parser} Expected ';' after expression");
        return NULL;
    }
    getNextToken(lexer);  // Consume ';'
    return createExpressionStatement(expr);
}

ASTNode* parseWhileStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'while'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("{parser} Expected '{' after while condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("{parser} Expected '}' after while body");
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
        error("{parser} Expected '{' after for condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("{parser} Expected '}' after for body");
        return NULL;
    }
    getNextToken(lexer);  // Consume '}'
    return createForStatement(initializer, condition, increment, body);
}

ASTNode* parseVarDeclaration(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'const' or 'var'
    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("{parser} Expected variable name");
        return NULL;
    }
    char* varName = _strdup(currentToken.value.stringValue);
    getNextToken(lexer);  // Consume variable name
    if (currentToken.type != TOKEN_ASSIGN) {
        error("{parser} Expected '=' after variable name");
        return NULL;
    }
    getNextToken(lexer);  // Consume '='
    ASTNode* initializer = parseExpression(lexer);
    return createVarDeclarationNode(varName, initializer);
}

ASTNode* parseBlock(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing block...\n");
    ASTNode* block = createBlock();

    getNextToken(lexer);  // Consume '{'
    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* statement = parseStatement(lexer);
        if (statement == NULL) {
            fprintf(stderr, "{parser} Error: Failed to parse statement\n");
            free(block);
            return NULL;
        }
        addStatementToBlock(block, statement);
    }

    if (currentToken.type != TOKEN_RBRACE) {
        fprintf(stderr, "{parser} Error: Expected '}', but got %.*s\n", currentToken.length, currentToken.start);
        free(block);
        return NULL;
    }

    getNextToken(lexer);  // Consume '}'
    printf("{parser} [DEBUG] Block parsed successfully\n");
    return block;
}



// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing function declaration...\n");

    // Consume 'fn'
    getNextToken(lexer);

    if (currentToken.start == NULL) {
        fprintf(stderr, "Error: currentToken.start is NULL after 'fn'\n");
        return NULL;
    }

    // Check for function name
    if (currentToken.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error: Expected function name, but got %.*s\n", currentToken.length, currentToken.start);
        return NULL;
    }

    // Parse function name
    char* functionName = strndup(currentToken.start, currentToken.length);
    if (functionName == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for function name\n");
        return NULL;
    }
    printf("{parser} [DEBUG] Function name: %s\n", functionName);

    // Consume the function name token
    getNextToken(lexer);
    printf("{parser} [DEBUG] After function name, current token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

    // Check for '(' token
    if (currentToken.type != TOKEN_LPAREN) {
        fprintf(stderr, "Error: Expected '(', but got %.*s\n", currentToken.length, currentToken.start);
        free(functionName);
        return NULL;
    }

    // Consume '(' token
    getNextToken(lexer);
    printf("{parser} [DEBUG] After '(', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

    // Parse parameters (if any)
    while (currentToken.type != TOKEN_RPAREN) {
        if (currentToken.type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Error: Expected parameter name, but got %.*s\n", currentToken.length, currentToken.start);
            free(functionName);
            return NULL;
        }
        char* paramName = strndup(currentToken.start, currentToken.length);
        if (paramName == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for parameter name\n");
            free(functionName);
            return NULL;
        }
        getNextToken(lexer); // Consume parameter name
        printf("{parser} [DEBUG] After parameter name, current token: %.*s, Type: %d, Line: %d, Column: %d\n",
               currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

        // Check for ':' token
        if (currentToken.type != TOKEN_COLON) {
            fprintf(stderr, "Error: Expected ':', but got %.*s\n", currentToken.length, currentToken.start);
            free(paramName);
            free(functionName);
            return NULL;
        }
        getNextToken(lexer); // Consume ':'
        printf("{parser} [DEBUG] After ':', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
               currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

        // Parse parameter type
        if (currentToken.type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Error: Expected parameter type, but got %.*s\n", currentToken.length, currentToken.start);
            free(paramName);
            free(functionName);
            return NULL;
        }
        char* paramType = strndup(currentToken.start, currentToken.length);
        if (paramType == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for parameter type\n");
            free(paramName);
            free(functionName);
            return NULL;
        }
        printf("{parser} [DEBUG] Parameter: %s of type %s\n", paramName, paramType);
        free(paramName);
        free(paramType);

        getNextToken(lexer); // Consume parameter type
        printf("{parser} [DEBUG] After parameter type, current token: %.*s, Type: %d, Line: %d, Column: %d\n",
               currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

        if (currentToken.type != TOKEN_COMMA && currentToken.type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: Expected ',' or ')' after parameter, but got %.*s\n", currentToken.length, currentToken.start);
            free(functionName);
            return NULL;
        }
        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer); // Consume ','
            printf("{parser} [DEBUG] After ',', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
                   currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);
        }
    }

    getNextToken(lexer); // Consume ')'
    printf("{parser} [DEBUG] After ')', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

    // Check for '->' return type indicator
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer); // Consume '->'
        printf("{parser} [DEBUG] After '->', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
               currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

        // Parse return type
        if (currentToken.type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Error: Expected return type, but got %.*s\n", currentToken.length, currentToken.start);
            free(functionName);
            return NULL;
        }
        char* returnType = strndup(currentToken.start, currentToken.length);
        if (returnType == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for return type\n");
            free(functionName);
            return NULL;
        }
        printf("{parser} [DEBUG] Return type: %s\n", returnType);
        free(returnType);

        getNextToken(lexer); // Consume return type
        printf("{parser} [DEBUG] After return type, current token: %.*s, Type: %d, Line: %d, Column: %d\n",
               currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);
    }

    // Check for '{' token to start function body
    if (currentToken.type != TOKEN_LBRACE) {
        fprintf(stderr, "Error: Expected '{', but got %.*s\n", currentToken.length, currentToken.start);
        free(functionName);
        return NULL;
    }
    getNextToken(lexer); // Consume '{'
    printf("{parser} [DEBUG] After '{', current token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);

    // Parse function body
    ASTNode* body = parseBlock(lexer);
    if (body == NULL) {
        fprintf(stderr, "Error: Failed to parse function body\n");
        free(functionName);
        return NULL;
    }

    // Create function AST node
    ASTNode* functionNode = createFunctionNode(functionName, body);
    free(functionName);

    return functionNode;
}
// </parseFunctionDeclaration>

ASTNode* parseProgram(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing program...\n");

    ASTNode* program = createBlock();
    if (program == NULL) {
        fprintf(stderr, "Error: Failed to create program block\n");
        return NULL;
    }
    printf("{parser} [DEBUG] Program block initialized...\n");

    getNextToken(lexer);  // Initialize token stream
    while (currentToken.type != TOKEN_EOF) {
        printf("{parser} [DEBUG] Parsing token: %.*s, Type: %d\n", currentToken.length, currentToken.start, currentToken.type);

        switch (currentToken.type) {
            case TOKEN_KW_PUBLIC:
                printf("{parser} [DEBUG] Found 'public' keyword\n");
                getNextToken(lexer);  // Consume 'public'
                printf("{parser} [DEBUG] After consuming 'public', token: %.*s, Type: %d\n", currentToken.length, currentToken.start, currentToken.type);

                if (currentToken.type == TOKEN_KW_FN) {
                    printf("{parser} [DEBUG] Found 'fn' after 'public'\n");
                    ASTNode* function = parseFunctionDeclaration(lexer);
                    if (function == NULL) {
                        fprintf(stderr, "Error: Failed to parse function declaration\n");
                        freeAST(program);
                        return NULL;
                    }
                    addStatementToBlock(program, function);  // Add function to the program's AST
                } else {
                    fprintf(stderr, "Error: Expected 'fn' after 'public', but got %.*s\n", currentToken.length, currentToken.start);
                    freeAST(program);
                    return NULL;
                }
                break;

            case TOKEN_KW_FN:
                printf("{parser} [DEBUG] Found 'fn' keyword\n");
                ASTNode* function = parseFunctionDeclaration(lexer);
                if (function == NULL) {
                    fprintf(stderr, "Error: Failed to parse function declaration\n");
                    freeAST(program);
                    return NULL;
                }
                addStatementToBlock(program, function);  // Add function to the program's AST
                break;

            default:
                fprintf(stderr, "Error: Unexpected token at top level: %.*s, Type: %d\n", currentToken.length, currentToken.start, currentToken.type);
                freeAST(program);
                return NULL;
        }

        getNextToken(lexer);
    }
    printf("{parser} [DEBUG] Finished parsing program\n");
    return program;
}




