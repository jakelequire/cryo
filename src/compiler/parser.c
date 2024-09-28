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

void printLine(const char *source, int line, Arena *arena, CompilerState *state)
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
ASTNode *parseProgram(Lexer *lexer, CryoSymbolTable *table, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing program...");

    source = lexer->start;

    ParsingContext context = {
        false,
        0};

    ASTNode *program = createProgramNode(arena, state);
    if (!program)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create program node");
        return NULL;
    }

    getNextToken(lexer, arena, state);

    logMessage("INFO", __LINE__, "Parser", "Parsing statements...");

    while (currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, table, &context, arena, state);
        if (statement)
        {
            // traverseAST(statement, table);
            addStatementToProgram(program, table, statement, arena, state);
            printf("\n\n--------------------------------------------------\n\n");
            printf("Adding Statement: ");
            printAST(statement, 0, arena);
            printf("\n\n--------------------------------------------------\n\n");
            // addASTNodeSymbol(table, statement, arena);
            // printSymbolTable(table, arena);
            dumpCompilerState(*state);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse statement.");
            error("Failed to parse statement.", "parseProgram", table, arena, state);
            return NULL;
        }
    }

    return program;
}
// </parseProgram>

/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state)
{
    logMessage("CRITICAL", __LINE__, "Parser", "Consuming Token: %s", CryoTokenToString(type));

    pushCallStack(&callStack, functionName, currentToken.line);

    if (currentToken.type == type)
    {
        getNextToken(lexer, arena, state);
    }
    else
    {
        error((char *)message, (char *)functionName, table, arena, state);
    }

    debugCurrentToken(arena, state);
}
// </consume>

// <getNextToken>
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state)
{
    // printf("[Parser] @getNextToken | Current Token before: Type=%d, Start=%.*s, Length=%d\n", currentToken.type, currentToken.length, currentToken.start, currentToken.length);
    // logMessage("INFO", __LINE__, "Parser", "Current Token before: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(currentToken.type), currentToken.length, currentToken.start, currentToken.length);
    if (isAtEnd(lexer, state))
    {
        currentToken.type = TOKEN_EOF;
        currentToken.start = lexer->current;
        currentToken.length = 0;
        currentToken.line = lexer->line;
        currentToken.column = lexer->column;
        return;
    }
    currentToken = get_next_token(lexer, state);
    // logMessage("INFO", __LINE__, "Parser", "Current Token after: Type=%s, Start=%.*s, Length=%d", CryoTokenToString(currentToken.type), currentToken.length, currentToken.start, currentToken.length);
}
// </getNextToken>

// <peekNextUnconsumedToken>
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state)
{
    if (isAtEnd(lexer, state))
    {
        return currentToken;
    }
    Lexer tempLexer = *lexer;
    Token nextToken = peekNextToken(&tempLexer, state);
    return nextToken;
}
// </peekNextUnconsumedToken>

// <error>
void error(char *message, char *functionName, CryoSymbolTable *table, Arena *arena, CompilerState *state)
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
    printLine(source, line - 1, arena, state);
    printf("%d ", line);
    printLine(source, line, arena, state);

    for (int i = line; i < column + line; i++)
    {
        printf(" ");
    }

    printf(" ^ %s\n", message);
    printf("\n------------------------------------------------------------------------\n\n");
    freeCallStack(&callStack);
    exit(1);
}
// </error>

// <debugCurrentToken>
void debugCurrentToken(Arena *arena, CompilerState *state)
{
    // printf("[Parser DEBUG] Current Token: %s, Lexeme: %.*s\n",
    //        CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
    logMessage("DEBUG", __LINE__, "Parser", "Current Token: %s, Lexeme: %.*s",
               CryoTokenToString(currentToken.type), currentToken.length, currentToken.start);
}
// </debugCurrentToken>

// <getNamespaceName>
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state)
{
    char *namespaceName = NULL;
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(currentToken.start, currentToken.length);
        getNextToken(lexer, arena, state);
    }
    else
    {
        error("Expected a namespace name", "getNamespaceName", NULL, arena, state);
    }
    return namespaceName;
}
// </getNamespaceName>

/* ====================================================================== */
/* @DataType_Management                                                   */

// <getCryoDataType>
CryoDataType getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Getting data typestring: %s", typeStr);
    CryoDataType type = CryoDataTypeStringToType((char *)typeStr);
    if (type == DATA_TYPE_UNKNOWN)
    {
        error("Unknown data type", "getCryoDataType", NULL, arena, state);
    }

    logMessage("INFO", __LINE__, "Parser", "Data type: %s", CryoDataTypeToString(type));
    return type;
}
// </getCryoDataType>

// <parseType>
CryoDataType parseType(Lexer *lexer, ParsingContext *context, CryoSymbolTable *table, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing type...");
    CryoDataType type = DATA_TYPE_UNKNOWN;

    switch (currentToken.type)
    {
    case TOKEN_KW_VOID:
    case TOKEN_KW_INT:
    case TOKEN_KW_STRING:
    case TOKEN_KW_BOOL:
        type = getCryoDataType(strndup(currentToken.start, currentToken.length), arena, state);
        break;

    case TOKEN_IDENTIFIER:
        type = getCryoDataType(strndup(currentToken.start, currentToken.length), arena, state);
        break;

    default:
        error("Expected a type identifier", "getNextToken", table, arena, state);
        break;
    }

    logMessage("INFO", __LINE__, "Parser", "Parsed type: %s", CryoDataTypeToString(type));
    return type;
}
// </parseType>

