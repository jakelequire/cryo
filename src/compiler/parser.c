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


void printLine(const char* source, int line) {
    const char* lineStart = source;
    while (*lineStart && line > 1) {
        if (*lineStart == '\n') line--;
        lineStart++;
    }

    const char* lineEnd = lineStart;
    while (*lineEnd && *lineEnd != '\n') {
        lineEnd++;
    }

    printf("%.*s\n", (int)(lineEnd - lineStart), lineStart);
}


/* ====================================================================== */
// Scope-Declared Current Token
Token currentToken;
const char* source;



/* ====================================================================== */
/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */

// <parseProgram>
ASTNode* parseProgram(Lexer* lexer, CryoSymbolTable *table) {
    printf("\n[Parser] Parsing program...\n");

    source = lexer->start;

    ParsingContext context = {
        false,
        0
    };

    ASTNode* program = createProgramNode();
    if (!program) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create program node\n");
        return NULL;
    }

    getNextToken(lexer);

    printf("[Parser] Starting to parse statements...\n");

    while (currentToken.type != TOKEN_EOF) {
        ASTNode* statement = parseStatement(lexer, table, &context);
        if (statement) {
            traverseAST(statement, table);
            addStatementToProgram(program, table, statement);

        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(program);
            return NULL;
        }
    }

    return program;
}
// </parseProgram>



/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char* message, const char* functionName, CryoSymbolTable *table) {
    printf("\n<*> [Parser] Consuming token: %s (Expecting: %s) ",
        CryoTokenToString(currentToken.type),
        CryoTokenToString(type)
    );
    printf("@Fn <%s>\n\n", functionName);

    pushCallStack(&callStack, functionName, currentToken.line);

    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        error((char*)message, (char*)functionName, table);
    }

    debugCurrentToken();
}
// </consume>


// <getNextToken>
void getNextToken(Lexer *lexer) {
    printf("[Parser] @getNextToken | Current Token before: Type=%d, Start=%.*s, Length=%d\n", currentToken.type, currentToken.length, currentToken.start, currentToken.length);
    if (isAtEnd(lexer)) {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        currentToken.line = lexer->line;
        currentToken.column = lexer->column;
        return;
    }
    currentToken = get_next_token(lexer);
    printf("[Parser] @getNextToken | Next Token after: Type=%d, Start=%.*s, Length=%d\n", currentToken.type, currentToken.length, currentToken.start, currentToken.length);
}
// </getNextToken>

// <peekNextUnconsumedToken>
Token peekNextUnconsumedToken(Lexer *lexer) {
    if (isAtEnd(lexer)) {
        return currentToken;
    }
    return get_next_token(lexer);
}
// </peekNextUnconsumedToken>


// <error>
void error(char* message, char* functionName, CryoSymbolTable *table) {
    int line = currentToken.line;
    int column = currentToken.column;


    printSymbolTable(table);

    printf("\n\n");
    printStackTrace(&callStack);

    printf("\n<!> [Parser] Error: %s at line %d, column %d\n", message, line, column);
    printf("@Function: <%s>\n", functionName);
    printf("------------------------------------------------------------------------\n\n");
    // Print the line containing the error
    printf("%d ", line - 1);
    printLine(source, line - 1);
    printf("%d ", line);
    printLine(source, line);

    for (int i = line; i < column + line; i++) {
        printf(" ");
    }
    
    printf("^ %s\n", message);
    printf("\n------------------------------------------------------------------------\n\n");
    freeCallStack(&callStack);
    exit(1);
}
// </error>


