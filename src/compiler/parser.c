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


// Scope-Declared Current Token
Token currentToken;


// Error handling function
// <error>
void error(const char* message) {
    fprintf(stderr, "[Parser] Error: %s at line %d, column %d\n", message, currentToken.line, currentToken.column);
    exit(1);
}
// </error>


// Function to get the next token from the lexer
// <getNextToken>
void getNextToken(Lexer *lexer) {
    if (isAtEnd(lexer)) {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        return;
    }
    currentToken = get_next_token(lexer);
}
// </getNextToken>


// Function to consume a token and check its type
// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char *message) {
    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        error(message);
    }
}
// </consume>


// Function to parse a primary expression
// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer* lexer) {
    switch (currentToken.type) {
        case TOKEN_INT_LITERAL: {
            int value = atoi(currentToken.start);
            getNextToken(lexer);
            ASTNode* literalNode = createLiteralExpr(value);
            printf("[Parser] Created Literal Node: %d\n", value);
            return literalNode;
        }
        case TOKEN_IDENTIFIER: {
            char* name = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);
            ASTNode* variableNode = createVariableExpr(name);
            printf("[Parser] Created Variable Node: %s\n", name);
            return variableNode;
        }
        default:
            error("Expected an expression");
            return NULL;
    }
}
// </parsePrimaryExpression>


// Function to parse a variable declaration
// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer) {
    printf("[Parser] Entering parseVarDeclaration\n");
    getNextToken(lexer); // Consume 'const' or 'var'

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("[Parser] Expected variable name");
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer); // Consume variable name

    if (currentToken.type != TOKEN_ASSIGN) {
        error("[Parser] Expected '=' after variable name");
    }
    getNextToken(lexer); // Consume '='

    ASTNode* initializer = parsePrimaryExpression(lexer);

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    // Create and return the variable declaration node
    ASTNode* varDeclNode = createVarDeclarationNode(varName, initializer, currentToken.line);
    printf("[Parser] Created Variable Declaration Node: %s\n", varName);
    printf("[Parser] Variable Declaration Node Type: %d\n", varDeclNode->type);
    return varDeclNode;
}
// </parseVarDeclaration>


// Function to parse a statement
// <parseStatement>
ASTNode* parseStatement(Lexer* lexer) {
    printf("[Parser] Parsing statement...\n");

    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            printf("[Parser] Parsing variable declaration.\n");
            return parseVarDeclaration(lexer);
        case TOKEN_EOF:
            printf("[Parser] Reached end of file.\n");
            return NULL;
        default:
            // Error since this is not a valid statement
            error("Expected statement");
            return NULL;
            break;
    }

}
// </parseStatement>

// <createProgramNode>
ASTNode* createProgramNode() {
    printf("[AST] Creating Program Node....\n");
    ASTNode* node = createASTNode(NODE_PROGRAM);
    if (!node) {
        printf("[AST] Failed to create program node\n");
        return NULL;
    }
    node->data.program.statements = NULL;
    node->data.program.stmtCount = 0;
    node->data.program.stmtCapacity = 0;

    printf("[AST] Program Node Created\n");
    return node;
}
// </createProgramNode>

// Function to add a statement to the program node
// <addStatementToProgram>
void addStatementToProgram(ASTNode* programNode, ASTNode* statement) {
    if (!programNode || programNode->type != NODE_PROGRAM) {
        fprintf(stderr, "[AST] Invalid program node\n");
        return;
    }

    if (programNode->data.program.stmtCount >= programNode->data.program.stmtCapacity) {
        int newCapacity = programNode->data.program.stmtCapacity == 0 ? 2 : programNode->data.program.stmtCapacity * 2;
        programNode->data.program.statements = realloc(programNode->data.program.statements, newCapacity * sizeof(ASTNode*));
        programNode->data.program.stmtCapacity = newCapacity;
    }

    programNode->data.program.statements[programNode->data.program.stmtCount++] = statement;
}
// </addStatementToProgram>


// Function to parse the entire program
// <parseProgram>
ASTNode* parseProgram(Lexer* lexer) {
    printf("[Parser] Parsing program\n");

    ASTNode* programNode = createProgramNode();
    if (!programNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create program node\n");
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

    getNextToken(lexer);

    while (currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer);
        if (stmt) {
            addStatementToProgram(programNode, stmt);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(programNode);
            return NULL;
        }
    }

    return programNode;
}
// </parseProgram>




// ---------------------------------------------------------------------------------------------------------------------
// DEBUGGING FUNCTIONS

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
