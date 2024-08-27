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

// #ifndef HAVE_STRNDUP
//  <strndup>
// char* strndup(const char* s, size_t n) {
//     size_t len = strnlen(s, n);
//     char* new_str = (char*)malloc(len + 1);
//     if (new_str == NULL) {
//         return NULL;
//     }
//     memcpy(new_str, s, len);
//     new_str[len] = '\0';
//     return new_str;
// }
//  </strndup>
// #endif

void printLine(const char *source, int line, Arena *arena)
{
    const char *lineStart = source;
    while (*lineStart && line > 1)
    {
        if (*lineStart == '\n')
            line--;
        lineStart++;
    }

    const char *lineEnd = lineStart;
    while (*lineEnd && *lineEnd != '\n')
    {
        lineEnd++;
    }

    printf("%.*s\n", (int)(lineEnd - lineStart), lineStart);
}

/* ====================================================================== */
// Scope-Declared Current Token
Token currentToken;
const char *source;

/* ====================================================================== */
/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */

// <parseProgram>
ASTNode *parseProgram(Lexer *lexer, CryoSymbolTable *table, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing program...");

    source = lexer->start;

    ParsingContext context = {
        false,
        0};

    ASTNode *program = createProgramNode(arena);
    if (!program)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create program node");
        return NULL;
    }

    getNextToken(lexer, arena);

    logMessage("INFO", __LINE__, "Parser", "Parsing statements...");

    while (currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, table, &context, arena);
        if (statement->metaData->type == NODE_NAMESPACE)
        {
            addStatementToProgram(program, table, statement, arena);
        }
        if (statement)
        {
            // traverseAST(statement, table);
            addStatementToProgram(program, table, statement, arena);
            addASTNodeSymbol(table, statement, arena);
            // printSymbolTable(table, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse statement.");
            error("Failed to parse statement.", "parseProgram", table, arena);
            return NULL;
        }
    }

    return program;
}
// </parseProgram>

/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, CryoSymbolTable *table, Arena *arena)
{
    logMessage("CRITICAL", __LINE__, "Parser", "Consuming Token: %s", CryoTokenToString(type));

    pushCallStack(&callStack, functionName, currentToken.line);

    if (currentToken.type == type)
    {
        getNextToken(lexer, arena);
    }
    else
    {
        error((char *)message, (char *)functionName, table, arena);
    }

    debugCurrentToken(arena);
}
// </consume>

// <getNextToken>
void getNextToken(Lexer *lexer, Arena *arena)
{
    // printf("[Parser] @getNextToken | Current Token before: Type=%d, Start=%.*s, Length=%d\n", currentToken.type, currentToken.length, currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Current Token before: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(currentToken.type), currentToken.length, currentToken.start, currentToken.length);
    if (isAtEnd(lexer))
    {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        currentToken.line = lexer->line;
        currentToken.column = lexer->column;
        return;
    }
    currentToken = get_next_token(lexer);
    logMessage("INFO", __LINE__, "Parser", "Current Token after: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(currentToken.type), currentToken.length, currentToken.start, currentToken.length);
}
// </getNextToken>

// <peekNextUnconsumedToken>
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena)
{
    if (isAtEnd(lexer))
    {
        return currentToken;
    }
    return get_next_token(lexer);
}
// </peekNextUnconsumedToken>

// <error>
void error(char *message, char *functionName, CryoSymbolTable *table, Arena *arena)
{
    int line = currentToken.line;
    int column = currentToken.column;

    printSymbolTable(table, arena);

    printf("\n\n");
    printStackTrace(&callStack);

    printf("\n<!> [Parser] Error: %s at line %d, column %d\n", message, line, column);
    printf("@Function: <%s>\n", functionName);
    printf("------------------------------------------------------------------------\n\n");
    // Print the line containing the error
    printf("%d ", line - 1);
    printLine(source, line - 1, arena);
    printf("%d ", line);
    printLine(source, line, arena);

    for (int i = line; i < column + line; i++)
    {
        printf(" ");
    }

    printf("^ %s\n", message);
    printf("\n------------------------------------------------------------------------\n\n");
    freeCallStack(&callStack);
    exit(1);
}
// </error>