// <debugCurrentToken>
void debugCurrentToken() {
    printf("[Parser DEBUG] Current Token: %s, Lexeme: %.*s\n",
            CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
}
// </debugCurrentToken>



/* ====================================================================== */
/* @DataType_Management                                                   */

// <getCryoDataType>
CryoDataType getCryoDataType(const char* typeStr) {
    printf("[Parser] Getting data type for input: %s\n", typeStr);
    CryoDataType type = CryoDataTypeStringToType((char*)typeStr);
    return type;
}
// </getCryoDataType>


// <parseType>
CryoDataType parseType(Lexer* lexer, ParsingContext* context, CryoSymbolTable *table) {
    printf("[Parser] Parsing type...\n");
    CryoDataType type = DATA_TYPE_UNKNOWN;

    switch (currentToken.type) {
        case TOKEN_KW_VOID:
        case TOKEN_KW_INT:
        case TOKEN_KW_STRING:
        case TOKEN_KW_BOOL:
            type = getCryoDataType(strndup(currentToken.start, currentToken.length));
            break;

        case TOKEN_IDENTIFIER:
            type = getCryoDataType(strndup(currentToken.start, currentToken.length));
            break;

        default:
            error("Expected a type identifier", "getNextToken", table);
            break;
    }
    return type;
}
// </parseType>


// <getOperatorPrecedence>
int getOperatorPrecedence(CryoTokenType type) {
    switch(type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 1;

        case TOKEN_STAR:
        case TOKEN_SLASH:
            return 2;

        case TOKEN_OP_EQ:
        case TOKEN_OP_NEQ:
            return 3;

        case TOKEN_OP_LT:
        case TOKEN_OP_LTE:
        case TOKEN_OP_GT:
        case TOKEN_OP_GTE:
            return 4;

        case TOKEN_OP_AND:
            return 5;

        case TOKEN_OP_OR:
            return 6;

        default:
            return 0;
    }
}
// </getOperatorPrecedence>


/* ====================================================================== */
/* @Parser_Management                                                     */

// <addStatementToProgram>
void addStatementToProgram(ASTNode* programNode, CryoSymbolTable *table, ASTNode* statement) {
    printf("[Parser] Adding statement to program...\n");
    if (!programNode || programNode->type != NODE_PROGRAM) {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    printf("[AST] Before adding statement: stmtCount = %zu, stmtCapacity = %zu\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
    if (programNode->data.program.stmtCount >= programNode->data.program.stmtCapacity) {
        programNode->data.program.stmtCapacity = (programNode->data.program.stmtCapacity > 0) ? (programNode->data.program.stmtCapacity * 2) : 1;
        programNode->data.program.statements = (ASTNode**)realloc(programNode->data.program.statements, programNode->data.program.stmtCapacity * sizeof(ASTNode*));
    }

    programNode->data.program.statements[programNode->data.program.stmtCount++] = statement;
    printf("[AST] After adding statement: stmtCount = %zu, stmtCapacity = %zu\n", programNode->data.program.stmtCount, programNode->data.program.stmtCapacity);
}
// </addStatementToProgram>



/* ====================================================================== */
/* @ASTNode_Parsing - Expressions & Statements                            */

// <parseStatement>
ASTNode* parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing statement...\n");

    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            return parseVarDeclaration(lexer, table, context);

        case TOKEN_KW_PUBLIC:
            return parsePublicDeclaration(lexer, table, context);

        case TOKEN_KW_RETURN:
            printf("[Parser] Parsing return statement\n");
            return parseReturnStatement(lexer, table, context);

        case TOKEN_KW_FOR:
            return parseForLoop(lexer, table, context);

        case TOKEN_KW_IMPORT:
            return parseImport(lexer, table, context);

        case TOKEN_KW_EXTERN:
            return parseExtern(lexer, table, context);

        case TOKEN_IDENTIFIER:
            if (currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer).type == TOKEN_LPAREN) {
                char* functionName = strndup(currentToken.start, currentToken.length);
                return parseFunctionCall(lexer, table, context, functionName);
            }
            return parseExpressionStatement(lexer, table, context);

        case TOKEN_EOF:
            return NULL;

        default:
            error("Expected a statement", "parseStatement", table);
            return NULL;
    }
}
// </parseStatement>


// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing primary expression...\n");

    ASTNode* node = NULL;

    switch(currentToken.type) {
        case TOKEN_INT_LITERAL:
            printf("[Parser] Parsing integer literal\n");
            node = createIntLiteralNode(atoi(currentToken.start));
            getNextToken(lexer);
            return node;

        case TOKEN_STRING_LITERAL:
            printf("[Parser] Parsing string literal\n");
            char* str = strndup(currentToken.start + 1, currentToken.length - 2);
            node = createStringLiteralNode(str);
            getNextToken(lexer);
            return node;

        case TOKEN_BOOLEAN_LITERAL:
            printf("[Parser] Parsing boolean literal\n");
            node = createBooleanLiteralNode(strcmp(currentToken.start, "true") == 0);
            getNextToken(lexer);
            return node;

        case TOKEN_IDENTIFIER:
            printf("[Parser] Parsing identifier\n");
            node = createIdentifierNode(strndup(currentToken.start, currentToken.length));
            getNextToken(lexer);
            return node;

        default:
            error("Expected an expression", "parsePrimaryExpression", table);
            return NULL;
    }
}
// </parsePrimaryExpression>


// <parseExpression>
ASTNode* parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing expression...\n");
    ASTNode* left = parsePrimaryExpression(lexer, table, context);
    if (!left) {
        error("Expected an expression.", "parseExpression", table);
    }
    return parseBinaryExpression(lexer, table, context, left, 1);
}
// </parseExpression>


// <parseExpressionStatement>
ASTNode* parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing expression statement...\n");
    ASTNode* expression = parseExpression(lexer, table, context);
    printf("[Parser] Expression parsed: %s\n", CryoNodeTypeToString(expression->type));
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", table);
    return createExpressionStatement(expression);
}
// </parseExpressionStatement>


// <parseBinaryExpression>
ASTNode* parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, ASTNode* left, int precedence) {
    while (true) {
        int currentPrecedence = getOperatorPrecedence(currentToken.type);
        if (currentPrecedence < precedence) {
            return left;
        }

        CryoTokenType operator = currentToken.type;
        printf("[Parser] Operator type: %s\n", CryoTokenToString(operator));
        CryoOperatorType op = CryoTokenToOperator(operator);
        if (op == OPERATOR_NA) {
            error("Invalid operator.", "parseBinaryExpression", table);
            return NULL;
        }

        getNextToken(lexer);  // consume operator

        ASTNode* right = parsePrimaryExpression(lexer, table, context);
        if (!right) {
            error("Expected an expression on the right side of the binary operator.", "parseBinaryExpression", table);
        }

        int nextPrecedence = getOperatorPrecedence(currentToken.type);
        if (currentPrecedence < nextPrecedence) {
            right = parseBinaryExpression(lexer, table, context, right, currentPrecedence + 1);
        }

        left = createBinaryExpr(left, right, op);
    }
}
// </parseBinaryExpression>


// <parseUnaryExpression>
ASTNode* parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing unary expression...\n");
    CryoTokenType operator;
    ASTNode* right;

    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG) {
        operator = currentToken.type;
        getNextToken(lexer);
        right = parseUnaryExpression(lexer, table, context);
        return createUnaryExpr(operator, right);
    }

    return createUnaryExpr(operator, right);
}
// </parseUnaryExpression>


// <parsePublicDeclaration>
ASTNode* parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing public declaration...\n");
    consume(lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", table);

    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            return parseVarDeclaration(lexer, table, context);
        case TOKEN_KW_FN:
            return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PUBLIC);

        default:
            error("Expected a declaration.", "parsePublicDeclaration", table);
            return NULL;
    }
}
// </parsePublicDeclaration>



