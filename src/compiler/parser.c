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


char* tokenTypeToString(CryoTokenType type) {
    switch (type) {
        case TOKEN_INT_LITERAL:
            return "INT_LITERAL";
        case TOKEN_STRING_LITERAL:
            return "STRING_LITERAL";
        case TOKEN_BOOLEAN_LITERAL:
            return "BOOLEAN_LITERAL";
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_OP_PLUS:
            return "OP_PLUS";
        case TOKEN_OP_MINUS:
            return "OP_MINUS";
        case TOKEN_OP_STAR: 
            return "OP_STAR";
        case TOKEN_OP_SLASH:
            return "OP_SLASH";
        case TOKEN_KW_CONST:
            return "KW_CONST";
        case TOKEN_KW_MUT:
            return "KW_MUT";
        case TOKEN_COLON:
            return "COLON";
        case TOKEN_ASSIGN:
            return "ASSIGN";
        case TOKEN_SEMICOLON:
            return "SEMICOLON";
        case TOKEN_EOF:
            return "EOF";
        default:
            return "UNKNOWN";
    }
}


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


// Helper function to map type strings to CryoDataType
CryoDataType getCryoDataType(const char* typeStr) {
    if (strcmp(typeStr, "int") == 0) {
        return DATA_TYPE_INT;
    } else if (strcmp(typeStr, "float") == 0) {
        return DATA_TYPE_FLOAT;
    } else if (strcmp(typeStr, "string") == 0) {
        return DATA_TYPE_STRING;
    } else if (strcmp(typeStr, "bool") == 0) {
        return DATA_TYPE_BOOLEAN;
    } else if (strcmp(typeStr, "void") == 0) {
        return DATA_TYPE_VOID;
    } else if (strcmp(typeStr, "null") == 0) {
        return DATA_TYPE_NULL;
    } else {
        return DATA_TYPE_UNKNOWN;
    }
}


// Function to parse a primary expression
// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer* lexer) {
    switch (currentToken.type) {
        case TOKEN_INT_LITERAL: {
            char buffer[32];
            strncpy(buffer, currentToken.start, currentToken.length);
            buffer[currentToken.length] = '\0';
            int value = atoi(buffer);
            printf("[Parser Debug] Token: %.*s, Parsed Value: %d\n", currentToken.length, currentToken.start, value);
            getNextToken(lexer);
            ASTNode* literalNode = createLiteralExpr(value);
            printf("[Parser] Created IntLiteral Node: %d\n", value);
            return literalNode;
        }
        case TOKEN_STRING_LITERAL: {
            char* value = strndup(currentToken.start + 1, currentToken.length - 2);
            getNextToken(lexer);
            ASTNode* stringNode = createStringLiteralExpr(value);
            printf("[Parser] Created String Node: %s\n", value);
            return stringNode;
        }
        case TOKEN_BOOLEAN_LITERAL: {
            int value = (strncmp(currentToken.start, "true", currentToken.length) == 0) ? 1 : 0;
            getNextToken(lexer);
            ASTNode* booleanNode = createBooleanLiteralExpr(value);
            printf("[Parser] Created Boolean Node: %d\n", value);
            return booleanNode;
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


// <parseExpression>
ASTNode* parseExpression(Lexer* lexer) {
    printf("[Parser] Parsing expression...\n");
    ASTNode* left = parsePrimaryExpression(lexer);

    while (currentToken.type == TOKEN_OP_PLUS || 
           currentToken.type == TOKEN_OP_MINUS || 
           currentToken.type == TOKEN_OP_STAR || 
           currentToken.type == TOKEN_OP_SLASH) {
        CryoTokenType operatorType = currentToken.type;
        printf("[Parser] Found operator: %s\n", tokenTypeToString(operatorType));
        getNextToken(lexer);
        ASTNode* right = parsePrimaryExpression(lexer);
        left = createBinaryExpr(left, right, operatorType);
    }

    return left;
}
// </parseExpression>


// Function to parse a variable declaration
// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer) {
    printf("[Parser] Entering parseVarDeclaration\n");
    getNextToken(lexer);

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("[Parser] Expected variable name");
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    CryoDataType dataType = DATA_TYPE_UNKNOWN;

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer);
        if (currentToken.type != TOKEN_IDENTIFIER) {
            error("[Parser] Expected type name");
        }
        char* varType = strndup(currentToken.start, currentToken.length);
        dataType = getCryoDataType(varType);
        free(varType);
        getNextToken(lexer);
    } else {
        error("[Parser] Expected ':' after variable name");
    }

    if (currentToken.type != TOKEN_ASSIGN) {
        error("[Parser] Expected '=' after variable name");
    }
    getNextToken(lexer);

    ASTNode* initializer = parseExpression(lexer);
    if (initializer == NULL) {
        error("[Parser] Expected expression after '='");
    } 

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    ASTNode* varDeclNode = createVarDeclarationNode(varName, dataType, initializer, currentToken.line);
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
            error("Expected statement");
            return NULL;
    }
}
// </parseStatement>


// <createProgramNode>
ASTNode* createProgramNode() {
    ASTNode* node = createASTNode(NODE_PROGRAM);
    if (!node) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program node\n");
        return NULL;
    }
    node->data.program.stmtCount = 0;
    node->data.program.stmtCapacity = 2;
    node->data.program.statements = (ASTNode**)calloc(node->data.program.stmtCapacity, sizeof(ASTNode*));
    if (!node->data.program.statements) {
        fprintf(stderr, "[AST] [ERROR] Failed to allocate memory for program statements\n");
        free(node);
        return NULL;
    }
    return node;
}
// </createProgramNode>


// Function to add a statement to the program node
// <addStatementToProgram>
void addStatementToProgram(ASTNode* programNode, ASTNode* statement) {
    if (!programNode || programNode->type != NODE_PROGRAM) {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    printf("[AST_DEBUG] Before adding statement: stmtCount = %d, stmtCapacity = %d\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
    if (programNode->data.program.stmtCount >= programNode->data.program.stmtCapacity) {
        int newCapacity = programNode->data.program.stmtCapacity == 0 ? 2 : programNode->data.program.stmtCapacity * 2;
        printf("[AST_DEBUG] Increasing stmtCapacity to: %d\n", newCapacity);
        programNode->data.program.statements = realloc(programNode->data.program.statements, newCapacity * sizeof(ASTNode*));
        if (!programNode->data.program.statements) {
            fprintf(stderr, "[AST_ERROR] Failed to reallocate memory for program statements\n");
            return;
        }
        programNode->data.program.stmtCapacity = newCapacity;
    }

    programNode->data.program.statements[programNode->data.program.stmtCount++] = statement;
    printf("[AST_DEBUG] After adding statement: stmtCount = %d, stmtCapacity = %d\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
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