// <debugCurrentToken>
void debugCurrentToken(Arena *arena)
{
    // printf("[Parser DEBUG] Current Token: %s, Lexeme: %.*s\n",
    //        CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
    logMessage("DEBUG", __LINE__, "Parser", "Current Token: %s, Lexeme: %.*s",
               CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
}
// </debugCurrentToken>

// <getNamespaceName>
char *getNamespaceName(Lexer *lexer, Arena *arena)
{
    char *namespaceName = NULL;
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(currentToken.start, currentToken.length);
        getNextToken(lexer, arena);
    }
    else
    {
        error("Expected a namespace name", "getNamespaceName", NULL, arena);
    }
    return namespaceName;
}
// </getNamespaceName>

/* ====================================================================== */
/* @DataType_Management                                                   */

// <getCryoDataType>
CryoDataType getCryoDataType(const char *typeStr, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Getting data typestring: %s", typeStr);
    CryoDataType type = CryoDataTypeStringToType((char *)typeStr);
    if (type == DATA_TYPE_UNKNOWN)
    {
        error("Unknown data type", "getCryoDataType", NULL, arena);
    }

    logMessage("INFO", __LINE__, "Parser", "Data type: %s", CryoDataTypeToString(type));
    return type;
}
// </getCryoDataType>

// <parseType>
CryoDataType parseType(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing type...");
    CryoDataType type = DATA_TYPE_UNKNOWN;

    switch (currentToken.type)
    {
    case TOKEN_KW_VOID:
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
    case TOKEN_KW_BOOL:
        type = getCryoDataType(strndup(currentToken.start, currentToken.length), arena);
        break;

    case TOKEN_IDENTIFIER:
        type = getCryoDataType(strndup(currentToken.start, currentToken.length), arena);
        break;

    default:
        error("Expected a type identifier", "getNextToken", table, arena);
        break;
    }

    logMessage("INFO", __LINE__, "Parser", "Parsed type: %s", CryoDataTypeToString(type));
    return type;
}
// </parseType>

// <getOperatorPrecedence>
int getOperatorPrecedence(CryoTokenType type, Arena *arena)
{
    switch (type)
    {
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
void addStatementToProgram(ASTNode *programNode, CryoSymbolTable *table, ASTNode *statement, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Adding statement to program...");
    if (!programNode || programNode->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    CryoProgram *program = programNode->data.program;

    logMessage("INFO", __LINE__, "Parser", "Before adding statement: statementCount = %zu, statementCapacity = %zu",
               program->statementCount, program->statementCapacity);

    if (program->statementCount >= program->statementCapacity)
    {
        program->statementCapacity = (program->statementCapacity > 0) ? (program->statementCapacity * 2) : 1;
        program->statements = (ASTNode **)realloc(program->statements,
                                                  program->statementCapacity * sizeof(ASTNode *));
        if (!program->statements)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for program statements");
            exit(EXIT_FAILURE);
            return;
        }
    }

    program->statements[program->statementCount++] = statement;

    logMessage("INFO", __LINE__, "Parser", "After adding statement: statementCount = %zu, statementCapacity = %zu",
               program->statementCount, program->statementCapacity);
}
// </addStatementToProgram>

/* ====================================================================== */
/* @ASTNode_Parsing - Expressions & Statements                            */

// <parseStatement>
ASTNode *parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing statement...");

    switch (currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena);

    case TOKEN_KW_PUBLIC:
        return parsePublicDeclaration(lexer, table, context, arena);

    case TOKEN_KW_RETURN:
        return parseReturnStatement(lexer, table, context, arena);

    case TOKEN_KW_FOR:
        return parseForLoop(lexer, table, context, arena);

    case TOKEN_KW_IMPORT:
        return parseImport(lexer, table, context, arena);

    case TOKEN_KW_EXTERN:
        return parseExtern(lexer, table, context, arena);

    case TOKEN_IDENTIFIER:
        if (currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena).type == TOKEN_LPAREN)
        {
            char *functionName = strndup(currentToken.start, currentToken.length);
            return parseFunctionCall(lexer, table, context, functionName, arena);
        }
        return parseExpressionStatement(lexer, table, context, arena);

    case TOKEN_KW_NAMESPACE:
        return parseNamespace(lexer, table, context, arena);

    case TOKEN_EOF:
        return NULL;

    default:
        error("Expected a statement", "parseStatement", table, arena);
        return NULL;
    }
}
// </parseStatement>

