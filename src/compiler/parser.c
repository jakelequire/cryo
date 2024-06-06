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

#include "compiler/parser.h"


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
    fprintf(stderr, "[Parser] Error: %s at line %d, column %d\n", message, currentToken.line, currentToken.column);
    exit(1); // Exit on error to prevent further processing
}
// </error>


// <getNextToken>
void getNextToken(Lexer *lexer) {
    printf("[Parser] Getting next token...\n");
        if (isAtEnd(lexer)) {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        return;
    }

    currentToken = get_next_token(lexer);
    printf("[Parser] Token: %.*s, Type: %d, Line: %d, Column: %d\n",
           currentToken.length, currentToken.start, currentToken.type, currentToken.line, currentToken.column);
}
// </getNextToken>


// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char *message) {
    printf("[Parser] Consuming token: expected: %d, actual: %d\n", type, currentToken.type);
    if (currentToken.type == type) {
        getNextToken(lexer);
        printf("[Parser] Token consumed. New current token: %d\n", currentToken.type);
    } else {
        error(message);
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
        case TOKEN_INT_LITERAL: {
            int value = atoi(currentToken.start);
            getNextToken(lexer);  // Consume the literal
            return createLiteralExpr(value);
        }
        case TOKEN_IDENTIFIER: {
            char* name = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);  // Consume the identifier
            if (currentToken.type == TOKEN_LPAREN) {
                getNextToken(lexer);  // Consume '('
                // Parse parameters if any (not implemented here)
                consume(lexer, TOKEN_RPAREN, "Expected ')' after function call");
                return createFunctionCallNode(name, NULL, 0);
            }
            return createVariableExpr(name);
        }
        case TOKEN_LPAREN: {
            getNextToken(lexer);  // Consume '('
            ASTNode* expr = parseExpression(lexer);
            consume(lexer, TOKEN_RPAREN, "Expected ')' after expression");
            return expr;
        }
        default:
            error("Expected an expression");
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

    while (true) {
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
    ASTNode* left = parsePrimaryExpression(lexer);
    while (true) {
        switch (currentToken.type) {
            case TOKEN_OP_PLUS:
            case TOKEN_OP_MINUS:
            case TOKEN_OP_STAR:
            case TOKEN_OP_SLASH: {
                CryoTokenType operatorType = currentToken.type;
                getNextToken(lexer);  // Consume the operator
                ASTNode* right = parsePrimaryExpression(lexer);
                
                // Debug logging
                printf("Creating binary expression: left=%p, right=%p, operator=%d\n", left, right, operatorType);
                
                left = createBinaryExpr(left, right, operatorType);
                
                // Debug logging
                printf("Created binary expression: result=%p\n", left);
                
                break;
            }
            case TOKEN_SEMICOLON:
                getNextToken(lexer);  // Consume the semicolon
                return left;
            case TOKEN_EOF:
                return left;
            default:
                return left;
        }
    }
}
// </parseExpression>


// <parseStatement>
ASTNode* parseStatement(Lexer* lexer) {
    if (currentToken.type == TOKEN_INT_LITERAL || currentToken.type == TOKEN_IDENTIFIER) {
        ASTNode* expr = parseExpression(lexer);
        if (currentToken.type == TOKEN_SEMICOLON) {
            getNextToken(lexer);  // Consume the semicolon
            return expr;
        } else if (currentToken.type == TOKEN_EOF) {
            return expr;
        } else {
            error("Expected ';' after expression");
            return NULL;
        }
    }
    printf("[Parser] Parsing statement...\n");
    switch (currentToken.type) {
        case TOKEN_KW_PUBLIC:
        case TOKEN_KW_FN:
            printf("[Parser] Parsing function declaration...\n");
            return parseFunctionDeclaration(lexer);
        case TOKEN_KW_RETURN:
            printf("[Parser] Parsing return statement...\n");
            return parseReturnStatement(lexer);
        case TOKEN_KW_IF:
            printf("[Parser] Parsing if statement...\n");
            return parseIfStatement(lexer);
        case TOKEN_KW_WHILE:
            printf("[Parser] Parsing while statement...\n");
            return parseWhileStatement(lexer);
        case TOKEN_KW_FOR:
            printf("[Parser] Parsing for statement...\n");
            return parseForStatement(lexer);
        case TOKEN_KW_CONST:
        case TOKEN_KW_LET:
            printf("[Parser] Parsing variable declaration...\n");
            return parseVarDeclaration(lexer);
        case TOKEN_LBRACE:
            printf("[Parser] Parsing block...\n");
            return parseBlock(lexer);
        default:
            printf("[Parser] Parsing expression statement...\n");
            return parseExpressionStatement(lexer);
    }
    error("Expected a statement!!");
    return NULL;
}
// </parseStatement>


