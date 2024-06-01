/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *  
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          * 
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "parser.h"


#ifndef HAVE_STRNDUP
// <strndup>
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
// </strndup>
#endif

// Scope-Declared Current Token.
Token currentToken;


// <error>
void error(const char* message) {
    fprintf(stderr, "{parser} Error: %s at line %d, column %d\n", message, currentToken.line, currentToken.column);
    exit(1); // Exit on error to prevent further processing
}
// </error>


// <getNextToken>
void getNextToken(Lexer *lexer) {
    printf("{parser} [DEBUG] Getting next token...\n");
    currentToken = get_next_token(lexer);
    printf("{parser} [DEBUG] Token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);
}
// </getNextToken>


// <consume>
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
// </consume>


// <getOperatorPrecedence>
int getOperatorPrecedence(CryoTokenType type) {
    switch (type) {
        case TOKEN_OP_PLUS:
        case TOKEN_OP_MINUS:
            return 1;
        case TOKEN_OP_STAR:
        case TOKEN_OP_SLASH:
            return 2;
        default:
            return 0;
    }
}
// </getOperatorPrecedence>


// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer* lexer) {
    switch (currentToken.type) {
        case TOKEN_TYPE_INT: {
            int value = currentToken.value.intValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(value);
        }
        case TOKEN_TYPE_FLOAT: {
            float value = currentToken.value.floatValue;
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(*(int*)&value); // Correct casting
        }
        case TOKEN_TYPE_STRING_LITERAL: {
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
            char* name = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);  // Consume the identifier
            if (currentToken.type == TOKEN_LPAREN) {
                // Function call
                getNextToken(lexer);  // Consume '('
                if (currentToken.type == TOKEN_RPAREN) {
                    getNextToken(lexer);  // Consume ')'
                    return createFunctionCallNode(name, NULL, 0);
                } else {
                    // Parse parameters if any
                    // Note: Add parameter parsing logic if needed
                }
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
            printf("{parser} [ERROR] debug: %d\n", currentToken.type);
            printf("{parser} [ERROR] debug: %.*s\n", currentToken.length, currentToken.start);
            error("{parser} Expected an expression");
            exit(1);
            return NULL;
    }
}
// </parsePrimaryExpression>


// <parseUnaryExpression>
ASTNode* parseUnaryExpression(Lexer* lexer) {
    if (currentToken.type == TOKEN_OP_MINUS || currentToken.type == TOKEN_OP_NOT) {
        CryoTokenType operator = currentToken.type;
        getNextToken(lexer);  // Consume the operator
        ASTNode* operand = parseUnaryExpression(lexer);
        return createUnaryExpr(operator, operand);
    }
    return parsePrimaryExpression(lexer);
}
// </parseUnaryExpression>


// <parseBinaryExpression>
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
// </parseBinaryExpression>


// <parseExpression>
ASTNode* parseExpression(Lexer* lexer) {
    // Handle primary expressions
    return parseBinaryExpression(lexer, 0);
}
// </parseExpression>


// <parseStatement>
ASTNode* parseStatement(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing statement...\n");
    switch (currentToken.type) {
        case TOKEN_KW_PUBLIC:
        case TOKEN_KW_FN:
            printf("{parser} [DEBUG] Parsing function declaration...\n");
            return parseFunctionDeclaration(lexer);
        case TOKEN_KW_RETURN:
            printf("{parser} [DEBUG] Parsing return statement...\n");
            return parseReturnStatement(lexer);
        case TOKEN_KW_IF:
            printf("{parser} [DEBUG] Parsing if statement...\n");
            return parseIfStatement(lexer);
        case TOKEN_KW_WHILE:
            printf("{parser} [DEBUG] Parsing while statement...\n");
            return parseWhileStatement(lexer);
        case TOKEN_KW_FOR:
            printf("{parser} [DEBUG] Parsing for statement...\n");
            return parseForStatement(lexer);
        case TOKEN_KW_CONST:
        case TOKEN_KW_LET:
            printf("{parser} [DEBUG] Parsing variable declaration...\n");
            return parseVarDeclaration(lexer);
        case TOKEN_LBRACE:
            printf("{parser} [DEBUG] Parsing block...\n");
            return parseBlock(lexer);
        default:
            printf("{parser} [DEBUG] Parsing expression statement...\n");
            return parseExpressionStatement(lexer);
    }
}
// </parseStatement>


// <parseIfStatement>
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
// </parseIfStatement>


// <parseReturnStatement>
ASTNode* parseReturnStatement(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing return statement...\n");
    consume(lexer, TOKEN_KW_RETURN, "Expected 'return' keyword");

    ASTNode* returnValue = NULL;
    if (currentToken.type != TOKEN_SEMICOLON) {
        returnValue = parseExpression(lexer);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after return value");

    return createReturnStatement(returnValue);
}
// </parseReturnStatement>


// <parseExpressionStatement>
ASTNode* parseExpressionStatement(Lexer* lexer) {
    printf("{parser} [DEBUG] Parsing expression statement...\n");
    ASTNode* expr = parseExpression(lexer);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after expression");
    return createExpressionStatement(expr);
}
// </parseExpressionStatement>


// <parseWhileStatement>
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
// </parseWhileStatement>


// <parseForStatement>
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
// </parseForStatement>


// <parseVarDeclaration>
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
// </parseVarDeclaration>


// <parseBlock>
ASTNode* parseBlock(Lexer* lexer) {
    ASTNode* blockNode = createASTNode(NODE_BLOCK);
    blockNode->data.block.statements = malloc(sizeof(ASTNode*) * INITIAL_STATEMENT_CAPACITY);
    blockNode->data.block.stmtCount = 0;
    blockNode->data.block.stmtCapacity = INITIAL_STATEMENT_CAPACITY;

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer);
        if (stmt) {
            if (blockNode->data.block.stmtCount >= blockNode->data.block.stmtCapacity) {
                blockNode->data.block.stmtCapacity *= 2;
                blockNode->data.block.statements = realloc(blockNode->data.block.statements, sizeof(ASTNode*) * blockNode->data.block.stmtCapacity);
            }
            blockNode->data.block.statements[blockNode->data.block.stmtCount++] = stmt;
            printf("{parser} [DEBUG] Block statement parsed\n");
        } else {
            // Handle parse errors
            printf("{parser} [ERROR] Failed to parse block statement\n");
            freeAST(blockNode);
            return NULL;
        }
    }

    if (currentToken.type != TOKEN_RBRACE) {
        error("{parser} Expected '}' at the end of block");
        freeAST(blockNode);
        return NULL;
    }

    getNextToken(lexer);  // Consume '}'
    printf("{parser} [DEBUG] Finished parsing block\n");
    return blockNode;
}
// </parseBlock>


// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer* lexer) {
    printf("{parser} [DEBUG] Starting function declaration...\n");

    if (currentToken.type == TOKEN_KW_PUBLIC) {
        printf("{parser} [DEBUG] Consuming 'public' keyword...\n");
        consume(lexer, TOKEN_KW_PUBLIC, "Expected 'public'");
    }
    consume(lexer, TOKEN_KW_FN, "Expected 'fn'");
    
    Token name = currentToken;
    printf("{parser} [DEBUG] Function name: %.*s\n", name.length, name.start);
    consume(lexer, TOKEN_IDENTIFIER, "Expected function name");
    
    consume(lexer, TOKEN_LPAREN, "Expected '('");

    // Parse parameters (if any)
    ASTNode* paramList = NULL;
    if (currentToken.type != TOKEN_RPAREN) {
        paramList = parseParamList(lexer);
    }
    consume(lexer, TOKEN_RPAREN, "Expected ')'");

    // Check for '->' return type syntax
    ASTNode* returnType = NULL;
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        printf("{parser} [DEBUG] Parsing return type...\n");
        getNextToken(lexer);  // Consume '->'
        returnType = parseType(lexer);
    } else {
        // Handle cases with no explicit return type (optional)
        printf("{parser} [DEBUG] No explicit return type, assuming void...\n");
        returnType = createTypeNode(TOKEN_TYPE_VOID); // Assuming a default void type if none is specified
    }

    // Parse function body
    printf("{parser} [DEBUG] Parsing function body...\n");
    consume(lexer, TOKEN_LBRACE, "Expected '{'");
    ASTNode* body = parseBlock(lexer);
    consume(lexer, TOKEN_RBRACE, "Expected '}'");

    // Create and return the function declaration AST node
    return createFunctionDeclNode(name.start, paramList, returnType, body);
}
// </parseFunctionDeclaration>