ASTNode *parseNamespace(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing namespace...");
    // Equivalent to `package <name>` like in Go.
    consume(lexer, TOKEN_KW_NAMESPACE, "Expected 'namespace' keyword.", "parseNamespace", table, arena);

    ASTNode *node = NULL;

    char *namespaceName = NULL;
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(currentToken.start, currentToken.length);
        node = createNamespaceNode(namespaceName, arena);
        getNextToken(lexer, arena);
    }
    else
    {
        error("Expected a namespace name", "parseNamespace", table, arena);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseNamespace", table, arena);

    return node;
}

// <parsePrimaryExpression>
ASTNode *parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing primary expression...");
    logMessage("INFO", __LINE__, "Parser", "Current Token: %s", CryoTokenToString(currentToken.type));

    ASTNode *node = NULL;

    switch (currentToken.type)
    {
    case TOKEN_INT_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing integer literal");
        node = createIntLiteralNode(atoi(currentToken.start), arena);
        getNextToken(lexer, arena);
        return node;

    case TOKEN_STRING_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing string literal");
        char *str = strndup(currentToken.start + 1, currentToken.length - 2);
        node = createStringLiteralNode(str, arena);
        getNextToken(lexer, arena);
        return node;

    case TOKEN_BOOLEAN_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing boolean literal");
        node = createBooleanLiteralNode(strcmp(currentToken.start, "true") == 0, arena);
        getNextToken(lexer, arena);
        return node;

    case TOKEN_LBRACKET:
        logMessage("INFO", __LINE__, "Parser", "Parsing array literal");
        return parseArrayLiteral(lexer, table, context, arena);

    case TOKEN_IDENTIFIER:
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier");
        node = createIdentifierNode(strndup(currentToken.start, currentToken.length), arena);
        getNextToken(lexer, arena);
        return node;

    default:
        error("Expected an expression", "parsePrimaryExpression", table, arena);
        return NULL;
    }
}
// </parsePrimaryExpression>

// <parseExpression>
ASTNode *parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression...");
    ASTNode *left = parsePrimaryExpression(lexer, table, context, arena);
    if (!left)
    {
        error("Expected an expression.", "parseExpression", table, arena);
    }
    return parseBinaryExpression(lexer, table, context, left, 1, arena);
}
// </parseExpression>

// <parseExpressionStatement>
ASTNode *parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression statement...");

    ASTNode *expression = parseExpression(lexer, table, context, arena);

    logMessage("INFO", __LINE__, "Parser", "Expression parsed: %s", CryoNodeTypeToString(expression->metaData->type));

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", table, arena);

    return createExpressionStatement(expression, arena);
}
// </parseExpressionStatement>

// <parseBinaryExpression>
ASTNode *parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, ASTNode *left, int precedence, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing binary expression...");
    while (true)
    {
        int currentPrecedence = getOperatorPrecedence(currentToken.type, arena);
        if (currentPrecedence < precedence)
        {
            return left;
        }

        CryoTokenType operator= currentToken.type;
        logMessage("INFO", __LINE__, "Parser", "Current operator: %s", CryoTokenToString(operator));
        CryoOperatorType op = CryoTokenToOperator(operator);
        if (op == OPERATOR_NA)
        {
            error("Invalid operator.", "parseBinaryExpression", table, arena);
            return NULL;
        }

        getNextToken(lexer, arena); // consume operator

        ASTNode *right = parsePrimaryExpression(lexer, table, context, arena);
        if (!right)
        {
            error("Expected an expression on the right side of the binary operator.", "parseBinaryExpression", table, arena);
        }

        int nextPrecedence = getOperatorPrecedence(currentToken.type, arena);
        if (currentPrecedence < nextPrecedence)
        {
            right = parseBinaryExpression(lexer, table, context, right, currentPrecedence + 1, arena);
        }

        left = createBinaryExpr(left, right, op, arena);
    }
}
// </parseBinaryExpression>

// <parseUnaryExpression>
ASTNode *parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing unary expression...");
    CryoTokenType operator;
    ASTNode *right;

    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG)
    {
        operator= currentToken.type;
        getNextToken(lexer, arena);
        right = parseUnaryExpression(lexer, table, context, arena);
        return createUnaryExpr(operator, right, arena);
    }

    return createUnaryExpr(operator, right, arena);
}
// </parseUnaryExpression>