/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode* parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing block...\n");
    consume(lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", table);

    context->scopeLevel++;

    ASTNode* block = createBlockNode();
    while (currentToken.type != TOKEN_RBRACE) {
        ASTNode* statement = parseStatement(lexer, table, context);
        if (statement) {
            addStatementToBlock(block, statement);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(block);
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end block.", "parseBlock", table);
    context->scopeLevel--;
    return block;
}
// </parseBlock>


// <parseFunctionBlock>
ASTNode* parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing function block...\n");
    context->scopeLevel++;

    ASTNode* functionBlock = createBlockNode();
    if (!functionBlock) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create function block node\n");
        return NULL;
    }

    consume(lexer, TOKEN_LBRACE, "Expected `{` to start function block.", "parseFunctionBlock", table);

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* statement = parseStatement(lexer, table, context);
        if (statement) {
            printf("[Parser] Adding statement to function block\n");
            addStatementToBlock(functionBlock, statement);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(functionBlock);
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end function block.", "parseFunctionBlock", table);
    context->scopeLevel--;
    printf("[Parser] Exiting function block\n");
    return functionBlock;
}
// </parseFunctionBlock>



/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer, CryoSymbolTable *table, ParsingContext* context) {
    printf("[Parser] Entering parseVarDeclaration\n");

    bool isMutable = currentToken.type == TOKEN_KW_MUT;
    bool isConstant = currentToken.type == TOKEN_KW_CONST;
    bool isReference = currentToken.type == TOKEN_AMPERSAND;

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant) {
        getNextToken(lexer);
    }

    if (currentToken.type != TOKEN_IDENTIFIER) {
        printf("<#> [Parser] DEBUG: Current Token: %s\n ", CryoTokenToString(currentToken.type));
        error("[Parser] Expected variable name.", "parseVarDeclaration", table);
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    CryoDataType dataType = DATA_TYPE_UNKNOWN;

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer);
        char* varType = strndup(currentToken.start, currentToken.length);
        dataType = getCryoDataType(varType);
        if(dataType == DATA_TYPE_UNKNOWN) {
            error("[Parser] Unknown data type." , "parseVarDeclaration", table);
        }
        free(varType);
        getNextToken(lexer);
    } else {
        error("[Parser] Expected ':' after variable name.", "parseVarDeclaration", table);
    }

    if (currentToken.type != TOKEN_EQUAL) {
        error("[Parser] Expected '=' after type.", "parseVarDeclaration", table);
    }
    getNextToken(lexer);

    if(currentToken.type == TOKEN_AMPERSAND) {
        isReference = true;
        getNextToken(lexer);
    }

    ASTNode* initializer = parseExpression(lexer, table, context);
    if (initializer == NULL) {
        error("[Parser] Expected expression after '='.", "parseVarDeclaration", table);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", table);

    ASTNode* varDeclNode = createVarDeclarationNode(varName, dataType, initializer, isMutable, isConstant, isReference);
    printf("[Parser] Created Variable Declaration Node: %s\n", varName);
    printf("[Parser] Variable Declaration Node Type: %d\n", varDeclNode->type);
    return varDeclNode;
}
// </parseVarDeclaration>



/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility) {
    printf("[Parser] Parsing function declaration...\n");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", table);

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier", "parseFunctionDeclaration", table);
        return NULL;
    }

    char* functionName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    ASTNode* params = parseParameterList(lexer, table, context);

    CryoDataType returnType = DATA_TYPE_VOID;  // Default return type
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        printf("[Parser] Found return type arrow\n");
        getNextToken(lexer);
        returnType = parseType(lexer, context, table);
        getNextToken(lexer);
    } else {
        error("Expected `->` for return type.", "parseFunctionDeclaration", table);
    }

    printf("[Parser] Function return type: %s\n", CryoDataTypeToString(returnType));

    // Ensure the next token is `{` for the function block
    if (currentToken.type != TOKEN_LBRACE) {
        printf("<DEBUG> [Parser] Current Token: %s\n ", CryoTokenToString(currentToken.type));
        error("Expected `{` to start function block.", "parseFunctionDeclaration", table);
        return NULL;
    }

    // Parse the function block
    ASTNode* functionBlock = parseFunctionBlock(lexer, table, context);
    if (!functionBlock) {
        error("Failed to parse function block.", "parseFunctionDeclaration", table);
        return NULL;
    }

    return createFunctionNode(visibility, functionName, params, functionBlock, returnType);
}
// </parseFunctionDeclaration>