// <getOperatorPrecedence>
int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state)
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
void addStatementToProgram(ASTNode *programNode, CryoSymbolTable *table, ASTNode *statement, Arena *arena, CompilerState *state)
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
        program->statements = (ASTNode **)realloc(program->statements, sizeof(ASTNode *) * program->statementCapacity);
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
ASTNode *parseStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing statement...");

    switch (currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena, state);

    case TOKEN_KW_PUBLIC:
        return parsePublicDeclaration(lexer, table, context, arena, state);

    case TOKEN_KW_RETURN:
        return parseReturnStatement(lexer, table, context, arena, state);

    case TOKEN_KW_FOR:
        return parseForLoop(lexer, table, context, arena, state);

    case TOKEN_KW_IMPORT:
        return parseImport(lexer, table, context, arena, state);

    case TOKEN_KW_EXTERN:
        logMessage("INFO", __LINE__, "Parser", "Parsing extern declaration...");
        return parseExtern(lexer, table, context, arena, state);

    case TOKEN_KW_STRUCT:
        return parseStructDeclaration(lexer, table, context, arena, state);

    case TOKEN_KW_DEBUGGER:
        parseDebugger(lexer, table, context, arena, state);
        return NULL;

    case TOKEN_IDENTIFIER:
        if (currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing function call...");
            char *functionName = strndup(currentToken.start, currentToken.length);
            return parseFunctionCall(lexer, table, context, functionName, arena, state);
        }
        else
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing identifier...");
            return parsePrimaryExpression(lexer, table, context, arena, state);
        }

    case TOKEN_KW_NAMESPACE:
        return parseNamespace(lexer, table, context, arena, state);

    case TOKEN_KW_IF:
        return parseIfStatement(lexer, table, context, arena, state);

    case TOKEN_EOF:
        return NULL;

    default:
        error("Expected a statement", "parseStatement", table, arena, state);
        return NULL;
    }
}
// </parseStatement>

/// @brief This function handles the `debugger` keyword. Which is used to pause the program execution.
void parseDebugger(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing debugger statement...");
    consume(lexer, TOKEN_KW_DEBUGGER, "Expected 'debugger' keyword.", "parseDebugger", table, arena, state);
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseDebugger", table, arena, state);
    logMessage("INFO", __LINE__, "Parser", "Debugger statement parsed.");

    DEBUG_BREAKPOINT;
    return;
}

ASTNode *parseNamespace(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing namespace...");
    // Equivalent to `package <name>` like in Go.
    consume(lexer, TOKEN_KW_NAMESPACE, "Expected 'namespace' keyword.", "parseNamespace", table, arena, state);

    ASTNode *node = NULL;

    char *namespaceName = NULL;
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(currentToken.start, currentToken.length);
        node = createNamespaceNode(namespaceName, arena, state);
        getNextToken(lexer, arena, state);
    }
    else
    {
        error("Expected a namespace name", "parseNamespace", table, arena, state);
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseNamespace", table, arena, state);

    return node;
}

// <parsePrimaryExpression>
ASTNode *parsePrimaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing primary expression...");
    logMessage("INFO", __LINE__, "Parser", "Current Token: %s", CryoTokenToString(currentToken.type));

    ASTNode *node = NULL;

    switch (currentToken.type)
    {
    case TOKEN_INT_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing integer literal");
        node = createIntLiteralNode(atoi(currentToken.start), arena, state);
        getNextToken(lexer, arena, state);
        return node;

    case TOKEN_STRING_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing string literal");
        char *str = strndup(currentToken.start + 1, currentToken.length - 2);
        printf("String: %s\n", str);
        node = createStringLiteralNode(str, arena, state);
        getNextToken(lexer, arena, state);
        return node;

    case TOKEN_BOOLEAN_LITERAL:
        logMessage("INFO", __LINE__, "Parser", "Parsing boolean literal");
        char *booleanValueStr = strndup(currentToken.start, currentToken.length);
        if (strcmp(booleanValueStr, "true") != 0 && strcmp(booleanValueStr, "false") != 0)
        {
            error("Invalid boolean value", "parsePrimaryExpression", table, arena, state);
        }
        int booleanValue = strcmp(booleanValueStr, "true") == 0 ? 1 : 0;
        node = createBooleanLiteralNode(booleanValue, arena, state);
        getNextToken(lexer, arena, state);
        return node;

    case TOKEN_LBRACKET:
        logMessage("INFO", __LINE__, "Parser", "Parsing array literal");
        return parseArrayLiteral(lexer, table, context, arena, state);

    case TOKEN_IDENTIFIER:
    {
        // Peek to see if the next token is `[` for array indexing
        if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LBRACKET)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing array indexing");
            return parseArrayIndexing(lexer, table, context, NULL, arena, state);
        }
        // Peek to see if the next token is `=` for assignment
        else if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_EQUAL)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing assignment");
            return parseAssignment(lexer, table, context, NULL, arena, state);
        }
        // Peek to see if the next token is `(` to start a function call.
        else if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing function call");
            char *functionName = strndup(currentToken.start, currentToken.length);
            return parseFunctionCall(lexer, table, context, functionName, arena, state);
        }
        else
        {
            logMessage("INFO", __LINE__, "Parser", "Parsing identifier, next token: %s", CryoTokenToString(peekNextUnconsumedToken(lexer, arena, state).type));
        }
        logMessage("INFO", __LINE__, "Parser", "Parsing identifier");
        // Check to see if it exists in the symbol table as a variable or parameter
        node = createIdentifierNode(strndup(currentToken.start, currentToken.length), table, arena, state);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
    case TOKEN_MINUS:
    case TOKEN_BANG:
        logMessage("INFO", __LINE__, "Parser", "Parsing unary expression");
        return parseUnaryExpression(lexer, table, context, arena, state);

    default:
        error("Expected an expression", "parsePrimaryExpression", table, arena, state);
        return NULL;
    }

    // Check for array indexing after an identifier or other primary expression
    while (currentToken.type == TOKEN_LBRACKET)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing array indexing");
        node = parseArrayIndexing(lexer, table, context, NULL, arena, state);
    }
    return node;
}
// </parsePrimaryExpression>

