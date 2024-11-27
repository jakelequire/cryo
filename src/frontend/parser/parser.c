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
#include "frontend/parser.h"

/* ====================================================================== */
/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */

// <parseProgram>
ASTNode *parseProgram(Lexer *lexer, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing program...");

    ParsingContext context = {
        false,
        0,
        "default",
        NULL,
        {TOKEN_UNKNOWN}};

    ASTNode *program = createProgramNode(arena, state, typeTable);
    if (!program)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create program node");
        return NULL;
    }

    getNextToken(lexer, arena, state, typeTable);

    logMessage("INFO", __LINE__, "Parser", "Parsing statements...");

    while (lexer->currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, table, &context, arena, state, typeTable);
        if (statement)
        {
            // traverseAST(statement, table);
            addStatementToProgram(program, table, statement, arena, state, typeTable);
            logMessage("INFO", __LINE__, "Parser", "Statement added to program");
            printTypeTable(typeTable);
            if (statement->metaData->type == NODE_NAMESPACE)
            {
                // Initialize the `this`context to the namespace
                setDefaultThisContext(statement->data.cryoNamespace->name, &context, typeTable);
            }
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse statement.");
            parsingError("Failed to parse statement.", "parseProgram", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
        logMessage("INFO", __LINE__, "Parser", "Next token after statement: %s", CryoTokenToString(lexer->currentToken.type));
    }

    return program;
}
// </parseProgram>

/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(int line, Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    logMessage("INFO", line, "Parser", "Consuming Token: %s", CryoTokenToString(type));

    // pushCallStack(&callStack, functionName, lexer->currentToken.line);

    addTokenToContext(context, lexer->currentToken);

    if (lexer->currentToken.type == type)
    {
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError((char *)message, (char *)functionName, table, arena, state, lexer, lexer->source, typeTable);
    }

    debugCurrentToken(lexer, arena, state, typeTable);
}
// </consume>

// <getNextToken>
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    // printf("[Parser] @getNextToken | Current Token before: Type=%d, Start=%.*s, Length=%d\n", lexer->currentToken.type, lexer->currentToken.length, lexer->currentToken.start, lexer->currentToken.length);
    // logMessage("INFO", __LINE__, "Parser", "Current Token before: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(lexer->currentToken.type), lexer->currentToken.length, lexer->currentToken.start, lexer->currentToken.length);
    if (isAtEnd(lexer, state))
    {
        lexer->currentToken.type = TOKEN_EOF;
        lexer->currentToken.start = lexer->current;
        lexer->currentToken.length = 0;
        lexer->currentToken.line = lexer->line;
        lexer->currentToken.column = lexer->column;
        return;
    }
    Token *nextToken = (Token *)malloc(sizeof(Token));
    *nextToken = get_next_token(lexer, state);
    lexer->currentToken = *nextToken;
    // logMessage("INFO", __LINE__, "Parser", "Current Token after: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(lexer->currentToken.type), lexer->currentToken.length, lexer->currentToken.start, lexer->currentToken.length);
}
// </getNextToken>

// <peekNextUnconsumedToken>
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    if (isAtEnd(lexer, state))
    {
        return lexer->currentToken;
    }
    Token nextToken = peekNextToken(lexer, state);
    return nextToken;
}
// </peekNextUnconsumedToken>

// <getNamespaceName>
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    char *namespaceName = NULL;
    if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError("Expected a namespace name", "getNamespaceName", NULL, arena, state, lexer, lexer->source, typeTable);
    }
    return namespaceName;
}
// </getNamespaceName>

/* ====================================================================== */
/* @DataType_Management                                                   */

// <getCryoDataType>
DataType *getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state, Lexer *lexer, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Getting data typestring: %s", typeStr);
    DataType *type = parseDataType(typeStr, typeTable);
    if (!type)
    {
        parsingError("Unknown data type", "getCryoDataType", NULL, arena, state, lexer, lexer->source, typeTable);
    }

    logMessage("INFO", __LINE__, "Parser", "Data type: %s", DataTypeToString(type));
    return type;
}
// </getCryoDataType>

// <parseType>
DataType *parseType(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing type...");
    DataType *type = NULL;

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_VOID:
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
    case TOKEN_KW_BOOL:
        type = getCryoDataType(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, typeTable);
        break;

    case TOKEN_IDENTIFIER:
        // This is a custom type such as a struct or enum (TODO: Implement)
        type = getCryoDataType(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, typeTable);
        break;

    default:
        parsingError("Expected a type identifier", "getNextToken", table, arena, state, lexer, lexer->source, typeTable);
        break;
    }

    logMessage("INFO", __LINE__, "Parser", "Parsed type: %s", DataTypeToString(type));
    return type;
}
// </parseType>

// *NEW*
TypeContainer *parseTypeIdentifier(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    TypeContainer *type = (TypeContainer *)ARENA_ALLOC(arena, sizeof(TypeContainer));
    type->isArray = false;
    type->arrayDimensions = 0;

    // Get the base type name
    char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);

    // Check if it's a primitive type
    if (isPrimitiveType(typeName))
    {
        type->baseType = PRIMITIVE_TYPE;
        type->primitive = getPrimativeTypeFromString(typeName);
    }
    else
    {
        // Look up custom type
        type->baseType = STRUCT_TYPE; // or other custom type
        type->custom.name = typeName;

        // TODO: Implement `lookupStructType` function
        // type->custom.structDef = lookupStructType(table, typeName);
    }

    // Handle array dimensions
    while (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_LBRACKET)
    {
        type->isArray = true;
        type->arrayDimensions++;

        // Skip '[' and ']'
        getNextToken(lexer, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
    }

    return type;
}

// <getOperatorPrecedence>
int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    printf("[Parser] @getOperatorPrecedence | Operator: %s\n", CryoOperatorToString(type));
    switch (type)
    {
    case OPERATOR_ADD:
    case OPERATOR_SUB:
    case OPERATOR_INCREMENT:
    case OPERATOR_DECREMENT:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 1", CryoOperatorToString(type));
        return 1;
    case OPERATOR_MUL:
    case OPERATOR_DIV:
    case OPERATOR_MOD:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 2", CryoOperatorToString(type));
        return 2;
    case OPERATOR_LT:
    case OPERATOR_GT:
    case OPERATOR_LTE:
    case OPERATOR_GTE:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 3", CryoOperatorToString(type));
        return 3;
    case OPERATOR_EQ:
    case OPERATOR_NEQ:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 4", CryoOperatorToString(type));
        return 4;
    case OPERATOR_AND:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 5", CryoOperatorToString(type));
        return 5;
    case OPERATOR_OR:
        logMessage("INFO", __LINE__, "Parser", "Operator: %s, Precedence: 6", CryoOperatorToString(type));
        return 6;
    default:
        return 0;
    }
}
// </getOperatorPrecedence>

/* ====================================================================== */
/* @Parser_Management                                                     */

// <addStatementToProgram>
void addStatementToProgram(ASTNode *programNode, CryoSymbolTable *table, ASTNode *statement, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    char *curModule = getCurrentNamespace(table);
    if (!curModule)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to get current module");
        exit(EXIT_FAILURE);
    }
    logMessage("INFO", __LINE__, "Parser", "(%s) Adding statement to program...", curModule);
    if (!programNode || programNode->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    CryoProgram *program = programNode->data.program;

    logMessage("INFO", __LINE__, "Parser", "(%s) Before adding statement: statementCount = %zu, statementCapacity = %zu",
               curModule, program->statementCount, program->statementCapacity);

    if (program->statementCount >= program->statementCapacity)
    {
        program->statementCapacity = (program->statementCapacity > 0) ? (program->statementCapacity * 2) : 1;
        program->statements = (ASTNode **)realloc(program->statements, sizeof(ASTNode *) * program->statementCapacity);
        if (!program->statements)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to reallocate memory for program statements");
            exit(EXIT_FAILURE);
            return;
        }
    }

    program->statements[program->statementCount++] = statement;

    logMessage("INFO", __LINE__, "Parser", "(%s) After adding statement: statementCount = %zu, statementCapacity = %zu",
               curModule, program->statementCount, program->statementCapacity);
}
// </addStatementToProgram>

/* ====================================================================== */
/* @ASTNode_Parsing - Expressions & Statements                            */

