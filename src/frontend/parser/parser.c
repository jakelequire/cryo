/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "tools/cxx/IDGen.hpp"
#include "diagnostics/diagnostics.h"
#include "frontend/frontendSymbolTable.h"
#include "frontend/parser.h"

/* ====================================================================== */
/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */

// <parseProgram>
ASTNode *parseProgram(Lexer *lexer, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing program...");
    bool isModuleFile = state->isModuleFile;
    if (isModuleFile)
    {
        logMessage(LMI, "INFO", "Parser", "%s%sModule File Parsing%s", BOLD, CYAN, COLOR_RESET);
    }

    const char *source = lexer->source;
    const char *fileName = lexer->fileName;
    GDM->frontendState->setSourceCode(GDM->frontendState, strdup(source));
    GDM->frontendState->setCurrentFile(GDM->frontendState, strdup(fileName));

    // Initialize the parsing context
    ParsingContext *context = createParsingContext();
    context->isParsingModuleFile = isModuleFile;

    // Create the program node
    ASTNode *program = createProgramNode(arena, state, lexer);
    if (!program)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create program node");
        return NULL;
    }
    context->programNodePtr = program;
    getNextToken(lexer, arena, state);

    while (lexer->currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, context, arena, state);
        if (statement)
        {
            if (statement->metaData->type == NODE_DISCARD)
            {
                logMessage(LMI, "INFO", "Parser", "Discarding statement");
                // Just skip the statement
                continue;
            }
            else
            {
                addStatementToProgram(program, statement, arena, state);
                logMessage(LMI, "INFO", "Parser", "Statement parsed successfully");
            }

            if (statement->metaData->type == NODE_NAMESPACE)
            {
                const char *namespaceName = statement->data.cryoNamespace->name;
                // Initialize the `this`context to the namespace
                setDefaultThisContext(namespaceName, context);
            }
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse statement");
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse statement", lexer->currentToken.line, __FILE__, __func__)
            return NULL;
        }
        logMessage(LMI, "INFO", "Parser", "Next token after statement: %s", CryoTokenToString(lexer->currentToken.type));
    }

    buildASTTreeLinks(program);

    DEBUG_PRINT_FILTER({
        DTM->symbolTable->printTable(DTM->symbolTable);
        GDM->printModuleFileCache(GDM);
    });

    return program;
}
// </parseProgram>

/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(int line, Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Consuming token... [%s]", CryoTokenToString(type));
    // pushCallStack(&callStack, functionName, lexer->currentToken.line);

    addTokenToContext(context, lexer->currentToken);

    if (lexer->currentToken.type == type)
    {
        getNextToken(lexer, arena, state);
    }
    else
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, message, line, __FILE__, functionName)
    }

    debugCurrentToken(lexer, arena, state);
}
// </consume>

// <getNextToken>
void getNextToken(Lexer *lexer, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    if (isAtEnd(lexer, state))
    {
        lexer->currentToken.type = TOKEN_EOF;
        lexer->currentToken.start = lexer->current;
        lexer->currentToken.length = 0;
        lexer->currentToken.line = lexer->line;
        lexer->currentToken.column = lexer->column;
        return;
    }

    if (GDM->frontendState)
    {
        // Set exact position (not increment)
        GDM->frontendState->incrementLine(GDM->frontendState, lexer->line);
        GDM->frontendState->incrementColumn(GDM->frontendState, lexer->column);
    }

    Token *nextToken = (Token *)malloc(sizeof(Token));
    *nextToken = get_next_token(lexer, state);
    lexer->currentToken = *nextToken;
}
// </getNextToken>

// <peekNextUnconsumedToken>
Token peekNextUnconsumedToken(Lexer *lexer, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    if (isAtEnd(lexer, state))
    {
        return lexer->currentToken;
    }
    Token nextToken = peekNextToken(lexer, state);
    return nextToken;
}
// </peekNextUnconsumedToken>

// <getNamespaceName>
char *getNamespaceName(Lexer *lexer, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    char *namespaceName = NULL;
    if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        getNextToken(lexer, arena, state);
    }
    else
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected identifier for namespace name", __LINE__, __FILE__, __func__)
    }
    return namespaceName;
}
// </getNamespaceName>

// <isOperator>
bool isOperator(CryoTokenType type)
{
    __STACK_FRAME__
    switch (type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_STAR:
    case TOKEN_SLASH:
    case TOKEN_PERCENT:
    case TOKEN_OP_LT:
    case TOKEN_OP_GT:
    case TOKEN_OP_LTE:
    case TOKEN_OP_GTE:
    case TOKEN_OP_EQ:
    case TOKEN_OP_NEQ:
    case TOKEN_OP_AND:
    case TOKEN_OP_OR:
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
        return true;
    default:
        logMessage(LMI, "INFO", "Parser", "Not an operator");
        return false;
    }
}

/* ====================================================================== */
/* @DataType_Management                                                   */

// <getCryoDataType>
DataType *getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Getting data type for: %s", typeStr);
    DataType *type = DTM->parseType(typeStr);
    if (!type)
    {
        NEW_ERROR(GDM,
                  CRYO_ERROR_UNKNOWN_DATA_TYPE,
                  CRYO_SEVERITY_FATAL,
                  "Failed to parse data type",
                  __LINE__, __FILE__, __func__)
    }

    logMessage(LMI, "INFO", "Parser", "Data Type Found");
    return type;
}
// </getCryoDataType>