// <parsePublicDeclaration>
ASTNode *parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing public declaration...");
    consume(lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", table, arena);

    switch (currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PUBLIC, arena);

    default:
        error("Expected a declaration.", "parsePublicDeclaration", table, arena);
        return NULL;
    }
}
// </parsePublicDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode *parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing block...");
    consume(lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", table, arena);

    context->scopeLevel++;

    ASTNode *block = createBlockNode(arena);
    while (currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *statement = parseStatement(lexer, table, context, arena);
        if (statement)
        {
            addStatementToBlock(block, statement, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse block");
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end block.", "parseBlock", table, arena);
    context->scopeLevel--;
    return block;
}
// </parseBlock>

// <parseFunctionBlock>
ASTNode *parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function block...");
    context->scopeLevel++;

    ASTNode *functionBlock = createFunctionBlock(arena);
    if (!functionBlock)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create function block");
        return NULL;
    }

    consume(lexer, TOKEN_LBRACE, "Expected `{` to start function block.", "parseFunctionBlock", table, arena);

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, table, context, arena);
        if (statement)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding statement to function block...");
            addStatementToFunctionBlock(functionBlock, statement, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse function block");
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end function block.", "parseFunctionBlock", table, arena);
    context->scopeLevel--;

    logMessage("INFO", __LINE__, "Parser", "Function block parsed.");
    return functionBlock;
}
// </parseFunctionBlock>

/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode *parseVarDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing variable declaration...");
    bool isMutable = currentToken.type == TOKEN_KW_MUT;
    bool isConstant = currentToken.type == TOKEN_KW_CONST;
    bool isReference = currentToken.type == TOKEN_AMPERSAND;

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant)
    {
        getNextToken(lexer, arena);
    }

    // Parse the variable name
    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected variable name.");
        error("[Parser] Expected variable name.", "parseVarDeclaration", table, arena);
    }
    char *var_name = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer, arena);

    // Parse the variable type
    CryoDataType dataType = DATA_TYPE_UNKNOWN;
    if (currentToken.type == TOKEN_COLON)
    {
        getNextToken(lexer, arena);
        char *varType = strndup(currentToken.start, currentToken.length);

        dataType = getCryoDataType(varType, arena);
        if (dataType == DATA_TYPE_UNKNOWN)
        {
            error("[Parser] Unknown data type.", "parseVarDeclaration", table, arena);
        }
        getNextToken(lexer, arena);
    }
    else
    {
        error("[Parser] Expected ':' after variable name.", "parseVarDeclaration", table, arena);
    }

    // Parse the variable initializer
    if (currentToken.type != TOKEN_EQUAL)
    {
        error("[Parser] Expected '=' after type.", "parseVarDeclaration", table, arena);
    }
    getNextToken(lexer, arena);

    // Check if the variable is a reference (Note: This is not yet implemented in the lexer)
    if (currentToken.type == TOKEN_AMPERSAND)
    {

        isReference = true;
        getNextToken(lexer, arena);
    }

    // Parse the initializer expression
    ASTNode *initializer = parseExpression(lexer, table, context, arena);
    if (initializer == NULL)
    {
        error("[Parser] Expected expression after '='.", "parseVarDeclaration", table, arena);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", table, arena);

    logMessage("INFO", __LINE__, "Parser", "Variable declaration parsed.");

    ASTNode *varDeclNode = createVarDeclarationNode(var_name, dataType, initializer, isMutable, isConstant, isReference, arena);

    return varDeclNode;
}
// </parseVarDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode *parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function declaration...");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", table, arena);

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier", "parseFunctionDeclaration", table, arena);
        return NULL;
    }

    char *functionName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena);

    ASTNode **params = parseParameterList(lexer, table, context, arena);

    for (int i = 0; params[i] != NULL; i++)
    {
        logMessage("INFO", __LINE__, "Parser", "Adding parameter: %s", params[i]->data.varDecl->name);
    }

    CryoDataType returnType = DATA_TYPE_VOID; // Default return type
    if (currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage("INFO", __LINE__, "Parser", "Found return type arrow");
        getNextToken(lexer, arena);
        returnType = parseType(lexer, context, table, arena);
        getNextToken(lexer, arena);
    }
    else
    {
        error("Expected `->` for return type.", "parseFunctionDeclaration", table, arena);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", CryoDataTypeToString(returnType));

    // Ensure the next token is `{` for the function block
    if (currentToken.type != TOKEN_LBRACE)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected `{` to start function block.");
        error("Expected `{` to start function block.", "parseFunctionDeclaration", table, arena);
        return NULL;
    }

    // Parse the function block
    ASTNode *functionBlock = parseFunctionBlock(lexer, table, context, arena);
    if (!functionBlock)
    {
        error("Failed to parse function block.", "parseFunctionDeclaration", table, arena);
        return NULL;
    }

    return createFunctionNode(visibility, functionName, params, functionBlock, returnType, arena);
}
// </parseFunctionDeclaration>