// <parseExpression>
ASTNode *parseExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression...");

    return parseBinaryExpression(lexer, table, context, 1, arena, state);
}
// </parseExpression>

// <parseExpressionStatement>
ASTNode *parseExpressionStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing expression statement...");

    ASTNode *expression = parseExpression(lexer, table, context, arena, state);

    logMessage("INFO", __LINE__, "Parser", "Expression parsed: %s", CryoNodeTypeToString(expression->metaData->type));

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", table, arena, state);

    return createExpressionStatement(expression, arena, state);
}
// </parseExpressionStatement>

// <parseBinaryExpression>
ASTNode *parseBinaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing binary expression...");
    ASTNode *left = parsePrimaryExpression(lexer, table, context, arena, state);
    if (!left)
    {
        error("Expected an expression.", "parseBinaryExpression", table, arena, state);
        return NULL;
    }

    while (true)
    {
        logMessage("INFO", __LINE__, "Parser", "Current Token: %s", CryoTokenToString(currentToken.type));
        CryoTokenType operator= currentToken.type;
        printf("Operator: %s\n", CryoTokenToString(operator));
        CryoOperatorType _op = CryoTokenToOperator(operator);
        printf("Operator: %s\n", CryoOperatorToString(_op));
        int precedence = getOperatorPrecedence(_op, arena, state);
        printf("Precedence: %d\n", precedence);

        // if (operator== )
        // {
        //     // This means that the literal expression is an array indexing operation
        //     // <TOKEN_RBRACKET> <TOKEN_INT_LITERAL> <TOKEN_LBRACKET> = NODE_INDEX_EXPR
        //     consume()
        // }

        if (precedence < minPrecedence)
        {
            break;
        }

        getNextToken(lexer, arena, state); // consume operator

        // Parse the right side with a higher precedence
        ASTNode *right = parseBinaryExpression(lexer, table, context, precedence + 1, arena, state);
        if (!right)
        {
            error("Expected an expression on the right side of the operator.", "parseBinaryExpression", table, arena, state);
            return NULL;
        }

        // Create a new binary expression node
        CryoOperatorType op = CryoTokenToOperator(operator);
        if (op == OPERATOR_NA)
        {
            error("Invalid operator.", "parseBinaryExpression", table, arena, state);
            return NULL;
        }

        ASTNode *newNode = createBinaryExpr(left, right, op, arena, state);
        left = newNode;

        logMessage("INFO", __LINE__, "Parser", "Binary expression parsed: %s", CryoNodeTypeToString(newNode->metaData->type));
    }

    return left;
}
// </parseBinaryExpression>

// <parseUnaryExpression>
ASTNode *parseUnaryExpression(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing unary expression...");
    CryoTokenType operator;
    ASTNode *right;

    if (currentToken.type == TOKEN_MINUS || currentToken.type == TOKEN_BANG)
    {
        operator= currentToken.type;
        getNextToken(lexer, arena, state);
        right = parseUnaryExpression(lexer, table, context, arena, state);
        return createUnaryExpr(operator, right, arena, state);
    }
    if (currentToken.type == TOKEN_INCREMENT || currentToken.type == TOKEN_DECREMENT)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing unary expression...");
        char *cur_token = strndup(currentToken.start, currentToken.length);
        printf("<UNARY> Current Token: %s\n", cur_token);
        operator= currentToken.type;

        getNextToken(lexer, arena, state);
        right = parsePrimaryExpression(lexer, table, context, arena, state);
        if (!right)
        {
            error("Expected an operand", "parseUnaryExpression", table, arena, state);
            return NULL;
        }
    }

    return createUnaryExpr(operator, right, arena, state);
}
// </parseUnaryExpression>