// <parseStatement>
ASTNode *parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing statement...");

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_PUBLIC:
        return parsePublicDeclaration(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_PRIVATE:
        return parsePrivateDeclaration(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PUBLIC, arena, state, typeTable);

    case TOKEN_KW_RETURN:
        return parseReturnStatement(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_FOR:
        return parseForLoop(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_IMPORT:
        return parseImport(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_EXTERN:
        logMessage("INFO", __LINE__, "Parser", "Parsing extern declaration...");
        return parseExtern(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_STRUCT:
        return parseStructDeclaration(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_DEBUGGER:
        parseDebugger(lexer, table, context, arena, state, typeTable);
        return NULL;

    case TOKEN_KW_THIS:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing `this` context...");
        return parseThisContext(lexer, table, context, arena, state, typeTable);
    }

    case TOKEN_IDENTIFIER:
        if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_LPAREN)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing function call...");
            char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            return parseFunctionCall(lexer, table, context, functionName, arena, state, typeTable);
        }
        if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_DOUBLE_COLON)
        {
            printf("Scope Call Token String: %.*s\n", lexer->currentToken.length, lexer->currentToken.start);
            logMessage("INFO", __LINE__, "Parser", "Parsing Scope Call...");
            return parseScopeCall(lexer, table, context, arena, state, typeTable);
        }
        else
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing identifier...");
            return parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
        }

    case TOKEN_KW_NAMESPACE:
        return parseNamespace(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_IF:
        return parseIfStatement(lexer, table, context, arena, state, typeTable);

    case TOKEN_KW_WHILE:
        return parseWhileStatement(lexer, table, context, arena, state, typeTable);

    case TOKEN_EOF:
        return NULL;

    default:
        parsingError("Expected a statement", "parseStatement", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
}
// </parseStatement>

// <parseScopeCall>
ASTNode *parseScopeCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing scope call...");

    char *scopeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("Scope Name: %s\n", scopeName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier", "parseScopeCall", table, arena, state, typeTable, context);

    // Consume the double colon
    consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected a double colon", "parseScopeCall", table, arena, state, typeTable, context);

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("Function Name: %s\n", functionName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier", "parseScopeCall", table, arena, state, typeTable, context);

    // Check the symbol table for the identifier and find what kind of symbol it is.
    CryoSymbol *symbol = findSymbol(table, functionName, arena);
    if (!symbol)
    {
        parsingError("Symbol not found", "parseScopeCall", table, arena, state, lexer, lexer->source, typeTable);
    }
    logMessage("INFO", __LINE__, "Parser", "Symbol found: %s", symbol->name);

    CryoNodeType nodeType = symbol->nodeType;
    printf("Node Type: %s\n", CryoNodeTypeToString(nodeType));
    ASTNode *node = NULL;
    switch (nodeType)
    {
    case NODE_FUNCTION_DECLARATION:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing function call @parseScopeCall...");
        node = parseScopedFunctionCall(lexer, table, context, arena, state, strdup(functionName), scopeName, typeTable);
        break;
    }
    default:
        CONDITION_FAILED;
        break;
    }

    logMessage("INFO", __LINE__, "Parser", "Scope call parsed.");
    return node;
}
// </parseScopeCall>

ASTNode *parseScopedFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing scoped function call...");

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected a left parenthesis", "parseScopedFunctionCall", table, arena, state, typeTable, context);

    ASTNode *node = createScopedFunctionCall(arena, state, strdup(functionName), typeTable);

    // Get the arguments
    int argCount = 0;
    ASTNode **args = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *arg = parseExpression(lexer, table, context, arena, state, typeTable);
        if (arg)
        {
            args[argCount++] = arg;
        }
        else
        {
            parsingError("Failed to parse argument", "parseScopedFunctionCall", table, arena, state, lexer, lexer->source, typeTable);
        }

        if (lexer->currentToken.type != TOKEN_RPAREN)
        {
            consume(__LINE__, lexer, TOKEN_COMMA, "Expected a comma", "parseScopedFunctionCall", table, arena, state, typeTable, context);
        }
    }

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected a right parenthesis", "parseScopedFunctionCall", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseScopedFunctionCall", table, arena, state, typeTable, context);

    node->data.scopedFunctionCall->args = args;
    node->data.scopedFunctionCall->argCount = argCount;
    node->data.scopedFunctionCall->scopeName = strdup(scopeName);

    printAST(node, 0, arena);

    logMessage("INFO", __LINE__, "Parser", "Scoped function call parsed.");

    return node;
}

/// @brief This function handles the `debugger` keyword. Which is used to pause the program execution.
/// Note: This only stops the program on the parser side, not the runtime.
void parseDebugger(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing debugger statement...");
    consume(__LINE__, lexer, TOKEN_KW_DEBUGGER, "Expected 'debugger' keyword.", "parseDebugger", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseDebugger", table, arena, state, typeTable, context);
    logMessage("INFO", __LINE__, "Parser", "Debugger statement parsed.");

    DEBUG_BREAKPOINT;
    return;
}

ASTNode *parseNamespace(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing namespace...");
    // Equivalent to `package <name>` like in Go.
    consume(__LINE__, lexer, TOKEN_KW_NAMESPACE, "Expected 'namespace' keyword.", "parseNamespace", table, arena, state, typeTable, context);

    ASTNode *node = NULL;

    char *namespaceName = NULL;
    if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        node = createNamespaceNode(namespaceName, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError("Expected a namespace name", "parseNamespace", table, arena, state, lexer, lexer->source, typeTable);
    }

    setNamespace(table, namespaceName);
    addASTNodeSymbol(table, node, arena);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseNamespace", table, arena, state, typeTable, context);

    return node;
}

// <parsePrimaryExpression>
ASTNode *parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing primary expression...");
    logMessage("INFO", __LINE__, "Parser", "Current Token: %s", CryoTokenToString(lexer->currentToken.type));

    ASTNode *node = NULL;
    CryoTokenType prevToken = context->lastTokens[0].type;
    printf("\n@parsePrimaryExpression Previous Token: %s\n\n", CryoTokenToString(prevToken));

    switch (lexer->currentToken.type)
    {
    case TOKEN_INT_LITERAL:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing integer literal");
        node = createIntLiteralNode(atoi(lexer->currentToken.start), arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
        return node;
    }
    case TOKEN_STRING_LITERAL:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing string literal");
        const char *str = (const char *)strndup(lexer->currentToken.start, lexer->currentToken.length);
        printf("String: %s\n", str);
        node = createStringLiteralNode(str, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
        return node;
    }
    case TOKEN_BOOLEAN_LITERAL:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing boolean literal");
        char *booleanValueStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        if (strcmp(booleanValueStr, "true") != 0 && strcmp(booleanValueStr, "false") != 0)
        {
            parsingError("Invalid boolean value", "parsePrimaryExpression", table, arena, state, lexer, lexer->source, typeTable);
        }
        int booleanValue = strcmp(booleanValueStr, "true") == 0 ? 1 : 0;
        node = createBooleanLiteralNode(booleanValue, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
        return node;
    }
    case TOKEN_LBRACKET:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing array literal");
        return parseArrayLiteral(lexer, table, context, arena, state, typeTable);
    }
    case TOKEN_IDENTIFIER:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier expression");
        return parseIdentifierExpression(lexer, table, context, arena, state, typeTable);
    }
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
    case TOKEN_MINUS:
    case TOKEN_BANG:
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing unary expression");
        return parseUnaryExpression(lexer, table, context, arena, state, typeTable);
    }
    default:
    {
        NEW_COMPILER_ERROR(state, "ERROR", "Expected an expression or statement.", "parsePrimaryExpression");
        parsingError("Expected an expression", "parsePrimaryExpression", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    }

    // Check for dot notation after the primary expression
    while (lexer->currentToken.type == TOKEN_DOT)
    {
        node = parseDotNotation(lexer, table, context, arena, state, typeTable);
    }

    // Check for array indexing after an identifier or other primary expression
    while (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing array indexing");
        node = parseArrayIndexing(lexer, table, context, NULL, arena, state, typeTable);
    }
    return node;
}
// </parsePrimaryExpression>

ASTNode *parseIdentifierExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    CryoTokenType prevToken = context->lastTokens[0].type;
    char *curToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable).type;
    printf("\n@parseIdentifierExpression Previous Token: %s\n ", CryoTokenToString(prevToken));
    printf("\n@parseIdentifierExpression Current Token: %s\n\n", curToken);
    printf("\n@parseIdentifierExpression Next Token: %s\n\n", CryoTokenToString(nextToken));

    // Check for dot notation after the primary expression
    if (nextToken == TOKEN_DOT)
    {
        // This can either be a struct field or a function call.
        return parseDotNotation(lexer, table, context, arena, state, typeTable);
    }
    // Peek to see if the next token is `[` for array indexing
    if (nextToken == TOKEN_LBRACKET)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing array indexing for identifier: %s", curToken);
        return parseArrayIndexing(lexer, table, context, NULL, arena, state, typeTable);
    }
    // Peek to see if the next token is `;` for a statement
    if (nextToken == TOKEN_SEMICOLON)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), table, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
        return node;
    }
    // Peek to see if the next token is `)` for a statement
    if (nextToken == TOKEN_RPAREN)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), table, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
        return node;
    }

    // Peek to see if the next token is `=` for assignment
    else if (nextToken == TOKEN_EQUAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing assignment");
        return parseAssignment(lexer, table, context, NULL, arena, state, typeTable);
    }
    // Peek to see if the next token is `(` to start a function call.
    else if (nextToken == TOKEN_LPAREN)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing function call");
        char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        return parseFunctionCall(lexer, table, context, functionName, arena, state, typeTable);
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier, next token: %s", CryoTokenToString(peekNextUnconsumedToken(lexer, arena, state, typeTable).type));
    }
    logMessage("INFO", __LINE__, "Parser", "Parsing identifier");
    // Check to see if it exists in the symbol table as a variable or parameter
    ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), table, arena, state, typeTable);
    getNextToken(lexer, arena, state, typeTable);
    return node;
}