// <parseExternFunctionDeclaration>
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern function declaration...");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", table, arena);

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseExternFunctionDeclaration", table, arena);
        return NULL;
    }

    char *functionName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena);

    ASTNode **params = parseParameterList(lexer, table, context, arena);
    // get length of params
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    CryoDataType returnType = DATA_TYPE_VOID; // Default return type
    if (currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage("INFO", __LINE__, "Parser", "Found return type arrow");
        getNextToken(lexer, arena);
        returnType = parseType(lexer, context, table, arena);
        getNextToken(lexer, arena);
    }
    else
    {
        error("Expected `->` for return type.", "parseFunctionDeclaration", table, arena);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", CryoDataTypeToString(returnType));
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", table, arena);

    ASTNode *externFunc = createExternFuncNode(functionName, params, returnType, arena);

    return externFunc;
}
// </parseExternFunctionDeclaration>

// <parseFunctionCall>
ASTNode *parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *functionName, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function call...");

    ASTNode *functionCallNode = createFunctionCallNode(arena);
    functionCallNode->data.functionCall->name = strdup(functionName);
    functionCallNode->data.functionCall->argCount = 0;
    functionCallNode->data.functionCall->argCapacity = 8;
    functionCallNode->data.functionCall->args = (ASTNode **)ARENA_ALLOC(arena, functionCallNode->data.functionCall->argCapacity * sizeof(ASTNode *));

    CryoSymbol *funcSymbol = findSymbol(table, functionName, arena);
    if (!funcSymbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Function not found or is not a function.");
        logMessage("ERROR", __LINE__, "Parser", "Function name: %s", functionName);
        error("Function not found or is not a function.", "parseFunctionCall", table, arena);
        return NULL;
    }

    getNextToken(lexer, arena); // Consume the function name

    if (currentToken.type != TOKEN_RPAREN)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing arguments...");
        // Parse arguments with expected types
        for (int i = 0; currentToken.type != TOKEN_RPAREN; ++i)
        {
            // Look up the expected type from the symbol table
            CryoDataType type;
            if (funcSymbol->node->metaData->type == NODE_FUNCTION_DECLARATION)
            {
                logMessage("INFO", __LINE__, "Parser", "Function declaration");
                ASTNode *param = funcSymbol->node->data.functionDecl->params[i];
                type = param->data.varDecl->type;
            }

            if (funcSymbol->node->metaData->type == NODE_EXTERN_FUNCTION)
            {
                logMessage("INFO", __LINE__, "Parser", "Extern function declaration");
                ASTNode *param = funcSymbol->node->data.externFunction->params[i];
                type = param->data.varDecl->type;
            }

            ASTNode *arg = parseArgumentsWithExpectedType(lexer, table, context, type, arena);
            if (!arg)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to parse argument.");
                error("Expected argument expression.", "parseFunctionCall", table, arena);
                return NULL;
            }
            addArgumentToFunctionCall(table, functionCallNode, arg, arena);

            if (currentToken.type == TOKEN_COMMA)
            {
                getNextToken(lexer, arena); // Consume the comma and continue parsing the next argument
            }
        }
    }

    // Ensure argument count matches
    if (functionCallNode->data.functionCall->argCount != funcSymbol->argCount)
    {
        logMessage("ERROR", __LINE__, "Parser", "Argument count mismatch for function call.");
        error("Argument count mismatch for function call.", "parseFunctionCall", table, arena);
        return NULL;
    }

    consume(lexer, TOKEN_RPAREN, "Expected ')' after arguments.", "parseFunctionCall", table, arena);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after function call.", "parseFunctionCall", table, arena);

    logMessage("INFO", __LINE__, "Parser", "Function call parsed.");

    return functionCallNode;
}
// </parseFunctionCall>