// <parseIfStatement>
ASTNode* parseIfStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'if'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("[Parser] Expected '{' after if condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* thenBranch = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("[Parser] Expected '}' after if body");
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
                error("[Parser] Expected '}' after else body");
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
    printf("[Parser] Parsing return statement...\n");
    ASTNode* returnNode = createASTNode(NODE_RETURN_STATEMENT);

    // Consume the 'return' keyword
    getNextToken(lexer);

    // Parse the return value if it exists
    if (currentToken.type != TOKEN_SEMICOLON) {
        returnNode->data.returnStmt.returnValue = parseExpression(lexer);
    } else {
        returnNode->data.returnStmt.returnValue = NULL;
    }

    printf("[Parser] Before consuming semicolon after return value: Current Token: %d\n", currentToken.type);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after return value");
    printf("[Parser] Semicolon consumed after return value. Current Token: %d\n", currentToken.type);
    return returnNode;
}
// </parseReturnStatement>


// <parseExpressionStatement>
ASTNode* parseExpressionStatement(Lexer* lexer) {
    printf("[Parser] Parsing expression statement...\n");
    ASTNode* expr = parseExpression(lexer);
    VERBOSE_CRYO_ASTNODE_LOG("parser", expr);
    printf("[Parser] Before consuming semicolon: Current Token: %d\n", currentToken.type);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after expression");
    printf("[Parser] Semicolon consumed after expression statement. Current Token: %d\n", currentToken.type);
    return createExpressionStatement(expr);
}
// </parseExpressionStatement>


// <parseWhileStatement>
ASTNode* parseWhileStatement(Lexer* lexer) {
    getNextToken(lexer);  // Consume 'while'
    ASTNode* condition = parseExpression(lexer);
    if (currentToken.type != TOKEN_LBRACE) {
        error("[Parser] Expected '{' after while condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("[Parser] Expected '}' after while body");
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
        error("[Parser] Expected '{' after for condition");
        return NULL;
    }
    getNextToken(lexer);  // Consume '{'
    ASTNode* body = parseStatement(lexer);
    if (currentToken.type != TOKEN_RBRACE) {
        error("[Parser] Expected '}' after for body");
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
        error("[Parser] Expected variable name");
        return NULL;
    }
    char* varName = _strdup(currentToken.value.stringValue);
    getNextToken(lexer);  // Consume variable name
    if (currentToken.type != TOKEN_ASSIGN) {
        error("[Parser] Expected '=' after variable name");
        return NULL;
    }
    getNextToken(lexer);  // Consume '='
    ASTNode* initializer = parseExpression(lexer);
    return createVarDeclarationNode(varName, initializer);
}
// </parseVarDeclaration>





// <parseStatements>
ASTNode* parseStatements(Lexer* lexer) {
    // Implementation for parsing multiple statements inside a block
    ASTNode* firstStmt = parseStatement(lexer);
    ASTNode* currentStmt = firstStmt;

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* nextStmt = parseStatement(lexer);
        currentStmt->nextSibling = nextStmt;
        currentStmt = nextStmt;
    }

    return firstStmt;
}
// </parseStatements>


// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer* lexer) {
    ASTNode* node = createASTNode(NODE_FUNCTION_DECLARATION);
    if (!node) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for function declaration node\n");
        return NULL;
    }

    if (currentToken.type == TOKEN_KW_PUBLIC || currentToken.type == TOKEN_KW_PRIVATE) {
        node->data.functionDecl.visibility = strdup(currentToken.start);
        getNextToken(lexer); // Consume visibility keyword
    }

    if (currentToken.type != TOKEN_KW_FN) {
        error("Expected 'fn' keyword");
    }
    getNextToken(lexer); // Consume 'fn' keyword

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
    }
    node->data.functionDecl.name = strdup(currentToken.start);
    getNextToken(lexer); // Consume function name

    if (currentToken.type != TOKEN_LPAREN) {
        error("Expected '(' after function name");
    }
    getNextToken(lexer); // Consume '('

    if (currentToken.type != TOKEN_RPAREN) {
        node->data.functionDecl.params = parseParameterList(lexer);
    }
    if (currentToken.type != TOKEN_RPAREN) {
        error("Expected ')' after parameter list");
    }
    getNextToken(lexer); // Consume ')'

    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer); // Consume '->'
        node->data.functionDecl.returnType = parseType(lexer);
    }

    node->data.functionDecl.body = parseBlock(lexer);

    return node;
}
// </parseFunctionDeclaration>


// <parseParameterList>
ASTNode* parseParameterList(Lexer* lexer) {
    ASTNode* paramListNode = createASTNode(NODE_PARAM_LIST);
    if (!paramListNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for parameter list node\n");
        return NULL;
    }

    paramListNode->data.paramList.params = malloc(sizeof(ASTNode*) * INITIAL_PARAM_CAPACITY);
    paramListNode->data.paramList.paramCount = 0;
    paramListNode->data.paramList.paramCapacity = INITIAL_PARAM_CAPACITY;

    while (currentToken.type != TOKEN_RPAREN && currentToken.type != TOKEN_EOF) {
        ASTNode* paramNode = parseParameter(lexer);
        if (paramNode) {
            if (paramListNode->data.paramList.paramCount >= paramListNode->data.paramList.paramCapacity) {
                paramListNode->data.paramList.paramCapacity *= 2;
                paramListNode->data.paramList.params = realloc(paramListNode->data.paramList.params, sizeof(ASTNode*) * paramListNode->data.paramList.paramCapacity);
            }
            paramListNode->data.paramList.params[paramListNode->data.paramList.paramCount++] = paramNode;
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse parameter\n");
            freeAST(paramListNode);
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer); // Consume ','
        }
    }

    return paramListNode;
}