// <parsePublicDeclaration>
ASTNode *parsePublicDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing public declaration...");
    consume(lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", table, arena, state);

    switch (currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, table, context, arena, state);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, table, context, VISIBILITY_PUBLIC, arena, state);

    default:
        error("Expected a declaration.", "parsePublicDeclaration", table, arena, state);
        return NULL;
    }
}
// </parsePublicDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode *parseBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing block...");
    consume(lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", table, arena, state);

    context->scopeLevel++;

    ASTNode *block = createBlockNode(arena, state);
    while (currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *statement = parseStatement(lexer, table, context, arena, state);
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

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end block.", "parseBlock", table, arena, state);
    context->scopeLevel--;
    return block;
}
// </parseBlock>

// <parseFunctionBlock>
ASTNode *parseFunctionBlock(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function block...");
    context->scopeLevel++;

    ASTNode *functionBlock = createFunctionBlock(arena, state);
    if (!functionBlock)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create function block");
        return NULL;
    }

    consume(lexer, TOKEN_LBRACE, "Expected `{` to start function block.", "parseFunctionBlock", table, arena, state);

    while (currentToken.type != TOKEN_RBRACE && currentToken.type != TOKEN_EOF)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing statement... %s", CryoTokenToString(currentToken.type));
        ASTNode *statement = parseStatement(lexer, table, context, arena, state);
        if (statement)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding statement to function block...");
            // DEBUG_ARENA_PRINT(arena);
            // addASTNodeSymbol(table, statement, arena);
            addStatementToFunctionBlock(functionBlock, statement, arena, state);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse function block");
            return NULL;
        }
    }

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end function block.", "parseFunctionBlock", table, arena, state);
    context->scopeLevel--;

    logMessage("INFO", __LINE__, "Parser", "Function block parsed.");
    return functionBlock;
}
// </parseFunctionBlock>

/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode *parseVarDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing variable declaration...");
    bool isMutable = currentToken.type == TOKEN_KW_MUT;
    bool isConstant = currentToken.type == TOKEN_KW_CONST;
    bool isReference = currentToken.type == TOKEN_AMPERSAND;

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant)
    {
        getNextToken(lexer, arena, state);
    }

    // Parse the variable name
    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected variable name.");
        error("[Parser] Expected variable name.", "parseVarDeclaration", table, arena, state);
    }
    char *var_name = strndup(currentToken.start, currentToken.length);
    getNextToken(lexer, arena, state);

    // Parse the variable type
    CryoDataType dataType = DATA_TYPE_UNKNOWN;
    if (currentToken.type == TOKEN_COLON)
    {
        getNextToken(lexer, arena, state);
        char *varType = strndup(currentToken.start, currentToken.length);

        dataType = getCryoDataType(varType, arena, state);
        if (dataType == DATA_TYPE_UNKNOWN)
        {
            error("[Parser] Unknown data type.", "parseVarDeclaration", table, arena, state);
        }
        getNextToken(lexer, arena, state);
    }
    else
    {
        error("[Parser] Expected ':' after variable name.", "parseVarDeclaration", table, arena, state);
    }

    // Parse the variable initializer
    if (currentToken.type != TOKEN_EQUAL)
    {
        error("[Parser] Expected '=' after type.", "parseVarDeclaration", table, arena, state);
    }
    getNextToken(lexer, arena, state);

    // Check if the variable is a reference (Note: This is not yet implemented in the lexer)
    if (currentToken.type == TOKEN_AMPERSAND)
    {
        isReference = true;
        getNextToken(lexer, arena, state);
    }

    // Parse the initializer expression
    ASTNode *initializer = parseExpression(lexer, table, context, arena, state);
    if (initializer == NULL)
    {
        error("[Parser] Expected expression after '='.", "parseVarDeclaration", table, arena, state);
    }

    if (initializer->metaData->type == NODE_FUNCTION_CALL)
    {
        logMessage("INFO", __LINE__, "Parser", "Function call detected.");
    }
    else
    {
        consume(lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", table, arena, state);
    }

    logMessage("INFO", __LINE__, "Parser", "Variable declaration parsed.");

    ASTNode *varDeclNode = createVarDeclarationNode(var_name, dataType, initializer, isMutable, isConstant, isReference, false, arena, state);
    if (initializer->metaData->type == NODE_INDEX_EXPR)
    {
        printf("\n\n\n");
        logMessage("INFO", __LINE__, "Parser", "Index expression detected.");
        varDeclNode->data.varDecl->indexExpr = initializer;
        varDeclNode->data.varDecl->hasIndexExpr = true;
    }

    addASTNodeSymbol(table, varDeclNode, arena);

    return varDeclNode;
}
// </parseVarDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode *parseFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function declaration...");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", table, arena, state);

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier", "parseFunctionDeclaration", table, arena, state);
        return NULL;
    }

    char *functionName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, table, context, arena, strdup(functionName), state);

    for (int i = 0; params[i] != NULL; i++)
    {
        logMessage("INFO", __LINE__, "Parser", "Adding parameter: %s", params[i]->data.varDecl->name);
    }

    CryoDataType returnType = DATA_TYPE_VOID; // Default return type
    if (currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage("INFO", __LINE__, "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, table, arena, state);
        getNextToken(lexer, arena, state);
    }
    else
    {
        error("Expected `->` for return type.", "parseFunctionDeclaration", table, arena, state);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", CryoDataTypeToString(returnType));

    // Ensure the next token is `{` for the function block
    if (currentToken.type != TOKEN_LBRACE)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected `{` to start function block.");
        error("Expected `{` to start function block.", "parseFunctionDeclaration", table, arena, state);
        return NULL;
    }

    // Definition of the function
    ASTNode *functionDefNode = createFunctionNode(visibility, functionName, params, NULL, returnType, arena, state);
    if (!functionDefNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create function node.");
        error("Failed to create function node.", "parseFunctionDeclaration", table, arena, state);
        return NULL;
    }
    addASTNodeSymbol(table, functionDefNode, arena);

    // Parse the function block
    ASTNode *functionBlock = parseFunctionBlock(lexer, table, context, arena, state);
    if (!functionBlock)
    {
        error("Failed to parse function block.", "parseFunctionDeclaration", table, arena, state);
        return NULL;
    }

    ASTNode *functionNode = createFunctionNode(visibility, functionName, params, functionBlock, returnType, arena, state);
    addASTNodeSymbol(table, functionNode, arena);
    return functionNode;
}
// </parseFunctionDeclaration>

