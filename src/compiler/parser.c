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
#include "compiler/token.h"

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


void debugCurrentToken() {
    printf("[Parser DEBUG] Current Token: %s, Lexeme: %.*s\n",
           CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
}

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
    printf("[Parser] Consuming token: %s (Expecting: %s)\n",
            CryoTokenToString(currentToken.type), CryoTokenToString(type));
    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        error(message);
    }
}
// </consume>


// Helper function to map type strings to CryoDataType
// <getCryoDataType>
CryoDataType getCryoDataType(const char* typeStr) {
    printf("\n[Parser DEBUG] Getting CryoDataType for: %s\n", typeStr);  // Add this line to debug
    if (strcmp(typeStr, "int") == 0) {
        return DATA_TYPE_INT;
    } else if (strcmp(typeStr, "float") == 0) {
        return DATA_TYPE_FLOAT;
    } else if (strcmp(typeStr, "string") == 0) {
        return DATA_TYPE_STRING;
    } else if (strcmp(typeStr, "boolean") == 0) {
        return DATA_TYPE_BOOLEAN;
    } else if (strcmp(typeStr, "void") == 0) {
        return DATA_TYPE_VOID;
    } else if (strcmp(typeStr, "int[]") == 0) {
        return DATA_TYPE_INT_ARRAY;
    } else if (strcmp(typeStr, "float[]") == 0) {
        return DATA_TYPE_FLOAT_ARRAY;
    } else if (strcmp(typeStr, "string[]") == 0) {
        return DATA_TYPE_STRING_ARRAY;
    } else if (strcmp(typeStr, "boolean[]") == 0) {
        return DATA_TYPE_BOOLEAN_ARRAY;
    } else {
        return DATA_TYPE_UNKNOWN;
    }
}

// </getCryoDataType>


// Function to get the precedence of an operator
// <parsePublicDeclaration>
ASTNode* parsePublicDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing public declaration...\n");
    getNextToken(lexer); // Consume 'public'

    if (currentToken.type == TOKEN_KW_FN) {
        return parseFunctionDeclaration(lexer, VISIBILITY_PUBLIC, context);
    } else if (currentToken.type == TOKEN_KW_CONST || currentToken.type == TOKEN_KW_MUT) {
        return parseVarDeclaration(lexer, context);
    } else {
        error("Expected function or variable declaration after 'public'");
        return NULL;
    }
}
// </parsePublicDeclaration>


// Function to parse a primary expression
// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer* lexer, ParsingContext* context) {
    switch (currentToken.type) {
        case TOKEN_INT_LITERAL: {
            char buffer[32];
            strncpy(buffer, currentToken.start, currentToken.length);
            buffer[currentToken.length] = '\0';
            int value = atoi(buffer);
            getNextToken(lexer);
            return createLiteralExpr(value);
        }
        case TOKEN_STRING_LITERAL: {
            char* value = strndup(currentToken.start + 1, currentToken.length - 2);
            getNextToken(lexer);
            return createStringLiteralExpr(value);
        }
        case TOKEN_BOOLEAN_LITERAL: {
            int value = (strncmp(currentToken.start, "true", currentToken.length) == 0) ? 1 : 0;
            getNextToken(lexer);
            return createBooleanLiteralExpr(value);
        }
        case TOKEN_IDENTIFIER: {
            char* identifier = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);

            // Check if the next token is a left parenthesis and context is not parsing an if condition

            if (currentToken.type == TOKEN_LPAREN && context->isParsingIfCondition) {
                printf("\n\n\n[!!Parser!!] Function calls are not allowed in if conditions\n\n\n"); // Debugging
            }
            else if (currentToken.type == TOKEN_LPAREN && !context->isParsingIfCondition) {
                return parseFunctionCall(lexer, identifier, context);
            } 
            else {
                return createVariableExpr(identifier);
            }
        }
        case NODE_RETURN_STATEMENT: {
            return parseReturnStatement(lexer, context);
        }

        case TOKEN_ITER_VAR: {
            char* identifier = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);
            return createVariableExpr(identifier);
        }

        default:
            error("Expected an expression");
            return NULL;
    }
}
// </parsePrimaryExpression>