// <parseExternFunctionDeclaration>
ASTNode* parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing extern function declaration...\n");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", table);

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier.", "parseExternFunctionDeclaration", table);
        return NULL;
    }

    ASTNode* externNode = createExternFuncNode();
    if (!externNode) {
        error("Failed to create extern function node.", "parseExternFunctionDeclaration", table);
        return NULL;
    }

    char* functionName = strndup(currentToken.start, currentToken.length);
    externNode->data.externNode.decl.function->name = strdup(functionName);
    externNode->data.externNode.decl.function->paramCount = 0;
    externNode->data.externNode.decl.function->paramCapacity = 4;
    externNode->data.externNode.decl.function->params = (ASTNode**)malloc(externNode->data.externNode.decl.function->paramCapacity * sizeof(ASTNode*));
    externNode->data.externNode.decl.function->visibility = VISIBILITY_EXTERN;
    printf("[Parser] Function name: %s\n", functionName);

    getNextToken(lexer);
    consume(lexer, TOKEN_LPAREN, "Expected '(' after function name.", "parseExternFunctionDeclaration", table);

    if (currentToken.type != TOKEN_RPAREN) {
        printf("[Parser] Parsing function arguments...\n");
        while (currentToken.type != TOKEN_RPAREN) {
            ASTNode* param = parseParameter(lexer, table, context);
            if (!param) {
                error("Expected parameter.", "parseExternFunctionDeclaration", table);
                return NULL;
            }
            addParameterToExternDecl(table, externNode, param);

            if (currentToken.type == TOKEN_COMMA) {
                getNextToken(lexer);
            } else if (currentToken.type != TOKEN_RPAREN) {
                error("Expected ',' or ')'.", "parseExternFunctionDeclaration", table);
                return NULL;
            }
        }
    }

    getNextToken(lexer);  // Consume the closing ')'

    CryoDataType returnType;
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer);
        returnType = parseType(lexer, context, table);
        externNode->data.externNode.decl.function->returnType = returnType;
        getNextToken(lexer);
    } else {
        externNode->data.externNode.decl.function->returnType = DATA_TYPE_VOID;  // Default return type
    }


    printf("\n\n<#!> [Parser] Extern Function Return Type: %s \n", CryoDataTypeToString(returnType));
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", table);

    return externNode;
}

// </parseExternFunctionDeclaration>


// <parseFunctionCall>
ASTNode* parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char* functionName) {
    printf("[Parser] Parsing function call...\n");

    ASTNode* functionCallNode = createFunctionCallNode();
    functionCallNode->data.functionCall.name = functionName;
    functionCallNode->data.functionCall.argCount = 0;
    functionCallNode->data.functionCall.argCapacity = 8;
    functionCallNode->data.functionCall.args = (ASTNode**)malloc(functionCallNode->data.functionCall.argCapacity * sizeof(ASTNode*));

    CryoSymbol* funcSymbol = findSymbol(table, functionName);
    if (!funcSymbol) {
        printf("[SymTable - Error] Function '%s' not found or is not a function\n", functionName);
        printf("[Parser] Function Type = %s\n", CryoNodeTypeToString(funcSymbol->nodeType));
        error("Function not found or is not a function.", "parseFunctionCall", table);
        freeAST(functionCallNode);
        return NULL;
    }

    if (currentToken.type != TOKEN_RPAREN) {
        // Parse arguments with expected types
        for (int i = 0; currentToken.type != TOKEN_RPAREN; ++i) {
            if (i >= funcSymbol->argCount) {
                error("Too many arguments provided to function.", "parseFunctionCall", table);
                freeAST(functionCallNode);
                return NULL;
            }

            CryoDataType* expectedType = funcSymbol->paramTypes[i];
            CryoDataType type = parseType(lexer, context, table);
            ASTNode* arg = parseArgumentsWithExpectedType(lexer, table, context, type);
            if (!arg) {
                error("Expected argument expression.", "parseFunctionCall", table);
                freeAST(functionCallNode);
                return NULL;
            }
            addArgumentToFunctionCall(table, functionCallNode, arg);

            if (currentToken.type == TOKEN_COMMA) {
                getNextToken(lexer);  // Consume the comma and continue parsing the next argument
            }
        }
    }

    // Ensure argument count matches
    if (functionCallNode->data.functionCall.argCount != funcSymbol->argCount) {
        error("Argument count mismatch for function call.", "parseFunctionCall", table);
        freeAST(functionCallNode);
        return NULL;
    }

    consume(lexer, TOKEN_RPAREN, "Expected ')' after arguments.", "parseFunctionCall", table);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after function call.", "parseFunctionCall", table);

    return functionCallNode;
}
// </parseFunctionCall>