// </parseParameterList>


// <parseParameter>
ASTNode* parseParameter(Lexer* lexer) {
    ASTNode* paramNode = createASTNode(NODE_VAR_DECLARATION);
    if (!paramNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for parameter node\n");
        return NULL;
    }

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected parameter name");
    }
    paramNode->data.varDecl.name = strdup(currentToken.start);
    getNextToken(lexer); // Consume parameter name

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer); // Consume ':'
        paramNode->data.varDecl.type = parseType(lexer);
    }

    return paramNode;
}
// </parseParameter>


// <createTypeNode>
ASTNode* createTypeNode(CryoTokenType type) {
    ASTNode* node = createASTNode(NODE_TYPE);
    node->data.value = type;
    return node;
}
// </createTypeNode>


// <parseType>
ASTNode* parseType(Lexer* lexer) {
    ASTNode* typeNode = createASTNode(NODE_TYPE);
    if (!typeNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for type node\n");
        return NULL;
    }

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected type name");
    }
    typeNode->data.varName.varName = strdup(currentToken.start);
    getNextToken(lexer); // Consume type name

    return typeNode;
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

// <parseBlock>

// <parseBlock>
ASTNode* parseBlock(Lexer* lexer) {
    ASTNode* blockNode = createASTNode(NODE_BLOCK);
    if (!blockNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for block node\n");
        return NULL;
    }

    blockNode->data.block.statements = malloc(sizeof(ASTNode*) * INITIAL_STATEMENT_CAPACITY);
    blockNode->data.block.stmtCount = 0;
    blockNode->data.block.stmtCapacity = INITIAL_STATEMENT_CAPACITY;

    if (currentToken.type != TOKEN_LBRACE) {
        error("[Parser] Expected '{' at the beginning of block");
    }
    getNextToken(lexer); // Consume '{'

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer);
        if (stmt) {
            if (blockNode->data.block.stmtCount >= blockNode->data.block.stmtCapacity) {
                blockNode->data.block.stmtCapacity *= 2;
                blockNode->data.block.statements = realloc(blockNode->data.block.statements, sizeof(ASTNode*) * blockNode->data.block.stmtCapacity);
            }
            blockNode->data.block.stmtCount++;
            printf("[Parser] Block statement parsed\n");
        } else {
            printf("[Parser] [ERROR] Failed to parse block statement\n");
            freeAST(blockNode);
            return NULL;
        }
    }

    if (currentToken.type != TOKEN_RBRACE) {
        error("[Parser] Expected '}' at the end of block");
        freeAST(blockNode);
        return NULL;
    }

    getNextToken(lexer); // Consume '}'
    printf("[Parser] [DEBUG] Finished parsing block\n");
    return blockNode;
}
// </parseBlock>


// <addStatementToProgram>
void addStatementToProgram(ASTNode* program, ASTNode* statement) {
    // Ensure program node is valid
    if (program->type != NODE_PROGRAM) {
        fprintf(stderr, "Invalid program node.\n");
        return;
    }

    // Reallocate memory if necessary
    if (program->data.program.stmtCount >= program->data.program.stmtCapacity) {
        program->data.program.stmtCapacity *= 2;
        program->data.program.statements = realloc(program->data.program.statements, sizeof(ASTNode*) * program->data.program.stmtCapacity);
    }

    // Add statement to the program
    program->data.program.statements[program->data.program.stmtCount++] = statement;
}
// </addStatementToProgram>


// <parseProgram>
ASTNode* parseProgram(Lexer* lexer) {
    ASTNode* programNode = createASTNode(NODE_PROGRAM);
    if (!programNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for program node\n");
        return NULL;
    }
    programNode->data.program.statements = malloc(sizeof(ASTNode*) * INITIAL_STATEMENT_CAPACITY);
    if (!programNode->data.program.statements) {
        fprintf(stderr, "[Parser] [ERROR] Failed to allocate memory for statements\n");
        free(programNode);
        return NULL;
    }
    programNode->data.program.stmtCount = 0;
    programNode->data.program.stmtCapacity = INITIAL_STATEMENT_CAPACITY;

    printf("[Parser] [DEBUG] Program node allocated\n");

    getNextToken(lexer); // Initialize the first token
    while (currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer);

        if (stmt) {
            addStatementToProgram(programNode, stmt);
            printf("[Parser] [DEBUG] Statement parsed and added to program node\n");
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(programNode);
            return NULL;
        }
    }

    printf("[Parser] [DEBUG] Finished parsing program\n");
    return programNode;
}
// </parseProgram>

// <parser>
int parser(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Parser] Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    ASTNode* program = parseProgram(&lexer);
    if (!program) {
        fprintf(stderr, "[Parser] [ERROR] Failed to parse program\n");
        free(source);
        return 1;
    }

    printAST(program, 0);

    free(source);
    freeAST(program);
    return 0;
}
// </parser>