// <parseReturnStatement>
ASTNode *parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing return statement...");
    consume(lexer, TOKEN_KW_RETURN, "Expected `return` keyword.", "parseReturnStatement", table, arena);

    ASTNode *expression = NULL;
    if (currentToken.type != TOKEN_SEMICOLON)
    {
        expression = parseExpression(lexer, table, context, arena);
        printf("[Parser] Parsed return expression\n");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseReturnStatement", table, arena);
    ASTNode *returnNode = createReturnNode(expression, arena);
    logMessage("INFO", __LINE__, "Parser", "Return statement parsed.");
    return returnNode;
}
// </parseReturnStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

// <parseParameter>
ASTNode *parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter...");

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseParameter", table, arena);
        return NULL;
    }

    char *paramName = strndup(currentToken.start, currentToken.length);

    getNextToken(lexer, arena);

    consume(lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", table, arena);

    CryoDataType paramType = parseType(lexer, context, table, arena);
    // consume data type:
    getNextToken(lexer, arena);
    ASTNode *node = createParamNode(paramName, paramType, arena);
    return node;
}
// </parseParameter>

// <parseParameterList>
ASTNode **parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter list...");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start parameter list.", "parseParameterList", table, arena);

    ASTNode **paramListNode = (ASTNode **)ARENA_ALLOC(arena, 8 * sizeof(ASTNode *));
    if (!paramListNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to allocate memory for parameter list.");
        return NULL;
    }

    int paramCount = 0;
    while (currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *param = parseParameter(lexer, table, context, arena);
        if (param)
        {
            if (param->metaData->type == NODE_VAR_DECLARATION)
            {
                logMessage("INFO", __LINE__, "Parser", "Adding parameter: %s", param->data.varDecl->name);
                paramListNode[paramCount] = param;
                paramCount++;
            }
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse parameter.");
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena);
        }
    }

    paramListNode[paramCount] = NULL; // Null terminate the parameter array

    logMessage("INFO", __LINE__, "Parser", "Parameter count: %d", paramCount);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end parameter list.", "parseParameterList", table, arena);

    return paramListNode;
}
// </parseParameterList>

// <parseArguments>
ASTNode *parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments...");

    if (currentToken.type != TOKEN_IDENTIFIER && currentToken.type != TOKEN_INT_LITERAL && currentToken.type != TOKEN_STRING_LITERAL && currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        error("Expected an identifier.", "parseArguments", table, arena);
        return NULL;
    }

    char *argName = strndup(currentToken.start, currentToken.length);
    bool isLiteral = false;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an integer literal");
        isLiteral = true;
    }
    else if (currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a string literal");
        isLiteral = true;
    }
    else if (currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a boolean literal");
        isLiteral = true;
    }

    // Resolve the type using the symbol table
    CryoSymbol *symbol = findSymbol(table, argName, arena);
    CryoDataType argType = symbol ? symbol->valueType : DATA_TYPE_UNKNOWN;

    // Consume the argument name
    getNextToken(lexer, arena);

    return createArgsNode(argName, argType, isLiteral, arena);
}
// </parseArguments>

// <parseArgumentList>
ASTNode *parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing argument list...");
    ASTNode *argListNode = createArgumentListNode(arena);
    if (argListNode == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create argument list node.");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *arg = parseArguments(lexer, table, context, arena);
        if (arg)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding argument to list...");
            addArgumentToList(table, argListNode, arg, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse argument.");
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena);
        }
    }

    return argListNode;
}
// </parseArgumentList>

// <parseArgumentsWithExpectedType>
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoDataType expectedType, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments with expected type...");

    if (currentToken.type != TOKEN_IDENTIFIER && currentToken.type != TOKEN_INT_LITERAL && currentToken.type != TOKEN_STRING_LITERAL && currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        error("Expected an identifier.", "parseArgumentsWithExpectedType", table, arena);
        return NULL;
    }

    char *argName = strndup(currentToken.start, currentToken.length);
    bool isLiteral = false;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an integer literal");
        expectedType = DATA_TYPE_INT;
        isLiteral = true;
    }
    else if (currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a string literal");
        expectedType = DATA_TYPE_STRING;
        isLiteral = true;
    }
    else if (currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a boolean literal");
        expectedType = DATA_TYPE_BOOLEAN;
        isLiteral = true;
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is not a literal");
        // Resolve the type using the symbol table
        CryoSymbol *symbol = findSymbol(table, argName, arena);
        expectedType = symbol ? symbol->valueType : DATA_TYPE_UNKNOWN;
        isLiteral = false;
    }

    // Consume the argument name
    getNextToken(lexer, arena);

    logMessage("INFO", __LINE__, "Parser", "Creating argument node with expected type: %s", CryoDataTypeToString(expectedType));
    logMessage("INFO", __LINE__, "Parser", "Argument name: %s", strdup(argName));

    return createArgsNode(argName, expectedType, isLiteral, arena);
}
// </parseArgumentsWithExpectedType>