// <parseExternFunctionDeclaration>
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern function declaration...");
    consume(lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", table, arena, state);

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseExternFunctionDeclaration", table, arena, state);
        return NULL;
    }

    char *functionName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, table, context, arena, strdup(functionName), state);
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
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, table, arena, state);
        getNextToken(lexer, arena, state);
    }
    else
    {
        error("Expected `->` for return type.", "parseFunctionDeclaration", table, arena, state);
    }

    logMessage("INFO", __LINE__, "Parser", "Function Return Type: %s", CryoDataTypeToString(returnType));
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", table, arena, state);

    ASTNode *externFunc = createExternFuncNode(functionName, params, returnType, arena, state);

    addASTNodeSymbol(table, externFunc, arena);

    return externFunc;
}
// </parseExternFunctionDeclaration>

// <parseFunctionCall>
ASTNode *parseFunctionCall(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *functionName, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing function call...");
    consume(lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseFunctionCall", table, arena, state);

    ASTNode *functionCallNode = createFunctionCallNode(arena, state);
    functionCallNode->data.functionCall->name = strdup(functionName);
    functionCallNode->data.functionCall->argCount = 0;
    functionCallNode->data.functionCall->argCapacity = 8;
    functionCallNode->data.functionCall->args = (ASTNode **)ARENA_ALLOC(arena, functionCallNode->data.functionCall->argCapacity * sizeof(ASTNode *));

    CryoSymbol *funcSymbol = findSymbol(table, functionName, arena);
    if (!funcSymbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Function not found or is not a function.");
        logMessage("ERROR", __LINE__, "Parser", "Function name: %s", functionName);
        error("Function not found or is not a function.", "parseFunctionCall", table, arena, state);
        return NULL;
    }

    getNextToken(lexer, arena, state); // Consume the function name

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

            ASTNode *arg = parseArgumentsWithExpectedType(lexer, table, context, type, arena, state);
            if (!arg)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to parse argument.");
                error("Expected argument expression.", "parseFunctionCall", table, arena, state);
                return NULL;
            }
            addArgumentToFunctionCall(table, functionCallNode, arg, arena, state);

            if (currentToken.type == TOKEN_COMMA)
            {
                getNextToken(lexer, arena, state); // Consume the comma and continue parsing the next argument
            }
        }
    }

    consume(lexer, TOKEN_RPAREN, "Expected ')' after arguments.", "parseFunctionCall", table, arena, state);
    consume(lexer, TOKEN_SEMICOLON, "Expected ';' after function call.", "parseFunctionCall", table, arena, state);

    // Ensure argument count matches
    if (functionCallNode->data.functionCall->argCount != funcSymbol->argCount)
    {
        logMessage("ERROR", __LINE__, "Parser", "Argument count mismatch for function call.");
        error("Argument count mismatch for function call.", "parseFunctionCall", table, arena, state);
        return NULL;
    }

    logMessage("INFO", __LINE__, "Parser", "Function call parsed.");

    return functionCallNode;
}
// </parseFunctionCall>

// <parseReturnStatement>
ASTNode *parseReturnStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing return statement...");
    consume(lexer, TOKEN_KW_RETURN, "Expected `return` keyword.", "parseReturnStatement", table, arena, state);

    CryoDataType returnType = DATA_TYPE_VOID;
    ASTNode *expression = NULL;
    if (currentToken.type != TOKEN_SEMICOLON)
    {
        expression = parseExpression(lexer, table, context, arena, state);
        printf("[Parser] Parsed return expression\n");
    }

    if (expression)
    {
        logMessage("INFO", __LINE__, "Parser", "Return expression: %s", CryoNodeTypeToString(expression->metaData->type));
        if (expression->metaData->type == NODE_LITERAL_EXPR)
        {
            returnType = expression->data.literal->dataType;
            printf("[Parser] Return expression data type: %s\n", CryoDataTypeToString(returnType));
        }
        if (expression->metaData->type == NODE_BINARY_EXPR)
        {
            returnType = DATA_TYPE_INT;
            printf("[Parser] Return expression data type: %s\n", CryoDataTypeToString(returnType));
        }
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "No return expression.");
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseReturnStatement", table, arena, state);
    ASTNode *returnNode = createReturnNode(expression, returnType, arena, state);
    logMessage("INFO", __LINE__, "Parser", "Return statement parsed.");
    return returnNode;
}
// </parseReturnStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