// Entry point for parsing expressions
ASTNode* parseExpression(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing expression...\n");
    ASTNode* left = parsePrimaryExpression(lexer, context);

    while (currentToken.type == OPERATOR_ADD || currentToken.type == OPERATOR_SUB || 
           currentToken.type == OPERATOR_MUL || currentToken.type == OPERATOR_DIV || 
           currentToken.type == OPERATOR_LT || currentToken.type == OPERATOR_GT || 
           currentToken.type == OPERATOR_LTE || currentToken.type == OPERATOR_GTE || 
           currentToken.type == OPERATOR_EQ || currentToken.type == OPERATOR_NEQ) {
        CryoTokenType operatorType = currentToken.type;
        printf("[Parser] Found operator: %s\n", CryoTokenToString(operatorType));
        getNextToken(lexer);
        ASTNode* right = parsePrimaryExpression(lexer, context);
        left = createBinaryExpr(left, right, operatorType);
    }

    return left;
}



// Function to parse a variable declaration
// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Entering parseVarDeclaration\n");
    getNextToken(lexer);
    printf("[Parser] Current token: %s\n", CryoTokenToString(currentToken.type));

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("[Parser] Expected variable name");
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer); // Consume the identifier
    printf("[Parser] Current token: %s\n", CryoTokenToString(currentToken.type));

    CryoDataType dataType = DATA_TYPE_UNKNOWN;

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer);
        printf("[Parser] Current token after colon: %s\n", CryoTokenToString(currentToken.type));
        if (currentToken.type == TOKEN_TYPE_INT_ARRAY ||
            currentToken.type == TOKEN_TYPE_FLOAT_ARRAY || 
            currentToken.type == TOKEN_TYPE_STRING_ARRAY ||
            currentToken.type == TOKEN_TYPE_BOOLEAN_ARRAY ||
            currentToken.type == TOKEN_KW_INT ||
            currentToken.type == TOKEN_KW_STRING ||
            currentToken.type == TOKEN_KW_BOOL ||
            currentToken.type == TOKEN_KW_VOID) {
            char* varType = strndup(currentToken.start, currentToken.length);
            printf("[Parser] Identified variable type: %s\n", varType);
            dataType = getCryoDataType(varType);
            free(varType);
            getNextToken(lexer);
            printf("[Parser] Current token after type: %s\n", CryoTokenToString(currentToken.type));
        } else {
            error("[Parser] Expected type name");
        }
    } else {
        error("[Parser] Expected ':' after variable name");
    }

    if (currentToken.type != TOKEN_ASSIGN) {
        error("[Parser] Expected '=' after variable name");
    }
    getNextToken(lexer);
    printf("[Parser] Current token after '=': %s\n", CryoTokenToString(currentToken.type));

    ASTNode* initializer;
    if (currentToken.type == TOKEN_LBRACKET) {
        initializer = parseArrayLiteral(lexer, context);
    } else {
        initializer = parseExpression(lexer, context);
    }

    if (initializer == NULL) {
        error("[Parser] Expected expression after '='");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    bool isGlobal = (context->scopeLevel == 0);
    ASTNode* varDeclNode = createVarDeclarationNode(varName, dataType, initializer, currentToken.line, isGlobal);
    printf("[Parser] Created Variable Declaration Node: %s\n", varName);
    printf("[Parser] Variable Declaration Node Type: %d\n", varDeclNode->type);
    return varDeclNode;
}
// </parseVarDeclaration>



/*
        case NODE_EXTERN_STATEMENT:
            node->data.externNode.type = NODE_UNKNOWN;
            node->data.externNode.decl.function->body = NULL;
            node->data.externNode.decl.function->name = NULL;
            node->data.externNode.decl.function->params = NULL;
            node->data.externNode.decl.function->paramCount = 0;
typedef struct {
    char* name;
    ASTNode** params;
    int paramCount;
    struct ASTNode* body;
    CryoDataType returnType;
} FunctionDeclNode;

typedef struct ASTNode {
    enum CryoNodeType type;
    int line;  // Line number for error reporting
    struct ASTNode* firstChild;  // First child node (for linked list structure)
    struct ASTNode* nextSibling; // Next sibling node (for linked list structure)

    union {
        int value;  // For literal number nodes

        struct {
            char* modulePath;
        } importStatementNode;

        struct externNode {
            CryoNodeType type;
            union decl {
                FunctionDeclNode* function;
                // Will add more types here
            } decl;
        } externNode;

        // ...
*/
// <parseExternFunctionDeclaration>
ASTNode* parseExternFunctionDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Entering parseExternFunctionDeclaration\n");
    consume(lexer, TOKEN_KW_FN, "Expected 'function' keyword");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
    }

    char* functionName = strndup(currentToken.start, currentToken.length);
    printf("[Parser] Function name: %s\n", functionName);
    getNextToken(lexer);

    ASTNode* params = parseParameterList(lexer, context);
    if (params && params->data.paramList.paramCapacity == 0) {
        params = NULL;
    }
    printf("[Parser] Parameters parsed\n");

    CryoDataType returnType = DATA_TYPE_VOID;
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer);
        if (currentToken.type != TOKEN_KW_VOID && currentToken.type != TOKEN_KW_INT && currentToken.type != TOKEN_KW_FLOAT && currentToken.type != TOKEN_KW_STRING && currentToken.type != TOKEN_KW_BOOL) {
            error("Expected return type after '->'");
        }
        char* returnTypeStr = strndup(currentToken.start, currentToken.length);
        returnType = getCryoDataType(returnTypeStr);
        printf("[Parser] Return type: %s\n", returnTypeStr);
        free(returnTypeStr);
        getNextToken(lexer);
    }

    ASTNode* externNode = createExternNode();
    if (!externNode) {
        error("Failed to create extern node");
    }

    // Initialize the function declaration node within the extern node
    externNode->data.externNode.decl.function->name = functionName;
    externNode->data.externNode.decl.function->params = params ? params->data.paramList.params : NULL;
    externNode->data.externNode.decl.function->paramCount = params ? params->data.paramList.paramCount : 0;
    externNode->data.externNode.decl.function->returnType = returnType;
    
    printf("[Parser] Extern node created\n");
    return externNode;
}
// </parseExternFunctionDeclaration>



// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer* lexer, CryoVisibilityType visibility, ParsingContext* context) {
    printf("[Parser] Parsing function declaration...\n");
    consume(lexer, TOKEN_KW_FN, "Expected 'function' keyword");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected function name");
    }
    char* functionName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    ASTNode* params = parseParameterList(lexer, context);
    if (params->data.paramList.paramCapacity == 0) {
        params = NULL;  // Set to NULL if no parameters are present
    }

    CryoDataType returnType = DATA_TYPE_VOID;  // Default to void
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer);
        if (currentToken.type != TOKEN_KW_VOID && currentToken.type != TOKEN_KW_INT &&
            currentToken.type != TOKEN_KW_FLOAT && currentToken.type != TOKEN_KW_STRING &&
            currentToken.type != TOKEN_KW_BOOL) {
            error("Expected return type after '->'");
        }
        char* returnTypeStr = strndup(currentToken.start, currentToken.length);
        returnType = getCryoDataType(returnTypeStr);
        printf("\n\n[Parser] Return type identified: %s, Enum: %d\n", returnTypeStr, returnType);  // Add this line to debug
        free(returnTypeStr);
        getNextToken(lexer);
    }

    ASTNode* body = parseFunctionBlock(lexer, context);
    ASTNode* functionNode = createFunctionNode(functionName, params, body, returnType);
    functionNode->data.functionDecl.visibility = visibility;
    functionNode->data.functionDecl.returnType = returnType;

    printf("[Parser] Created Function Declaration Node: %s with return type: %d\n", functionName, returnType);
    return functionNode;
}
// </parseFunctionDeclaration>


// <parseParameterList>
ASTNode* parseParameterList(Lexer* lexer, ParsingContext* context) {
    ASTNode* paramList = createASTNode(NODE_PARAM_LIST);
    paramList->data.paramList.paramCount = 0;

    getNextToken(lexer);  // Consume '('



    while (currentToken.type != TOKEN_RPAREN && currentToken.type != TOKEN_EOF) {
        if (currentToken.type == TOKEN_IDENTIFIER) {
            char* paramName = strndup(currentToken.start, currentToken.length);
            getNextToken(lexer);
            consume(lexer, TOKEN_COLON, "Expected ':' after parameter name");

            char* paramType = strndup(currentToken.start, currentToken.length);
            CryoDataType dataType = getCryoDataType(paramType);
            free(paramType);
            getNextToken(lexer);

            ASTNode* paramNode = createVarDeclarationNode(paramName, dataType, NULL, currentToken.line, false);
            addChildNode(paramList, paramNode);  // Ensure addChildNode works for PARAM_LIST
            paramList->data.paramList.paramCount++;

            if (currentToken.type == TOKEN_COMMA) {
                getNextToken(lexer);
            } else if (currentToken.type != TOKEN_RPAREN) {
                error("Expected ',' or ')' after parameter");
            }
        } else {
            error("Expected parameter name");
        }
    }

    if (currentToken.type == TOKEN_RPAREN) {
        getNextToken(lexer);  // Consume the ')'
    } else {
        error("Expected ')' after parameters");
    }

    return paramList;
}
// </parseParameterList>