// <addParameterToList>
void addParameterToList(CryoSymbolTable *table, ASTNode *paramListNode, ASTNode *param, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Adding parameter to list...");
    if (paramListNode->metaData->type == NODE_PARAM_LIST)
    {
        ParamNode *paramList = paramListNode->data.paramList;
        if (paramList->paramCount >= paramList->paramCapacity)
        {
            paramList->paramCapacity *= 2;
            paramList->params = (CryoVariableNode **)realloc(paramList->params, paramList->paramCapacity * sizeof(CryoVariableNode *));
            if (!paramList->params)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for parameters.");
                return;
            }
        }

        paramList->params[paramList->paramCount++] = param->data.varDecl;
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected parameter list node.");
    }
}
// </addParameterToList>

// <addArgumentToList>
void addArgumentToList(CryoSymbolTable *table, ASTNode *argListNode, ASTNode *arg, Arena *arena)
{
    printf("[Parser] Adding argument to list\n");
    if (argListNode->metaData->type == NODE_ARG_LIST)
    {
        ArgNode *argList = argListNode->data.argList;
        if (argList->argCount >= argList->argCapacity)
        {
            argList->argCapacity *= 2;
            argList->args = (CryoVariableNode **)realloc(argList->args, argList->argCapacity * sizeof(CryoVariableNode *));
            if (!argList->args)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for arguments.");
                return;
            }
        }

        argList->args[argList->argCount++] = arg->data.varDecl;
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected argument list node.");
    }
}
// </addArgumentToList>

// <addArgumentToFunctionCall>
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode *functionCallNode, ASTNode *arg, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Adding argument to function call...");
    if (functionCallNode->metaData->type == NODE_FUNCTION_CALL)
    {
        FunctionCallNode *funcCall = functionCallNode->data.functionCall;
        if (funcCall->argCount >= funcCall->argCapacity)
        {
            funcCall->argCapacity *= 2;
            funcCall->args = (ASTNode **)realloc(funcCall->args, funcCall->argCapacity * sizeof(ASTNode *));
            if (!funcCall->args)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for arguments.");
                return;
            }
        }

        funcCall->args[funcCall->argCount++] = arg;
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected function call node.");
    }
}
// </addArgumentToFunctionCall>

// <addParameterToExternDecl>
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode *externDeclNode, ASTNode *param, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Adding parameter to extern declaration...");
    if (externDeclNode->metaData->type == NODE_EXTERN_FUNCTION)
    {
        ExternNode *externDecl = externDeclNode->data.externNode;
        if (externDecl->externNode->data.functionDecl->paramCount >= externDecl->externNode->data.functionDecl->paramCapacity)
        {
            externDecl->externNode->data.functionDecl->paramCapacity *= 2;
            externDecl->externNode->data.functionDecl->params = (ASTNode **)realloc(externDecl->externNode->data.functionDecl->params, externDecl->externNode->data.functionDecl->paramCapacity * sizeof(ASTNode **));
            if (!externDecl->externNode->data.functionDecl->params)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for parameters.");
                return;
            }

            externDecl->externNode->data.functionDecl->params[externDecl->externNode->data.functionDecl->paramCount++] = param;

            logMessage("INFO", __LINE__, "Parser", "Parameter added to extern declaration.");
        }

        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Expected extern declaration node.");
        }
    }
}
// </addParameterToExternDecl>

/* ====================================================================== */
/* @ASTNode_Parsing - Modules & Externals                                 */

// <parseImport>
ASTNode *parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing import...");
    consume(lexer, TOKEN_KW_IMPORT, "Expected `import` keyword.", "parseImport", table, arena);

    if (currentToken.type != TOKEN_STRING_LITERAL)
    {
        error("Expected a string literal", "parseImport", table, arena);
        return NULL;
    }

    char *moduleName = strndup(currentToken.start + 1, currentToken.length - 2);
    getNextToken(lexer, arena);

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImport", table, arena);
    return createImportNode(moduleName, arena);
}
// </parseImport>