// <parseParameter>
ASTNode *parseParameter(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter...");

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseParameter", table, arena, state);
        return NULL;
    }

    char *paramName = strndup(currentToken.start, currentToken.length);

    getNextToken(lexer, arena, state);

    consume(lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", table, arena, state);

    CryoDataType paramType = parseType(lexer, context, table, arena, state);
    // consume data type:
    getNextToken(lexer, arena, state);
    ASTNode *node = createParamNode(strdup(paramName), strdup(functionName), paramType, arena, state);
    addASTNodeSymbol(table, node, arena);
    return node;
}
// </parseParameter>

// <parseParameterList>
ASTNode **parseParameterList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing parameter list...");
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start parameter list.", "parseParameterList", table, arena, state);

    ASTNode **paramListNode = (ASTNode **)ARENA_ALLOC(arena, 8 * sizeof(ASTNode *));
    if (!paramListNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to allocate memory for parameter list.");
        return NULL;
    }

    int paramCount = 0;
    while (currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *param = parseParameter(lexer, table, context, arena, functionName, state);
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

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    paramListNode[paramCount] = NULL; // Null terminate the parameter array

    logMessage("INFO", __LINE__, "Parser", "Parameter count: %d", paramCount);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end parameter list.", "parseParameterList", table, arena, state);

    return paramListNode;
}
// </parseParameterList>