DataType *parseGenericDataTypeInstantiation(DataType *type, Lexer *lexer, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` in generic type instantiation", "parseGenericDataTypeInstantiation", arena, state, NULL);

    // Find the generic type definition
    DataType *genericType = DTM->symbolTable->lookup(DTM->symbolTable, type->typeName);
    if (!genericType)
    {
        NEW_ERROR(GDM,
                  CRYO_ERROR_NULL_DATA_TYPE,
                  CRYO_SEVERITY_FATAL,
                  "Failed to resolve generic type, DataType is NULL",
                  __LINE__, __FILE__, __func__)
        return NULL;
    }

    logMessage(LMI, "INFO", "Parser", "Generic type found: %s", genericType->typeName);

    int expectedParamCount = genericType->container->type.genericType->paramCount;
    DataType **concreteTypes = (DataType **)malloc(expectedParamCount * sizeof(DataType *));
    int paramCount = 0;

    logMessage(LMI, "INFO", "Parser", "Expected param count: %d", expectedParamCount);

    // Parse concrete type arguments
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (paramCount >= expectedParamCount)
        {
            NEW_ERROR(GDM,
                      CRYO_ERROR_TOO_MANY_TYPE_ARGUMENTS,
                      CRYO_SEVERITY_FATAL,
                      "Too many type arguments",
                      __LINE__, __FILE__, __func__)
            return NULL;
        }

        DataType *concreteType = parseType(lexer, NULL, arena, state);
        logMessage(LMI, "INFO", "Parser", "Concrete type: %s", concreteType->typeName);
        concreteTypes[paramCount++] = concreteType;

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            NEW_ERROR(GDM,
                      CRYO_ERROR_SYNTAX,
                      CRYO_SEVERITY_FATAL,
                      "Expected `>` in generic type instantiation",
                      __LINE__, __FILE__, __func__)
            return NULL;
        }
    }

    if (paramCount != expectedParamCount)
    {
        NEW_ERROR(GDM,
                  CRYO_ERROR_INCORRECT_TYPE_ARGUMENT_COUNT,
                  CRYO_SEVERITY_FATAL,
                  "Incorrect number of type arguments",
                  __LINE__, __FILE__, __func__)
        return NULL;
    }
    logMessage(LMI, "INFO", "Parser", "Generic type instantiation parsed successfully");
    // Create the generic type instantiation
    DataType *instantiatedType = DTM->generics->createGenericTypeInstance(genericType, concreteTypes, paramCount);
    logMessage(LMI, "INFO", "Parser", "Instantiated type: %s", instantiatedType->typeName);
    return instantiatedType;
}

// <parseType>
DataType *parseType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing type...");
    const char *typeTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);

    if (context->inGenericContext)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing generic type: %s", typeTokenStr);
        return getCryoDataType(typeTokenStr, arena, state, lexer);
    }

    logMessage(LMI, "INFO", "Parser", "Data Type String: `%s`", typeTokenStr);

    DataType *dataType = NULL;

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_VOID:
    case TOKEN_TYPE_I8:
    case TOKEN_TYPE_I16:
    case TOKEN_TYPE_I32:
    case TOKEN_TYPE_I64:
    case TOKEN_TYPE_I128:
    case TOKEN_KW_STRING:
    case TOKEN_KW_BOOL:
    case TOKEN_KW_ANY:
        logMessage(LMI, "INFO", "Parser", "Parsing primitive type: %s", typeTokenStr);
        dataType = getCryoDataType(typeTokenStr, arena, state, lexer);
        break;

    case TOKEN_IDENTIFIER:
        logMessage(LMI, "INFO", "Parser", "Parsing custom type: %s", typeTokenStr);
        // type = getCryoDataType(typeTokenStr, arena, state, lexer);
        dataType = DTM->resolveType(DTM, typeTokenStr);
        break;

    default:
        NEW_ERROR(GDM,
                  CRYO_ERROR_SYNTAX,
                  CRYO_SEVERITY_FATAL,
                  "Invalid type token",
                  __LINE__, __FILE__, __func__)
        break;
    }

    if (dataType == NULL)
    {
        NEW_ERROR(GDM,
                  CRYO_ERROR_NULL_DATA_TYPE,
                  CRYO_SEVERITY_FATAL,
                  "Failed to parse data type",
                  __LINE__, __FILE__, __func__)
        return NULL;
    }

    // Check to see if the next token is a `*`
    if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_STAR)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing pointer type: %s", typeTokenStr);
        getNextToken(lexer, arena, state);
        DataType *baseType = dataType->clone(dataType);
        dataType = DTM->dataTypes->createPointerType(typeTokenStr, baseType, false);
    }

    // Check to see if the next token is a `&`
    if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_AMPERSAND)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing reference type: %s", typeTokenStr);
        getNextToken(lexer, arena, state);
        dataType->isReference = true;
    }

    logMessage(LMI, "INFO", "Parser", "Type parsed successfully");
    return dataType;
}
// </parseType>

TypeContainer *parseTypeIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    TypeContainer *type = (TypeContainer *)ARENA_ALLOC(arena, sizeof(TypeContainer));

    // Get the base type name
    char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);

    // Check if it's a primitive type
    if (type->typeOf == PRIM_TYPE)
    {
        type->typeOf = PRIM_TYPE;
        type->primitive = DTM->primitives->getPrimitiveType(typeName);
    }
    else
    {
        // Look up custom type
        type->typeOf = OBJECT_TYPE; // or other custom type

        // TODO: Implement `lookupStructType` function
        // type->custom.structDef = lookupStructType( typeName);
    }

    // Handle array dimensions
    while (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LBRACKET)
    {
        // Skip '[' and ']'
        getNextToken(lexer, arena, state);
        getNextToken(lexer, arena, state);
    }

    return type;
}

// <getOperatorPrecedence>
int getOperatorPrecedence(CryoOperatorType type, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Getting operator precedence...");
    switch (type)
    {
    case OPERATOR_ADD:
    case OPERATOR_SUB:
    case OPERATOR_INCREMENT:
    case OPERATOR_DECREMENT:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 1", CryoOperatorToString(type));
        return 1;
    case OPERATOR_MUL:
    case OPERATOR_DIV:
    case OPERATOR_MOD:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 2", CryoOperatorToString(type));
        return 2;
    case OPERATOR_LT:
    case OPERATOR_GT:
    case OPERATOR_LTE:
    case OPERATOR_GTE:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 3", CryoOperatorToString(type));
        return 3;
    case OPERATOR_EQ:
    case OPERATOR_NEQ:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 4", CryoOperatorToString(type));
        return 4;
    case OPERATOR_AND:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 5", CryoOperatorToString(type));
        return 5;
    case OPERATOR_OR:
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: 6", CryoOperatorToString(type));
        return 6;
    default:
        return 0;
    }
}
// </getOperatorPrecedence>

/* ====================================================================== */
/* @Parser_Management                                                     */

// <addStatementToProgram>
void addStatementToProgram(ASTNode *programNode, ASTNode *statement, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding statement to program...");
    if (!programNode || programNode->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    CryoProgram *program = programNode->data.program;

    logMessage(LMI, "INFO", "Parser", "Before adding statement: statementCount = %zu, statementCapacity = %zu",
               program->statementCount, program->statementCapacity);

    if (program->statementCount >= program->statementCapacity)
    {
        program->statementCapacity = (program->statementCapacity > 0) ? (program->statementCapacity * 2) : 1;
        program->statements = (ASTNode **)realloc(program->statements, sizeof(ASTNode *) * program->statementCapacity);
        if (!program->statements)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for statements");
            exit(EXIT_FAILURE);
            return;
        }
    }

    program->statements[program->statementCount++] = statement;

    logMessage(LMI, "INFO", "Parser", "After adding statement: statementCount = %zu, statementCapacity = %zu",
               program->statementCount, program->statementCapacity);
}
// </addStatementToProgram>

/* ====================================================================== */
/* @ASTNode_Parsing - Expressions & Statements                            */

// <parseStatement>
ASTNode *parseStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing statement...");

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state);

    case TOKEN_KW_PUBLIC:
        return parsePublicDeclaration(lexer, context, arena, state);

    case TOKEN_KW_PRIVATE:
        return parsePrivateDeclaration(lexer, context, arena, state);

    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PUBLIC, arena, state);

    case TOKEN_KW_RETURN:
        return parseReturnStatement(lexer, context, arena, state);

    case TOKEN_KW_FOR:
        return parseForLoop(lexer, context, arena, state);

    case TOKEN_KW_IMPORT:
        return handleImportParsing(lexer, context, arena, state);

    case TOKEN_KW_EXTERN:
        return parseExtern(lexer, context, arena, state);

    case TOKEN_KW_STRUCT:
        return parseStructDeclaration(lexer, context, arena, state);

    case TOKEN_KW_IMPLEMENT:
        return parseImplementation(lexer, context, arena, state);

    case TOKEN_KW_DEBUGGER:
        parseDebugger(lexer, context, arena, state);
        return NULL;

    case TOKEN_KW_USING:
        return parseUsingKeyword(lexer, context, arena, state);

    case TOKEN_KW_THIS:
        return parseThisContext(lexer, context, arena, state);

    case TOKEN_KW_STATIC:
        return parseStaticKeyword(lexer, context, arena, state);

    case TOKEN_KW_CLASS:
        return parseClassDeclaration(false, lexer, context, arena, state);

    case TOKEN_KW_TYPE:
        return parseTypeDeclaration(lexer, context, arena, state);

    case TOKEN_IDENTIFIER:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier...");
        // Check for function call: {identifier}(
        if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing function call...");
            char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            return parseFunctionCall(lexer, context, functionName, arena, state);
        }
        // Check for scoped function call: {identifier}::
        else if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DOUBLE_COLON)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing scope call...");
            return parseScopeCall(lexer, context, arena, state);
        }
        // Check for property access: {identifier}.
        else if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing property access...");
            return parsePropertyAccess(lexer, context, arena, state);
        }
        // Check for increment: {identifier}++
        else if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_INCREMENT)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing increment...");
            return parseUnaryExpression(lexer, context, arena, state);
        }
        // Check for decrement: {identifier}--
        else if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DECREMENT)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing decrement...");
            return parseUnaryExpression(lexer, context, arena, state);
        }
        // Check for assignment: {identifier} =
        else if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_ASSIGN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing variable assignment...");
            char *varName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            return parseAssignment(lexer, context, varName, arena, state);
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "Parsing variable assignment...");
            return parsePrimaryExpression(lexer, context, arena, state);
        }
        break;
    }

    case TOKEN_KW_NAMESPACE:
        return parseNamespace(lexer, context, arena, state);

    case TOKEN_KW_IF:
        return parseIfStatement(lexer, context, arena, state);

    case TOKEN_KW_WHILE:
        return parseWhileStatement(lexer, context, arena, state);

    case TOKEN_AT:
        return parseAnnotation(lexer, context, arena, state);

    case TOKEN_KW_BREAK:
        return parseBreakStatement(lexer, context, arena, state);
    case TOKEN_KW_CONTINUE:
        return parseContinueStatement(lexer, context, arena, state);

    case TOKEN_EOF:
        return NULL;

    default:
    {
        const char *tokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "ERROR", "Parser", "Unexpected token: %s", tokenStr);
        char *errorMessage = (char *)malloc(256);
        snprintf(errorMessage, 256, "Unexpected token: %s", tokenStr);
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, errorMessage, __LINE__, __FILE__, __func__)
        return NULL;
    }
    }
}
// </parseStatement>

ASTNode *handlePragmaArgs(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Handling pragma arguments...");

    getNextToken(lexer, arena, state);

    if (lexer->currentToken.type != TOKEN_LBRACKET)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected `[` after `@pragma`", __LINE__, __FILE__, __func__)
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` after `@pragma`", "handlePragmaArgs", arena, state, context);

    while (lexer->currentToken.type != TOKEN_RBRACKET)
    {
        if (lexer->currentToken.type == TOKEN_IDENTIFIER)
        {
            char *pragmaName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            logMessage(LMI, "INFO", "Parser", "Pragma Name: %s", pragmaName);
            getNextToken(lexer, arena, state);
            context->addPragmaArg(context, pragmaName);
        }
        else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
        {
            // Remove the `"` from the string literal
            char *pragmaValue = strndup(lexer->currentToken.start + 1, lexer->currentToken.length - 2);
            logMessage(LMI, "INFO", "Parser", "Pragma Value: %s", pragmaValue);
            getNextToken(lexer, arena, state);
            context->addPragmaArg(context, pragmaValue);
        }
        else
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected identifier or string literal in pragma arguments", __LINE__, __FILE__, __func__)
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` after pragma arguments", "handlePragmaArgs", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Pragma arguments handled successfully");

    return createAnnotationNode("pragma", "undef", arena, state, lexer);
}

ASTNode *parseAnnotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing annotation...");
    switch (lexer->currentToken.type)
    {
    case TOKEN_AT:
    {
        consume(__LINE__, lexer, TOKEN_AT, "Expected '@' symbol", "parseAnnotation", arena, state, context);
        char *annotationName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        if (!annotationName)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to allocate memory for annotation name", __LINE__, __FILE__, __func__)
            return NULL;
        }
        if (strcmp(annotationName, "pragma") == 0)
        {
            return handlePragmaArgs(lexer, context, arena, state);
        }

        getNextToken(lexer, arena, state);

        char *annotationValue = NULL;
        if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
        {
            annotationValue = strndup(lexer->currentToken.start, lexer->currentToken.length);
            getNextToken(lexer, arena, state);
        }

        return createAnnotationNode(annotationName, annotationValue, arena, state, lexer);
    }
    default:
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected '@' symbol", __LINE__, __FILE__, __func__)
        return NULL;
    }
    }

    return NULL;
}

ASTNode *parseStaticKeyword(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected 'static' keyword", "parseStaticKeyword", arena, state, context);

    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state).type;
    switch (nextToken)
    {
    case TOKEN_KW_CLASS:
    {
        return parseClassDeclaration(true, lexer, context, arena, state);
    }
    case TOKEN_KW_STRUCT:
    {
        DEBUG_BREAKPOINT;
    }
    case TOKEN_KW_FN:
    {
        DEBUG_BREAKPOINT;
    }
    default:
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected 'class', 'struct', or 'fn' after 'static'", __LINE__, __FILE__, __func__)
        return NULL;
    }
    }
    return NULL;
}

// <parseScopeCall>
ASTNode *parseScopeCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing scope call...");

    char *scopeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Scope Name: %s", scopeName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier", "parseScopeCall", arena, state, context);

    // Consume the double colon
    consume(__LINE__, lexer, TOKEN_DOUBLE_COLON, "Expected a double colon", "parseScopeCall", arena, state, context);

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function Name: %s", functionName);

    FrontendSymbol *resolvedSymbol = FEST->lookupInScope(FEST, scopeName, functionName);
    if (!resolvedSymbol)
    {
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL, "Failed to resolve symbol", __LINE__, __FILE__, __func__)
        return NULL;
    }
    logMessage(LMI, "INFO", "Parser", "Resolved symbol: %s", resolvedSymbol->name);
    resolvedSymbol->print(resolvedSymbol);

    ASTNode *resolvedASTNode = resolvedSymbol->node;
    if (!resolvedASTNode)
    {
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL, "Resolved symbol has no AST node", __LINE__, __FILE__, __func__)
        return NULL;
    }

    // Consume the identifier
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier", "parseScopeCall", arena, state, context);

    CryoNodeType resolvedNodeType = resolvedASTNode->metaData->type;
    ASTNode *node = NULL;
    if (resolvedNodeType == NODE_METHOD)
    {
        logMessage(LMI, "INFO", "Parser", "Resolved node is a method");
        node = parseScopedMethodCall(functionName, scopeName, resolvedASTNode, lexer, context, arena, state);
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Resolved node is not a method");
        return NULL;
    }
    if (!node)
    {
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL, "Failed to create AST node", __LINE__, __FILE__, __func__)
        return NULL;
    }
    node->print(node);

    return node;
}

ASTNode *parseScopedMethodCall(const char *methodName, const char *scopeName, ASTNode *methodNode,
                               Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing scoped function call...");

    ASTNode *node = createScopedFunctionCall(arena, state, methodName, lexer);

    // Get the arguments
    int argCount = 0;
    ASTNode **args = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    ASTNode *argList = parseArgumentList(lexer, context, arena, state);
    if (!argList)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse argument list", __LINE__, __FILE__, __func__)
        return NULL;
    }
    if (argList->metaData->type == NODE_ARG_LIST)
    {
        int i = 0;
        for (i = 0; i < argList->data.argList->argCount; i++)
        {
            args[i] = argList->data.argList->args[i];
            argCount++;
        }
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseScopedFunctionCall", arena, state, context);

    node->data.scopedFunctionCall->args = args;
    node->data.scopedFunctionCall->argCount = argCount;
    node->data.scopedFunctionCall->scopeName = scopeName;
    node->data.scopedFunctionCall->type = methodNode->data.method->type;

    logMessage(LMI, "INFO", "Parser", "Scoped function call parsed.");

    return node;
}

ASTNode *parseScopedFunctionCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing scoped function call...");

    ASTNode *node = createScopedFunctionCall(arena, state, functionName, lexer);

    // Get the arguments
    int argCount = 0;
    DataType **argTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    ASTNode **args = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    ASTNode *argList = parseArgumentList(lexer, context, arena, state);
    if (!argList)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse argument list", __LINE__, __FILE__, __func__)
        return NULL;
    }
    if (argList->metaData->type == NODE_ARG_LIST)
    {
        int i = 0;
        for (i = 0; i < argList->data.argList->argCount; i++)
        {
            args[i] = argList->data.argList->args[i];
            argTypes[i] = DTM->astInterface->getTypeofASTNode(args[i]);
            argCount++;
        }
    }
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseScopedFunctionCall", arena, state, context);

    node->data.scopedFunctionCall->args = args;
    node->data.scopedFunctionCall->argCount = argCount;
    node->data.scopedFunctionCall->scopeName = scopeName;

    logMessage(LMI, "INFO", "Parser", "Scoped function call parsed.");

    return node;
}

/// @brief This function handles the `debugger` keyword. Which is used to pause the program execution.
/// Note: This only stops the program on the parser side, not the runtime.
void parseDebugger(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing debugger statement...");
    consume(__LINE__, lexer, TOKEN_KW_DEBUGGER, "Expected 'debugger' keyword.", "parseDebugger", arena, state, context);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseDebugger", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Debugger statement parsed.");

    DEBUG_BREAKPOINT;
    return;
}

ASTNode *parseNamespace(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing namespace...");
    consume(__LINE__, lexer, TOKEN_KW_NAMESPACE, "Expected 'namespace' keyword.", "parseNamespace", arena, state, context);

    ASTNode *node = NULL;

    // Use a fixed buffer for the namespace name
    char namespaceBuffer[1024] = {0}; // Adjust size as needed
    int bufferPos = 0;

    // Parse first part of namespace
    if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        // Copy the first identifier to our buffer
        int len = lexer->currentToken.length;
        if (bufferPos + len < sizeof(namespaceBuffer))
        {
            memcpy(namespaceBuffer + bufferPos, lexer->currentToken.start, len);
            bufferPos += len;
        }
        getNextToken(lexer, arena, state);

        // Parse subsequent parts (if any)
        while (lexer->currentToken.type == TOKEN_DOT)
        {
            // Append the dot
            if (bufferPos + 1 < sizeof(namespaceBuffer))
            {
                namespaceBuffer[bufferPos++] = '.';
            }
            getNextToken(lexer, arena, state);

            // After a dot, we must have another identifier
            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                len = lexer->currentToken.length;
                if (bufferPos + len < sizeof(namespaceBuffer))
                {
                    memcpy(namespaceBuffer + bufferPos, lexer->currentToken.start, len);
                    bufferPos += len;
                }
                getNextToken(lexer, arena, state);
            }
            else
            {
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected an identifier after dot", __LINE__, __FILE__, __func__)
                break;
            }
        }

        // Null-terminate the string
        namespaceBuffer[bufferPos] = '\0';
    }
    else
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected an identifier for namespace name", __LINE__, __FILE__, __func__)
        return NULL;
    }

    // Use your arena allocator to create a copy of the namespace string
    char *namespaceName = NULL;
    if (bufferPos > 0)
    {
        namespaceName = strdup(namespaceBuffer);
        node = createNamespaceNode(namespaceName, arena, state, lexer);
        createNamespaceScope(context, namespaceName);
        FEST->enterNamespace(FEST, namespaceName);
    }

    GDM->addModuleFileCache(GDM, namespaceName, lexer->fileName, lexer->source);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseNamespace", arena, state, context);

    return node;
}

// <parsePrimaryExpression>
ASTNode *parsePrimaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing primary expression...");
    logMessage(LMI, "INFO", "Parser", "Current Token: %s", CryoTokenToString(lexer->currentToken.type));

    ASTNode *node = NULL;
    CryoTokenType prevToken = context->lastTokens[0].type;
    logMessage(LMI, "INFO", "Parser", "Previous Token: %s", CryoTokenToString(prevToken));

    switch (lexer->currentToken.type)
    {
    case TOKEN_INT_LITERAL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing integer literal");
        char *intStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        int intVal = charToInt(intStr);
        if (intVal == 0 && strcmp(intStr, "0") != 0)
        {
            NEW_ERROR(GDM,
                      CRYO_ERROR_SYNTAX,
                      CRYO_SEVERITY_FATAL,
                      "Invalid integer literal",
                      __LINE__, __FILE__, __func__)
        }
        // Check for overflow
        if (intVal < INT_MIN || intVal > INT_MAX)
        {
            NEW_ERROR(GDM,
                      CRYO_ERROR_INTEGER_OVERFLOW,
                      CRYO_SEVERITY_FATAL,
                      "Integer overflow",
                      __LINE__, __FILE__, __func__)
        }
        node = createIntLiteralNode(intVal, arena, state, lexer);
        if (context->integerContextType != NULL)
        {
            node->data.literal->type = context->integerContextType;
        }
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_STRING_LITERAL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing string literal");
        const char *str = (const char *)strndup(lexer->currentToken.start, lexer->currentToken.length);
        node = createStringLiteralNode(str, arena, state, lexer, context);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_BOOLEAN_LITERAL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing boolean literal");
        char *booleanValueStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        if (strcmp(booleanValueStr, "true") != 0 && strcmp(booleanValueStr, "false") != 0)
        {
            NEW_ERROR(GDM,
                      CRYO_ERROR_SYNTAX,
                      CRYO_SEVERITY_FATAL,
                      "Invalid boolean literal",
                      __LINE__, __FILE__, __func__)
        }
        int booleanValue = strcmp(booleanValueStr, "true") == 0 ? 1 : 0;
        node = createBooleanLiteralNode(booleanValue, arena, state, lexer);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_LBRACKET:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array literal");
        return parseArrayLiteral(lexer, context, arena, state);
    }
    case TOKEN_IDENTIFIER:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier expression");
        return parseIdentifierExpression(lexer, context, arena, state);
    }
    case TOKEN_KW_NEW:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing new expression");
        return parseNewExpression(lexer, context, arena, state);
    }
    case TOKEN_KW_NULL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing null expression");
        return parseNullExpression(lexer, context, arena, state);
    }
    case TOKEN_KW_TYPEOF:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing typeof expression");
        return parseTypeofIdentifier(lexer, context, arena, state);
    }
    case TOKEN_KW_THIS:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing this expression");
        return parseThisExpression(lexer, context, arena, state);
    }
    case TOKEN_LPAREN:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing parenthesized expression");
        return parseParenthesizedExpression(lexer, context, arena, state);
    }
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
    case TOKEN_MINUS:
    case TOKEN_BANG:
    case TOKEN_STAR:
    case TOKEN_AMPERSAND:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing unary expression");
        return parseUnaryExpression(lexer, context, arena, state);
    }
    default:
    {
        char *curTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "ERROR", "Parser", "Unexpected token: %s", curTokenStr);
        NEW_ERROR(GDM,
                  CRYO_ERROR_SYNTAX,
                  CRYO_SEVERITY_FATAL,
                  "Invalid primary expression",
                  __LINE__, __FILE__, __func__)
        return NULL;
    }
    }

    // Check for dot notation after the primary expression
    while (lexer->currentToken.type == TOKEN_DOT)
    {
        node = parseDotNotation(lexer, context, arena, state);
    }

    // Check for array indexing after an identifier or other primary expression
    while (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array indexing");
        node = parseArrayIndexing(lexer, context, NULL, arena, state);
    }

    return node;
}
// </parsePrimaryExpression>

ASTNode *parseIdentifierExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    CryoTokenType prevToken = context->lastTokens[0].type;
    char *curToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state).type;

    if (nextToken == TOKEN_KW_NEW)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing new expression");
        return parseNewExpression(lexer, context, arena, state);
    }

    // Check for dot notation after the primary expression
    if (nextToken == TOKEN_DOT)
    {
        // This can either be a struct field or a function call.
        return parseDotNotation(lexer, context, arena, state);
    }
    // Peek to see if the next token is `[` for array indexing
    if (nextToken == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array indexing");
        return parseArrayIndexing(lexer, context, NULL, arena, state);
    }
    // Peek to see if the next token is `;` for a statement
    if (nextToken == TOKEN_SEMICOLON)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context);
        getNextToken(lexer, arena, state);

        return node;
    }
    // Peek to see if the next token is `)` to end a function call.
    if (nextToken == TOKEN_RPAREN)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context);
        getNextToken(lexer, arena, state);
        return node;
    }

    // Peek to see if the next token is `=` for assignment
    if (nextToken == TOKEN_EQUAL)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing assignment");
        return parseAssignment(lexer, context, NULL, arena, state);
    }
    // Peek to see if the next token is `(` to start a function call.
    if (nextToken == TOKEN_LPAREN)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing function call");
        char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        return parseFunctionCall(lexer, context, functionName, arena, state);
    }
    // Peek to see if the next token is `::` for a scope call
    if (nextToken == TOKEN_DOUBLE_COLON)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing scope call");
        return parseScopeCall(lexer, context, arena, state);
    }

    // Peek to see if the next token is `++` or `--`
    if (nextToken == TOKEN_INCREMENT || nextToken == TOKEN_DECREMENT)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing increment/decrement");
        return parseUnaryExpression(lexer, context, arena, state);
    }

    logMessage(LMI, "INFO", "Parser", "Parsing identifier expression: %s", curToken);
    // Check to see if it exists in the symbol table as a variable or parameter
    ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context);
    getNextToken(lexer, arena, state);
    return node;
}

// <parseExpression>
ASTNode *parseExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing expression...");

    return parseBinaryExpression(lexer, context, 1, arena, state);
}
// </parseExpression>

// <parseExpressionStatement>
ASTNode *parseExpressionStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing expression statement...");

    ASTNode *expression = parseExpression(lexer, context, arena, state);

    logMessage(LMI, "Parser", "Expression parsed: %s", CryoNodeTypeToString(expression->metaData->type));

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", arena, state, context);

    return createExpressionStatement(expression, arena, state, lexer);
}
// </parseExpressionStatement>

// <parseBinaryExpression>
ASTNode *parseBinaryExpression(Lexer *lexer, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing binary expression...");
    ASTNode *left = parsePrimaryExpression(lexer, context, arena, state);
    if (!left)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse primary expression", __LINE__, __FILE__, __func__)
        return NULL;
    }

    CryoTokenType curTok = lexer->currentToken.type;
    if (curTok == TOKEN_SEMICOLON || curTok == TOKEN_RPAREN)
    {
        logMessage(LMI, "INFO", "Parser", "End of expression");
        return left;
    }

    while (true)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing binary expression... Current Token: %s", CryoTokenToString(lexer->currentToken.type));
        CryoTokenType opToken = lexer->currentToken.type;

        if (opToken == TOKEN_SEMICOLON || opToken == TOKEN_RPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "End of expression");
            break;
        }

        CryoOperatorType _op = CryoTokenToOperator(opToken);
        int precedence = getOperatorPrecedence(_op, arena, state);
        logMessage(LMI, "INFO", "Parser", "Operator: %s, Precedence: %d", CryoOperatorToString(_op), precedence);

        if (precedence < minPrecedence)
        {
            logMessage(LMI, "INFO", "Parser", "Precedence is less than minimum precedence");
            break;
        }

        getNextToken(lexer, arena, state); // consume operator
        logMessage(LMI, "INFO", "Parser", "Parsing binary expression... Current Token: %s", CryoTokenToString(lexer->currentToken.type));

        if (lexer->currentToken.type == TOKEN_SEMICOLON || lexer->currentToken.type == TOKEN_RPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "End of expression");
            break;
        }

        // Parse the right side with a higher precedence
        logMessage(LMI, "INFO", "Parser", "Parsing right side of binary expression...");
        ASTNode *right = parseBinaryExpression(lexer, context, precedence + 1, arena, state);
        if (!right)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse right side of binary expression", __LINE__, __FILE__, __func__)
            return NULL;
        }

        // Create a new binary expression node
        CryoOperatorType op = CryoTokenToOperator(opToken);
        if (op == OPERATOR_NA)
        {
            NEW_ERROR(GDM, CRYO_ERROR_INVALID_OPERATOR, CRYO_SEVERITY_FATAL, "Invalid operator", __LINE__, __FILE__, __func__)
            return NULL;
        }

        ASTNode *newNode = createBinaryExpr(left, right, op, arena, state, lexer);
        left = newNode;

        logMessage(LMI, "INFO", "Parser", "Binary expression parsed: %s", CryoNodeTypeToString(newNode->metaData->type));
    }

    logMessage(LMI, "INFO", "Parser", "Binary expression parsed.");

    return left;
}
// </parseBinaryExpression>

// <parseUnaryExpression>
ASTNode *parseUnaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing unary expression...");
    if (lexer->currentToken.type == TOKEN_AMPERSAND)
    {
        consume(__LINE__, lexer, TOKEN_AMPERSAND, "Expected identifier after &", "parseUnaryExpression", arena, state, context);

        // Parse the operand - must be an lvalue
        ASTNode *operand = parsePrimaryExpression(lexer, context, arena, state);
        if (!operand)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse operand for address-of operator", __LINE__, __FILE__, __func__)
            return NULL;
        }

        // Create address-of node
        ASTNode *node = createUnaryExpr(TOKEN_AMPERSAND, operand, arena, state, lexer);
        if (!node)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to create address-of node", __LINE__, __FILE__, __func__)
            return NULL;
        }

        // Set the result type to be a pointer to operand's type
        DataType *operandType = DTM->astInterface->getTypeofASTNode(operand);
        operandType->setPointer(operandType, true);

        node->data.unary_op->resultType = operandType;
        node->data.unary_op->op = TOKEN_ADDRESS_OF;
        node->data.unary_op->expression = operand;

        if (lexer->currentToken.type == TOKEN_SEMICOLON)
        {
            consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseUnaryExpression", arena, state, context);
        }

        return node;
    }
    if (lexer->currentToken.type == TOKEN_STAR)
    {
        consume(__LINE__, lexer, TOKEN_STAR, "Expected identifier after *", "parseUnaryExpression", arena, state, context);

        // Parse the operand - must be a pointer type
        ASTNode *operand = parsePrimaryExpression(lexer, context, arena, state);

        // Create dereference node
        ASTNode *node = createUnaryExpr(TOKEN_STAR, operand, arena, state, lexer);

        // Set the result type to be the type pointed to by operand's type
        DataType *operandType = DTM->astInterface->getTypeofASTNode(operand);
        // TODO: implement the code below. Need to fully implement the pointer type system
        // if (!operandType->isPointer)
        // {
        //     parsingError("Dereference operator must be applied to a pointer type", "parseUnaryExpression", arena, state, lexer, lexer->source);
        //     return NULL;
        // }
        node->data.unary_op->resultType = operandType;
        node->data.unary_op->op = TOKEN_DEREFERENCE;
        node->data.unary_op->expression = operand;

        return node;
    }
    CryoTokenType opToken;
    ASTNode *right;
    if (lexer->currentToken.type == TOKEN_MINUS || lexer->currentToken.type == TOKEN_BANG)
    {
        logMessage(LMI, "INFO", "Parser", "Current Token: %s", CryoTokenToString(lexer->currentToken.type));
        logMessage(LMI, "INFO", "Parser", "Parsing NOT or unary minus expression...");
        opToken = lexer->currentToken.type;
        getNextToken(lexer, arena, state);
        right = parsePrimaryExpression(lexer, context, arena, state);
        return createUnaryExpr(opToken, right, arena, state, lexer);
    }
    if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_INCREMENT ||
        peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DECREMENT)
    {
        // This is a postfix increment (e.g., x++)
        logMessage(LMI, "INFO", "Parser", "Parsing postfix increment expression...");
        if (lexer->currentToken.type == TOKEN_INCREMENT)
        {
            opToken = TOKEN_INCREMENT;
        }
        else
        {
            opToken = TOKEN_DECREMENT;
        }
        char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
        FrontendSymbol *sym = FEST->lookup(FEST, cur_token);
        if (!sym)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse primary expression", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }

        if (lexer->currentToken.type == TOKEN_IDENTIFIER)
        {
            consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier", "parseUnaryExpression", arena, state, context);
        }

        ASTNode *lhs = sym->node;
        if (!lhs)
        {
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse primary expression", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }

        if (sym->node->metaData->type == NODE_VAR_DECLARATION)
        {
            const char *varName = sym->name;
            DataType *varType = sym->type;
            ASTNode *varNameNode = createVarNameNode(strdup(varName), varType, arena, state, lexer);
            lhs = varNameNode;
        }

        ASTNode *postfixIncrement = createUnaryExpr(opToken, lhs, arena, state, lexer);
        postfixIncrement->data.unary_op->op = TOKEN_INCREMENT;
        postfixIncrement->data.unary_op->expression = lhs;
        postfixIncrement->data.unary_op->isPostfix = true;
        consume(__LINE__, lexer, TOKEN_INCREMENT, "Expected an increment operator", "parseUnaryExpression", arena, state, context);

        if (lexer->currentToken.type == TOKEN_SEMICOLON)
        {
            consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseUnaryExpression", arena, state, context);
        }
        return postfixIncrement;
    }

    return createUnaryExpr(opToken, right, arena, state, lexer);
}
// </parseUnaryExpression>

// <parsePublicDeclaration>
ASTNode *parsePublicDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing public declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PUBLIC, arena, state);
    case TOKEN_KW_MODULE:
        return parseModuleDeclaration(VISIBILITY_PUBLIC, lexer, context, arena, state);
    default:
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected a declaration.", __LINE__, __FILE__, __func__)
        return NULL;
    }
}
// </parsePublicDeclaration>

// <parsePrivateDeclaration>
ASTNode *parsePrivateDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing private declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PRIVATE, "Expected 'private' keyword.", "parsePrivateDeclaration", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PRIVATE, arena, state);
    case TOKEN_KW_MODULE:
        return parseModuleDeclaration(VISIBILITY_PRIVATE, lexer, context, arena, state);
    default:
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected a declaration.", __LINE__, __FILE__, __func__)
        return NULL;
    }
}
// </parsePrivateDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode *parseBlock(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing block...");
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", arena, state, context);

    context->scopeLevel++;

    ASTNode *block = createBlockNode(arena, state, lexer);
    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *statement = parseStatement(lexer, context, arena, state);
        if (statement)
        {
            addStatementToBlock(block, statement, arena, state, lexer);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse block");
            return NULL;
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end block.", "parseBlock", arena, state, context);
    context->scopeLevel--;
    return block;
}
// </parseBlock>

// <parseFunctionBlock>
ASTNode *parseFunctionBlock(DataType *returnType, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function block...");
    context->scopeLevel++;

    ASTNode *functionBlock = createFunctionBlock(arena, state, lexer);
    if (!functionBlock)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create function block");
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start function block.", "parseFunctionBlock", arena, state, context);

    while (lexer->currentToken.type != TOKEN_RBRACE && lexer->currentToken.type != TOKEN_EOF)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing statement... %s", CryoTokenToString(lexer->currentToken.type));
        ASTNode *statement = parseStatement(lexer, context, arena, state);
        if (statement)
        {
            if (statement->metaData->type == NODE_RETURN_STATEMENT)
            {
                ASTNode *returnStatement = statement->data.returnStatement->expression;
                if (returnStatement)
                {
                    DataType *returnDataType = DTM->astInterface->getTypeofASTNode(returnStatement);
                    returnType->unsafeCast(returnDataType, returnType);
                }
            }
            logMessage(LMI, "INFO", "Parser", "Adding statement to function block...");
            addStatementToFunctionBlock(functionBlock, statement, arena, state, lexer);
            if (lexer->currentToken.type == TOKEN_SEMICOLON)
            {
                getNextToken(lexer, arena, state);
            }
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse function block");
            return NULL;
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end function block.", "parseFunctionBlock", arena, state, context);
    context->scopeLevel--;

    logMessage(LMI, "INFO", "Parser", "Function block parsed.");
    return functionBlock;
}
// </parseFunctionBlock>

/* ====================================================================== */
/* @ASTNode_Parsing - Variables                                           */

// <parseVarDeclaration>
ASTNode *parseVarDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing variable declaration...");
    bool isMutable = lexer->currentToken.type == TOKEN_KW_MUT;
    bool isConstant = lexer->currentToken.type == TOKEN_KW_CONST;
    bool isReference = lexer->currentToken.type == TOKEN_AMPERSAND;

    const char *currentScopeID = getCurrentScopeID(context);

    // Skip the 'const' or 'mut' keyword
    if (isMutable || isConstant)
    {
        getNextToken(lexer, arena, state);
    }

    // Parse the variable name
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected variable name.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected variable name.", __LINE__, __FILE__, __func__)
    }
    char *var_name = strndup(lexer->currentToken.start, lexer->currentToken.length);
    getNextToken(lexer, arena, state);

    // Parse the variable type
    DataType *dataType = NULL;
    if (lexer->currentToken.type == TOKEN_COLON)
    {
        getNextToken(lexer, arena, state);
        char *varType = strndup(lexer->currentToken.start, lexer->currentToken.length);

        dataType = getCryoDataType(varType, arena, state, lexer);
        if (!dataType)
        {
            logMessage(LMI, "ERROR", "Parser", "Unknown data type.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_DATA_TYPE, CRYO_SEVERITY_FATAL, "Unknown data type.", __LINE__, __FILE__, __func__)
        }
        getNextToken(lexer, arena, state);
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Expected ':' after variable name.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected ':' for variable type.", __LINE__, __FILE__, __func__)
    }

    // Check if the data type is a primitive type
    if (dataType->container->typeOf == PRIM_TYPE)
    {
        switch (dataType->container->primitive)
        {
        case PRIM_INT:
        case PRIM_I8:
        case PRIM_I16:
        case PRIM_I32:
        case PRIM_I64:
        case PRIM_I128:
        {
            logMessage(LMI, "INFO", "Parser", "Primitive type detected.");
            context->setIntegerContextType(context, dataType);
            break;
        }
        case PRIM_STR:
            context->setStringContextType(context, dataType);
            break;
        case PRIM_STRING:
            context->setStringContextType(context, dataType);
            break;
        default:
        {
            logMessage(LMI, "ERROR", "Parser", "Unknown primitive type.");
            NEW_ERROR(GDM, CRYO_ERROR_UNKNOWN_DATA_TYPE, CRYO_SEVERITY_FATAL, "Unknown primitive type.", __LINE__, __FILE__, __func__)
            break;
        }
        }
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Data type is not a primitive type.");
    }

    // Check if the data type is followed by an array instantiation
    // i.e `i32[]` or `i32[10]`
    bool isArray = false;
    if (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array type...");
        DataType *arrayType = parseArrayInstantiation(dataType, lexer, context, arena, state);
        if (arrayType)
        {
            dataType = arrayType;
            isArray = true;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse array type.");
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse array type.", __LINE__, __FILE__, __func__)
        }
    }

    // Check if the variable ends with a semicolon to no-initialize
    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        logMessage(LMI, "INFO", "Parser", "Variable declaration without initialization.");
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", arena, state, context);
        ASTNode *uninitVarDecl = createVarDeclarationNode(var_name, dataType, NULL, isMutable, isConstant, isReference, false, true, arena, state, lexer);
        uninitVarDecl->data.varDecl->type = dataType;
        FEST->addSymbol(FEST, uninitVarDecl);
        return uninitVarDecl;
    }

    // Parse the variable initializer
    if (lexer->currentToken.type != TOKEN_EQUAL)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected '=' after variable name.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected '=' after variable name.", __LINE__, __FILE__, __func__)
    }
    getNextToken(lexer, arena, state);

    if (lexer->currentToken.type == TOKEN_AMPERSAND)
    {
        isReference = true;
        getNextToken(lexer, arena, state);
    }

    // Parse the initializer expression
    ASTNode *initializer = parseExpression(lexer, context, arena, state);
    if (initializer == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse initializer expression.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse initializer expression.", __LINE__, __FILE__, __func__)
    }
    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", arena, state, context);
    }

    // Clear the context for the variable
    context->clearIntegerContextType(context);
    context->clearStringContextType(context);

    logMessage(LMI, "INFO", "Parser", "Variable declaration parsed.");

    ASTNode *varDeclNode = createVarDeclarationNode(var_name, dataType, initializer, isMutable, isConstant, isReference, false, false, arena, state, lexer);
    if (initializer->metaData->type == NODE_INDEX_EXPR)
    {
        logMessage(LMI, "INFO", "Parser", "Index expression detected.");
        varDeclNode->data.varDecl->indexExpr = initializer->data.indexExpr;
        varDeclNode->data.varDecl->hasIndexExpr = true;
    }
    if (initializer->metaData->type == NODE_ARRAY_LITERAL)
    {
        DataType *varType = varDeclNode->data.varDecl->type;
        if (varType->container->typeOf == ARRAY_TYPE)
        {
            varType->container->type.arrayType->size = initializer->data.array->elementCount;
        }
    }

    FEST->addSymbol(FEST, varDeclNode);

    return varDeclNode;
}
// </parseVarDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode *parseFunctionDeclaration(Lexer *lexer, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected function name.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionName);

    const char *functionScopeID = Generate64BitHashID(functionName);
    const char *namespaceScopeID = getNamespaceScopeID(context);
    setCurrentFunction(context, functionName, namespaceScopeID); // Context Manager

    FEST->enterScope(FEST, strdup(functionName), SCOPE_FUNCTION);

    getNextToken(lexer, arena, state);

    // Check for generic type parameters
    bool isGeneric = false;
    GenericType **genericParams = NULL;
    int genericParamCount = 0;
    if (lexer->currentToken.type == TOKEN_LESS)
    {
        isGeneric = true;
        context->inGenericContext = true; // Set generic context flag in ParsingContext

        // TODO: Implement generic type parameters
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Generic type parameters are not yet implemented.", __LINE__, __FILE__, __func__)
    }

    ASTNode **params = parseParameterList(lexer, context, arena, strdup(functionName), state);

    for (int i = 0; params[i] != NULL; i++)
    {
        logMessage(LMI, "INFO", "Parser", "Adding parameter: %s", params[i]->data.param->name);
    }
    int paramCount = 0;
    DataType **paramTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    for (int i = 0; params[i] != NULL; i++)
    {
        paramTypes[i] = params[i]->data.param->type;
        FEST->addSymbol(FEST, params[i]);
        paramCount++;
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage(LMI, "INFO", "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, arena, state);
        getNextToken(lexer, arena, state);
    }
    else
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected `->` for return type.", __LINE__, __FILE__, __func__)
    }

    logMessage(LMI, "INFO", "Parser", "Function Return Type: %s", returnType->debug->toString(returnType));
    DataType *functionType = DTM->functionTypes->createFunctionType(paramTypes, paramCount, returnType);

    // Initialize the function symbol
    if (isGeneric)
    {
    }
    else
    {
        DTM->symbolTable->addEntry(
            DTM->symbolTable,
            functionScopeID,
            functionName,
            functionType);
    }

    // Ensure the next token is `{` for the function block
    if (lexer->currentToken.type != TOKEN_LBRACE)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected `{` to start function block.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected `{` to start function block.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    // Definition of the function
    ASTNode *functionDefNode = createFunctionNode(visibility, strdup(functionName), params, NULL, returnType, arena, state, lexer);
    if (!functionDefNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create function node.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to create function node.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    functionDefNode->data.functionDecl->paramTypes = paramTypes;
    functionDefNode->data.functionDecl->paramCount = paramCount;
    functionDefNode->data.functionDecl->parentScopeID = getNamespaceScopeID(context);
    functionDefNode->data.functionDecl->functionScopeID = Generate64BitHashID(functionName);

    // Parse the function block
    ASTNode *functionBlock = parseFunctionBlock(returnType, lexer, context, arena, state);
    if (!functionBlock)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse function block.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    ASTNode *functionNode = createFunctionNode(visibility, strdup(functionName), params, functionBlock, functionType, arena, state, lexer);
    functionNode->data.functionDecl->type = functionType;

    // Set the generic parameters if they exist
    if (genericParamCount > 0 && genericParams != NULL)
    {
        functionNode->data.functionDecl->genericParams = genericParams;
        functionNode->data.functionDecl->genericParamCount = genericParamCount;
    }

    // Check if the function has variadic parameters
    bool isVariadic = false;
    int lastParamIndex = functionNode->data.functionDecl->paramCount - 1;
    if (lastParamIndex >= 0)
    {
        ASTNode *lastParam = functionNode->data.functionDecl->params[lastParamIndex];
        if (lastParam->data.param->isVariadic)
        {
            isVariadic = true;
            functionNode->data.functionDecl->isVariadic = true;
        }
    }

    context->inGenericContext = false; // Reset generic context flag in ParsingContext
    resetCurrentFunction(context);     // Context Manager

    FEST->exitScope(FEST); // Exit the function scope

    return functionNode;
}
// </parseFunctionDeclaration>

// <parseExternFunctionDeclaration>
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing extern function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected function name.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    const char *namespaceScopeID = getNamespaceScopeID(context);
    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, context, arena, strdup(functionName), state);
    // get length of params
    int paramCount = 0;
    DataType **paramTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    for (int i = 0; params[i] != NULL; i++)
    {
        DataType *paramType = params[i]->data.param->type;
        logMessage(LMI, "INFO", "Parser", "Parameter type: %s", paramType->debug->toString(paramType));
        paramTypes[i] = params[i]->data.param->type;
        paramCount++;
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage(LMI, "INFO", "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, arena, state);
        getNextToken(lexer, arena, state);
    }
    else
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected `->` for return type.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    logMessage(LMI, "INFO", "Parser", "Function Return Type: %s", returnType->debug->toString(returnType));
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", arena, state, context);

    DataType *functionType = DTM->functionTypes->createFunctionType(paramTypes, paramCount, returnType);
    if (!functionType)
    {
        NEW_ERROR(GDM, CRYO_ERROR_NULL_DATA_TYPE, CRYO_SEVERITY_FATAL, "Failed to create function type.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    ASTNode *externFunc = createExternFuncNode(strdup(functionName), params, paramCount, functionType, arena, state, lexer);

    DTM->symbolTable->addEntry(
        DTM->symbolTable,
        namespaceScopeID,
        strdup(functionName),
        functionType);

    return externFunc;
}
// </parseExternFunctionDeclaration>

/// If the extern keywords next token is a string literal, it will follow notation simmilar
/// to how Rusts syntax looks like.
/// extern "C" { ... }
/// Function defintions must follow Cryo Syntax.
ASTNode *parseExternModuleDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing extern module declaration...");
    char *__moduleName__ = strndup(lexer->currentToken.start, lexer->currentToken.length);
    __moduleName__[lexer->currentToken.length - 1] = '\0';
    __moduleName__++;

    const char *moduleName = strdup(__moduleName__);
    logMessage(LMI, "INFO", "Parser", "Safe module name: %s", moduleName);
    consume(__LINE__, lexer, TOKEN_STRING_LITERAL, "Expected a string literal for extern declaration.", "parseExternModuleDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_LBRACE)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected '{' after extern module declaration.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected '{' after extern module declaration.", "parseExternModuleDeclaration", arena, state, context);

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        if (lexer->currentToken.type == TOKEN_KW_FN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing extern function declaration...");
            ASTNode *externFunc = parseExternFunctionDeclaration(lexer, context, arena, state);
            if (!externFunc)
            {
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse extern function declaration.", __LINE__, __FILE__, __func__)
                return NULL;
            }
            DTM->symbolTable->addEntry(
                DTM->symbolTable,
                moduleName,
                externFunc->data.externFunction->name,
                externFunc->data.externFunction->type);
            // Add to the program node
            addStatementToProgram(context->programNodePtr, externFunc, arena, state);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Expected 'function' keyword for extern function declaration.");
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected 'function' keyword for extern function declaration.", __LINE__, __FILE__, __func__)
        }
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected '}' to end extern module declaration.", "parseExternModuleDeclaration", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Extern module declaration parsed.");

    return createDiscardNode(arena, state, lexer);
}

// <parseFunctionCall>
ASTNode *parseFunctionCall(Lexer *lexer, ParsingContext *context,
                           char *functionName, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function call...");

    Token token = lexer->currentToken;

    // Check if the function name is a struct declaration.
    // In this case, we have to create a new struct instance and redirect the parser
    // to the struct declaration.
    DataType *dataType = DTM->symbolTable->lookup(DTM->symbolTable, functionName);
    if (dataType->container->primitive == PRIM_OBJECT)
    {
        logMessage(LMI, "INFO", "Parser", "Struct declaration detected.");
        return parseNewStructObject(functionName, lexer, context, arena, state);
    }

    // Create function call node
    ASTNode *functionCallNode = createFunctionCallNode(arena, state, lexer);
    functionCallNode->data.functionCall->name = strdup(functionName);
    functionCallNode->data.functionCall->argCount = 0;
    functionCallNode->data.functionCall->argCapacity = 8;
    functionCallNode->data.functionCall->args = (ASTNode **)ARENA_ALLOC(arena,
                                                                        functionCallNode->data.functionCall->argCapacity * sizeof(ASTNode *));

    char *functionNameToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionNameToken);

    // Consume function name
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
            "parseFunctionCall", arena, state, context);
    // Look up function in global symbol table (NEW)
    const char *currentScopeID = getCurrentScopeID(context);
    DataType *functionTypeSymbol = DTM->symbolTable->getEntry(
        DTM->symbolTable,
        currentScopeID,
        functionName);
    if (functionTypeSymbol)
    {
        logMessage(LMI, "INFO", "Parser", "Function type symbol found.");
        functionTypeSymbol->debug->printType(functionTypeSymbol);
        DataType **paramTypes = functionTypeSymbol->container->type.functionType->paramTypes;
        int paramCount = functionTypeSymbol->container->type.functionType->paramCount;
        logMessage(LMI, "INFO", "Parser", "Function type symbol param count: %d", paramCount);
        for (int i = 0; i < paramCount; i++)
        {
            logMessage(LMI, "INFO", "Parser", "Function type symbol param type: %s", paramTypes[i]->debug->toString(paramTypes[i]));
        }
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Function type symbol not found.");
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                  "Function undefined.", __LINE__, __FILE__, __func__)
    }

    if (lexer->currentToken.type != TOKEN_LPAREN)
    {
        logMessage(LMI, "ERROR", "Parser",
                   "Expected '(' after function name, got: %s", CryoTokenToString(lexer->currentToken.type));

        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected '(' after function name, got: %s", __LINE__, __FILE__, __func__)

        CONDITION_FAILED;
    }

    // Consume '('
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected '(' after function name.",
            "parseFunctionCall", arena, state, context);

    // Parse arguments if any
    if (lexer->currentToken.type != TOKEN_RPAREN)
    {
        for (int i = 0; lexer->currentToken.type != TOKEN_RPAREN; ++i)
        {
            CryoTokenType token = lexer->currentToken.type;
            if (lexer->currentToken.isOperator)
            {
                // Parse the Unary Expression
                ASTNode *arg = parseUnaryExpression(lexer, context, arena, state);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                continue;
            }
            logMessage(LMI, "INFO", "Parser", "Current Token Type: %s", CryoTokenToString(token));
            switch (token)
            {
            case TOKEN_INT_LITERAL:
            {
                DataType *expectedType = DTM->primitives->createInt();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                break;
            }

            case TOKEN_STRING_LITERAL:
            {
                char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
                int stringLength = strlen(stringLiteral);
                DataType *expectedType = DTM->primitives->createStr();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                break;
            }

            case TOKEN_BOOLEAN_LITERAL:
            {
                bool booleanValue = lexer->currentToken.type == TOKEN_KW_TRUE ? true : false;
                DataType *expectedType = DTM->primitives->createBoolean();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                break;
            }

            case TOKEN_IDENTIFIER:
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);
                logMessage(LMI, "INFO", "Parser", "Identifier: %s", identifier);
                Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
                logMessage(LMI, "INFO", "Parser", "Next Token: %s", CryoTokenToString(nextToken.type));
                if (nextToken.type == TOKEN_DOT)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing dot notation...");
                    ASTNode *dotNode = parseDotNotation(lexer, context, arena, state);
                    addArgumentToFunctionCall(functionCallNode, dotNode, arena, state);
                }
                else if (nextToken.type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing function call...");
                    ASTNode *funcCallNode = parseFunctionCall(lexer, context, identifier, arena, state);
                    addArgumentToFunctionCall(functionCallNode, funcCallNode, arena, state);
                }
                else
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing identifier...");
                    ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state);
                    if (!arg)
                    {
                        logMessage(LMI, "ERROR", "Parser", "Failed to parse identifier.");
                        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                                  "Failed to parse identifier.", __LINE__, __FILE__, __func__)
                        return NULL;
                    }
                    addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                }
                break;
            }

            case TOKEN_KW_THIS:
            {
                if (!context->thisContext)
                {
                    NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                              "Cannot use `this` keyword outside of a class context.", __LINE__, __FILE__, __func__)
                    return NULL;
                }

                // Consume `this` keyword
                consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.",
                        "parseFunctionCall", arena, state, context);

                if (lexer->currentToken.type == TOKEN_DOT)
                {
                    // Consume `.`
                    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` after `this` keyword.",
                            "parseFunctionCall", arena, state, context);
                }
                else
                {
                    NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                              "Expected `.` after `this` keyword.", __LINE__, __FILE__, __func__)
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
                                "parseFunctionCall", arena, state, context);
                        DataType *expectedType = contextProp->type;
                        expectedType->debug->printType(expectedType);
                        ASTNode *arg = contextNode;
                        addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                        break;
                    }

                    if (i == thisContextPropCount - 1)
                    {
                        NEW_ERROR(GDM, CRYO_ERROR_INVALID_ACCESS, CRYO_SEVERITY_FATAL,
                                  "Invalid `this` context. Property not found.", __LINE__, __FILE__, __func__)
                        return NULL;
                    }

                    if (lexer->currentToken.type == TOKEN_RPAREN)
                    {
                        break;
                    }
                }
                break;
            }

            case TOKEN_KW_TYPEOF:
            {
                ASTNode *arg = parseExpression(lexer, context, arena, state);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state);
                break;
            }

            case TOKEN_SEMICOLON:
            {
                consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after function call.",
                        "parseFunctionCall", arena, state, context);
                break;
            }

            case TOKEN_COMMA:
            {
                consume(__LINE__, lexer, TOKEN_COMMA, "Expected ',' between arguments.",
                        "parseFunctionCall", arena, state, context);
                break;
            }

            default:
            {
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                          "Invalid token in function call.", __LINE__, __FILE__, __func__)
                return NULL;
            }
            }

            // Handle comma between arguments
            if (lexer->currentToken.type == TOKEN_COMMA)
            {
                consume(__LINE__, lexer, TOKEN_COMMA, "Expected comma between arguments.",
                        "parseFunctionCall", arena, state, context);
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
            "parseFunctionCall", arena, state, context);

    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after function call.",
                "parseFunctionCall", arena, state, context);
    }

    return functionCallNode;
}
// </parseFunctionCall>