// <parseReturnStatement>
ASTNode* parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing return statement...\n");
    consume(lexer, TOKEN_KW_RETURN, "Expected `return` keyword.", "parseReturnStatement", table);

    ASTNode* expression = NULL;
    if (currentToken.type != TOKEN_SEMICOLON) {
        expression = parseExpression(lexer, table, context);
        printf("[Parser] Parsed return expression\n");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseReturnStatement", table);
    ASTNode* returnNode = createReturnNode(expression);
    printf("[AST] Created Return Node: Type = %d\n", returnNode->type);
    return returnNode;
}
// </parseReturnStatement>



/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

// <parseParameter>
ASTNode* parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing parameter...\n");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier.", "parseParameter", table);
        return NULL;
    }

    char* paramName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    consume(lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", table);

    CryoDataType paramType = parseType(lexer, context, table);
    // consume data type:
    getNextToken(lexer);
    return createParamNode(paramName, paramType);
}
// </parseParameter>


// <parseParameterList>
ASTNode* parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing parameter list...\n");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start parameter list.", "parseParameterList", table);

    ASTNode* paramListNode = createParamListNode();
    if(paramListNode == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create parameter list node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN) {
        ASTNode* param = parseParameter(lexer, table, context);
        if (param) {
            addParameterToList(table, paramListNode, param);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse parameter\n");
            freeAST(paramListNode);
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        }
    }

    consume(lexer, TOKEN_RPAREN, "Expected `)` to end parameter list.", "parseParameterList", table);
    return paramListNode;
}
// </parseParameterList>


// <parseArguments>
ASTNode* parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing arguments...\n");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier.", "parseArguments", table);
        return NULL;
    }

    char* argName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    // Resolve the type using the symbol table
    CryoSymbol* symbol = findSymbol(table, argName);
    CryoDataType argType = symbol ? symbol->valueType : DATA_TYPE_UNKNOWN;

    return createArgsNode(argName, argType);
}
// </parseArguments>


// <parseArgumentList>
ASTNode* parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing argument list...\n");
    ASTNode* argListNode = createArgumentListNode();
    if (argListNode == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create argument list node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN) {
        ASTNode* arg = parseArguments(lexer, table, context);
        if (arg) {
            addArgumentToList(table, argListNode, arg);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse argument\n");
            freeAST(argListNode); 
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        }
    }

    return argListNode;
}
// </parseArgumentList>

// <parseArgumentsWithExpectedType>
ASTNode* parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoDataType expectedType) {
    printf("[Parser] Parsing arguments...\n");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier.", "parseArgumentsWithExpectedType", table);
        return NULL;
    }

    char* argName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    return createArgsNode(argName, expectedType);
}
// </parseArgumentsWithExpectedType>