// <parseExpression>
ASTNode *parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression...");

    return parseBinaryExpression(lexer, table, context, 1, arena, state, typeTable);
}
// </parseExpression>

// <parseExpressionStatement>
ASTNode *parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression statement...");

    ASTNode *expression = parseExpression(lexer, table, context, arena, state, typeTable);

    logMessage("INFO", __LINE__, "Parser", "Expression parsed: %s", CryoNodeTypeToString(expression->metaData->type));

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", table, arena, state, typeTable, context);

    return createExpressionStatement(expression, arena, state, typeTable);
}
// </parseExpressionStatement>

// <parseBinaryExpression>
ASTNode *parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing binary expression...");
    ASTNode *left = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
    if (!left)
    {
        parsingError("Expected an expression.", "parseBinaryExpression", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    while (true)
    {
        logMessage("INFO", __LINE__, "Parser", "Current Token: %s", CryoTokenToString(lexer->currentToken.type));
        CryoTokenType opToken = lexer->currentToken.type;

        if (opToken == TOKEN_SEMICOLON || opToken == TOKEN_RPAREN)
        {
            logMessage("INFO", __LINE__, "Parser", "End of expression");
            break;
        }

        printf("Operator: %s\n", CryoTokenToString(opToken));
        CryoOperatorType _op = CryoTokenToOperator(opToken);
        printf("Operator: %s\n", CryoOperatorToString(_op));
        int precedence = getOperatorPrecedence(_op, arena, state, typeTable);
        printf("Precedence: %d\n", precedence);

        if (precedence < minPrecedence)
        {
            logMessage("INFO", __LINE__, "Parser", "End of expression");
            break;
        }

        getNextToken(lexer, arena, state, typeTable); // consume operator

        // Parse the right side with a higher precedence
        ASTNode *right = parseBinaryExpression(lexer, table, context, precedence + 1, arena, state, typeTable);
        if (!right)
        {
            parsingError("Expected an expression on the right side of the operator.", "parseBinaryExpression", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }

        // Create a new binary expression node
        CryoOperatorType op = CryoTokenToOperator(opToken);
        if (op == OPERATOR_NA)
        {
            parsingError("Invalid operator.", "parseBinaryExpression", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }

        ASTNode *newNode = createBinaryExpr(left, right, op, arena, state, typeTable);
        left = newNode;

        logMessage("INFO", __LINE__, "Parser", "Binary expression parsed: %s", CryoNodeTypeToString(newNode->metaData->type));
    }

    logMessage("INFO", __LINE__, "Parser", "Binary expression parsed.");

    return left;
}
// </parseBinaryExpression>

// <parseUnaryExpression>
ASTNode *parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing unary expression...");
    CryoTokenType opToken;
    ASTNode *right;

    if (lexer->currentToken.type == TOKEN_MINUS || lexer->currentToken.type == TOKEN_BANG)
    {
        opToken = lexer->currentToken.type;
        getNextToken(lexer, arena, state, typeTable);
        right = parseUnaryExpression(lexer, table, context, arena, state, typeTable);
        return createUnaryExpr(opToken, right, arena, state, typeTable);
    }
    if (lexer->currentToken.type == TOKEN_INCREMENT || lexer->currentToken.type == TOKEN_DECREMENT)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing unary expression...");
        char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
        printf("<UNARY> Current Token: %s\n", cur_token);
        opToken = lexer->currentToken.type;

        getNextToken(lexer, arena, state, typeTable);
        right = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
        if (!right)
        {
            parsingError("Expected an operand", "parseUnaryExpression", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
    }

    return createUnaryExpr(opToken, right, arena, state, typeTable);
}
// </parseUnaryExpression>

// <parsePublicDeclaration>
ASTNode *parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing public declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", table, arena, state, typeTable, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena, state, typeTable);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PUBLIC, arena, state, typeTable);

    default:
        parsingError("Expected a declaration.", "parsePublicDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
}
// </parsePublicDeclaration>

// <parsePrivateDeclaration>
ASTNode *parsePrivateDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing private declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PRIVATE, "Expected 'private' keyword.", "parsePrivateDeclaration", table, arena, state, typeTable, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena, state, typeTable);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PRIVATE, arena, state, typeTable);

    default:
        parsingError("Expected a declaration.", "parsePrivateDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
}
// </parsePrivateDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode *parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing block...");
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", table, arena, state, typeTable, context);

    context->scopeLevel++;

    ASTNode *block = createBlockNode(arena, state, typeTable);
    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *statement = parseStatement(lexer, table, context, arena, state, typeTable);
        if (statement)
        {
            addStatementToBlock(block, statement, arena, state);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse block");
            return NULL;
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end block.", "parseBlock", table, arena, state, typeTable, context);
    context->scopeLevel--;
    return block;
}
// </parseBlock>

// <parseFunctionBlock>
ASTNode *parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function block...");
    context->scopeLevel++;

    ASTNode *functionBlock = createFunctionBlock(arena, state, typeTable);
    if (!functionBlock)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create function block");
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start function block.", "parseFunctionBlock", table, arena, state, typeTable, context);

    while (lexer->currentToken.type != TOKEN_RBRACE && lexer->currentToken.type != TOKEN_EOF)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing statement... %s", CryoTokenToString(lexer->currentToken.type));
        ASTNode *statement = parseStatement(lexer, table, context, arena, state, typeTable);
        if (statement)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding statement to function block...");
            // DEBUG_ARENA_PRINT(arena);
            // addASTNodeSymbol(table, statement, arena);
            addStatementToFunctionBlock(functionBlock, statement, arena, state);
            if (lexer->currentToken.type == TOKEN_SEMICOLON)
            {
                getNextToken(lexer, arena, state, typeTable);
            }
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse function block");
            return NULL;
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end function block.", "parseFunctionBlock", table, arena, state, typeTable, context);
    context->scopeLevel--;

    logMessage("INFO", __LINE__, "Parser", "Function block parsed.");
    return functionBlock;
}
// </parseFunctionBlock>