// <parseReturnStatement>
ASTNode *parseReturnStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing return statement...");

    if (lexer->currentToken.type == TOKEN_KW_RETURN)
    {
        consume(__LINE__, lexer, TOKEN_KW_RETURN, "Expected `return` keyword.", "parseReturnStatement", arena, state, context);
    }

    DataType *returnType = DTM->primitives->createVoid();
    ASTNode *expression = NULL;
    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        expression = parseExpression(lexer, context, arena, state);
        logMessage(LMI, "INFO", "Parser", "Return expression: %s", CryoNodeTypeToString(expression->metaData->type));
    }

    if (expression)
    {
        logMessage(LMI, "INFO", "Parser", "Return expression: %s", CryoNodeTypeToString(expression->metaData->type));
        if (expression->metaData->type == NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "INFO", "Parser", "Return Expression: Literal");
            expression->print(expression);
            expression->print(expression);
            returnType = expression->data.literal->type;
            logMessage(LMI, "INFO", "Parser", "Return expression data type: %s", returnType->debug->toString(returnType));
        }
        else if (expression->metaData->type == NODE_BINARY_EXPR)
        {
            logMessage(LMI, "INFO", "Parser", "Return Expression: Binary");
            expression->print(expression);
            returnType = DTM->primitives->createInt();
            logMessage(LMI, "INFO", "Parser", "Return expression data type: %s", returnType->debug->toString(returnType));
        }
        else if (expression->metaData->type == NODE_FUNCTION_CALL)
        {
            logMessage(LMI, "INFO", "Parser", "Return Expression: Function Call");
            expression->print(expression);
            returnType = expression->data.functionCall->returnType;
            logMessage(LMI, "INFO", "Parser", "Return expression data type: %s", returnType->debug->toString(returnType));
        }
        else if (expression->metaData->type == NODE_VAR_NAME)
        {
            logMessage(LMI, "INFO", "Parser", "Return Expression: Variable Name");
            expression->print(expression);
            returnType = expression->data.varName->type;
            logMessage(LMI, "INFO", "Parser", "Return expression data type: %s", returnType->debug->toString(returnType));
        }
        else if (expression->metaData->type == NODE_NULL_LITERAL)
        {
            logMessage(LMI, "INFO", "Parser", "Return Expression: Null Literal");
            expression->print(expression);
            returnType = DTM->primitives->createNull();
            logMessage(LMI, "INFO", "Parser", "Return expression data type: %s", returnType->debug->toString(returnType));
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Invalid return expression.");
            DEBUG_BREAKPOINT;
        }
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "No return expression.");
        returnType = DTM->primitives->createVoid();
    }

    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseReturnStatement", arena, state, context);
    }

    ASTNode *returnNode = createReturnNode(expression, returnType, arena, state, lexer);
    logMessage(LMI, "INFO", "Parser", "Return statement parsed.");
    return returnNode;
}
// </parseReturnStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Parameters                                          */