// <parseExtern>
ASTNode *parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern...");
    consume(lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", table, arena);

    switch (currentToken.type)
    {
    case TOKEN_KW_FN:
        return parseExternFunctionDeclaration(lexer, table, context, arena);

    default:
        error("Expected an extern declaration.", "parseExtern", table, arena);
        return NULL;
    }

    return NULL;
}
// </parseExtern>

/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode *parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing if statement...");
    consume(lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", table, arena);
    context->isParsingIfCondition = true;

    ASTNode *condition = parseExpression(lexer, table, context, arena);
    ASTNode *ifBlock = parseBlock(lexer, table, context, arena);
    ASTNode *elseBlock = NULL;

    if (currentToken.type == TOKEN_KW_ELSE)
    {
        getNextToken(lexer, arena);
        if (currentToken.type == TOKEN_KW_IF)
        {
            elseBlock = parseIfStatement(lexer, table, context, arena);
        }
        else
        {
            elseBlock = parseBlock(lexer, table, context, arena);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock, arena);
}
// </parseIfStatement>

// <parseForLoop>
ASTNode *parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing for loop...");
    consume(lexer, TOKEN_KW_FOR, "Expected `for` keyword.", "parseForLoop", table, arena);

    consume(lexer, TOKEN_LPAREN, "Expected `(` to start for loop.", "parseForLoop", table, arena);

    ASTNode *init = parseStatement(lexer, table, context, arena);
    ASTNode *condition = parseExpression(lexer, table, context, arena);
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table, arena);

    ASTNode *update = parseExpression(lexer, table, context, arena);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", table, arena);

    ASTNode *body = parseBlock(lexer, table, context, arena);
    return createForStatement(init, condition, update, body, arena);
}
// </parseForLoop>

// <parseWhileStatement>
ASTNode *parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing while statement...");
    consume(lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", table, arena);
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", table, arena);

    ASTNode *condition = parseExpression(lexer, table, context, arena);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", table, arena);

    ASTNode *body = parseBlock(lexer, table, context, arena);
    return createWhileStatement(condition, body, arena);
}
// </parseWhileStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode *parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing array literal...");
    consume(lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", table, arena);

    ASTNode *elements = createArrayLiteralNode(arena);
    if (elements == NULL)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RBRACKET)
    {
        ASTNode *element = parseExpression(lexer, table, context, arena);
        if (element)
        {
            addElementToArrayLiteral(table, elements, element, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse array element.");
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena);
        }
    }

    consume(lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", table, arena);
    return elements;
}
// </parseArrayLiteral>

// <addElementToArrayLiteral>
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode *arrayLiteral, ASTNode *element, Arena *arena)
{
    logMessage("INFO", __LINE__, "Parser", "Adding element to array literal...");
    if (arrayLiteral->metaData->type == NODE_ARRAY_LITERAL)
    {
        if (arrayLiteral->data.array->elementCount >= arrayLiteral->data.array->elementCapacity)
        {
            int newCapacity = arrayLiteral->data.array->elementCapacity * 2;
            if (newCapacity == 0)
                newCapacity = 6; // Handle the case when capacity is 0

            ASTNode **newElements = (ASTNode **)realloc(arrayLiteral->data.array->elements, newCapacity * sizeof(ASTNode *));
            if (!newElements)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for array elements.");
                return;
            }

            arrayLiteral->data.array->elements = newElements;
            arrayLiteral->data.array->elementCapacity = newCapacity;
        }

        arrayLiteral->data.array->elements[arrayLiteral->data.array->elementCount++] = element;
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected array literal node.");
    }
}
// <addElementToArrayLiteral>

/* =========================================================== */
/* @DEBUG | Used to debug the parser in a different executable */
int parser(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "[Parser] Usage: %s <path_to_file>\n", argv[0]);
        return 1;
    }

    char *source = readFile(argv[1]);
    if (source == NULL)
        return 1;

    Lexer lexer;
    initLexer(&lexer, source);

    Arena *arena;

    ASTNode *program = parseProgram(&lexer, NULL, arena);
    if (!program)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to parse program\n");
        free(source);
        return 1;
    }

    printAST(program, 0, arena);

    free(source);
    return 0;
}