/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode *parseVarDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing variable declaration...");
    bool isMutable = lexer->currentToken.type == TOKEN_KW_MUT;
    bool isConstant = lexer->currentToken.type == TOKEN_KW_CONST;
    bool isReference = lexer->currentToken.type == TOKEN_AMPERSAND;

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant)
    {
        getNextToken(lexer, arena, state, typeTable);
    }

    // Parse the variable name
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected variable name.");
        parsingError("[Parser] Expected variable name.", "parseVarDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }
    char *var_name = strndup(lexer->currentToken.start, lexer->currentToken.length);
    getNextToken(lexer, arena, state, typeTable);

    // Parse the variable type
    DataType *dataType = NULL;
    if (lexer->currentToken.type == TOKEN_COLON)
    {
        getNextToken(lexer, arena, state, typeTable);
        char *varType = strndup(lexer->currentToken.start, lexer->currentToken.length);

        dataType = getCryoDataType(varType, arena, state, lexer, typeTable);
        if (!dataType)
        {
            parsingError("[Parser] Unknown data type.", "parseVarDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        }
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError("[Parser] Expected ':' after variable name.", "parseVarDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }

    // Parse the variable initializer
    if (lexer->currentToken.type != TOKEN_EQUAL)
    {
        parsingError("[Parser] Expected '=' after type.", "parseVarDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }
    getNextToken(lexer, arena, state, typeTable);

    // Check if the variable is a reference (Note: This is not yet implemented in the lexer)
    if (lexer->currentToken.type == TOKEN_AMPERSAND)
    {
        isReference = true;
        getNextToken(lexer, arena, state, typeTable);
    }

    // Parse the initializer expression
    ASTNode *initializer = parseExpression(lexer, table, context, arena, state, typeTable);
    if (initializer == NULL)
    {
        parsingError("[Parser] Expected expression after '='.", "parseVarDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }

    if (initializer->metaData->type == NODE_FUNCTION_CALL)
    {
        logMessage("INFO", __LINE__, "Parser", "Function call detected.");
    }
    else
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", table, arena, state, typeTable, context);
    }

    logMessage("INFO", __LINE__, "Parser", "Variable declaration parsed.");

    ASTNode *varDeclNode = createVarDeclarationNode(var_name, dataType, initializer, isMutable, isConstant, isReference, false, arena, state, typeTable);
    if (initializer->metaData->type == NODE_INDEX_EXPR)
    {
        printf("\n\n\n");
        logMessage("INFO", __LINE__, "Parser", "Index expression detected.");
        varDeclNode->data.varDecl->indexExpr = initializer->data.indexExpr;
        varDeclNode->data.varDecl->hasIndexExpr = true;
    }

    addASTNodeSymbol(table, varDeclNode, arena);

    return varDeclNode;
}
// </parseVarDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode *parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", table, arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state, typeTable);

    ASTNode **params = parseParameterList(lexer, table, context, arena, strdup(functionName), state, typeTable);

    for (int i = 0; params[i] != NULL; i++)
    {
        logMessage("INFO", __LINE__, "Parser", "Adding parameter: %s", params[i]->data.varDecl->name);
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage("INFO", __LINE__, "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state, typeTable);
        returnType = parseType(lexer, context, table, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError("Expected `->` for return type.", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", DataTypeToString(returnType));

    // Ensure the next token is `{` for the function block
    if (lexer->currentToken.type != TOKEN_LBRACE)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected `{` to start function block.");
        parsingError("Expected `{` to start function block.", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    // Definition of the function
    ASTNode *functionDefNode = createFunctionNode(visibility, functionName, params, NULL, returnType, arena, state, typeTable);
    if (!functionDefNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create function node.");
        parsingError("Failed to create function node.", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    addASTNodeSymbol(table, functionDefNode, arena);

    // Parse the function block
    ASTNode *functionBlock = parseFunctionBlock(lexer, table, context, arena, state, typeTable);
    if (!functionBlock)
    {
        parsingError("Failed to parse function block.", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    ASTNode *functionNode = createFunctionNode(visibility, functionName, params, functionBlock, returnType, arena, state, typeTable);
    addASTNodeSymbol(table, functionNode, arena);
    return functionNode;
}
// </parseFunctionDeclaration>

// <parseExternFunctionDeclaration>
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", table, arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseExternFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state, typeTable);

    ASTNode **params = parseParameterList(lexer, table, context, arena, strdup(functionName), state, typeTable);
    // get length of params
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage("INFO", __LINE__, "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state, typeTable);
        returnType = parseType(lexer, context, table, arena, state, typeTable);
        getNextToken(lexer, arena, state, typeTable);
    }
    else
    {
        parsingError("Expected `->` for return type.", "parseFunctionDeclaration", table, arena, state, lexer, lexer->source, typeTable);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", DataTypeToString(returnType));
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", table, arena, state, typeTable, context);

    ASTNode *externFunc = createExternFuncNode(functionName, params, returnType, arena, state, typeTable);

    addASTNodeSymbol(table, externFunc, arena);

    return externFunc;
}
// </parseExternFunctionDeclaration>

// <parseFunctionCall>
ASTNode *parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context,
                           char *functionName, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function call...");

    Token token = lexer->currentToken;

    // Check if the function name is a struct declaration.
    // In this case, we have to create a new struct instance and redirect the parser
    // to the struct declaration.
    // if (isStructDeclaration(typeTable, functionName))
    // {
    //     logMessage("INFO", __LINE__, "Parser", "Struct declaration detected.");
    //     return parseStructInstance(functionName, lexer, table, context, arena, state, typeTable);
    // }

    // Create function call node
    ASTNode *functionCallNode = createFunctionCallNode(arena, state, typeTable);
    functionCallNode->data.functionCall->name = strdup(functionName);
    functionCallNode->data.functionCall->argCount = 0;
    functionCallNode->data.functionCall->argCapacity = 8;
    functionCallNode->data.functionCall->args = (ASTNode **)ARENA_ALLOC(arena,
                                                                        functionCallNode->data.functionCall->argCapacity * sizeof(ASTNode *));

    // Look up function in symbol table
    CryoSymbol *funcSymbol = findSymbol(table, functionName, arena);
    if (!funcSymbol)
    {
        logMessage("ERROR", __LINE__, "Parser",
                   "Function not found: %s", functionName);
        parsingError("Function not found.", "parseFunctionCall", table, arena, state,
                     lexer, lexer->source, typeTable);
        return NULL;
    }

    char *functionNameToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionNameToken);

    // Consume function name
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
            "parseFunctionCall", table, arena, state, typeTable, context);

    ASTNode **expectedArgs = (ASTNode **)ARENA_ALLOC(arena, 8 * sizeof(ASTNode *));
    DataType **expectedTypes = (DataType **)ARENA_ALLOC(arena, 8 * sizeof(DataType *));

    // Parse arguments
    CryoNodeType functionType = funcSymbol->node->metaData->type;
    switch (functionType)
    {
    case NODE_EXTERN_FUNCTION:
    {
        ASTNode *externFuncNode = funcSymbol->node;
        logASTNode(externFuncNode);
        ASTNode **params = externFuncNode->data.externFunction->params;
        int paramCount = externFuncNode->data.externFunction->paramCount;
        for (int i = 0; i < paramCount; ++i)
        {
            DataType *expectedType = params[i]->data.param->type;
        }

        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        ASTNode *funcDeclNode = funcSymbol->node;
        logASTNode(funcDeclNode);
        ASTNode **params = funcDeclNode->data.functionDecl->params;
        int paramCount = funcDeclNode->data.functionDecl->paramCount;
        for (int i = 0; i < paramCount; ++i)
        {
            DataType *expectedType = params[i]->data.param->type;
        }

        break;
    }
    default:
    {
        logMessage("ERROR", __LINE__, "Parser",
                   "Invalid function type: %s", CryoNodeTypeToString(functionType));
        parsingError("Invalid function type.", "parseFunctionCall",
                     table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    }

    if (lexer->currentToken.type != TOKEN_LPAREN)
    {
        logMessage("ERROR", __LINE__, "Parser",
                   "Expected '(' after function name, got: %s", CryoTokenToString(lexer->currentToken.type));

        parsingError("Expected '(' after function name.", "parseFunctionCall",
                     table, arena, state, lexer, lexer->source, typeTable);

        CONDITION_FAILED;
    }

    // Consume '('
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected '(' after function name.",
            "parseFunctionCall", table, arena, state, typeTable, context);

    // Parse arguments if any
    if (lexer->currentToken.type != TOKEN_RPAREN)
    {
        for (int i = 0; lexer->currentToken.type != TOKEN_RPAREN; ++i)
        {

            CryoTokenType token = lexer->currentToken.type;
            printf("\n\n Current Token in view: %s\n", CryoTokenToString(token));

            switch (token)
            {
            case TOKEN_INT_LITERAL:
            {
                DataType *expectedType = createPrimitiveIntType();
                ASTNode *arg = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
                addArgumentToFunctionCall(table, functionCallNode, arg, arena, state, typeTable);
                break;
            }

            case TOKEN_STRING_LITERAL:
            {
                char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
                int stringLength = strlen(stringLiteral);
                DataType *expectedType = createPrimitiveStringType(stringLength);
                ASTNode *arg = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
                addArgumentToFunctionCall(table, functionCallNode, arg, arena, state, typeTable);
                break;
            }

            case TOKEN_BOOLEAN_LITERAL:
            {
                DataType *expectedType = createPrimitiveBooleanType();
                ASTNode *arg = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
                addArgumentToFunctionCall(table, functionCallNode, arg, arena, state, typeTable);
                break;
            }

            case TOKEN_IDENTIFIER:
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);
                logMessage("INFO", __LINE__, "Parser", "Identifier: %s", identifier);
                Token nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable);
                logMessage("INFO", __LINE__, "Parser", "Next Token: %s", CryoTokenToString(nextToken.type));
                if (nextToken.type == TOKEN_DOT)
                {
                    logMessage("INFO", __LINE__, "Parser", "Parsing dot notation...");
                    ASTNode *dotNode = parseDotNotation(lexer, table, context, arena, state, typeTable);
                    addArgumentToFunctionCall(table, functionCallNode, dotNode, arena, state, typeTable);
                }
                else if (nextToken.type == TOKEN_LPAREN)
                {
                    logMessage("INFO", __LINE__, "Parser", "Parsing function call...");
                    ASTNode *funcCallNode = parseFunctionCall(lexer, table, context, identifier, arena, state, typeTable);
                    addArgumentToFunctionCall(table, functionCallNode, funcCallNode, arena, state, typeTable);
                }
                else
                {
                    logMessage("INFO", __LINE__, "Parser", "Parsing identifier...");
                    ASTNode *arg = parsePrimaryExpression(lexer, table, context, arena, state, typeTable);
                    addArgumentToFunctionCall(table, functionCallNode, arg, arena, state, typeTable);
                }
                break;
            }

            case TOKEN_KW_THIS:
            {
                if (!context->thisContext)
                {
                    parsingError("Invalid use of `this` keyword.", "parseFunctionCall",
                                 table, arena, state, lexer, lexer->source, typeTable);
                    return NULL;
                }

                // Consume `this` keyword
                consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.",
                        "parseFunctionCall", table, arena, state, typeTable, context);

                if (lexer->currentToken.type == TOKEN_DOT)
                {
                    // Consume `.`
                    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` after `this` keyword.",
                            "parseFunctionCall", table, arena, state, typeTable, context);
                }
                else
                {
                    parsingError("Expected `.` after `this` keyword.", "parseFunctionCall",
                                 table, arena, state, lexer, lexer->source, typeTable);
                    return NULL;
                }

                char *memberName = strndup(lexer->currentToken.start, lexer->currentToken.length);
                int thisContextPropCount = context->thisContext->propertyCount;
                for (int i = 0; i < thisContextPropCount; ++i)
                {
                    PropertyNode *contextProp = context->thisContext->properties[i]->data.property;
                    ASTNode *contextNode = context->thisContext->properties[i];
                    if (strcmp(contextProp->name, memberName) == 0)
                    {
                        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
                                "parseFunctionCall", table, arena, state, typeTable, context);
                        DataType *expectedType = contextProp->type;
                        logDataType(expectedType);
                        ASTNode *arg = contextNode;
                        addArgumentToFunctionCall(table, functionCallNode, arg, arena, state, typeTable);
                        break;
                    }

                    if (i == thisContextPropCount - 1)
                    {
                        parsingError("Property not found in `this` context.", "parseFunctionCall",
                                     table, arena, state, lexer, lexer->source, typeTable);
                        return NULL;
                    }

                    if (lexer->currentToken.type == TOKEN_RPAREN)
                    {
                        break;
                    }
                }
                break;
            }
            }

            // Handle comma between arguments
            if (lexer->currentToken.type == TOKEN_COMMA)
            {
                consume(__LINE__, lexer, TOKEN_COMMA, "Expected comma between arguments.",
                        "parseFunctionCall", table, arena, state, typeTable, context);
            }

            // Handle end of arguments
            if (lexer->currentToken.type == TOKEN_RPAREN)
            {
                break;
            }
        }
    }

    // Final tokens
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected ')' after arguments.",
            "parseFunctionCall", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after function call.",
            "parseFunctionCall", table, arena, state, typeTable, context);

    // Validate argument count
    if (functionCallNode->data.functionCall->argCount != funcSymbol->argCount)
    {
        logMessage("ERROR", __LINE__, "Parser",
                   "Argument count mismatch. Expected: %d, Got: %d",
                   funcSymbol->argCount, functionCallNode->data.functionCall->argCount);
        parsingError("Argument count mismatch.", "parseFunctionCall",
                     table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    return functionCallNode;
}
// </parseFunctionCall>

// <parseReturnStatement>
ASTNode *parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing return statement...");

    if (lexer->currentToken.type == TOKEN_KW_RETURN)
    {
        consume(__LINE__, lexer, TOKEN_KW_RETURN, "Expected `return` keyword.", "parseReturnStatement", table, arena, state, typeTable, context);
    }

    DataType *returnType = createPrimitiveVoidType();
    ASTNode *expression = NULL;
    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        expression = parseExpression(lexer, table, context, arena, state, typeTable);
        printf("[Parser] Parsed return expression\n");
    }

    if (expression)
    {
        logMessage("INFO", __LINE__, "Parser", "Return expression: %s", CryoNodeTypeToString(expression->metaData->type));
        if (expression->metaData->type == NODE_LITERAL_EXPR)
        {
            returnType = expression->data.literal->type;
            printf("[Parser] Return expression data type: %s\n", DataTypeToString(returnType));
        }
        if (expression->metaData->type == NODE_BINARY_EXPR)
        {
            returnType = createPrimitiveIntType();
            printf("[Parser] Return expression data type: %s\n", DataTypeToString(returnType));
        }
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "No return expression.");
        returnType = createPrimitiveVoidType();
    }

    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseReturnStatement", table, arena, state, typeTable, context);
    }

    ASTNode *returnNode = createReturnNode(expression, returnType, arena, state, typeTable);
    logMessage("INFO", __LINE__, "Parser", "Return statement parsed.");
    return returnNode;
}
// </parseReturnStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

// <parseParameter>
ASTNode *parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter...");

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseParameter", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);

    getNextToken(lexer, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", table, arena, state, typeTable, context);

    DataType *paramType = parseType(lexer, context, table, arena, state, typeTable);
    // consume data type:
    getNextToken(lexer, arena, state, typeTable);
    ASTNode *node = createParamNode(strdup(paramName), strdup(functionName), paramType, arena, state, typeTable);
    addASTNodeSymbol(table, node, arena);
    return node;
}
// </parseParameter>

// <parseParameterList>
ASTNode **parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter list...");
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start parameter list.", "parseParameterList", table, arena, state, typeTable, context);

    ASTNode **paramListNode = (ASTNode **)ARENA_ALLOC(arena, 8 * sizeof(ASTNode *));
    if (!paramListNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to allocate memory for parameter list.");
        return NULL;
    }

    int paramCount = 0;
    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *param = parseParameter(lexer, table, context, arena, functionName, state, typeTable);
        if (param)
        {
            if (param->metaData->type == NODE_PARAM)
            {
                logMessage("INFO", __LINE__, "Parser", "Adding parameter: %s", param->data.param->name);
                paramListNode[paramCount] = param;
                paramCount++;
            }
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse parameter.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
        }
    }

    paramListNode[paramCount] = NULL; // Null terminate the parameter array

    logMessage("INFO", __LINE__, "Parser", "Parameter count: %d", paramCount);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end parameter list.", "parseParameterList", table, arena, state, typeTable, context);

    return paramListNode;
}
// </parseParameterList>

// <parseArguments>
ASTNode *parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments...");

    Token currentToken = lexer->currentToken;

    if (currentToken.type != TOKEN_IDENTIFIER &&
        currentToken.type != TOKEN_INT_LITERAL &&
        currentToken.type != TOKEN_STRING_LITERAL &&
        currentToken.type != TOKEN_BOOLEAN_LITERAL &&
        currentToken.type != TOKEN_KW_THIS)
    {
        parsingError("Expected an identifier.", "parseArguments", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *argName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    bool isLiteral = false;
    CryoNodeType nodeType = NODE_UNKNOWN;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an integer literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a string literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a boolean literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_KW_THIS)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is `this` keyword");
        nodeType = NODE_THIS;
        return parseThisContext(lexer, table, context, arena, state, typeTable);
    }
    else if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an identifier");
        nodeType = NODE_VAR_DECLARATION;
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is not a literal");
    }

    // Resolve the type using the symbol table
    CryoSymbol *symbol = findSymbol(table, argName, arena);
    DataType *argType = symbol->type;

    // Consume the argument name
    getNextToken(lexer, arena, state, typeTable);

    return createArgsNode(argName, argType, nodeType, isLiteral, arena, state, typeTable);
}
// </parseArguments>

// <parseArgumentList>
ASTNode *parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing argument list...");
    ASTNode *argListNode = createArgumentListNode(arena, state, typeTable);
    if (argListNode == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create argument list node.");
        return NULL;
    }

    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *arg = parseArguments(lexer, table, context, arena, state, typeTable);
        if (arg)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding argument to list...");
            addArgumentToList(table, argListNode, arg, arena, state, typeTable);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse argument.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
        }
    }

    return argListNode;
}
// </parseArgumentList>

// <parseArgumentsWithExpectedType>
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, DataType *expectedType, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    if (lexer->currentToken.type == TOKEN_LPAREN)
    {
        consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start arguments.", "parseArgumentsWithExpectedType", table, arena, state, typeTable, context);
    }
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments with expected type...");
    if (
        lexer->currentToken.type != TOKEN_IDENTIFIER &&
        lexer->currentToken.type != TOKEN_KW_THIS &&
        lexer->currentToken.type != TOKEN_INT_LITERAL &&
        lexer->currentToken.type != TOKEN_STRING_LITERAL &&
        lexer->currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected an identifier, got: %s", CryoTokenToString(lexer->currentToken.type));
        parsingError("Expected an identifier.", "parseArgumentsWithExpectedType", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    VALIDATE_TYPE(expectedType);

    char *argName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    bool usingDotNotation = false;
    bool isLiteral = false;
    CryoNodeType nodeType = NODE_UNKNOWN;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an integer literal");
        expectedType = createPrimitiveIntType();
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a string literal");
        char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
        int stringLength = strlen(stringLiteral);
        expectedType = createPrimitiveStringType(stringLength);
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;

        // Trim the quotes from the string literal
        argName = strndup(lexer->currentToken.start + 1, lexer->currentToken.length - 2);
    }
    else if (lexer->currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a boolean literal");
        expectedType = createPrimitiveBooleanType();
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is an identifier: %s", argName);
        // Check for dot notation
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable);
        Token prevToken = context->lastTokens[1];
        logMessage("INFO", __LINE__, "Parser", "Prev token: %s", CryoTokenToString(prevToken.type));
        if (nextToken.type == TOKEN_DOT)
        {
            logMessage("INFO", __LINE__, "Parser", "Dot notation detected.");
            usingDotNotation = true;
        }

        if (usingDotNotation)
        {
            logMessage("INFO", __LINE__, "Parser", "Dot notation detected.");
            // Parse through the dot notation
            printFormattedType(expectedType);
            ASTNode *dotExpr = parseDotNotation(lexer, table, context, arena, state, typeTable);
            if (!dotExpr)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to parse dot expression.");
                parsingError("Failed to parse dot expression.", "parseArgumentsWithExpectedType", table, arena, state, lexer, lexer->source, typeTable);
                return NULL;
            }

            logMessage("INFO", __LINE__, "Parser", "Dot expression parsed.");
            return dotExpr;
        }
        else
        {
            logMessage("INFO", __LINE__, "Parser", "Argument is not using dot notation.");
        }

        if (prevToken.type == TOKEN_KW_THIS)
        {
            logMessage("INFO", __LINE__, "Parser", "Argument is a 'this' keyword");
            return parseForThisValueProperty(lexer, expectedType, table, context, arena, state, typeTable);
        }

        nodeType = NODE_VAR_NAME;
        // Try to find the symbol in the symbol table
        CryoSymbol *symbol = findSymbol(table, argName, arena);
        if (!symbol)
        {
            logMessage("ERROR", __LINE__, "Parser", "Symbol not found in the symbol table. < %s >", argName);
            parsingError("Symbol not found in the symbol table.", "parseArgumentsWithExpectedType", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
            return NULL;
        }
        expectedType = symbol->type;
        isLiteral = false;
    }
    else if (lexer->currentToken.type == TOKEN_KW_THIS)
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is a 'this' keyword");
        return parseExpectedTypeArgWithThisKW(lexer, expectedType, table, context, arena, state, typeTable);
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Argument is not a literal");
        // Resolve the type using the symbol table
        CryoSymbol *symbol = findSymbol(table, argName, arena);
        if (!symbol)
        {
            logMessage("ERROR", __LINE__, "Parser", "Symbol not found in the symbol table.");
            parsingError("Symbol not found in the symbol table.", "parseArgumentsWithExpectedType", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
        expectedType = symbol->type;
        logMessage("INFO", __LINE__, "Parser", "Argument type: %s", DataTypeToString(expectedType));
        isLiteral = false;
    }

    // Consume the argument name
    getNextToken(lexer, arena, state, typeTable);

    logMessage("INFO", __LINE__, "Parser", "Creating argument node with expected type: %s", DataTypeToString(expectedType));
    logMessage("INFO", __LINE__, "Parser", "Argument name: %s", strdup(argName));

    return createArgsNode(argName, expectedType, nodeType, isLiteral, arena, state, typeTable);
}
// </parseArgumentsWithExpectedType>

// <parseExpectedTypeArgWithThisKW>
ASTNode *parseExpectedTypeArgWithThisKW(Lexer *lexer, DataType *expectedType, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Argument is a 'this' keyword");
    // We have to check if the this keyword is using dot notation
    Token nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected 'this' keyword.", "parseExpectedTypeArgWithThisKW", table, arena, state, typeTable, context);

    if (nextToken.type == TOKEN_DOT)
    {
        logMessage("INFO", __LINE__, "Parser", "Dot notation detected.");
        consume(__LINE__, lexer, TOKEN_DOT, "Expected '.' after 'this' keyword.", "parseExpectedTypeArgWithThisKW", table, arena, state, typeTable, context);

        // Check the `thisContext` in the parsing context to see if it's inside a struct
        if (context->thisContext == NULL)
        {
            logMessage("ERROR", __LINE__, "Parser", "Expected 'this' keyword to be used inside a struct.");
            parsingError("Expected 'this' keyword to be used inside a struct.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        logMessage("INFO", __LINE__, "Parser", "Accessing struct properties...");
        ASTNode **accessProperties = context->thisContext->properties;
        int propertyCount = context->thisContext->propertyCount;
        logMessage("INFO", __LINE__, "Parser", "Property count: %d", propertyCount);

        // Check the next token identifier and match it with the properties of the struct
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            logMessage("ERROR", __LINE__, "Parser", "Expected identifier after 'this' keyword. Received: %s", CryoTokenToString(lexer->currentToken.type));
            parsingError("Expected identifier after 'this' keyword.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        logMessage("INFO", __LINE__, "Parser", "Property name: %s", strndup(lexer->currentToken.start, lexer->currentToken.length));
        ASTNode *matchedProperty = NULL;
        char *propertyName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        for (int i = 0; i < propertyCount; i++)
        {
            char *accessPropName = strdup(accessProperties[i]->data.property->name);
            logMessage("INFO", __LINE__, "Parser", "Access property name: %s", accessPropName);
            if (strcmp(accessPropName, strdup(propertyName)) == 0)
            {
                expectedType = accessProperties[i]->data.property->type;
                matchedProperty = accessProperties[i];

                logMessage("INFO", __LINE__, "Parser", "Matched property: %s", accessPropName);
                break;
            }
        }

        if (!matchedProperty)
        {
            logMessage("ERROR", __LINE__, "Parser", "Property not found in struct.");
            parsingError("Property not found in struct.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, typeTable);
        if (!propAccessNode)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to create property access node.");
            parsingError("Failed to create property access node.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        VALIDATE_TYPE(expectedType);

        return propAccessNode;
    }

    // If the this keyword is not using dot notation, then it's just a reference to the struct
    // For now though, the compiler will only support dot notation for accessing struct properties
    logMessage("ERROR", __LINE__, "Parser", "Expected dot notation after 'this' keyword.");
    parsingError("Expected dot notation after 'this' keyword.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
}
// </parseExpectedTypeArgWithThisKW>

// <addParameterToList>
void addParameterToList(CryoSymbolTable *table, ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state, TypeTable *typeTable)
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
        param->data.varDecl->isMutable = true;
        paramList->params[paramList->paramCount++] = param->data.varDecl;
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected parameter list node.");
    }
}
// </addParameterToList>

// <addArgumentToList>
void addArgumentToList(CryoSymbolTable *table, ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state, TypeTable *typeTable)
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
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode *functionCallNode, ASTNode *arg, Arena *arena, CompilerState *state, TypeTable *typeTable)
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
        logMessage("INFO", __LINE__, "Parser", "Added argument to function call node.");
        logASTNode(arg);
    }
    else
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected function call node.");
    }
}
// </addArgumentToFunctionCall>

// <addParameterToExternDecl>
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state, TypeTable *typeTable)
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
ASTNode *parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing import...");
    consume(__LINE__, lexer, TOKEN_KW_IMPORT, "Expected `import` keyword.", "parseImport", table, arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseImport", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    char *moduleName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseImport", table, arena, state, typeTable, context);

    if (lexer->currentToken.type == TOKEN_DOUBLE_COLON)
    {
        getNextToken(lexer, arena, state, typeTable);
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            parsingError("Expected an identifier.", "parseImport", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        char *subModuleName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseImport", table, arena, state, typeTable, context);

        ASTNode *importNode = createImportNode(strdup(moduleName), strdup(subModuleName), arena, state, typeTable);
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImport", table, arena, state, typeTable, context);

        if (strcmp(strdup(moduleName), "std") == 0)
        {
            importNode->data.import->isStdModule = true;
        }

        addASTNodeSymbol(table, importNode, arena);
        importTypeDefinitions(moduleName, subModuleName, table, arena, state, typeTable);
        return importNode;
    }

    ASTNode *importNode = createImportNode(moduleName, NULL, arena, state, typeTable);
    addASTNodeSymbol(table, importNode, arena);
    if (strcmp(moduleName, "std") == 0)
    {
        importNode->data.import->isStdModule = true;
    }

    importTypeDefinitions(moduleName, NULL, table, arena, state, typeTable);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImport", table, arena, state, typeTable, context);
    return importNode;
}
// </parseImport>

// <importTypeDefinitions>
void importTypeDefinitions(const char *module, const char *subModule, CryoSymbolTable *table, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Importing type definitions...");
    char *symbolName = subModule ? concatStrings(module, concatStrings("::", subModule)) : strdup(module);
    char *_subModule = subModule ? strdup(subModule) : NULL;

    CryoSymbol *symbol = findImportedSymbol(table, symbolName, _subModule, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to find imported symbol.");
        return;
    }

    logMessage("INFO", __LINE__, "Parser", "Found imported symbol.");

    bool isStdModule = strcmp(module, "std") == 0;
    if (isStdModule && subModule != NULL)
    {
        const char *filePath = getSTDFilePath(subModule);
        logMessage("INFO", __LINE__, "Parser", "Importing standard module definitions...");
        ASTNode *externRoot = compileForProgramNode(filePath);
        if (!externRoot)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to create external AST tree.");
            return;
        }
        logMessage("INFO", __LINE__, "Parser", "Importing module definitions...");
        importAstTreeDefs(externRoot, table, arena, state);
        logMessage("INFO", __LINE__, "Parser", "Importing module definitions (submodule)...");
        printSymbolTable(table);
        return;
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Importing module definitions...");
        importAstTreeDefs(symbol->node, table, arena, state);
    }

    printSymbolTable(table);

    logMessage("ERROR", __LINE__, "Parser", "Failed to import type definitions.");
    return;
}
// </importTypeDefinitions>

// <parseExtern>
ASTNode *parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern...");
    consume(__LINE__, lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", table, arena, state, typeTable, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_FN:
        return parseExternFunctionDeclaration(lexer, table, context, arena, state, typeTable);

    default:
        parsingError("Expected an extern declaration.", "parseExtern", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    return NULL;
}
// </parseExtern>

/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode *parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing if statement...");
    consume(__LINE__, lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", table, arena, state, typeTable, context);
    context->isParsingIfCondition = true;

    ASTNode *condition = parseIfCondition(lexer, table, context, arena, state, typeTable);
    ASTNode *ifBlock = parseBlock(lexer, table, context, arena, state, typeTable);
    ASTNode *elseBlock = NULL;

    if (lexer->currentToken.type == TOKEN_KW_ELSE)
    {
        getNextToken(lexer, arena, state, typeTable);
        if (lexer->currentToken.type == TOKEN_KW_IF)
        {
            elseBlock = parseIfStatement(lexer, table, context, arena, state, typeTable);
        }
        else
        {
            elseBlock = parseBlock(lexer, table, context, arena, state, typeTable);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock, arena, state, typeTable);
}
// </parseIfStatement>

ASTNode *parseIfCondition(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing if condition...");
    char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("\n\n[Parser] Current token: %s\n\n", cur_token);

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start if condition.", "parseIfCondition", table, arena, state, typeTable, context);

    char *cur_token_cpy = strndup(lexer->currentToken.start, lexer->currentToken.length);

    printf("\n\n[Parser] Current token: %s\n\n", cur_token_cpy);
    ASTNode *condition = parseExpression(lexer, table, context, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end if condition.", "parseIfCondition", table, arena, state, typeTable, context);

    return condition;
}

// <parseForLoop>
ASTNode *parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing for loop...");
    consume(__LINE__, lexer, TOKEN_KW_FOR, "Expected `for` keyword.", "parseForLoop", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start for loop.", "parseForLoop", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_DOLLAR, "Expected `$` to start initilizer with an iterable.", "parseForLoop", table, arena, state, typeTable, context);

    // The current token is <TOKEN_IDENTIFIER> which is the name of the iterable
    char *iterableName = strndup(lexer->currentToken.start, lexer->currentToken.length);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier for the iterable.", "parseForLoop", table, arena, state, typeTable, context);
    // Check the type of for loop were in.
    // For now, the structure is:
    // for($iterable: <type> = <expression>; <condition>; <update>)
    DataType *iterDataType = createUnknownType();
    getNextToken(lexer, arena, state, typeTable);
    char *iterableType = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("\n\nType: %s\n\n", iterableType);
    DataType *dataType = CryoDataTypeStringToType(iterableType);
    if (dataType == NULL || dataType->container->baseType == UNKNOWN_TYPE)
    {
        parsingError("Unknown data type.", "parseForLoop", table, arena, state, lexer, lexer->source, typeTable);
    }
    printf("DataType: %s\n", DataTypeToString(dataType));

    getNextToken(lexer, arena, state, typeTable);
    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` after iterable type.", "parseForLoop", table, arena, state, typeTable, context);

    ASTNode *iterable = parseExpression(lexer, table, context, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table, arena, state, typeTable, context);

    printf("\n\nDataType in ForLoop init: %s\n\n", DataTypeToString(dataType));
    ASTNode *init = createVarDeclarationNode(iterableName, dataType, iterable, false, false, false, true, arena, state, typeTable);

    addASTNodeSymbol(table, init, arena);

    ASTNode *condition = parseExpression(lexer, table, context, arena, state, typeTable);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table, arena, state, typeTable, context);

    char *__curToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("Current Token Going into `parseExpression`: %s\n", __curToken);

    ASTNode *update = parseExpression(lexer, table, context, arena, state, typeTable);
    char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);

    printf("\n\n[Parser] Current token: %s\n\n", cur_token);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", table, arena, state, typeTable, context);

    ASTNode *body = parseBlock(lexer, table, context, arena, state, typeTable);

    return createForStatement(init, condition, update, body, arena, state, typeTable);
}
// </parseForLoop>

// <parseWhileStatement>
ASTNode *parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing while statement...");
    consume(__LINE__, lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", table, arena, state, typeTable, context);

    ASTNode *condition = parseExpression(lexer, table, context, arena, state, typeTable);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", table, arena, state, typeTable, context);

    ASTNode *body = parseBlock(lexer, table, context, arena, state, typeTable);
    return createWhileStatement(condition, body, arena, state, typeTable);
}
// </parseWhileStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode *parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing array literal...");
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", table, arena, state, typeTable, context);

    ASTNode *elements = createArrayLiteralNode(arena, state, typeTable);
    if (elements == NULL)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    int elementCount = 0;
    DataType **elementTypes = (DataType **)calloc(ARRAY_CAPACITY, sizeof(DataType *));

    while (lexer->currentToken.type != TOKEN_RBRACKET)
    {
        ASTNode *element = parseExpression(lexer, table, context, arena, state, typeTable);
        if (element)
        {
            addElementToArrayLiteral(table, elements, element, arena, state, typeTable);
            logMessage("INFO", __LINE__, "Parser", "Element added to array literal.");

            DataType *elType = DataTypeFromNode(element);
            elementTypes[elementCount] = elType;
            logMessage("INFO", __LINE__, "Parser", "Element type: %s", DataTypeToString(elType));

            elementCount++;
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse array element.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
        }
    }

    elements->data.array->elementTypes = elementTypes;
    elements->data.array->elementCount = elementCount;
    elements->data.array->elementCapacity = elementCount;
    elements->data.array->type = wrapArrayType(createArrayTypeContainer(elementTypes[0], elementTypes, elementCount, 0));

    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", table, arena, state, typeTable, context);
    return elements;
}
// </parseArrayLiteral>

// <addElementToArrayLiteral>
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state, TypeTable *typeTable)
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

// <parseArrayIndexing>
ASTNode *parseArrayIndexing(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing array indexing...");
    char *arrName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *arrCpyName = strdup(arrName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseArrayIndexing", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array indexing.", "parseArrayIndexing", table, arena, state, typeTable, context);

    printf("[Parser] Array name: %s\n", arrCpyName);
    ASTNode *arrNode = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));
    // Find the array in the symbol table
    CryoSymbol *symbol = findSymbol(table, arrCpyName, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Array not found.");
        parsingError("Array not found.", "parseArrayIndexing", table, arena, state, lexer, lexer->source, typeTable);
        exit(1);
        return NULL;
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Array found.");
        arrNode = symbol->node;
    }

    ASTNode *index = parseExpression(lexer, table, context, arena, state, typeTable);
    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array indexing.", "parseArrayIndexing", table, arena, state, typeTable, context);
    printf("[Parser] Array name: %s\n", strdup(arrCpyName));
    return createIndexExprNode(strdup(arrCpyName), arrNode, index, arena, state, typeTable);
}
// </parseArrayIndexing>

/* ====================================================================== */
/* @ASTNode_Parsing - Assignments                                         */
ASTNode *parseAssignment(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *varName, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing assignment...");
    char *_varName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *varNameCpy = strdup(_varName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseAssignment", table, arena, state, typeTable, context);
    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for assignment.", "parseAssignment", table, arena, state, typeTable, context);
    logMessage("INFO", __LINE__, "Parser", "Variable name: %s", varNameCpy);

    // Find the variable in the symbol table
    CryoSymbol *symbol = findSymbol(table, varNameCpy, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Variable not found.");
        parsingError("Variable not found.", "parseAssignment", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    ASTNode *oldValue = symbol->node;
    ASTNode *newValue = parseExpression(lexer, table, context, arena, state, typeTable);

    bool isMutable = false;
    CryoNodeType oldType = oldValue->metaData->type;
    switch (oldType)
    {
    case NODE_VAR_DECLARATION:
    {
        logMessage("INFO", __LINE__, "Parser", "Old value is a variable declaration.");
        isMutable = oldValue->data.varDecl->isMutable;
        break;
    }
    case NODE_PARAM:
    {
        logMessage("INFO", __LINE__, "Parser", "Old value is a parameter.");
        isMutable = true;
        break;
    }
    default:
    {
        logMessage("ERROR", __LINE__, "Parser", "Old value is not a variable declaration.");
        parsingError("Old value is not a variable declaration.", "parseAssignment", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    }

    // Check if the symbol is mutable
    if (!isMutable)
    {
        logMessage("ERROR", __LINE__, "Parser", "Variable is not mutable.");
        parsingError("Variable is not mutable.", "parseAssignment", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseAssignment", table, arena, state, typeTable, context);
    printf("\n\nFinished parsing assignment\n");

    ASTNode *assignment = createVarReassignment(strdup(varNameCpy), oldValue, newValue, arena, state, typeTable);
    printf("\n\nAssignment Node Type %s\n", CryoNodeTypeToString(assignment->metaData->type));
    return assignment;
}

ASTNode *parseThisContext(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing `this` context...");
    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.", "parseThisContext", table, arena, state, typeTable, context);

    if (context->thisContext == NULL)
    {
        parsingError("This context not in scope.", "parseThisContext", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    CryoTokenType currentToken = lexer->currentToken.type;

    ASTNode *thisNode;
    if (currentToken == TOKEN_DOT)
    {
        consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseThisContext", table, arena, state, typeTable, context);
        thisNode = parseDotNotation(lexer, table, context, arena, state, typeTable);
    }
    else
    {
        thisNode = createThisNode(arena, state, typeTable);
    }

    // Check if we are setting a property of the `this` context with `=`
    if (lexer->currentToken.type == TOKEN_EQUAL)
    {
        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for property reassignment.", "parseThisContext", table, arena, state, typeTable, context);
        char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);

        ASTNode *newValue = parseExpression(lexer, table, context, arena, state, typeTable);

        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseThisContext", table, arena, state, typeTable, context);

        ASTNode *propReasignment = createPropertyReassignmentNode(thisNode, propName, newValue, arena, state, typeTable);
        return propReasignment;
    }

    logMessage("INFO", __LINE__, "Parser", "Finished parsing `this` context.");
    return thisNode;
}

ASTNode *parseLHSIdentifier(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, DataType *typeOfNode)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing LHS identifier...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseLHSIdentifier", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    printf("\n\n\n\nType of data type:");
    printFormattedType(typeOfNode);
    printf("\n\n\n\n");

    DEBUG_BREAKPOINT;
}

ASTNode *parseDotNotation(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing dot notation...");

    CryoTokenType currentToken = lexer->currentToken.type;

    switch (currentToken)
    {
    case TOKEN_KW_THIS:
    {
        return parseThisContext(lexer, table, context, arena, state, typeTable);
    }
    case TOKEN_IDENTIFIER:
    {
        return parseIdentifierDotNotation(lexer, table, context, arena, state, typeTable);
    }
    default:
    {
        parsingError("Expected `this` keyword.", "parseDotNotation", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseIdentifierDotNotation(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logParsingContext(context);

    // Check the last token in the context
    Token lastToken = context->lastTokens[1]; // We +1 because the last token should be a dot.
    Token currentToken = lexer->currentToken;

    if (lastToken.type == TOKEN_KW_THIS)
    {
        ThisContext *thisContext = context->thisContext;
        char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseIdentifierDotNotation", table, arena, state, typeTable, context);

        // Validate the property name with the struct
        if (thisContext->nodeType == NODE_STRUCT_DECLARATION)
        {
            ASTNode **properties = thisContext->properties;
            int propertyCount = thisContext->propertyCount;

            for (int i = 0; i < propertyCount; i++)
            {
                PropertyNode *property = properties[i]->data.property;
                if (strcmp(property->name, propName) == 0)
                {
                    printf("Property found & Correct: %s\n", propName);
                    return createPropertyAccessNode(properties[i], thisContext->nodeName, arena, state, typeTable);
                }
                else
                {
                    printf("Property not found: %s\n", propName);
                }
            }

            parsingError("Property not found in struct.", "parseIdentifierDotNotation", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
        else
        {
            parsingError("Expected a struct declaration.", "parseIdentifierDotNotation", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
    }
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        // Get the identifier name
        char *identifierName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        printf("Identifier name: %s\n", identifierName);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseIdentifierDotNotation", table, arena, state, typeTable, context);

        // Look up the identifier in the symbol table
        CryoSymbol *symbol = findSymbol(table, identifierName, arena);
        if (!symbol)
        {
            parsingError("Identifier not found.", "parseIdentifierDotNotation", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }

        ASTNode *identifierNode = symbol->node;
        DataType *typeOfNode = DataTypeFromNode(identifierNode);
        DataType *typeFromSymbol = symbol->type;

        VALIDATE_TYPE(typeOfNode);
        VALIDATE_TYPE(typeFromSymbol);

        printf("\n\n\nType of data type:\n");
        // logVerboseDataType(typeOfNode);

        if (lexer->currentToken.type == TOKEN_DOT)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing dot notation with identifier...");
            return parseDotNotationWithType(identifierNode, typeFromSymbol, lexer, table, context, arena, state, typeTable);
        }

        DEBUG_BREAKPOINT;
    }

    char *unexpectedTokenStr = (char *)malloc(256);
    char *tokenStr = CryoTokenToString(lexer->currentToken.type);
    sprintf(unexpectedTokenStr, "Unexpected token in dot notation. Received: %s", tokenStr);
    parsingError(unexpectedTokenStr, "parseIdentifierDotNotation", table, arena, state, lexer, lexer->source, typeTable);
    return NULL;
}

ASTNode *parseDotNotationWithType(ASTNode *object, DataType *typeOfNode, Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing dot notation with type...");
    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseDotNotationWithType", table, arena, state, typeTable, context);
    // The first identifier is the type of the node we are accessing
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier after `.`.", "parseDotNotationWithType", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Prop name: %s", propName);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseDotNotationWithType", table, arena, state, typeTable, context);

    if (typeOfNode->container->baseType == STRUCT_TYPE)
    {
        logMessage("INFO", __LINE__, "Parser", "Type of node is a struct.");
        VALIDATE_TYPE(typeOfNode);

        StructType *structType = typeOfNode->container->custom.structDef;
        logStructType(structType);
        const char *structName = typeOfNode->container->custom.structDef->name;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(object, propName, typeOfNode, lexer, table, context, arena, state, typeTable);
            return methodCallNode;
        }

        logMessage("INFO", __LINE__, "Parser", "Struct name: %s", structName);
        logMessage("INFO", __LINE__, "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        ASTNode *property = findStructProperty(structType, (const char *)propName);
        if (property)
        {
            logMessage("INFO", __LINE__, "Parser", "Property found in struct, name: %s", propName);
            return createStructPropertyAccessNode(object, property, (const char *)propName, typeOfNode, arena, state, typeTable);
        }
        else
        {
            printTypeTable(typeTable);
            printf("Property Attempted: %s\n", propName);
            parsingError("Property not found in struct.", "parseDotNotationWithType", table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseForThisValueProperty(Lexer *lexer, DataType *expectedType, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing for this value property...");
    if (context->thisContext == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected 'this' keyword to be used inside a struct.");
        parsingError("Expected 'this' keyword to be used inside a struct.", "parseForThisValueProperty", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    // This assumes the `this` and `.` tokens have already been consumed,
    ASTNode **accessProperties = context->thisContext->properties;
    int propertyCount = context->thisContext->propertyCount;

    // Check the next token identifier and match it with the properties of the struct
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected identifier after 'this' keyword. Received: %s", CryoTokenToString(lexer->currentToken.type));
        parsingError("Expected identifier after 'this' keyword.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    ASTNode *matchedProperty = NULL;
    char *propertyName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    for (int i = 0; i < propertyCount; i++)
    {
        char *accessPropName = strdup(accessProperties[i]->data.property->name);
        if (strcmp(accessPropName, strdup(propertyName)) == 0)
        {
            expectedType = accessProperties[i]->data.property->type;
            matchedProperty = accessProperties[i];
            break;
        }
    }

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseExpectedTypeArgWithThisKW", table, arena, state, typeTable, context);

    if (!matchedProperty)
    {
        logMessage("ERROR", __LINE__, "Parser", "Property not found in struct.");
        logMessage("ERROR", __LINE__, "Parser", "Property name: %s", propertyName);

        parsingError("Property not found in struct.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, typeTable);
    if (!propAccessNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create property access node.");
        parsingError("Failed to create property access node.", "parseExpectedTypeArgWithThisKW", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    VALIDATE_TYPE(expectedType);

    return propAccessNode;
}