void validateParameterList(ASTNode **params, int paramCount, Arena *arena, CompilerState *state)
{
    for (int i = 0; i < paramCount; i++)
    {
        if (params[i]->data.param->isVariadic && i != paramCount - 1)
        {
            // Error: variadic parameter must be the last parameter
            logMessage(LMI, "ERROR", "Parser", "Variadic parameter must be the last parameter");
            // Handle the error appropriately...
        }
    }
}

// <parseParameter>
ASTNode *parseParameter(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing parameter...");
    Token currentToken = lexer->currentToken;

    // Handle ellipsis (varargs syntax)
    if (currentToken.type == TOKEN_ELLIPSIS)
    {
        // Consume the ellipsis token
        consume(__LINE__, lexer, TOKEN_ELLIPSIS, "Expected ellipsis.", "parseParameter", arena, state, context);

        // Check if there's an identifier after the ellipsis
        if (lexer->currentToken.type == TOKEN_IDENTIFIER)
        {
            // This is a named variadic parameter (e.g., ...args: T[])
            char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            getNextToken(lexer, arena, state);
            logMessage(LMI, "INFO", "Parser", "Variadic parameter name: %s", paramName);

            // Expect a colon for type annotation
            consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after variadic parameter name.", "parseParameter", arena, state, context);

            // Parse the element type (this should handle generic types too)
            DataType *elementType = parseType(lexer, context, arena, state);
            if (!elementType)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse element type for variadic parameter");
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse element type for variadic parameter", __LINE__, __FILE__, __func__)
                return NULL;
            }

            logMessage(LMI, "INFO", "Parser", "Variadic parameter element type: %s", elementType->debug->toString(elementType));

            // Create parameter node first
            ASTNode *node = createParamNode(strdup(paramName), strdup(functionName), NULL, arena, state, lexer);
            node->data.param->isVariadic = true;

            if (elementType->isArray)
            {
                // Regular array type for variadic parameter
                node->data.param->type = elementType;
                node->data.param->variadicElementType = elementType;
            }
            else
            {
                // Error: variadic parameter must have array type
                logMessage(LMI, "ERROR", "Parser", "Variadic parameter must have array type");
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Variadic parameter must have array type", __LINE__, __FILE__, __func__)
                return NULL;
            }
            DataType *paramType = node->data.param->type;
            logMessage(LMI, "INFO", "Parser", "Variadic parameter type: %s", paramType->debug->toString(paramType));

            // Consume the parameter type token
            getNextToken(lexer, arena, state);

            // Debug output
            node->print(node);

            return node;
        }
        else
        {
            // This is an unnamed variadic parameter (e.g., just ...)
            logMessage(LMI, "INFO", "Parser", "Unnamed variadic parameter");
            ASTNode *node = createParamNode("...", functionName, DTM->compilerDefs->create_VA_ARGS(), arena, state, lexer);
            node->data.param->isVariadic = true;
            node->print(node);
            return node;
        }
    }

    if (currentToken.type != TOKEN_IDENTIFIER &&
        currentToken.type != TOKEN_INT_LITERAL &&
        currentToken.type != TOKEN_STRING_LITERAL &&
        currentToken.type != TOKEN_BOOLEAN_LITERAL &&
        currentToken.type != TOKEN_KW_THIS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected an identifier, received: %s",
                   CryoTokenToString(currentToken.type));
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected an identifier, received: %s", __LINE__, __FILE__, __func__)
        return NULL;
    }

    char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Parameter name: %s", strdup(paramName));

    if (!DTM->validation->isIdentifierDataType(paramName))
    {
        logMessage(LMI, "ERROR", "Parser", "Parameter name cannot be a data type.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Parameter name cannot be a data type.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    getNextToken(lexer, arena, state);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", arena, state, context);

    DataType *paramType = parseType(lexer, context, arena, state);
    if (!paramType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse parameter type");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Failed to parse parameter type", __LINE__, __FILE__, __func__)
        return NULL;
    }

    logMessage(LMI, "INFO", "Parser", "Parameter Type Resolved...");

    const char *paramTypeStr = paramType->debug->toString(paramType);
    logMessage(LMI, "INFO", "Parser", "<!> Parameter type: %s", paramTypeStr);

    // Consume data type token
    getNextToken(lexer, arena, state);

    // Create parameter node
    ASTNode *node = createParamNode(strdup(paramName), strdup(functionName), paramType, arena, state, lexer);
    node->data.param->type = paramType;

    FEST->addSymbol(FEST, node);

    return node;
}
// </parseParameter>