// <addParameterToList>
void addParameterToList(CryoSymbolTable *table, ASTNode* paramListNode, ASTNode* param) {
    printf("[Parser] Adding parameter to list\n");
    if (paramListNode->type == NODE_PARAM_LIST) {
        if (paramListNode->data.paramList.paramCount >= paramListNode->data.paramList.paramCapacity) {
            paramListNode->data.paramList.paramCapacity *= 2;
            paramListNode->data.paramList.params = (ASTNode**)realloc(paramListNode->data.paramList.params, paramListNode->data.paramList.paramCapacity * sizeof(ASTNode*));
        }

        paramListNode->data.paramList.params[paramListNode->data.paramList.paramCount++] = param;
    } else {
        fprintf(stderr, "[Parser] [ERROR] Expected parameter list node, got %s\n", CryoNodeTypeToString(paramListNode->type));
    }
}
// </addParameterToList>


// <addArgumentToList>
void addArgumentToList(CryoSymbolTable *table, ASTNode* argListNode, ASTNode* arg) {
    printf("[Parser] Adding argument to list\n");
    if (argListNode->type == NODE_ARG_LIST) {
        if (argListNode->data.argList.argCount >= argListNode->data.argList.argCapacity) {
            argListNode->data.argList.argCapacity *= 2;
            argListNode->data.argList.args = (ASTNode**)realloc(argListNode->data.argList.args, argListNode->data.argList.argCapacity * sizeof(ASTNode*));
        }

        argListNode->data.argList.args[argListNode->data.argList.argCount++] = arg;
    } else {
        fprintf(stderr, "[Parser] [ERROR] Expected argument list node, got %s\n", CryoNodeTypeToString(argListNode->type));
    }
}
// </addArgumentToList>


// <addArgumentToFunctionCall>
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode* functionCallNode, ASTNode* arg) {
    printf("[Parser] Adding argument to function call\n");
    if (functionCallNode->type == NODE_FUNCTION_CALL) {
        if (functionCallNode->data.functionCall.argCount >= functionCallNode->data.functionCall.argCapacity) {
            functionCallNode->data.functionCall.argCapacity *= 2;
            functionCallNode->data.functionCall.args = (ASTNode**)realloc(functionCallNode->data.functionCall.args, functionCallNode->data.functionCall.argCapacity * sizeof(ASTNode*));
        }

        functionCallNode->data.functionCall.args[functionCallNode->data.functionCall.argCount++] = arg;
    } else {
        fprintf(stderr, "[Parser] [ERROR] Expected function call node, got %s\n", CryoNodeTypeToString(functionCallNode->type));
    }
}
// </addArgumentToFunctionCall>


// <addParameterToExternDecl>
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode* externDeclNode, ASTNode* param) {
    printf("[Parser] Adding parameter to extern declaration\n");
    if (externDeclNode->type == NODE_EXTERN_FUNCTION) {
        if (externDeclNode->data.externNode.decl.function->paramCount >= externDeclNode->data.externNode.decl.function->paramCapacity) {
            externDeclNode->data.externNode.decl.function->paramCapacity *= 2;
            externDeclNode->data.externNode.decl.function->params = (ASTNode**)realloc(externDeclNode->data.externNode.decl.function->params, externDeclNode->data.externNode.decl.function->paramCapacity * sizeof(ASTNode*));
        }

        externDeclNode->data.externNode.decl.function->params[externDeclNode->data.externNode.decl.function->paramCount++] = param;
    } else {
        fprintf(stderr, "[Parser] [ERROR] Expected extern function node, got %s\n", CryoNodeTypeToString(externDeclNode->type));
    }
}
// </addParameterToExternDecl>



/* ====================================================================== */
/* @ASTNode_Parsing - Modules & Externals                                 */