// <parseArguments>
ASTNode *parseArguments(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments...");

    if (currentToken.type != TOKEN_IDENTIFIER && currentToken.type != TOKEN_INT_LITERAL && currentToken.type != TOKEN_STRING_LITERAL && currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        error("Expected an identifier.", "parseArguments", table, arena, state);
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
    getNextToken(lexer, arena, state);

    return createArgsNode(argName, argType, isLiteral, arena, state);
}
// </parseArguments>

// <parseArgumentList>
ASTNode *parseArgumentList(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing argument list...");
    ASTNode *argListNode = createArgumentListNode(arena, state);
    if (argListNode == NULL)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create argument list node.");
        return NULL;
    }

    while (currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *arg = parseArguments(lexer, table, context, arena, state);
        if (arg)
        {
            logMessage("INFO", __LINE__, "Parser", "Adding argument to list...");
            addArgumentToList(table, argListNode, arg, arena, state);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse argument.");
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    return argListNode;
}
// </parseArgumentList>

// <parseArgumentsWithExpectedType>
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, CryoDataType expectedType, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing arguments with expected type...");

    if (currentToken.type != TOKEN_IDENTIFIER && currentToken.type != TOKEN_INT_LITERAL && currentToken.type != TOKEN_STRING_LITERAL && currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        error("Expected an identifier.", "parseArgumentsWithExpectedType", table, arena, state);
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
        logMessage("INFO", __LINE__, "Parser", "Argument type: %s", CryoDataTypeToString(expectedType));
        isLiteral = false;
    }

    // Consume the argument name
    getNextToken(lexer, arena, state);

    logMessage("INFO", __LINE__, "Parser", "Creating argument node with expected type: %s", CryoDataTypeToString(expectedType));
    logMessage("INFO", __LINE__, "Parser", "Argument name: %s", strdup(argName));

    return createArgsNode(argName, expectedType, isLiteral, arena, state);
}
// </parseArgumentsWithExpectedType>

// <addParameterToList>
void addParameterToList(CryoSymbolTable *table, ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state)
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
void addArgumentToList(CryoSymbolTable *table, ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state)
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
void addArgumentToFunctionCall(CryoSymbolTable *table, ASTNode *functionCallNode, ASTNode *arg, Arena *arena, CompilerState *state)
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
void addParameterToExternDecl(CryoSymbolTable *table, ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state)
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
ASTNode *parseImport(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing import...");
    consume(lexer, TOKEN_KW_IMPORT, "Expected `import` keyword.", "parseImport", table, arena, state);

    if (currentToken.type != TOKEN_STRING_LITERAL)
    {
        error("Expected a string literal", "parseImport", table, arena, state);
        return NULL;
    }

    char *moduleName = strndup(currentToken.start + 1, currentToken.length - 2);
    getNextToken(lexer, arena, state);

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImport", table, arena, state);
    return createImportNode(moduleName, arena, state);
}
// </parseImport>

// <parseExtern>
ASTNode *parseExtern(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing extern...");
    consume(lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", table, arena, state);

    switch (currentToken.type)
    {
    case TOKEN_KW_FN:
        return parseExternFunctionDeclaration(lexer, table, context, arena, state);

    default:
        error("Expected an extern declaration.", "parseExtern", table, arena, state);
        return NULL;
    }

    return NULL;
}
// </parseExtern>

/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode *parseIfStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing if statement...");
    consume(lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", table, arena, state);
    context->isParsingIfCondition = true;

    ASTNode *condition = parseIfCondition(lexer, table, context, arena, state);
    ASTNode *ifBlock = parseBlock(lexer, table, context, arena, state);
    ASTNode *elseBlock = NULL;

    if (currentToken.type == TOKEN_KW_ELSE)
    {
        getNextToken(lexer, arena, state);
        if (currentToken.type == TOKEN_KW_IF)
        {
            elseBlock = parseIfStatement(lexer, table, context, arena, state);
        }
        else
        {
            elseBlock = parseBlock(lexer, table, context, arena, state);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock, arena, state);
}
// </parseIfStatement>

ASTNode *parseIfCondition(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing if condition...");
    char *cur_token = strndup(currentToken.start, currentToken.length);
    printf("\n\n[Parser] Current token: %s\n\n", cur_token);

    consume(lexer, TOKEN_LPAREN, "Expected `(` to start if condition.", "parseIfCondition", table, arena, state);

    char *cur_token_cpy = strndup(currentToken.start, currentToken.length);

    printf("\n\n[Parser] Current token: %s\n\n", cur_token_cpy);
    ASTNode *condition = parseExpression(lexer, table, context, arena, state);

    consume(lexer, TOKEN_RPAREN, "Expected `)` to end if condition.", "parseIfCondition", table, arena, state);

    return condition;
}

// <parseForLoop>
ASTNode *parseForLoop(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing for loop...");
    consume(lexer, TOKEN_KW_FOR, "Expected `for` keyword.", "parseForLoop", table, arena, state);
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start for loop.", "parseForLoop", table, arena, state);
    consume(lexer, TOKEN_DOLLAR, "Expected `$` to start initilizer with an iterable.", "parseForLoop", table, arena, state);

    // The current token is <TOKEN_IDENTIFIER> which is the name of the iterable
    char *iterableName = strndup(currentToken.start, currentToken.length);

    consume(lexer, TOKEN_IDENTIFIER, "Expected an identifier for the iterable.", "parseForLoop", table, arena, state);
    // Check the type of for loop were in.
    // For now, the structure is:
    // for($iterable: <type> = <expression>; <condition>; <update>)
    CryoDataType iterDataType = DATA_TYPE_UNKNOWN;
    getNextToken(lexer, arena, state);
    char *iterableType = strndup(currentToken.start, currentToken.length);
    printf("\n\nType: %s\n\n", iterableType);
    CryoDataType dataType = CryoDataTypeStringToType(iterableType);
    if (dataType == DATA_TYPE_UNKNOWN)
    {
        error("Unknown data type.", "parseForLoop", table, arena, state);
    }
    printf("DataType: %s\n", CryoDataTypeToString(dataType));

    getNextToken(lexer, arena, state);
    consume(lexer, TOKEN_EQUAL, "Expected `=` after iterable type.", "parseForLoop", table, arena, state);

    ASTNode *iterable = parseExpression(lexer, table, context, arena, state);

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table, arena, state);

    printf("\n\nDataType in ForLoop init: %s\n\n", CryoDataTypeToString(dataType));
    ASTNode *init = createVarDeclarationNode(iterableName, dataType, iterable, false, false, false, true, arena, state);
    printAST(init, 0, arena);

    addASTNodeSymbol(table, init, arena);

    ASTNode *condition = parseExpression(lexer, table, context, arena, state);
    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon to separate for loop condition.", "parseForLoop", table, arena, state);

    char *__curToken = strndup(currentToken.start, currentToken.length);
    printf("Current Token Going into `parseExpression`: %s\n", __curToken);

    ASTNode *update = parseExpression(lexer, table, context, arena, state);
    char *cur_token = strndup(currentToken.start, currentToken.length);

    printf("\n\n[Parser] Current token: %s\n\n", cur_token);

    consume(lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", table, arena, state);

    ASTNode *body = parseBlock(lexer, table, context, arena, state);

    return createForStatement(init, condition, update, body, arena, state);
}
// </parseForLoop>

// <parseWhileStatement>
ASTNode *parseWhileStatement(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing while statement...");
    consume(lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", table, arena, state);
    consume(lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", table, arena, state);

    ASTNode *condition = parseExpression(lexer, table, context, arena, state);
    consume(lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", table, arena, state);

    ASTNode *body = parseBlock(lexer, table, context, arena, state);
    return createWhileStatement(condition, body, arena, state);
}
// </parseWhileStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode *parseArrayLiteral(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing array literal...");
    consume(lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", table, arena, state);

    ASTNode *elements = createArrayLiteralNode(arena, state);
    if (elements == NULL)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    while (currentToken.type != TOKEN_RBRACKET)
    {
        ASTNode *element = parseExpression(lexer, table, context, arena, state);
        if (element)
        {
            addElementToArrayLiteral(table, elements, element, arena, state);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse array element.");
            return NULL;
        }

        if (currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    consume(lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", table, arena, state);
    return elements;
}
// </parseArrayLiteral>

// <addElementToArrayLiteral>
void addElementToArrayLiteral(CryoSymbolTable *table, ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state)
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
ASTNode *parseArrayIndexing(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing array indexing...");
    char *arrName = strndup(currentToken.start, currentToken.length);
    char *arrCpyName = strdup(arrName);
    consume(lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseArrayIndexing", table, arena, state);
    consume(lexer, TOKEN_LBRACKET, "Expected `[` to start array indexing.", "parseArrayIndexing", table, arena, state);

    printf("[Parser] Array name: %s\n", arrCpyName);
    ASTNode *arrNode = ARENA_ALLOC(arena, sizeof(ASTNode));
    // Find the array in the symbol table
    CryoSymbol *symbol = findSymbol(table, arrCpyName, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Array not found.");
        error("Array not found.", "parseArrayIndexing", table, arena, state);
        exit(1);
        return NULL;
    }
    else
    {
        logMessage("INFO", __LINE__, "Parser", "Array found.");
        arrNode = symbol->node;
    }

    ASTNode *index = parseExpression(lexer, table, context, arena, state);
    consume(lexer, TOKEN_RBRACKET, "Expected `]` to end array indexing.", "parseArrayIndexing", table, arena, state);
    printf("[Parser] Array name: %s\n", strdup(arrCpyName));
    return createIndexExprNode(strdup(arrCpyName), arrNode, index, arena, state);
}
// </parseArrayIndexing>

/* ====================================================================== */
/* @ASTNode_Parsing - Assignments                                         */
ASTNode *parseAssignment(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, char *varName, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing assignment...");
    char *_varName = strndup(currentToken.start, currentToken.length);
    char *varNameCpy = strdup(_varName);
    consume(lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseAssignment", table, arena, state);
    consume(lexer, TOKEN_EQUAL, "Expected `=` for assignment.", "parseAssignment", table, arena, state);
    logMessage("INFO", __LINE__, "Parser", "Variable name: %s", varNameCpy);

    // Find the variable in the symbol table
    CryoSymbol *symbol = findSymbol(table, varNameCpy, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Variable not found.");
        error("Variable not found.", "parseAssignment", table, arena, state);
        return NULL;
    }
    printAST(symbol->node, 0, arena);
    ASTNode *oldValue = symbol->node;
    ASTNode *newValue = parseExpression(lexer, table, context, arena, state);

    // Check if the symbol is mutable
    bool isMutable = oldValue->data.varDecl->isMutable;
    if (!isMutable)
    {
        logMessage("ERROR", __LINE__, "Parser", "Variable is not mutable.");
        error("Variable is not mutable.", "parseAssignment", table, arena, state);
        return NULL;
    }

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseAssignment", table, arena, state);
    printf("\n\nFinished parsing assignment\n");

    ASTNode *assignment = createVarReassignment(strdup(varNameCpy), oldValue, newValue, arena, state);
    printf("\n\nAssignment Node Type %s\n", CryoNodeTypeToString(assignment->metaData->type));
    return assignment;
}

/* ====================================================================== */
/* @ASTNode_Parsing - Structures                                          */

// <parseStructDeclaration>
ASTNode *parseStructDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing struct declaration...");
    consume(lexer, TOKEN_KW_STRUCT, "Expected `struct` keyword.", "parseStructDeclaration", table, arena, state);

    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseStructDeclaration", table, arena, state);
        return NULL;
    }

    char *structName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Struct name: %s", structName);

    getNextToken(lexer, arena, state);

    consume(lexer, TOKEN_LBRACE, "Expected `{` to start struct declaration.", "parseStructDeclaration", table, arena, state);

    int PROPERTY_CAPACITY = 64;
    ASTNode **properties = (ASTNode **)ARENA_ALLOC(arena, PROPERTY_CAPACITY * sizeof(ASTNode *));

    int propertyCount = 0;

    while (currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *field = parseStructField(lexer, table, context, arena, state);
        if (field)
        {
            properties[propertyCount] = field;
            propertyCount++;
            addASTNodeSymbol(table, field, arena);
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse struct field.");
            return NULL;
        }
    }

    ASTNode *structNode = createStructNode(structName, properties, propertyCount, arena, state);
    addASTNodeSymbol(table, structNode, arena);

    consume(lexer, TOKEN_RBRACE, "Expected `}` to end struct declaration.", "parseStructDeclaration", table, arena, state);
    return structNode;
}

// <parseStructField>
ASTNode *parseStructField(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing struct field...");
    if (currentToken.type != TOKEN_IDENTIFIER)
    {
        error("Expected an identifier.", "parseStructField", table, arena, state);
        return NULL;
    }

    char *fieldName = strndup(currentToken.start, currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Field name: %s", fieldName);

    getNextToken(lexer, arena, state);

    consume(lexer, TOKEN_COLON, "Expected `:` after field name.", "parseStructField", table, arena, state);

    CryoDataType fieldType = parseType(lexer, context, table, arena, state);
    getNextToken(lexer, arena, state);

    // This is where we could add support for values in the struct fields
    // For now, this is just going to be a type declaration

    consume(lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseStructField", table, arena, state);

    return createFieldNode(fieldName, fieldType, NULL, arena, state);
}
// </parseStructField>