// <parseParameterList>
ASTNode **parseParameterList(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing parameter list...");
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start parameter list.", "parseParameterList", arena, state, context);

    ASTNode **paramListNode = (ASTNode **)malloc(16 * sizeof(ASTNode *));
    if (!paramListNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to allocate memory for parameter list.");
        return NULL;
    }

    int paramCount = 0;
    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *param = parseParameter(lexer, context, arena, functionName, state);
        if (param)
        {
            if (param->metaData->type == NODE_PARAM)
            {
                logMessage(LMI, "INFO", "Parser", "Adding parameter: %s", param->data.param->name);
                paramListNode[paramCount] = param;
                paramCount++;
            }
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse parameter.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    paramListNode[paramCount] = NULL; // Null terminate the parameter array

    logMessage(LMI, "INFO", "Parser", "Parameter count: %d", paramCount);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end parameter list.", "parseParameterList", arena, state, context);

    return paramListNode;
}
// </parseParameterList>

// <parseArguments>
ASTNode *parseArguments(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing arguments...");

    Token currentToken = lexer->currentToken;

    if (currentToken.type != TOKEN_IDENTIFIER &&
        currentToken.type != TOKEN_INT_LITERAL &&
        currentToken.type != TOKEN_STRING_LITERAL &&
        currentToken.type != TOKEN_BOOLEAN_LITERAL &&
        currentToken.type != TOKEN_KW_THIS &&
        currentToken.type != TOKEN_AMPERSAND &&
        currentToken.type != TOKEN_STAR)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected an identifier or literal, received: %s", CryoTokenToString(currentToken.type));
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL, "Expected an identifier or literal, received: %s", __LINE__, __FILE__, __func__)
        return NULL;
    }

    char *argName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    bool isLiteral = false;
    DataType *argType = NULL;
    CryoNodeType nodeType = NODE_UNKNOWN;

    if (currentToken.type == TOKEN_AMPERSAND ||
        currentToken.type == TOKEN_STAR)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing reference or pointer argument...");
        const char *argumentName = peekNextUnconsumedToken(lexer, arena, state).lexeme;
        logMessage(LMI, "INFO", "Parser", "Argument name: %s", argName);
        ASTNode *arg = parseUnaryExpression(lexer, context, arena, state);
        if (arg)
        {
            logMessage(LMI, "INFO", "Parser", "Adding argument to list...");
            argName = strdup(argumentName);
            logMessage(LMI, "INFO", "Parser", "Argument name: %s", argName);
            argType = DTM->astInterface->getTypeofASTNode(arg);
            logMessage(LMI, "INFO", "Parser", "Argument type: %s", argType->debug->toString(argType));
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse argument.");
            return NULL;
        }
    }

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is an integer literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
        argType = DTM->primitives->createI32();
    }
    else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a string literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
        argType = DTM->primitives->createPrimString(strndup(lexer->currentToken.start, lexer->currentToken.length));
    }
    else if (lexer->currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a boolean literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
        bool booleanValue = lexer->currentToken.type == TOKEN_KW_TRUE ? true : false;
        argType = DTM->primitives->createPrimBoolean(booleanValue);
    }
    else if (lexer->currentToken.type == TOKEN_KW_THIS)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is `this` keyword");
        nodeType = NODE_THIS;
        return parseThisContext(lexer, context, arena, state);
    }
    else if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Argument is a dot notation");
            return parseDotNotation(lexer, context, arena, state);
        }
        if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Argument is a function call");
            return parseFunctionCall(lexer, context, argName, arena, state);
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "Argument is an identifier");
            FrontendSymbol *sym = FEST->lookup(FEST, argName);
            if (sym)
            {
                logMessage(LMI, "INFO", "Parser", "Symbol found in symbol table: %s", argName);
                argType = sym->type;
                nodeType = NODE_VAR_NAME;
                logMessage(LMI, "INFO", "Parser", "Argument is a variable name");
                ASTNode *varNameNode = createVarNameNode(strdup(argName), argType, arena, state, lexer);
                consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseArguments", arena, state, context);
                return varNameNode;
            }
            else
            {
                logMessage(LMI, "ERROR", "Parser", "Symbol not found in symbol table.");
                NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL, "Symbol not found in symbol table.", __LINE__, __FILE__, __func__)
                return NULL;
            }
        }
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Argument is not a literal");
    }

    // Resolve the type using the symbol table
    logMessage(LMI, "INFO", "Parser", "Resolving argument type...");
    if (!isLiteral)
    {
        const char *curScopeID = getCurrentScopeID(context);
        FrontendSymbol *sym = FEST->lookup(FEST, argName);
        if (sym)
        {
            logMessage(LMI, "INFO", "Parser", "Symbol found in global table: %s", argName);
            argType = sym->type;
            nodeType = NODE_VAR_NAME;
            logMessage(LMI, "INFO", "Parser", "Argument is a variable name");
            ASTNode *varNameNode = createVarNameNode(strdup(argName), argType, arena, state, lexer);
            return varNameNode;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in global table.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL, "Symbol not found in global table.", __LINE__, __FILE__, __func__)
            return NULL;
        }
    }
    else
    {
        if (lexer->currentToken.type == TOKEN_INT_LITERAL)
        {
            argType = DTM->primitives->createI32();
        }
        else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
        {
            char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
            int stringLength = strlen(stringLiteral);
            argType = DTM->primitives->createPrimString(stringLiteral);
        }
        else if (lexer->currentToken.type == TOKEN_BOOLEAN_LITERAL)
        {
            bool booleanValue = lexer->currentToken.type == TOKEN_KW_TRUE ? true : false;
            argType = DTM->primitives->createPrimBoolean(booleanValue);
        }
    }

    // Consume the argument name
    getNextToken(lexer, arena, state);

    return createArgsNode(argName, argType, nodeType, isLiteral, arena, state, lexer);
}
// </parseArguments>

// <parseArgumentList>
ASTNode *parseArgumentList(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing argument list...");
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start argument list.", "parseArgumentList", arena, state, context);
    ASTNode *argListNode = createArgumentListNode(arena, state, lexer);
    if (argListNode == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create argument list node.");
        return NULL;
    }

    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        ASTNode *arg = parseArguments(lexer, context, arena, state);
        if (arg)
        {
            logMessage(LMI, "INFO", "Parser", "Adding argument to list...");
            addArgumentToList(argListNode, arg, arena, state);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse argument.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            logMessage(LMI, "INFO", "Parser", "Consuming comma...");
            getNextToken(lexer, arena, state);
        }
    }

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end argument list.", "parseArgumentList", arena, state, context);
    return argListNode;
}
// </parseArgumentList>