// <parseImport>
ASTNode* parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing import...\n");
    consume(lexer, TOKEN_KW_IMPORT, "Expected `import` keyword.", "parseImport", table);

    if (currentToken.type != TOKEN_STRING_LITERAL) {
        error("Expected a string literal", "parseImport", table);
        return NULL;
    }

    char* moduleName = strndup(currentToken.start + 1, currentToken.length - 2);
    getNextToken(lexer);

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImport", table);
    return createImportNode(moduleName);
}
// </parseImport>


// <parseExtern>
ASTNode* parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing extern...\n");
    consume(lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", table);

    switch(currentToken.type) {
        case TOKEN_KW_FN:
            return parseExternFunctionDeclaration(lexer, table, context);

        default:
            error("Expected an extern declaration.", "parseExtern", table);
            return NULL;
    }

    return NULL;
}
// </parseExtern>



/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode* parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing if statement...\n");
    consume(lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", table);
    context->isParsingIfCondition = true;

    ASTNode* condition = parseExpression(lexer, table, context);
    ASTNode* ifBlock = parseBlock(lexer, table, context);
    ASTNode* elseBlock = NULL;

    if (currentToken.type == TOKEN_KW_ELSE) {
        getNextToken(lexer);
        if (currentToken.type == TOKEN_KW_IF) {
            elseBlock = parseIfStatement(lexer, table, context);
        } else {
            elseBlock = parseBlock(lexer, table, context);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock);
}
// </parseIfStatement>


// <parseForLoop>
ASTNode* parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing for loop...\n");
    consume(lexer, TOKEN_KW_FOR, "Expected `for` keyword.", "parseForLoop", table);

    consume(lexer, TOKEN_LPAREN, "Expected `(` to start for loop.", "parseForLoop", table);

    ASTNode* init = parseStatement(lexer, table, context);
    ASTNode* condition = parseExpression(lexer, table, context);
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table);

    ASTNode* update = parseExpression(lexer, table, context);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", table);

    ASTNode* body = parseBlock(lexer, table, context);
    return createForStatement(init, condition, update, body);
}
// </parseForLoop>


// <parseWhileStatement>
ASTNode* parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing while statement...\n");
    consume(lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", table);
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", table);

    ASTNode* condition = parseExpression(lexer, table, context);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", table);

    ASTNode* body = parseBlock(lexer, table, context);
    return createWhileStatement(condition, body);
}
// </parseWhileStatement>



/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode* parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context) {
    printf("[Parser] Parsing array literal...\n");
    consume(lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", table);

    ASTNode* elements = createArrayLiteralNode();
    if (elements == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RBRACKET) {
        ASTNode* element = parseExpression(lexer, table, context);
        if (element) {
            addElementToArrayLiteral(table, elements, element);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse array element\n");
            freeAST(elements);
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        }
    }

    consume(lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", table);
    return elements;
}
// </parseArrayLiteral>


// <addElementToArrayLiteral>
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode* arrayLiteral, ASTNode* element) {
    printf("[Parser] Adding element to array literal\n");
    if (arrayLiteral->type == NODE_ARRAY_LITERAL) {
        if (arrayLiteral->data.arrayLiteral.elementCount >= arrayLiteral->data.arrayLiteral.elementCapacity) {
            arrayLiteral->data.arrayLiteral.elementCapacity *= 2;
            arrayLiteral->data.arrayLiteral.elements = (ASTNode**)realloc(arrayLiteral->data.arrayLiteral.elements, arrayLiteral->data.arrayLiteral.elementCapacity * sizeof(ASTNode*));
        }

        arrayLiteral->data.arrayLiteral.elements[arrayLiteral->data.arrayLiteral.elementCount++] = element;
    } else {
        fprintf(stderr, "[Parser] [ERROR] Expected array literal node, got %s\n", CryoNodeTypeToString(arrayLiteral->type));
    }
}
// <addElementToArrayLiteral>



/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */
int parser(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "[Parser] Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char* source = readFile(argv[1]);
    if (source == NULL) return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    ASTNode* program = parseProgram(&lexer, NULL);
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