// <parseReturnStatement>
ASTNode* parseReturnStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing return statement...\n");
    ASTNode* returnNode = createASTNode(NODE_RETURN_STATEMENT);

    getNextToken(lexer); // Consume 'return'

    // Parse the return value if present
    if (currentToken.type != TOKEN_SEMICOLON) {
        ASTNode* returnValue = parseExpression(lexer, context);
        if (returnValue) {
            returnNode->data.returnStmt.returnValue = returnValue;
            returnNode->data.returnStmt.returnType = returnValue->type;
            printf("[Parser] Return value parsed with type: %d\n", returnValue->type);
        } else {
            printf("[Parser] Error: Failed to parse return value\n");
        }
    } else {
        // No return value, set return type to void
        returnNode->data.returnStmt.returnValue = NULL;
        returnNode->data.returnStmt.returnType = DATA_TYPE_VOID;
        printf("[Parser] No return value\n");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after return value");
    printf("[Parser] Created Return Statement Node with return value of type: %d\n", returnNode->data.returnStmt.returnValue ? returnNode->data.returnStmt.returnValue->type : -1);
    return returnNode;
}
// </parseReturnStatement>


// <parseFunctionCall>
ASTNode* parseFunctionCall(Lexer* lexer, const char* functionName, ParsingContext* context) {
    printf("[Parser] Parsing function call for function: %s\n", functionName);
    consume(lexer, TOKEN_LPAREN, "Expected '(' after function name");
    
    ASTNode** arguments = NULL;
    int argCount = 0;
    
    if (currentToken.type != TOKEN_RPAREN) {
        arguments = (ASTNode**)malloc(sizeof(ASTNode*) * MAX_ARGUMENTS);
        do {
            arguments[argCount++] = parseExpression(lexer, context);
            if (argCount >= MAX_ARGUMENTS) {
                error("Too many arguments in function call");
            }
        } while (currentToken.type == TOKEN_COMMA);
    }
    
    consume(lexer, TOKEN_RPAREN, "Expected ')' after function arguments");
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after function call");
    
    ASTNode* node = createASTNode(NODE_FUNCTION_CALL);
    node->data.functionCall.name = strdup(functionName);
    node->data.functionCall.args = arguments;
    node->data.functionCall.argCount = argCount;
    
    printf("[Parser] Created Function Call Node: %s with %d arguments\n", functionName, argCount);
    return node;
}
// </parseFunctionCall>


// <parseIfStatement>
ASTNode* parseIfStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing if statement...\n");
    context->isParsingIfCondition = true;
    consume(lexer, TOKEN_KW_IF, "Expected 'if'");
    consume(lexer, TOKEN_LPAREN, "Expected '(' after 'if'");

    printf("[Parser] Parsing condition...\n");
    ASTNode* condition = parseExpression(lexer, context);
    context->isParsingIfCondition = false;

    if (!condition) {
        error("Expected condition expression after '('");
    }
    printf("[Parser] Condition parsed.\n");

    consume(lexer, TOKEN_RPAREN, "Expected ')' after condition");

    printf("[Parser] Parsing then branch...\n");
    ASTNode* thenBranch = parseBlock(lexer, context);
    ASTNode* elseBranch = NULL;

    if (currentToken.type == TOKEN_KW_ELSE) {
        getNextToken(lexer);
        elseBranch = parseBlock(lexer, context);
    }

    return createIfStatement(condition, thenBranch, elseBranch);
}
// </parseIfStatement>


// Function to parse a for loop
// <parseForLoop>
ASTNode* parseForLoop(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing for loop...\n");
    consume(lexer, TOKEN_KW_FOR, "Expected 'for'");
    consume(lexer, TOKEN_LPAREN, "Expected '(' after 'for'");
    
    // Parse the iterator variable
    Token iter_var_token = currentToken;

    consume(lexer, TOKEN_ITER_VAR, "Expected iterator variable '$'");

    // create the iterator variable node
    ASTNode* iter_var_init = createLiteralExpr(0);
    consume(lexer, TOKEN_ITER_STEP, "Expected '..' after iterator variable");
    
    // Parse the end expression
    ASTNode* end_expr = parseExpression(lexer, context);
    
    consume(lexer, TOKEN_RPAREN, "Expected ')' after end expression");
    
    // Parse the loop body
    ASTNode* body = parseBlock(lexer, context);
    
    // Create the for loop node in the AST
    ASTNode* for_node = createForStatement(createVariableExpr(strndup(iter_var_token.start, iter_var_token.length)), end_expr, createLiteralExpr(1), body);
    for_node->data.forStmt.initializer = iter_var_init;  // Set the initializer to 0
    return for_node;
}
// </parseForLoop>