// <parseArgumentsWithExpectedType>
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, ParsingContext *context, DataType *expectedType, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    if (lexer->currentToken.type == TOKEN_LPAREN)
    {
        consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start arguments.", "parseArgumentsWithExpectedType", arena, state, context);
    }
    logMessage(LMI, "INFO", "Parser", "Parsing arguments with expected type...");
    if (
        lexer->currentToken.type != TOKEN_IDENTIFIER &&
        lexer->currentToken.type != TOKEN_KW_THIS &&
        lexer->currentToken.type != TOKEN_INT_LITERAL &&
        lexer->currentToken.type != TOKEN_STRING_LITERAL &&
        lexer->currentToken.type != TOKEN_BOOLEAN_LITERAL)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected an identifier, got: %s", CryoTokenToString(lexer->currentToken.type));
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected an identifier, got: %s", __LINE__, __FILE__, __func__)
        return NULL;
    }

    char *argName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    bool usingDotNotation = false;
    bool isLiteral = false;
    CryoNodeType nodeType = NODE_UNKNOWN;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is an integer literal");
        expectedType = DTM->primitives->createInt();
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_STRING_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a string literal");
        char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
        int stringLength = strlen(stringLiteral);
        expectedType = DTM->primitives->createStr();
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;

        // Trim the quotes from the string literal
        argName = strndup(lexer->currentToken.start + 1, lexer->currentToken.length - 2);
    }
    else if (lexer->currentToken.type == TOKEN_BOOLEAN_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a boolean literal");
        bool booleanValue = lexer->currentToken.type == TOKEN_KW_TRUE ? true : false;
        expectedType = DTM->primitives->createBoolean();
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
    }
    else if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is an identifier: %s", argName);
        // Check for dot notation
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token prevToken = context->lastTokens[1];
        logMessage(LMI, "INFO", "Parser", "Prev token: %s", CryoTokenToString(prevToken.type));
        if (nextToken.type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Dot notation detected.");
            usingDotNotation = true;
        }

        // Look up the identifier in the symbol table

        char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Identifier: %s", identifier);

        // Look up the identifier in the symbol table
        FrontendSymbol *symbol = FEST->lookup(FEST, identifier);
        if (symbol)
        {
            logMessage(LMI, "INFO", "Parser", "Symbol found in global table: %s", identifier);
            expectedType = symbol->type;
            isLiteral = false;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in global table.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                      "Symbol not found in global table.", __LINE__, __FILE__, __func__)
            return NULL;
        }

        DataType *symbolDataType = symbol->type;
        if (!symbolDataType)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to get data type from symbol.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_DATA_TYPE, CRYO_SEVERITY_FATAL,
                      "Failed to get data type from symbol.", __LINE__, __FILE__, __func__)
            return NULL;
        }

        context->thisContext->type = symbolDataType;
        context->thisContext->nodeName = strdup(identifier);

        if (usingDotNotation)
        {
            logMessage(LMI, "INFO", "Parser", "Dot notation detected.");
            // Parse through the dot notation
            ASTNode *dotExpr = parseDotNotation(lexer, context, arena, state);
            if (!dotExpr)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse dot expression.");
                NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                          "Failed to parse dot expression.", __LINE__, __FILE__, __func__)
                return NULL;
            }

            logMessage(LMI, "INFO", "Parser", "Dot expression parsed.");
            return dotExpr;
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "Argument is not using dot notation.");
        }

        context->thisContext->type = expectedType;

        if (prevToken.type == TOKEN_KW_THIS)
        {
            logMessage(LMI, "INFO", "Parser", "Argument is a 'this' keyword");
            return parseForThisValueProperty(lexer, expectedType, context, arena, state);
        }

        nodeType = NODE_VAR_NAME;

        // New Global Symbol Table Lookup
        const char *curScopeID = getCurrentScopeID(context);
        FrontendSymbol *sym = FEST->lookup(FEST, argName);
        if (sym)
        {
            logMessage(LMI, "INFO", "Parser", "Symbol found in global table.");
            expectedType = sym->type;
            isLiteral = false;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in global table.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                      "Symbol not found in global table.", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }
    }
    else if (lexer->currentToken.type == TOKEN_KW_THIS)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a 'this' keyword");
        return parseExpectedTypeArgWithThisKW(lexer, expectedType, context, arena, state);
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Argument is not a literal");
        FrontendSymbol *sym = FEST->lookup(FEST, argName);
        if (!sym)
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in the global table.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                      "Symbol not found in the global table.", __LINE__, __FILE__, __func__)
            return NULL;
        }

        DataType *symbolType = sym->type;
        if (!symbolType)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to get data type from symbol.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_DATA_TYPE, CRYO_SEVERITY_FATAL,
                      "Failed to get data type from symbol.", __LINE__, __FILE__, __func__)
            return NULL;
        }
        expectedType = symbolType;
        logMessage(LMI, "INFO", "Parser", "Argument type: %s", expectedType->debug->toString(expectedType));
        isLiteral = false;
    }

    // Consume the argument name
    getNextToken(lexer, arena, state);

    logMessage(LMI, "INFO", "Parser", "Creating argument node with expected type: %s", expectedType->debug->toString(expectedType));
    logMessage(LMI, "INFO", "Parser", "Argument name: %s", strdup(argName));

    return createArgsNode(argName, expectedType, nodeType, isLiteral, arena, state, lexer);
}
// </parseArgumentsWithExpectedType>

// <parseExpectedTypeArgWithThisKW>
ASTNode *parseExpectedTypeArgWithThisKW(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Argument is a 'this' keyword");
    // We have to check if the this keyword is using dot notation
    Token nextToken = peekNextUnconsumedToken(lexer, arena, state);

    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected 'this' keyword.", "parseExpectedTypeArgWithThisKW", arena, state, context);

    if (nextToken.type == TOKEN_DOT)
    {
        logMessage(LMI, "INFO", "Parser", "Dot notation detected.");
        consume(__LINE__, lexer, TOKEN_DOT, "Expected '.' after 'this' keyword.", "parseExpectedTypeArgWithThisKW", arena, state, context);

        // Check the `thisContext` in the parsing context to see if it's inside a struct
        if (context->thisContext == NULL)
        {
            logMessage(LMI, "ERROR", "Parser", "Expected 'this' keyword to be used inside a struct.");
            NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                      "Expected 'this' keyword to be used inside a struct.", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Parser", "Accessing struct properties...");
        ASTNode **accessProperties = context->thisContext->properties;
        int propertyCount = context->thisContext->propertyCount;
        logMessage(LMI, "INFO", "Parser", "Property count: %d", propertyCount);

        // Check the next token identifier and match it with the properties of the struct
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            logMessage(LMI, "ERROR", "Parser", "Expected identifier after 'this' keyword. Received: %s", CryoTokenToString(lexer->currentToken.type));
            NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                      "Expected identifier after 'this' keyword. Received: %s", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Parser", "Property name: %s", strndup(lexer->currentToken.start, lexer->currentToken.length));
        ASTNode *matchedProperty = NULL;
        char *propertyName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        for (int i = 0; i < propertyCount; i++)
        {
            char *accessPropName = strdup(accessProperties[i]->data.property->name);
            logMessage(LMI, "INFO", "Parser", "Access property name: %s", accessPropName);
            if (strcmp(accessPropName, strdup(propertyName)) == 0)
            {
                expectedType = accessProperties[i]->data.property->type;
                matchedProperty = accessProperties[i];

                logMessage(LMI, "INFO", "Parser", "Matched property: %s", accessPropName);
                break;
            }
        }

        if (!matchedProperty)
        {
            logMessage(LMI, "ERROR", "Parser", "Property not found in struct.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_PROPERTY, CRYO_SEVERITY_FATAL,
                      "Property not found in struct.", __LINE__, __FILE__, __func__)
            CONDITION_FAILED;
        }

        ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, lexer);
        if (!propAccessNode)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                      "Failed to create property access node.", __LINE__, __FILE__, __func__)
        }
        propAccessNode->data.propertyAccess->objectTypeName = expectedType->debug->toString(expectedType);
        propAccessNode->data.propertyAccess->objectType = expectedType;

        return propAccessNode;
    }

    // If the this keyword is not using dot notation, then it's just a reference to the struct
    // For now though, the compiler will only support dot notation for accessing struct properties
    logMessage(LMI, "ERROR", "Parser", "Expected dot notation after 'this' keyword.");
    NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
              "Expected dot notation after 'this' keyword.", __LINE__, __FILE__, __func__)
}
// </parseExpectedTypeArgWithThisKW>

// <addParameterToList>
void addParameterToList(ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding parameter to list...");
    if (paramListNode->metaData->type == NODE_PARAM_LIST)
    {
        ParamNode *paramList = paramListNode->data.paramList;
        if (paramList->paramCount >= paramList->paramCapacity)
        {
            paramList->paramCapacity *= 2;
            paramList->params = (CryoVariableNode **)realloc(paramList->params, paramList->paramCapacity * sizeof(CryoVariableNode *));
            if (!paramList->params)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for parameters.");
                return;
            }
        }
        param->data.varDecl->isMutable = true;
        paramList->params[paramList->paramCount++] = param->data.varDecl;
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Expected parameter list node.");
    }
}
// </addParameterToList>

// <addArgumentToList>
void addArgumentToList(ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding argument to list...");
    if (argListNode->metaData->type == NODE_ARG_LIST)
    {
        ArgNode *argList = argListNode->data.argList;
        if (argList->argCount >= argList->argCapacity)
        {
            argList->argCapacity *= 2;
            argList->args = (ASTNode **)realloc(argList->args, argList->argCapacity * sizeof(ASTNode *));
            if (!argList->args)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for arguments.");
                return;
            }
        }

        logMessage(LMI, "INFO", "Parser", "Adding argument to list...");
        argList->args[argList->argCount++] = arg;
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Expected argument list node.");
    }
}
// </addArgumentToList>

// <addArgumentToFunctionCall>
void addArgumentToFunctionCall(ASTNode *functionCallNode, ASTNode *arg, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding argument to function call...");
    if (functionCallNode->metaData->type == NODE_FUNCTION_CALL)
    {
        FunctionCallNode *funcCall = functionCallNode->data.functionCall;
        if (funcCall->argCount >= funcCall->argCapacity)
        {
            funcCall->argCapacity *= 2;
            funcCall->args = (ASTNode **)realloc(funcCall->args, funcCall->argCapacity * sizeof(ASTNode *));
            if (!funcCall->args)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for arguments.");
                return;
            }
        }

        funcCall->args[funcCall->argCount++] = arg;
        logMessage(LMI, "INFO", "Parser", "Added argument to function call node.");
        logASTNode(arg);
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Expected function call node.");
    }
}
// </addArgumentToFunctionCall>

// <addParameterToExternDecl>
void addParameterToExternDecl(ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding parameter to extern declaration...");
    if (externDeclNode->metaData->type == NODE_EXTERN_FUNCTION)
    {
        ExternNode *externDecl = externDeclNode->data.externNode;
        if (externDecl->externNode->data.functionDecl->paramCount >= externDecl->externNode->data.functionDecl->paramCapacity)
        {
            externDecl->externNode->data.functionDecl->paramCapacity *= 2;
            externDecl->externNode->data.functionDecl->params = (ASTNode **)realloc(externDecl->externNode->data.functionDecl->params, externDecl->externNode->data.functionDecl->paramCapacity * sizeof(ASTNode **));
            if (!externDecl->externNode->data.functionDecl->params)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for parameters.");
                return;
            }

            externDecl->externNode->data.functionDecl->params[externDecl->externNode->data.functionDecl->paramCount++] = param;

            logMessage(LMI, "INFO", "Parser", "Parameter added to extern declaration.");
        }

        else
        {
            logMessage(LMI, "ERROR", "Parser", "Expected extern declaration node.");
        }
    }
}
// </addParameterToExternDecl>

/* ====================================================================== */
/* @ASTNode_Parsing - Modules & Externals                                 */

// <importTypeDefinitions>
void importTypeDefinitions(const char *module, const char *subModule, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Importing type definitions...");

    DEBUG_BREAKPOINT;
    return;
}
// </importTypeDefinitions>

// <parseExtern>
ASTNode *parseExtern(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing extern...");
    consume(__LINE__, lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_FN:
        return parseExternFunctionDeclaration(lexer, context, arena, state);
    case TOKEN_STRING_LITERAL:
        return parseExternModuleDeclaration(lexer, context, arena, state);
    default:
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected `fn` or string literal after `extern` keyword.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    return NULL;
}
// </parseExtern>

/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode *parseIfStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing if statement...");
    consume(__LINE__, lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", arena, state, context);
    context->isParsingIfCondition = true;

    ASTNode *condition = parseIfCondition(lexer, context, arena, state);
    ASTNode *ifBlock = parseBlock(lexer, context, arena, state);
    ASTNode *elseBlock = NULL;

    if (lexer->currentToken.type == TOKEN_KW_ELSE)
    {
        getNextToken(lexer, arena, state);
        if (lexer->currentToken.type == TOKEN_KW_IF)
        {
            elseBlock = parseIfStatement(lexer, context, arena, state);
        }
        else
        {
            elseBlock = parseBlock(lexer, context, arena, state);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock, arena, state, lexer);
}
// </parseIfStatement>

ASTNode *parseIfCondition(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing if condition...");
    char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Current token: %s", cur_token);

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start if condition.", "parseIfCondition", arena, state, context);

    char *cur_token_cpy = strndup(lexer->currentToken.start, lexer->currentToken.length);

    logMessage(LMI, "INFO", "Parser", "Current token: %s", cur_token_cpy);
    ASTNode *condition = parseExpression(lexer, context, arena, state);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end if condition.", "parseIfCondition", arena, state, context);

    return condition;
}

// <parseForLoop>
ASTNode *parseForLoop(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    // {VAR_DECL}? ; {CONDITION} ; {INCREMENT} {BLOCK}
    logMessage(LMI, "INFO", "Parser", "Parsing for loop...");
    consume(__LINE__, lexer, TOKEN_KW_FOR, "Expected `for` keyword.", "parseForLoop", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start for loop.", "parseForLoop", arena, state, context);

    ASTNode *init = NULL;
    ASTNode *condition = NULL;
    ASTNode *increment = NULL;
    ASTNode *body = NULL;

    logMessage(LMI, "INFO", "Parser", "Parsing for loop initialization...");
    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        init = parseVarDeclaration(lexer, context, arena, state);
    }
    init->print(init);

    logMessage(LMI, "INFO", "Parser", "Parsing for loop condition...");
    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        condition = parseExpression(lexer, context, arena, state);
    }
    condition->print(condition);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after for loop condition.", "parseForLoop", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Parsing for loop increment...");
    if (lexer->currentToken.type != TOKEN_RPAREN)
    {
        increment = parseExpression(lexer, context, arena, state);
    }
    increment->print(increment);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Parsing for loop block...");
    ASTNode *block = parseBlock(lexer, context, arena, state);
    if (block == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse for loop block.");
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Failed to parse for loop block.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    block->print(block);

    logMessage(LMI, "INFO", "Parser", "Creating for loop node...");
    ASTNode *forLoopNode = createForLoopNode(init, condition, increment, block, arena, state, lexer);
    if (forLoopNode == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create for loop node.");
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                  "Failed to create for loop node.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    forLoopNode->print(forLoopNode);

    logMessage(LMI, "INFO", "Parser", "For loop node created successfully.");
    return forLoopNode;
}
// </parseForLoop>

// <parseWhileStatement>
ASTNode *parseWhileStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing while statement...");
    consume(__LINE__, lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", arena, state, context);

    ASTNode *condition = parseExpression(lexer, context, arena, state);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", arena, state, context);

    ASTNode *body = parseBlock(lexer, context, arena, state);
    return createWhileStatement(condition, body, arena, state, lexer);
}
// </parseWhileStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode *parseArrayLiteral(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing array literal...");
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", arena, state, context);

    ASTNode *arrayNode = createArrayLiteralNode(arena, state, lexer);
    if (arrayNode == NULL)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }
    arrayNode->data.array->elementCapacity = ARRAY_CAPACITY;
    arrayNode->data.array->elementTypes = (DataType **)calloc(ARRAY_CAPACITY, sizeof(DataType *));
    int elementCount = 0;
    DataType *baseType = NULL;

    while (lexer->currentToken.type != TOKEN_RBRACKET)
    {
        ASTNode *element = parseExpression(lexer, context, arena, state);
        if (element)
        {
            addElementToArrayLiteral(arrayNode, element, arena, state);
            logMessage(LMI, "INFO", "Parser", "Element added to array literal.");

            DataType *elType = DTM->astInterface->getTypeofASTNode(element);
            if (elType == NULL)
            {
                NEW_ERROR(GDM, CRYO_ERROR_NULL_DATA_TYPE, CRYO_SEVERITY_FATAL,
                          "Failed to get data type from AST node.", __LINE__, __FILE__, __func__)
            }
            arrayNode->data.array->elementTypes[elementCount] = elType;
            logMessage(LMI, "INFO", "Parser", "Element type: %s", elType->debug->toString(elType));
            if (elementCount == 0)
            {
                baseType = elType;
            }
            elementCount++;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse array element.");
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
        }
    }

    arrayNode->data.array->elementCount = elementCount;

    DataType *arrayType = DTM->arrayTypes->createMonomorphicArray(baseType, elementCount);
    arrayNode->data.array->type = arrayType;

    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", arena, state, context);
    return arrayNode;
}
// </parseArrayLiteral>