// <parseParamList>
ASTNode* parseParamList(Lexer* lexer) {
    // Create a node for the parameter list
    ASTNode* paramList = createASTNode(NODE_PARAM_LIST);
    paramList->data.paramList.params = NULL;
    paramList->data.paramList.paramCount = 0;

    // Assuming a linked list structure for parameters
    ASTNode* currentParam = NULL;

    while (currentToken.type != TOKEN_RPAREN) {
        // Parse each parameter
        Token paramName = currentToken;
        consume(lexer, TOKEN_IDENTIFIER, "Expected parameter name");
        
        // Optionally, parse parameter type
        ASTNode* paramType = NULL;
        if (currentToken.type == TOKEN_COLON) {
            getNextToken(lexer);  // Consume ':'
            paramType = parseType(lexer);
        }

        ASTNode* paramNode = createParamNode(paramName.start, paramType);

        // Link parameters
        if (currentParam == NULL) {
            paramList->firstChild = paramNode;
        } else {
            currentParam->nextSibling = paramNode;
        }
        currentParam = paramNode;

        // Consume comma if present
        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);  // Consume ','
        }
    }

    return paramList;
}
// </parseParamList>


// <createTypeNode>
ASTNode* createTypeNode(CryoTokenType type) {
    ASTNode* node = createASTNode(NODE_TYPE);
    node->data.value = type;
    return node;
}
// </createTypeNode>


// <parseType>
ASTNode* parseType(Lexer* lexer) {
    // Assuming types are identifiers (e.g., int, float, etc.)
    Token typeName = currentToken;
    consume(lexer, TOKEN_IDENTIFIER, "Expected type name");
    return createTypeNode(typeName.type);
}
// </parseType>


// <createParamNode>
ASTNode* createParamNode(const char* name, ASTNode* type) {
    ASTNode* node = createASTNode(NODE_VAR_DECLARATION);
    node->data.varDecl.name = strdup(name);
    node->data.varDecl.initializer = type;
    return node;
}
// </createParamNode>


// <createFunctionDeclNode>
ASTNode* createFunctionDeclNode(const char* name, ASTNode* params, ASTNode* returnType, ASTNode* body) {
    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    node->data.functionDecl.name = strdup(name);
    node->data.functionDecl.params = params;
    node->data.functionDecl.returnType = returnType;
    node->data.functionDecl.body = body;
    return node;
}
// </createFunctionDeclNode>


// <parseProgram>
ASTNode* parseProgram(Lexer* lexer) {
    ASTNode* programNode = createASTNode(NODE_PROGRAM);
    if (!programNode) {
        fprintf(stderr, "{parser} [ERROR] Failed to allocate memory for program node\n");
        return NULL;
    }
    programNode->data.program.statements = malloc(sizeof(ASTNode*) * INITIAL_STATEMENT_CAPACITY);
    if (!programNode->data.program.statements) {
        fprintf(stderr, "{parser} [ERROR] Failed to allocate memory for statements\n");
        free(programNode);
        return NULL;
    }
    programNode->data.program.stmtCount = 0;
    programNode->data.program.stmtCapacity = INITIAL_STATEMENT_CAPACITY;

    printf("{parser} [DEBUG] Program node allocated\n");

    getNextToken(lexer); // Initialize the first token
    while (currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer);

        if (stmt) {
            addFunctionToProgram(programNode, stmt);
            printf("{parser} [DEBUG] Statement parsed and added to program node\n");
        } else {
            fprintf(stderr, "{parser} [ERROR] Failed to parse statement\n");
            freeAST(programNode);
            return NULL;
        }
    }

    printf("{parser} [DEBUG] Finished parsing program\n");
    return programNode;
}
// </parseProgram>

// <parser>
int parser(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "{parser} Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    ASTNode* program = parseProgram(&lexer);
    if (!program) {
        fprintf(stderr, "{parser} [ERROR] Failed to parse program\n");
        free(source);
        return 1;
    }

    printAST(program, 0);

    free(source);
    freeAST(program);
    return 0;
}
// </parser>
