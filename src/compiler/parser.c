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
ASTNode* parseProgram(Lexer* lexer) {
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
        ASTNode* statement = parseStatement(lexer, &context);
        if (statement) {
            addStatementToProgram(program, statement);
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
void consume(Lexer *lexer, CryoTokenType type, const char* message) {
    printf("[Parser] Consuming token: %s (Expecting: %s)\n",
        CryoTokenToString(currentToken.type),
        CryoTokenToString(type)
    );

    if (currentToken.type == type) {
        getNextToken(lexer);
    } else {
        error(message);
    }

    debugCurrentToken();
}
// </consume>


// <getNextToken>
void getNextToken(Lexer *lexer) {
    if (isAtEnd(lexer)) {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        currentToken.line = lexer->line;
        currentToken.column = lexer->column;
        return;
    }
    currentToken = get_next_token(lexer);
    printf("[Parser] @getNextToken | Next Token: Type=%d, Start=%.*s, Length=%d\n", currentToken.type, currentToken.length, currentToken.start, currentToken.length);
}
// </getNextToken>


// <error>
void error(char *message) {
    int line = currentToken.line;
    int column = currentToken.column;

    printf("\n<!> [Parser] Error: %s at line %d, column %d\n", message, line, column);
    printf("------------------------------------------------------------------------\n\n");
    // Print the line containing the error
    printf("%d ", line);
    printLine(source, line);

    for (int i = line; i < column + line; i++) {
        printf(" ");
    }
    
    printf("^ %s\n", message);
    printf("\n------------------------------------------------------------------------\n\n");
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
    CryoDataType type = CryoDataTypeStringToType(typeStr);
    return type;
}
// </getCryoDataType>


// <parseType>
CryoDataType parseType(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Parsing type...\n");
    CryoDataType type = DATA_TYPE_UNKNOWN;

    switch (currentToken.type) {
        case TOKEN_KW_VOID:
        case TOKEN_KW_INT:
        case TOKEN_KW_STRING:
        case TOKEN_KW_BOOL:
            type = getCryoDataType(strndup(currentToken.start, currentToken.length));
            getNextToken(lexer);  // Consume the type keyword
            break;

        case TOKEN_IDENTIFIER:
            type = getCryoDataType(strndup(currentToken.start, currentToken.length));
            getNextToken(lexer);  // Consume the type identifier
            break;

        default:
            error("Expected a type identifier");
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
void addStatementToProgram(ASTNode* programNode, ASTNode* statement) {
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
ASTNode* parseStatement(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing statement...\n");

    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            return parseVarDeclaration(lexer, context);

        case TOKEN_KW_PUBLIC:
            return parsePublicDeclaration(lexer, context);

        case TOKEN_KW_RETURN:
            printf("[Parser] Parsing return statement\n");
            return parseReturnStatement(lexer, context);

        case TOKEN_KW_FOR:
            return parseForLoop(lexer, context);

        case TOKEN_KW_IMPORT:
            return parseImport(lexer, context);

        case TOKEN_KW_EXTERN:
            return parseExtern(lexer, context);

        case TOKEN_IDENTIFIER:
            return parseExpressionStatement(lexer, context);

        case TOKEN_EOF:
            return NULL;

        default:
            error("Expected a statement");
            return NULL;
    }
}
// </parseStatement>


// <parsePrimaryExpression>
ASTNode* parsePrimaryExpression(Lexer *lexer, ParsingContext *context) {
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
            error("Expected an expression");
            return NULL;
    }
}
// </parsePrimaryExpression>


// <parseExpression>
ASTNode* parseExpression(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing expression...\n");
    ASTNode* left = parsePrimaryExpression(lexer, context);
    if (!left) {
        error("Expected an expression");
    }
    return parseBinaryExpression(lexer, context, left, 1);
}
// </parseExpression>


// <parseExpressionStatement>
ASTNode* parseExpressionStatement(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing expression statement...\n");
    ASTNode* expression = parseExpression(lexer, context);
    printf("[Parser] Expression parsed: %s\n", CryoNodeTypeToString(expression->type));
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon");
    return createExpressionStatement(expression);
}
// </parseExpressionStatement>


// <parseBinaryExpression>
ASTNode* parseBinaryExpression(Lexer *lexer, ParsingContext *context, ASTNode* left, int precedence) {
    while (true) {
        int currentPrecedence = getOperatorPrecedence(currentToken.type);
        if (currentPrecedence < precedence) {
            return left;
        }

        CryoTokenType operator = currentToken.type;
        printf("[Parser] Operator type: %s\n", CryoTokenToString(operator));
        CryoOperatorType op = CryoTokenToOperator(operator);
        if (op == OPERATOR_NA) {
            error("Invalid operator");
            return NULL;
        }

        getNextToken(lexer);  // consume operator

        ASTNode* right = parsePrimaryExpression(lexer, context);
        if (!right) {
            error("Expected an expression on the right side of the binary operator");
        }

        int nextPrecedence = getOperatorPrecedence(currentToken.type);
        if (currentPrecedence < nextPrecedence) {
            right = parseBinaryExpression(lexer, context, right, currentPrecedence + 1);
        }

        left = createBinaryExpr(left, right, op);
    }
}
// </parseBinaryExpression>


// <parseUnaryExpression>
ASTNode* parseUnaryExpression(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing unary expression...\n");
    CryoTokenType operator;
    ASTNode* right;

    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG) {
        operator = currentToken.type;
        getNextToken(lexer);
        right = parseUnaryExpression(lexer, context);
        return createUnaryExpr(operator, right);
    }

    return createUnaryExpr(operator, right);
}
// </parseUnaryExpression>


// <parsePublicDeclaration>
ASTNode* parsePublicDeclaration(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing public declaration...\n");
    consume(lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword");

    switch(currentToken.type) {
        case TOKEN_KW_CONST:
        case TOKEN_KW_MUT:
            return parseVarDeclaration(lexer, context);

        case TOKEN_KW_EXTERN:
            return parseExternFunctionDeclaration(lexer, context);

        case TOKEN_KW_FN:
            return parseFunctionDeclaration(lexer, context, VISIBILITY_PUBLIC);

        default:
            error("Expected a declaration");
            return NULL;
    }
}
// </parsePublicDeclaration>



/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode* parseBlock(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing block...\n");
    consume(lexer, TOKEN_LBRACE, "Expected `{` to start block");

    context->scopeLevel++;

    ASTNode* block = createBlockNode();
    while (currentToken.type != TOKEN_RBRACE) {
        ASTNode* statement = parseStatement(lexer, context);
        if (statement) {
            addStatementToBlock(block, statement);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(block);
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end block");
    context->scopeLevel--;
    return block;
}
// </parseBlock>


// <parseFunctionBlock>
ASTNode* parseFunctionBlock(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing function block...\n");
    context->scopeLevel++;

    ASTNode* functionBlock = createBlockNode();
    if (!functionBlock) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create function block node\n");
        return NULL;
    }

    consume(lexer, TOKEN_LBRACE, "Expected `{` to start function block");

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF) {
        ASTNode* statement = parseStatement(lexer, context);
        if (statement) {
            printf("[Parser] Adding statement to function block\n");
            addStatementToBlock(functionBlock, statement);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse statement\n");
            freeAST(functionBlock);
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end function block");
    context->scopeLevel--;
    printf("[Parser] Exiting function block\n");
    return functionBlock;
}
// </parseFunctionBlock>



/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode* parseVarDeclaration(Lexer* lexer, ParsingContext* context) {
    printf("[Parser] Entering parseVarDeclaration\n");

    bool isMutable = currentToken.type == TOKEN_KW_MUT;
    bool isConstant = currentToken.type == TOKEN_KW_CONST;
    bool isReference = currentToken.type == TOKEN_AMPERSAND;

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant) {
        getNextToken(lexer);
    }

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("[Parser] Expected variable name");
    }
    char* varName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    CryoDataType dataType = DATA_TYPE_UNKNOWN;

    if (currentToken.type == TOKEN_COLON) {
        getNextToken(lexer);
        char* varType = strndup(currentToken.start, currentToken.length);
        dataType = getCryoDataType(varType);
        if(dataType == DATA_TYPE_UNKNOWN) {
            error("[Parser] Unknown data type");
        }
        free(varType);
        getNextToken(lexer);
    } else {
        error("[Parser] Expected ':' after variable name");
    }

    if (currentToken.type != TOKEN_EQUAL) {
        error("[Parser] Expected '=' after type");
    }
    getNextToken(lexer);

    if(currentToken.type == TOKEN_AMPERSAND) {
        isReference = true;
        getNextToken(lexer);
    }

    ASTNode* initializer = parseExpression(lexer, context);
    if (initializer == NULL) {
        error("[Parser] Expected expression after '='");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration");

    ASTNode* varDeclNode = createVarDeclarationNode(varName, dataType, initializer, isMutable, isConstant, isReference);
    printf("[Parser] Created Variable Declaration Node: %s\n", varName);
    printf("[Parser] Variable Declaration Node Type: %d\n", varDeclNode->type);
    return varDeclNode;
}
// </parseVarDeclaration>



/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode* parseFunctionDeclaration(Lexer *lexer, ParsingContext *context, CryoVisibilityType visibility) {
    printf("[Parser] Parsing function declaration...\n");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier");
        return NULL;
    }

    char* functionName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    ASTNode* params = parseParameterList(lexer, context);

    CryoDataType returnType = DATA_TYPE_VOID;  // Default return type
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        printf("[Parser] Found return type arrow\n");
        getNextToken(lexer);
        returnType = parseType(lexer, context);
    } else {
        error("Expected `->` for return type");
    }

    printf("[Parser] Function return type: %s\n", CryoDataTypeToString(returnType));

    // Ensure the next token is `{` for the function block
    if (currentToken.type != TOKEN_LBRACE) {
        printf("<DEBUG> [Parser] Current Token: %s\n ", CryoTokenToString(currentToken.type));
        error("Expected `{` to start function block");
        return NULL;
    }

    // Parse the function block
    ASTNode* functionBlock = parseFunctionBlock(lexer, context);
    if (!functionBlock) {
        error("Failed to parse function block");
        return NULL;
    }

    return createFunctionNode(visibility, functionName, params, functionBlock, returnType);
}
// </parseFunctionDeclaration>


// <parseExternFunctionDeclaration>
ASTNode* parseExternFunctionDeclaration(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing extern function declaration...\n");
    consume(lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier");
        return NULL;
    }

    char* functionName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    ASTNode* params = parseParameterList(lexer, context);
    CryoDataType returnType;
    if (currentToken.type == TOKEN_RESULT_ARROW) {
        getNextToken(lexer);
        returnType = parseType(lexer, context);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon");
    return createExternDeclNode(functionName, params, returnType);
}
// </parseExternFunctionDeclaration>


// <parseFunctionCall>
ASTNode* parseFunctionCall(Lexer *lexer, ParsingContext *context, char* functionName) {
    printf("[Parser] Parsing function call...\n");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start function call");

    ASTNode* args = parseArgumentList(lexer, context);
    int argCount = 0;
    if (args) {
        argCount = args->data.functionCall.argCount;
    }
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end function call");

    return createFunctionCallNode(functionName, args, argCount);
}
// </parseFunctionCall>


// <parseReturnStatement>
ASTNode* parseReturnStatement(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing return statement...\n");
    consume(lexer, TOKEN_KW_RETURN, "Expected `return` keyword");

    ASTNode* expression = NULL;
    if (currentToken.type != TOKEN_SEMICOLON) {
        expression = parseExpression(lexer, context);
        printf("[Parser] Parsed return expression\n");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon");
    ASTNode* returnNode = createReturnNode(expression);
    printf("[AST] Created Return Node: Type = %d\n", returnNode->type);
    return returnNode;
}

// </parseReturnStatement>



/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

// <parseParameter>
ASTNode* parseParameter(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing parameter...\n");

    if (currentToken.type != TOKEN_IDENTIFIER) {
        error("Expected an identifier");
        return NULL;
    }

    char* paramName = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer);

    consume(lexer, TOKEN_COLON, "Expected `:` after parameter name");

    CryoDataType paramType = parseType(lexer, context);
    return createParamNode(paramName, paramType);
}
// </parseParameter>


// <parseParameterList>
ASTNode* parseParameterList(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing parameter list...\n");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start parameter list");

    ASTNode* paramListNode = createParamListNode();
    if(paramListNode == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create parameter list node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN) {
        ASTNode* param = parseParameter(lexer, context);
        if (param) {
            addParameterToList(paramListNode, param);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse parameter\n");
            freeAST(paramListNode);
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        }
    }

    consume(lexer, TOKEN_RPAREN, "Expected `)` to end parameter list");
    return paramListNode;
}
// </parseParameterList>


// <parseArgumentList>
ASTNode* parseArgumentList(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing argument list...\n");
    ASTNode* argListNode = createArgumentListNode();
    if (argListNode == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create argument list node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN) {
        ASTNode* arg = parseExpression(lexer, context);
        if (arg) {
            addArgumentToList(argListNode, arg);
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


// <addParameterToList>
void addParameterToList(ASTNode* paramListNode, ASTNode* param) {
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
void addArgumentToList(ASTNode* argListNode, ASTNode* arg) {
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



/* ====================================================================== */
/* @ASTNode_Parsing - Modules & Externals                                 */

// <parseImport>
ASTNode* parseImport(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing import...\n");
    consume(lexer, TOKEN_KW_IMPORT, "Expected `import` keyword");

    if (currentToken.type != TOKEN_STRING_LITERAL) {
        error("Expected a string literal");
        return NULL;
    }

    char* moduleName = strndup(currentToken.start + 1, currentToken.length - 2);
    getNextToken(lexer);

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon");
    return createImportNode(moduleName);
}
// </parseImport>


// <parseExtern>
ASTNode* parseExtern(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing extern...\n");
    consume(lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword");

    switch(currentToken.type) {
        case TOKEN_KW_FN:
            return parseExternFunctionDeclaration(lexer, context);

        default:
            error("Expected an extern declaration");
            return NULL;
    }

    return NULL;
}
// </parseExtern>



/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode* parseIfStatement(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing if statement...\n");
    consume(lexer, TOKEN_KW_IF, "Expected `if` keyword");
    context->isParsingIfCondition = true;

    ASTNode* condition = parseExpression(lexer, context);
    ASTNode* ifBlock = parseBlock(lexer, context);
    ASTNode* elseBlock = NULL;

    if (currentToken.type == TOKEN_KW_ELSE) {
        getNextToken(lexer);
        if (currentToken.type == TOKEN_KW_IF) {
            elseBlock = parseIfStatement(lexer, context);
        } else {
            elseBlock = parseBlock(lexer, context);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock);
}
// </parseIfStatement>


// <parseForLoop>
ASTNode* parseForLoop(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing for loop...\n");
    consume(lexer, TOKEN_KW_FOR, "Expected `for` keyword");

    consume(lexer, TOKEN_LPAREN, "Expected `(` to start for loop");

    ASTNode* init = parseStatement(lexer, context);
    ASTNode* condition = parseExpression(lexer, context);
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon");

    ASTNode* update = parseExpression(lexer, context);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end for loop");

    ASTNode* body = parseBlock(lexer, context);
    return createForStatement(init, condition, update, body);
}
// </parseForLoop>


// <parseWhileStatement>
ASTNode* parseWhileStatement(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing while statement...\n");
    consume(lexer, TOKEN_KW_WHILE, "Expected `while` keyword");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start while loop");

    ASTNode* condition = parseExpression(lexer, context);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end while loop");

    ASTNode* body = parseBlock(lexer, context);
    return createWhileStatement(condition, body);
}
// </parseWhileStatement>



/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode* parseArrayLiteral(Lexer *lexer, ParsingContext *context) {
    printf("[Parser] Parsing array literal...\n");
    consume(lexer, TOKEN_LBRACKET, "Expected `[` to start array literal");

    ASTNode* elements = createArrayLiteralNode();
    if (elements == NULL) {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RBRACKET) {
        ASTNode* element = parseExpression(lexer, context);
        if (element) {
            addElementToArrayLiteral(elements, element);
        } else {
            fprintf(stderr, "[Parser] [ERROR] Failed to parse array element\n");
            freeAST(elements);
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA) {
            getNextToken(lexer);
        }
    }

    consume(lexer, TOKEN_RBRACKET, "Expected `]` to end array literal");
    return elements;
}
// </parseArrayLiteral>


// <addElementToArrayLiteral>
void addElementToArrayLiteral(ASTNode* arrayLiteral, ASTNode* element) {
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