// <addElementToArrayLiteral>
void addElementToArrayLiteral(ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding element to array literal...");
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
                logMessage(LMI, "ERROR", "Parser", "Failed to reallocate memory for array elements.");
                return;
            }

            arrayLiteral->data.array->elements = newElements;
            arrayLiteral->data.array->elementCapacity = newCapacity;
        }

        arrayLiteral->data.array->elements[arrayLiteral->data.array->elementCount++] = element;
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Expected array literal node.");
    }
}
// <addElementToArrayLiteral>

// <parseArrayIndexing>
ASTNode *parseArrayIndexing(Lexer *lexer, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing array indexing...");
    char *arrName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *arrCpyName = strdup(arrName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseArrayIndexing", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array indexing.", "parseArrayIndexing", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Array name: %s", arrCpyName);
    ASTNode *arrNode = NULL;

    FrontendSymbol *sym = FEST->lookup(FEST, arrCpyName);
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Array not found.");
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                  "Array not found.", __LINE__, __FILE__, __func__)
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Array found.");
        if (sym->node->metaData->type == NODE_VAR_DECLARATION)
        {
            DataType *vartype = sym->type;
            const char *varName = sym->name;
            ASTNode *varNameNode = createVarNameNode(strdup(varName), vartype, arena, state, lexer);
            arrNode = varNameNode;
        }
        else if (sym->node->metaData->type == NODE_PARAM)
        {
            DataType *vartype = sym->type;
            const char *varName = sym->name;
            ASTNode *paramNode = sym->node;
            arrNode = paramNode;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Array is not a variable declaration, received: %s", CryoNodeTypeToString(sym->node->metaData->type));
            NEW_ERROR(GDM, CRYO_ERROR_INVALID_NODE_TYPE, CRYO_SEVERITY_FATAL,
                      "Array is not an indexable type.", __LINE__, __FILE__, __func__)
            return NULL;
        }
    }
    ASTNode *index = parseExpression(lexer, context, arena, state);
    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array indexing.", "parseArrayIndexing", arena, state, context);
    ASTNode *indexExprNode = createIndexExprNode(strdup(arrCpyName), arrNode, index, arena, state, lexer);
    bool isMultiDimensional = false;
    // If the next token in view is another `[`, this is a multi-dimensional array indexing.
    if (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        isMultiDimensional = true;
        while (lexer->currentToken.type != TOKEN_EQUAL)
        {
            consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array indexing.", "parseArrayIndexing", arena, state, context);
            ASTNode *index = parseExpression(lexer, context, arena, state);
            consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array indexing.", "parseArrayIndexing", arena, state, context);
            indexExprNode->data.indexExpr->addIndex(indexExprNode, index);
            logMessage(LMI, "INFO", "Parser", "Added index to multi-dimensional array indexing.");
        }
    }

    indexExprNode->data.indexExpr->isMultiDimensional = isMultiDimensional;

    return indexExprNode;
}
// </parseArrayIndexing>

/* ====================================================================== */
/* @ASTNode_Parsing - Assignments                                         */

ASTNode *parseAssignment(Lexer *lexer, ParsingContext *context, char *varName, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing assignment...");
    char *_varName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *varNameCpy = strdup(_varName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseAssignment", arena, state, context);
    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for assignment.", "parseAssignment", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Variable name: %s", varNameCpy);

    FrontendSymbol *sym = FEST->lookup(FEST, varNameCpy);
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find symbol.");
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                  "Failed to find symbol.", __LINE__, __FILE__, __func__)
    }
    ASTNode *symbolNode = sym->node;
    ASTNode *oldValue = symbolNode;
    ASTNode *newValue = parseExpression(lexer, context, arena, state);

    bool isMutable = false;
    CryoNodeType oldType = oldValue->metaData->type;
    switch (oldType)
    {
    case NODE_VAR_DECLARATION:
    {
        logMessage(LMI, "INFO", "Parser", "Old value is a variable declaration.");
        isMutable = oldValue->data.varDecl->isMutable;
        break;
    }
    case NODE_PARAM:
    {
        logMessage(LMI, "INFO", "Parser", "Old value is a parameter.");
        isMutable = true;
        break;
    }
    default:
    {
        logMessage(LMI, "ERROR", "Parser", "Old value is not a variable declaration.");
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_NODE_TYPE, CRYO_SEVERITY_FATAL,
                  "Old value is not a variable declaration.", __LINE__, __FILE__, __func__)
        DEBUG_BREAKPOINT;

        return NULL;
    }
    }

    // Check if the symbol is mutable
    if (!isMutable)
    {
        logMessage(LMI, "ERROR", "Parser", "Variable is not mutable.");
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_MUTABILITY, CRYO_SEVERITY_FATAL,
                  "Variable is not mutable.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseAssignment", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Creating assignment node...");

    ASTNode *assignment = createVarReassignment(strdup(varNameCpy), oldValue, newValue, arena, state, lexer);
    logMessage(LMI, "INFO", "Parser", "Assignment node created.");
    return assignment;
}

ASTNode *parseThisContext(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing `this` context...");
    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.", "parseThisContext", arena, state, context);

    if (context->thisContext == NULL)
    {
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                  "Expected `this` keyword to be used inside a struct.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    if (context->thisContext->isStatic)
    {
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                  "Expected `this` keyword to be used inside a non-static context.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    CryoTokenType currentToken = lexer->currentToken.type;
    const char *typeName = context->thisContext->nodeName;
    logMessage(LMI, "INFO", "Parser", "Current token: %s", CryoTokenToString(currentToken));
    logMessage(LMI, "INFO", "Parser", "Type name: %s", typeName);
    DataType *thisType = DTM->symbolTable->lookup(DTM->symbolTable, typeName);
    if (thisType == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find type for `this` context: %s", typeName);
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_TYPE, CRYO_SEVERITY_FATAL,
                  "Failed to find type for `this` context.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    thisType->debug->printVerbosType(thisType);

    FrontendSymbol *thisSymbol = FEST->lookup(FEST, context->thisContext->nodeName);
    if (thisSymbol == NULL)
    {
        FEST->printTable(FEST);

        logMessage(LMI, "ERROR", "Parser", "Failed to find symbol for `this` context: %s", context->thisContext->nodeName);
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                  "Failed to find symbol for `this` context.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    ASTNode *thisNode;
    if (currentToken == TOKEN_DOT)
    {
        consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseThisContext", arena, state, context);

        thisNode = parseDotNotation(lexer, context, arena, state);
        thisNode->print(thisNode);
    }
    else
    {
        thisNode = createThisNode(arena, state, lexer);
        thisNode->data.thisNode->objectType = thisType;
    }

    // Check if we are setting a property of the `this` context with `=`
    if (lexer->currentToken.type == TOKEN_EQUAL)
    {
        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for property reassignment.", "parseThisContext", arena, state, context);
        char *propName = (char *)malloc(sizeof(char) * 1024);

        if (thisNode->metaData->type == NODE_PROPERTY_ACCESS)
        {
            thisNode->print(thisNode);
            PropertyAccessNode *propAccessNode = thisNode->data.propertyAccess;
            strcpy(propName, propAccessNode->propertyName);
            logMessage(LMI, "INFO", "Parser", "Property name: %s", propName);
        }

        if (propName == NULL)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to allocate memory for property name.");
            DEBUG_BREAKPOINT;
        }

        ASTNode *newValue = parseExpression(lexer, context, arena, state);
        if (newValue == NULL)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse new value for property reassignment.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                      "Failed to parse new value for property reassignment.", __LINE__, __FILE__, __func__)
            return NULL;
        }

        if (lexer->currentToken.type == TOKEN_SEMICOLON)
            consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon...", "parseThisContext", arena, state, context);

        ASTNode *propReasignment = createPropertyReassignmentNode(thisNode, propName, newValue, arena, state, lexer);
        propReasignment->data.propertyReassignment->objectTypeName = context->thisContext->nodeName;
        propReasignment->data.propertyReassignment->objectType = thisType;
        propReasignment->data.propertyReassignment->value = newValue;

        newValue->print(newValue);

        propReasignment->print(propReasignment);
        logMessage(LMI, "INFO", "Parser", "Property reassignment node created.");

        ASTNode *prop_new_val = propReasignment->data.propertyReassignment->value;
        prop_new_val->print(prop_new_val);
        return propReasignment;
    }

    logMessage(LMI, "INFO", "Parser", "Finished parsing `this` context.");
    return thisNode;
}

ASTNode *parseLHSIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, DataType *typeOfNode)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing LHS identifier...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected identifier for LHS.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing dot notation...");

    CryoTokenType currentToken = lexer->currentToken.type;

    switch (currentToken)
    {
    case TOKEN_KW_THIS:
    {
        return parseThisContext(lexer, context, arena, state);
    }
    case TOKEN_IDENTIFIER:
    {
        return parseIdentifierDotNotation(lexer, context, arena, state);
    }
    default:
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected identifier or `this` keyword for dot notation.", __LINE__, __FILE__, __func__)
        return NULL;
    }
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseIdentifierDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logParsingContext(context);

    // Check the last token in the context
    Token lastToken = context->lastTokens[1]; // We +1 because the last token should be a dot.
    Token currentToken = lexer->currentToken;

    if (lastToken.type == TOKEN_KW_THIS)
    {
        ThisContext *thisContext = context->thisContext;
        char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseIdentifierDotNotation", arena, state, context);

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
                    logMessage(LMI, "INFO", "Parser", "Property found in struct. Name: %s", propName);
                    ASTNode *propertyAcessNode = createPropertyAccessNode(properties[i], propName, arena, state, lexer);
                    propertyAcessNode->data.propertyAccess->propertyIndex = i;
                    propertyAcessNode->data.propertyAccess->objectTypeName = thisContext->type->typeName;

                    return propertyAcessNode;
                }
                else
                {
                    logMessage(LMI, "ERROR", "Parser", "Property not found in struct.");
                }
            }

            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_PROPERTY, CRYO_SEVERITY_FATAL,
                      "Property not found in struct.", __LINE__, __FILE__, __func__)
            return NULL;
        }
        else
        {
            NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                      "Expected `this` keyword to be used inside a struct.", __LINE__, __FILE__, __func__)
            return NULL;
        }
    }
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        // Get the identifier name
        char *identifierName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Identifier name: %s", identifierName);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseIdentifierDotNotation", arena, state, context);

        FrontendSymbol *accessor = FEST->lookup(FEST, identifierName);
        if (!accessor)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find accessor.");
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_SYMBOL, CRYO_SEVERITY_FATAL,
                      "Failed to find accessor.", __LINE__, __FILE__, __func__)
            return NULL;
        }

        logMessage(LMI, "INFO", "Parser", "Accessor found: %s", accessor->name);

        ASTNode *symbolNode = accessor->node;
        symbolNode->print(symbolNode);
        DataType *symbolDataType = DTM->astInterface->getTypeofASTNode(symbolNode);

        if (symbolNode->metaData->type == NODE_VAR_DECLARATION)
        {
        }

        if (lexer->currentToken.type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing dot notation with identifier...");
            return parseDotNotationWithType(symbolNode, symbolDataType, lexer, context, arena, state);
        }

        DEBUG_BREAKPOINT;
    }

    char *unexpectedTokenStr = (char *)malloc(256);
    char *tokenStr = CryoTokenToString(lexer->currentToken.type);
    sprintf(unexpectedTokenStr, "Unexpected token in dot notation. Received: %s", tokenStr);
    NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
              unexpectedTokenStr, __LINE__, __FILE__, __func__)
    return NULL;
}