// <parseArrayLiteral>
ASTNode* parseArrayLiteral(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing array literal...\n");
    consume(lexer, TOKEN_LBRACKET, "Expected '[' to start array literal");

    ASTNode** elements = (ASTNode**)malloc(sizeof(ASTNode*) * INITIAL_CAPACITY);
    int elementCount = 0;
    int capacity = INITIAL_CAPACITY;

    while (currentToken.type != TOKEN_RBRACKET) {
        if (elementCount >= capacity) {
            capacity *= 2;
            elements = (ASTNode**)realloc(elements, sizeof(ASTNode*) * capacity);
        }

        ASTNode* element = parseExpression(lexer, context);
        if (!element) {
            error("Expected expression in array literal");
        }
        elements[elementCount++] = element;

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        } else if (currentToken.type != TOKEN_RBRACKET) {
            error("Expected ',' or ']' in array literal");
        }
    }

    consume(lexer, TOKEN_RBRACKET, "Expected ']' to end array literal");
    return createArrayLiteralNode(elements, elementCount);
}
// </parseArrayLiteral>


// <parseImport>
ASTNode* parseImport(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing import statement...\n");
    getNextToken(lexer); // Consume 'import'

    if (currentToken.type != TOKEN_STRING_LITERAL) {
        error("Expected string literal for import path");
    }

    char* modulePath = strndup(currentToken.start + 1, currentToken.length - 2); // Remove quotes
    getNextToken(lexer); // Consume string literal

    // Here you would load and process the module
    // Since we're disregarding symbol table management, we'll just print the modulePath for now
    printf("[Parser] Importing module: %s\n", modulePath);

    ASTNode* importNode = createImportNode(modulePath);

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after import statement");
    return importNode;
}
// </parseImport>


// <parseExtern>
ASTNode* parseExtern(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing extern declaration...\n");
    consume(lexer, TOKEN_KW_EXTERN, "Expected 'extern'");

    if (currentToken.type == TOKEN_KW_FN) {
        ASTNode* declNode = parseExternFunctionDeclaration(lexer, context);
        if (!declNode) {
            error("Failed to create AST node for extern function declaration");
        }

        consume(lexer, TOKEN_SEMICOLON, "Expected ';' after extern function declaration");
        return declNode;
    } else {
        error("Expected 'function' after 'extern'");
    }

    return NULL;
}
// </parseExtern>



// Function to parse a statement
// <parseStatement>
ASTNode* parseStatement(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing statement...\n");


    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            printf("[Parser] Parsing variable declaration.\n");
            return parseVarDeclaration(lexer, context);
        case TOKEN_KW_IF:
            return parseIfStatement(lexer, context);
        case TOKEN_KW_PUBLIC:
            return parsePublicDeclaration(lexer, context);
        case TOKEN_KW_PRIVATE:
            return parsePublicDeclaration(lexer, context);
        case TOKEN_KW_RETURN:
            return parseReturnStatement(lexer, context);
        case TOKEN_KW_FOR:
            return parseForLoop(lexer, context);
        case TOKEN_KW_IMPORT:
            return parseImport(lexer, context);
        case TOKEN_KW_EXTERN:
            return parseExtern(lexer, context);
        case TOKEN_IDENTIFIER:
            printf("[Parser] Identified Identifier: %.*s\n", currentToken.length, currentToken.start);
            return parsePrimaryExpression(lexer, context);
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


// <parseBlock>
ASTNode* parseBlock(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing block...\n");
    context->scopeLevel++;
    ASTNode* blockNode = createASTNode(NODE_BLOCK);
    consume(lexer, TOKEN_LBRACE, "Expected '{' to start block");

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, context);
        if (stmt) {
            addChildNode(blockNode, stmt);
        } else {
            error("Failed to parse statement in block");
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected '}' to end block");
    context->scopeLevel--;
    return blockNode;
}
// </parseBlock>


ASTNode* parseFunctionBlock(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing function block...\n");
    context->scopeLevel++;
    ASTNode* blockNode = createASTNode(NODE_FUNCTION_BLOCK);
    consume(lexer, TOKEN_LBRACE, "Expected '{' to start function block");

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, context);
        if (stmt) {
            addStatementToBlock(blockNode->data.functionBlock.block, stmt);
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected '}' to end function block");
    context->scopeLevel--;
    return blockNode;
}



// Function to parse the entire program
// <parseProgram>
ASTNode* parseProgram(Lexer* lexer) {
    printf("[Parser] Parsing program\n");

    ParsingContext context = {
        false,
        0
    }; // Initialize context

    ASTNode* programNode = createProgramNode();
    if (!programNode) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create program node\n");
        return NULL;
    }

    getNextToken(lexer);

    while (currentToken.type != TOKEN_EOF) {
        ASTNode* stmt = parseStatement(lexer, &context);
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