ASTNode *parseDotNotationWithType(ASTNode *accessor, DataType *typeOfNode, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing dot notation with type...");
    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseDotNotationWithType", arena, state, context);
    // The first identifier is the type of the node we are accessing
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected identifier for property access.", __LINE__, __FILE__, __func__)
        return NULL;
    }

    const char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Prop name: %s", propName);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseDotNotationWithType", arena, state, context);

    // If the type of node is a struct, we need to check if the property exists in the struct.
    if (typeOfNode->container->objectType == STRUCT_OBJ && typeOfNode->container->typeOf != POINTER_TYPE)
    {
        logMessage(LMI, "INFO", "Parser", "Type of node is a struct.");

        DTStructTy *structType = typeOfNode->container->type.structType;
        const char *structName = typeOfNode->typeName;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(accessor, propName, typeOfNode, lexer, context, arena, state);
            return methodCallNode;
        }

        logMessage(LMI, "INFO", "Parser", "@parseDotNotationWithType Struct name: %s", structName);
        logMessage(LMI, "INFO", "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        ASTNode *property = DTM->propertyTypes->findStructPropertyNode(structType, propName);
        if (property)
        {
            logMessage(LMI, "INFO", "Parser", "Property found in struct, name: %s", propName);
            ASTNode *propertyAccessNode = createStructPropertyAccessNode(accessor, property, propName, typeOfNode, arena, state, lexer);
            propertyAccessNode->data.propertyAccess->propertyIndex = DTM->propertyTypes->getStructPropertyIndex(typeOfNode, propName);
            propertyAccessNode->data.propertyAccess->objectTypeName = structName;
            logMessage(LMI, "INFO", "Parser", "Property access node created.");
            logMessage(LMI, "INFO", "Parser", "Property access node: %s", propertyAccessNode->data.propertyAccess->propertyName);

            if (lexer->currentToken.type == TOKEN_SEMICOLON)
            {
                consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseDotNotationWithType", arena, state, context);
            }

            return propertyAccessNode;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Property Attempted: %s", propName);
            NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_PROPERTY, CRYO_SEVERITY_FATAL,
                      "Property not found in struct.", __LINE__, __FILE__, __func__)
            return NULL;
        }
    }
    // If the type of node is a pointer to a struct, we need to dereference it first.
    else if (typeOfNode->container->typeOf == POINTER_TYPE && typeOfNode->container->type.pointerType->baseType->container->objectType == STRUCT_OBJ)
    {
        logMessage(LMI, "INFO", "Parser", "Type of node is a pointer to a struct.");

        DataType *structDataType = typeOfNode->container->type.pointerType->baseType;
        structDataType->debug->printType(structDataType);
        DTStructTy *structType = structDataType->container->type.structType;
        const char *structName = typeOfNode->typeName;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(accessor, propName, typeOfNode, lexer, context, arena, state);
            return methodCallNode;
        }

        logMessage(LMI, "INFO", "Parser", "@parseDotNotationWithType Struct name: %s", structName);
        logMessage(LMI, "INFO", "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        ASTNode *property = DTM->propertyTypes->findStructPropertyNode(structType, propName);
        if (property)
        {
            logMessage(LMI, "INFO", "Parser", "Property found in struct, name: %s", propName);
            ASTNode *propertyAccessNode = createStructPropertyAccessNode(accessor, property, propName, typeOfNode, arena, state, lexer);
            propertyAccessNode->data.propertyAccess->propertyIndex = DTM->propertyTypes->getStructPropertyIndex(typeOfNode, propName);
            propertyAccessNode->data.propertyAccess->objectTypeName = structName;
            logMessage(LMI, "INFO", "Parser", "Property access node created.");
            logMessage(LMI, "INFO", "Parser", "Property access node: %s", propertyAccessNode->data.propertyAccess->propertyName);

            if (lexer->currentToken.type == TOKEN_SEMICOLON)
            {
                consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseDotNotationWithType", arena, state, context);
            }

            return propertyAccessNode;
        }
    }
    // Return the property access node for *CLASS TYPES*.
    else if (typeOfNode->container->objectType == CLASS_OBJ)
    {
        logMessage(LMI, "INFO", "Parser", "Type of node is a class.");

        DTClassTy *classType = typeOfNode->container->type.classType;
        const char *className = typeOfNode->typeName;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(accessor, propName, typeOfNode, lexer, context, arena, state);
            return methodCallNode;
        }

        logMessage(LMI, "INFO", "Parser", "Class name: %s", className);
        logMessage(LMI, "INFO", "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        NEW_ERROR(GDM, CRYO_ERROR_I_UNIMPLMENTED_FUNCTION, CRYO_SEVERITY_FATAL,
                  "Class property access is not implemented yet.", __LINE__, __FILE__, __func__)
        return NULL;
        // ASTNode *property = GetClassProperty((const char *)propName, className);
        // if (property)
        // {
        //     logMessage(LMI, "INFO", "Parser", "Property found in class, name: %s", propName);
        //     return createClassPropertyAccessNode(accessor, property, (const char *)propName, typeOfNode, arena, state, lexer);
        // }
        // else
        // {
        //     logMessage(LMI, "ERROR", "Parser", "Property Attempted: %s", propName);
        //     NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_PROPERTY, CRYO_SEVERITY_FATAL,
        //               "Property not found in class.", __LINE__, __FILE__, __func__)
        //     return NULL;
        // }
    }
    else if (typeOfNode->container->typeOf == POINTER_TYPE)
    {
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Type of node is not a struct or class, received: %s", CryoNodeTypeToString(accessor->metaData->type));
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_TYPE, CRYO_SEVERITY_FATAL,
                  "Expected a struct or class type.", __LINE__, __FILE__, __func__)
    }
}

ASTNode *parseForThisValueProperty(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing for this value property...");
    if (context->thisContext == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected 'this' keyword to be used inside a struct.");
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                  "Expected 'this' keyword to be used inside a struct.", __LINE__, __FILE__, __func__)
    }

    // This assumes the `this` and `.` tokens have already been consumed,
    ASTNode **accessProperties = context->thisContext->properties;
    int propertyCount = context->thisContext->propertyCount;

    // Check the next token identifier and match it with the properties of the struct
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected identifier after 'this' keyword. Received: %s", CryoTokenToString(lexer->currentToken.type));
        NEW_ERROR(GDM, CRYO_ERROR_SYNTAX, CRYO_SEVERITY_FATAL,
                  "Expected identifier after 'this' keyword.", __LINE__, __FILE__, __func__)
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

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseExpectedTypeArgWithThisKW", arena, state, context);

    if (!matchedProperty)
    {
        logMessage(LMI, "ERROR", "Parser", "Property not found in struct.");
        logMessage(LMI, "ERROR", "Parser", "Property name: %s", propertyName);
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_PROPERTY, CRYO_SEVERITY_FATAL,
                  "Property not found in struct.", __LINE__, __FILE__, __func__)
    }

    ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, lexer);
    if (!propAccessNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                  "Failed to create property access node.", __LINE__, __FILE__, __func__)
    }
    propAccessNode->data.propertyAccess->propertyIndex = DTM->propertyTypes->getStructPropertyIndex(expectedType, propertyName);
    propAccessNode->data.propertyAccess->objectTypeName = expectedType->typeName;
    propAccessNode->data.propertyAccess->objectType = expectedType;

    return propAccessNode;
}

ASTNode *parseThisExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing this expression...");
    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.", "parseThisExpression", arena, state, context);

    if (context->thisContext == NULL)
    {
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_THIS_CONTEXT, CRYO_SEVERITY_FATAL,
                  "Expected `this` keyword to be used inside a struct.", __LINE__, __FILE__, __func__)
    }

    if (lexer->currentToken.type == TOKEN_DOT)
    {
        consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseThisExpression", arena, state, context);
        ASTNode *thisNode = parseDotNotation(lexer, context, arena, state);
        return thisNode;
    }

    return createThisNode(arena, state, lexer);
}

ASTNode *parseNewExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing new expression...");
    consume(__LINE__, lexer, TOKEN_KW_NEW, "Expected `new` keyword.", "parseNewExpression", arena, state, context);

    const char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseNewExpression", arena, state, context);
    int argCount = 0;

    logMessage(LMI, "INFO", "Parser", "Type name: %s", typeName);

    DataType *type = DTM->symbolTable->lookup(DTM->symbolTable, typeName);
    if (!type)
    {
        logMessage(LMI, "ERROR", "Parser", "Type not found.");
        NEW_ERROR(GDM, CRYO_ERROR_UNDEFINED_TYPE, CRYO_SEVERITY_FATAL,
                  "Type not found.", __LINE__, __FILE__, __func__)
    }

    logMessage(LMI, "INFO", "Parser", "Type found.");
    type->debug->printType(type);

    ASTNode *args = parseArgumentList(lexer, context, arena, state);
    if (!args)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse argument list.");
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                  "Failed to parse argument list.", __LINE__, __FILE__, __func__)
    }

    logASTNode(args);

    // We need to check if the type is a struct or a class
    if (type->container->objectType == STRUCT_OBJ)
    {
        logMessage(LMI, "INFO", "Parser", "Type is a struct.");
        DTStructTy *structType = type->container->type.structType;

        int ctorArgCount = structType->ctorParamCount;
        logMessage(LMI, "INFO", "Parser", "Constructor argument count: %d", ctorArgCount);

        argCount = args->data.argList->argCount;
        logMessage(LMI, "INFO", "Parser", "Argument count: %d", argCount);

        // Check if the argument count matches the constructor argument count
        if (ctorArgCount != argCount)
        {
            logMessage(LMI, "ERROR", "Parser", "Argument count mismatch.");
            NEW_ERROR(GDM, CRYO_ERROR_INVALID_ARGUMENT_COUNT, CRYO_SEVERITY_FATAL,
                      "Argument count mismatch.", __LINE__, __FILE__, __func__)
        }
    }
    // TODO: Implement
    else if (type->container->objectType == CLASS_OBJ)
    {
        logMessage(LMI, "INFO", "Parser", "Type is a class.");
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "Type is not a struct or class.");
        NEW_ERROR(GDM, CRYO_ERROR_INVALID_TYPE, CRYO_SEVERITY_FATAL,
                  "Type is not a struct or class.", __LINE__, __FILE__, __func__)
    }

    ASTNode **arguments = args->data.argList->args;

    ASTNode *objectNode = createObject(typeName, type, true, arguments, argCount, arena, state, lexer);
    if (!objectNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create object node.");
        NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                  "Failed to create object node.", __LINE__, __FILE__, __func__)
    }

    logASTNode(objectNode);

    return objectNode;
}

ASTNode *parseNullExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing null expression...");
    consume(__LINE__, lexer, TOKEN_KW_NULL, "Expected `null` keyword.", "parseNullExpression", arena, state, context);

    return createNullNode(arena, state, lexer);
}

// The `typeof` keyword is used to get the type of an identifier.
// e.g. `typeof(x)` would return the type of the variable `x` as a string.
ASTNode *parseTypeofIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing typeof identifier...");
    consume(__LINE__, lexer, TOKEN_KW_TYPEOF, "Expected `typeof` keyword.", "parseTypeofIdentifier", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start typeof expression.", "parseTypeofIdentifier", arena, state, context);

    ASTNode *identifier = parseExpression(lexer, context, arena, state);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end typeof expression.", "parseTypeofIdentifier", arena, state, context);

    return createTypeofNode(identifier, arena, state, lexer);
}

ASTNode *parseParenthesizedExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing parentheses expression...");
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start expression.", "parseParenthesesExpression", arena, state, context);

    ASTNode *expression = parseExpression(lexer, context, arena, state);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end expression.", "parseParenthesesExpression", arena, state, context);

    return expression;
}

ASTNode *parsePropertyAccess(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing property access...");
    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parsePropertyAccess", arena, state, context);

    ASTNode *propertyAccess = parseDotNotation(lexer, context, arena, state);
    return propertyAccess;
}

ASTNode *parseBreakStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing break statement...");
    consume(__LINE__, lexer, TOKEN_KW_BREAK, "Expected `break` keyword.", "parseBreakStatement", arena, state, context);

    if (lexer->currentToken.type == TOKEN_SEMICOLON)
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseBreakStatement", arena, state, context);

    return createBreakNode(arena, state, lexer);
}

ASTNode *parseContinueStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing continue statement...");
    consume(__LINE__, lexer, TOKEN_KW_CONTINUE, "Expected `continue` keyword.", "parseContinueStatement", arena, state, context);

    if (lexer->currentToken.type == TOKEN_SEMICOLON)
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseContinueStatement", arena, state, context);

    return createContinueNode(arena, state, lexer);
}

DataType *parseArrayInstantiation(DataType *baseType, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing array instantiation...");
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array instantiation.", "parseArrayInstantiation", arena, state, context);

    // Track dimensions for multidimensional arrays
    int *dimensionSizes = (int *)malloc(sizeof(int) * 10); // Start with capacity for 10 dimensions
    for (int i = 0; i < 10; i++)
    {
        dimensionSizes[i] = 0;
    }
    int dimensionCount = 0;
    int dimensionCapacity = 10;

    // Parse the first dimension
    bool isFirstDimensionStatic = false;
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        char *sizeStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        dimensionSizes[dimensionCount] = atoi(sizeStr);
        free(sizeStr);
        consume(__LINE__, lexer, TOKEN_INT_LITERAL, "Expected an integer literal.", "parseArrayInstantiation", arena, state, context);
        isFirstDimensionStatic = true;
    }
    dimensionCount++;

    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array instantiation.", "parseArrayInstantiation", arena, state, context);

    // Parse additional dimensions if present
    while (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Found additional array dimension...");
        consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array dimension.", "parseArrayInstantiation", arena, state, context);

        // Make sure we have enough capacity
        if (dimensionCount >= dimensionCapacity)
        {
            dimensionCapacity *= 2;
            dimensionSizes = (int *)realloc(dimensionSizes, sizeof(int) * dimensionCapacity);
        }

        // Parse this dimension's size
        if (lexer->currentToken.type == TOKEN_INT_LITERAL)
        {
            char *sizeStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
            dimensionSizes[dimensionCount] = atoi(sizeStr);
            free(sizeStr);
            consume(__LINE__, lexer, TOKEN_INT_LITERAL, "Expected an integer literal.", "parseArrayInstantiation", arena, state, context);
        }
        dimensionCount++;

        consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array dimension.", "parseArrayInstantiation", arena, state, context);
    }

    // If we have a multidimensional array, use the dedicated function
    if (dimensionCount > 1)
    {
        logMessage(LMI, "INFO", "Parser", "Creating multidimensional array with %d dimensions", dimensionCount);
        // Using the existing createMultiDimensionalStaticArray function
        DataType *arrayType = DTM->arrayTypes->createMultiDimensionalStaticArray(baseType, dimensionSizes, dimensionCount);

        if (arrayType == NULL)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create multidimensional array type.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                      "Failed to create multidimensional array type.", __LINE__, __FILE__, __func__)
        }

        return arrayType;
    }
    else
    {
        // Single dimension array - use existing functions
        DataType *arrayType = NULL;

        if (isFirstDimensionStatic)
        {
            // Create a static array with the specified size
            arrayType = DTM->arrayTypes->createStaticArray(baseType, dimensionSizes[0]);
            logMessage(LMI, "INFO", "Parser", "Created static array with size %d", dimensionSizes[0]);
        }
        else
        {
            // Create a dynamic array
            arrayType = DTM->arrayTypes->createDynamicArray(baseType);
            logMessage(LMI, "INFO", "Parser", "Created dynamic array");
        }

        if (arrayType == NULL)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create array type.");
            NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
                      "Failed to create array type.", __LINE__, __FILE__, __func__)
        }

        arrayType->debug->printVerbosType(arrayType);
        return arrayType;
    }

    // If we reach here, something went wrong
    logMessage(LMI, "ERROR", "Parser", "Failed to parse array instantiation.");
    NEW_ERROR(GDM, CRYO_ERROR_NULL_AST_NODE, CRYO_SEVERITY_FATAL,
              "Failed to parse array instantiation.", __LINE__, __FILE__, __func__)
    return NULL;
}
