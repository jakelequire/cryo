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
#include "tools/cxx/IDGen.hpp"
#include "symbolTable/cInterfaceTable.h"
#include "diagnostics/diagnostics.h"
#include "frontend/parser.h"

/* ====================================================================== */
/* @Entry_Point | Lexer passes tokens to the entry point to be processed. */

// <parseProgram>
ASTNode *parseProgram(Lexer *lexer, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing program...");

    bool isPrimaryTable = isPrimaryTable(globalTable);
    bool isDependencyTable = isDependencyTable(globalTable);
    if (isPrimaryTable)
    {
        logMessage(LMI, "INFO", "Parser", "%s%sPrimary Table Parsing%s", BOLD, GREEN, COLOR_RESET);
    }
    if (isDependencyTable)
    {
        logMessage(LMI, "INFO", "Parser", "%s%sDependency Table Parsing%s", BOLD, YELLOW, COLOR_RESET);
    }

    bool isModuleFile = state->isModuleFile;
    if (isModuleFile)
    {
        logMessage(LMI, "INFO", "Parser", "%s%sModule File Parsing%s", BOLD, CYAN, COLOR_RESET);
    }

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

    getNextToken(lexer, arena, state);
    logMessage(LMI, "INFO", "Parser", "Next token after program: %s", CryoTokenToString(lexer->currentToken.type));

    while (lexer->currentToken.type != TOKEN_EOF)
    {
        ASTNode *statement = parseStatement(lexer, context, arena, state, globalTable);
        if (statement)
        {
            addStatementToProgram(program, statement, arena, state, globalTable);
            logMessage(LMI, "INFO", "Parser", "Statement parsed successfully");

            if (statement->metaData->type == NODE_NAMESPACE)
            {
                const char *namespaceName = statement->data.cryoNamespace->name;
                // Initialize the `this`context to the namespace
                setDefaultThisContext(namespaceName, context);
                // Initialize the global symbol table
                if (isPrimaryTable)
                {
                    logMessage(LMI, "INFO", "Parser", "Creating primary symbol table for namespace: %s", namespaceName);
                    createPrimarySymbolTable(globalTable, namespaceName);
                }
                if (isDependencyTable)
                {
                    logMessage(LMI, "INFO", "Parser", "Creating dependency symbol table for namespace: %s", namespaceName);
                    initDependencySymbolTable(globalTable, namespaceName);
                }
            }
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse statement");
            parsingError("Failed to parse statement.", "parseProgram", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
        logMessage(LMI, "INFO", "Parser", "Next token after statement: %s", CryoTokenToString(lexer->currentToken.type));
    }

    buildASTTreeLinks(program);

    CompleteFrontend(globalTable);
    return program;
}
// </parseProgram>

/* ====================================================================== */
/* @Helper_Functions | Debugging, Errors, Walkers */

// <consume>
void consume(int line, Lexer *lexer, CryoTokenType type, const char *message, const char *functionName, Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Consuming token...");
    // pushCallStack(&callStack, functionName, lexer->currentToken.line);

    addTokenToContext(context, lexer->currentToken);

    if (lexer->currentToken.type == type)
    {
        getNextToken(lexer, arena, state);
    }
    else
    {
        parsingError((char *)message, (char *)functionName, arena, state, lexer, lexer->source, NULL);
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
        parsingError("Expected a namespace name", "getNamespaceName", arena, state, lexer, lexer->source, NULL);
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
DataType *getCryoDataType(const char *typeStr, Arena *arena, CompilerState *state, Lexer *lexer, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Getting data type for: %s", typeStr);
    DataType *type = DTM->parseType(typeStr);
    if (!type)
    {
        parsingError("Unknown data type", "getCryoDataType", arena, state, lexer, lexer->source, globalTable);
    }

    logMessage(LMI, "INFO", "Parser", "Data Type Found");
    return type;
}
// </getCryoDataType>

DataType *parseGenericDataTypeInstantiation(DataType *type, Lexer *lexer, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` in generic type instantiation", "parseGenericDataTypeInstantiation", arena, state, NULL);

    // Find the generic type definition
    DataType *genericType = ResolveDataType(globalTable, type->typeName);
    if (!genericType)
    {
        parsingError("Type is not generic", "parseGenericDataTypeInstantiation", arena, state, lexer, lexer->source, globalTable);
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
            parsingError("Too many type arguments", "parseGenericDataTypeInstantiation", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        DataType *concreteType = parseType(lexer, NULL, arena, state, globalTable);
        logMessage(LMI, "INFO", "Parser", "Concrete type: %s", concreteType->typeName);
        concreteTypes[paramCount++] = concreteType;

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in type argument list", "parseGenericDataTypeInstantiation", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }

    if (paramCount != expectedParamCount)
    {
        parsingError("Too few type arguments", "parseGenericDataTypeInstantiation", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    logMessage(LMI, "INFO", "Parser", "Generic type instantiation parsed successfully");
    // Create the generic type instantiation
    DataType *instantiatedType = DTM->generics->createGenericTypeInstance(genericType, concreteTypes, paramCount);
    logMessage(LMI, "INFO", "Parser", "Instantiated type: %s", instantiatedType->typeName);
    return instantiatedType;
}

// <parseType>
DataType *parseType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing type...");
    DataType *type = NULL;
    const char *typeTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);

    if (context->inGenericContext)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing generic type: %s", typeTokenStr);
        return getCryoDataType(typeTokenStr, arena, state, lexer, globalTable);
    }

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_VOID:
    case TOKEN_KW_INT:
    case TOKEN_TYPE_I8:
    case TOKEN_TYPE_I16:
    case TOKEN_TYPE_I32:
    case TOKEN_TYPE_I64:
    case TOKEN_TYPE_I128:
    case TOKEN_KW_STRING:
    case TOKEN_KW_BOOL:
    case TOKEN_KW_ANY:
        logMessage(LMI, "INFO", "Parser", "Parsing primitive type: %s", typeTokenStr);
        return getCryoDataType(typeTokenStr, arena, state, lexer, globalTable);
        break;

    case TOKEN_IDENTIFIER:
        logMessage(LMI, "INFO", "Parser", "Parsing custom type: %s", typeTokenStr);
        // type = getCryoDataType(typeTokenStr, arena, state, lexer, globalTable);
        return DTM->resolveType(DTM, typeTokenStr);
        break;

    default:
        parsingError("Expected a type identifier", "getNextToken", arena, state, lexer, lexer->source, globalTable);
        break;
    }

    logMessage(LMI, "INFO", "Parser", "Type parsed successfully");
    return type;
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
void addStatementToProgram(ASTNode *programNode, ASTNode *statement, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding statement to program...");
    if (!programNode || programNode->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[AST_ERROR] Invalid program node\n");
        return;
    }

    // Add the statement to the Gobal Symbol Table
    if (globalTable)
    {
        addNodeToSymbolTable(globalTable, statement);
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
ASTNode *parseStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing statement...");

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state, globalTable);

    case TOKEN_KW_PUBLIC:
        return parsePublicDeclaration(lexer, context, arena, state, globalTable);

    case TOKEN_KW_PRIVATE:
        return parsePrivateDeclaration(lexer, context, arena, state, globalTable);

    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PUBLIC, arena, state, globalTable);

    case TOKEN_KW_RETURN:
        return parseReturnStatement(lexer, context, arena, state, globalTable);

    case TOKEN_KW_FOR:
        return parseForLoop(lexer, context, arena, state, globalTable);

    case TOKEN_KW_IMPORT:
        return handleImportParsing(lexer, context, arena, state, globalTable);

    case TOKEN_KW_EXTERN:
        return parseExtern(lexer, context, arena, state, globalTable);

    case TOKEN_KW_STRUCT:
        return parseStructDeclaration(lexer, context, arena, state, globalTable);

    case TOKEN_KW_DEBUGGER:
        parseDebugger(lexer, context, arena, state);
        return NULL;

    case TOKEN_KW_USING:
        return parseUsingKeyword(lexer, context, arena, state, globalTable);

    case TOKEN_KW_THIS:
        return parseThisContext(lexer, context, arena, state, globalTable);

    case TOKEN_KW_STATIC:
        return parseStaticKeyword(lexer, context, arena, state, globalTable);

    case TOKEN_KW_CLASS:
        return parseClassDeclaration(false, lexer, context, arena, state, globalTable);

    case TOKEN_KW_TYPE:
        return parseTypeDeclaration(lexer, context, arena, state, globalTable);

    case TOKEN_IDENTIFIER:
        if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing function call...");
            char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
            return parseFunctionCall(lexer, context, functionName, arena, state, globalTable);
        }
        if (lexer->currentToken.type == TOKEN_IDENTIFIER && peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DOUBLE_COLON)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing scope call...");
            return parseScopeCall(lexer, context, arena, state, globalTable);
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "Parsing variable assignment...");
            return parsePrimaryExpression(lexer, context, arena, state, globalTable);
        }

    case TOKEN_KW_NAMESPACE:
        return parseNamespace(lexer, context, arena, state, globalTable);

    case TOKEN_KW_IF:
        return parseIfStatement(lexer, context, arena, state, globalTable);

    case TOKEN_KW_WHILE:
        return parseWhileStatement(lexer, context, arena, state, globalTable);

    case TOKEN_AT:
        return parseAnnotation(lexer, context, arena, state, globalTable);

    case TOKEN_EOF:
        return NULL;

    default:
        parsingError("Expected a statement", "parseStatement", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
}
// </parseStatement>

ASTNode *parseAnnotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing annotation...");
    switch (lexer->currentToken.type)
    {
    case TOKEN_AT:
    {
        consume(__LINE__, lexer, TOKEN_AT, "Expected '@' symbol", "parseAnnotation", arena, state, context);
        char *annotationName = strndup(lexer->currentToken.start, lexer->currentToken.length);

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
        parsingError("Expected '@' symbol", "parseAnnotation", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    }

    return NULL;
}

ASTNode *parseStaticKeyword(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected 'static' keyword", "parseStaticKeyword", arena, state, context);

    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state).type;
    switch (nextToken)
    {
    case TOKEN_KW_CLASS:
    {
        return parseClassDeclaration(true, lexer, context, arena, state, globalTable);
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
        parsingError("Expected 'class', 'struct', or 'function' keyword", "parseStaticKeyword", arena, state, lexer, lexer->source, globalTable);
    }
    }
    DEBUG_BREAKPOINT;
}

// <parseScopeCall>
ASTNode *parseScopeCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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

    TypeOfSymbol symType = GetScopeSymbolTypeFromName(globalTable, scopeName);
    const char *symTypeStr = TypeOfSymbolToString(globalTable, symType);
    logMessage(LMI, "INFO", "Parser", "Symbol Type: %s", symTypeStr);

    TypeofDataType typeOfDataType = GetTypeOfDataTypeFromName(globalTable, scopeName);
    const char *typeOfDataTypeStr = DTM->debug->typeofDataTypeToString(typeOfDataType);
    logMessage(LMI, "INFO", "Parser", "Type of Data Type: %s", typeOfDataTypeStr);

    Symbol *sym = NULL;
    switch (symType)
    {
    case TYPE_SYMBOL:
    {
        sym = FindMethodSymbol(globalTable, functionName, scopeName, typeOfDataType);
        break;
    }
    case FUNCTION_SYMBOL:
    {
        // sym = FindFunctionSymbol(globalTable, functionName, scopeName);
        sym = NULL;
        break;
    }
    default:
    {
        parsingError("Symbol not found", "parseScopeCall", arena, state, lexer, lexer->source, globalTable);
    }
    }

    if (sym == NULL)
    {
        parsingError("Symbol not found", "parseScopeCall", arena, state, lexer, lexer->source, globalTable);
    }

    ASTNode *symbolNode = GetASTNodeFromSymbol(globalTable, sym);

    CryoNodeType nodeType = symbolNode->metaData->type;
    logMessage(LMI, "INFO", "Parser", "Node Type: %s", CryoNodeTypeToString(nodeType));
    ASTNode *node = NULL;
    switch (nodeType)
    {
    case NODE_FUNCTION_DECLARATION:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing function call...");
        node = parseScopedFunctionCall(lexer, context, arena, state, strdup(functionName), scopeName, globalTable);
        break;
    }
    case NODE_METHOD:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing method call...");
        node = parseMethodScopeResolution(scopeName, lexer, context, arena, state, globalTable);
        break;
    }
    default:
        CONDITION_FAILED;
        break;
    }

    logMessage(LMI, "INFO", "Parser", "Scope call parsed.");
    return node;
}
// </parseScopeCall>

ASTNode *parseScopedFunctionCall(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, const char *functionName, const char *scopeName, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing scoped function call...");

    ASTNode *node = createScopedFunctionCall(arena, state, strdup(functionName), lexer);

    const char *scopeNameID = Generate64BitHashID(scopeName);
    FunctionSymbol *funcSymbol = GetFrontendScopedFunctionSymbol(globalTable, functionName, scopeNameID);
    if (!funcSymbol)
    {
        parsingError("Function symbol not found", "parseScopedFunctionCall", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    // Get the arguments
    int argCount = 0;
    DataType **argTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    ASTNode **args = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    ASTNode *argList = parseArgumentList(lexer, context, arena, state, globalTable);
    if (!argList)
    {
        parsingError("Failed to parse argument list", "parseScopedFunctionCall", arena, state, lexer, lexer->source, globalTable);
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

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected a right parenthesis", "parseScopedFunctionCall", arena, state, context);
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseScopedFunctionCall", arena, state, context);

    node->data.scopedFunctionCall->args = args;
    node->data.scopedFunctionCall->argCount = argCount;
    node->data.scopedFunctionCall->scopeName = strdup(scopeName);

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

ASTNode *parseNamespace(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing namespace...");
    // Equivalent to `package <name>` like in Go.
    consume(__LINE__, lexer, TOKEN_KW_NAMESPACE, "Expected 'namespace' keyword.", "parseNamespace", arena, state, context);

    ASTNode *node = NULL;

    char *namespaceName = NULL;
    if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        namespaceName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        node = createNamespaceNode(namespaceName, arena, state, lexer);
        getNextToken(lexer, arena, state);
    }
    else
    {
        parsingError("Expected a namespace name", "parseNamespace", arena, state, lexer, lexer->source, globalTable);
    }

    InitNamespace(globalTable, namespaceName);

    createNamespaceScope(context, namespaceName);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseNamespace", arena, state, context);

    return node;
}

// <parsePrimaryExpression>
ASTNode *parsePrimaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        printf("Int Value: %d\n", intVal);
        node = createIntLiteralNode(intVal, arena, state, lexer);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_STRING_LITERAL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing string literal");
        const char *str = (const char *)strndup(lexer->currentToken.start, lexer->currentToken.length);
        node = createStringLiteralNode(str, arena, state, lexer);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_BOOLEAN_LITERAL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing boolean literal");
        char *booleanValueStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
        if (strcmp(booleanValueStr, "true") != 0 && strcmp(booleanValueStr, "false") != 0)
        {
            parsingError("Invalid boolean value", "parsePrimaryExpression", arena, state, lexer, lexer->source, globalTable);
        }
        int booleanValue = strcmp(booleanValueStr, "true") == 0 ? 1 : 0;
        node = createBooleanLiteralNode(booleanValue, arena, state, lexer);
        getNextToken(lexer, arena, state);
        return node;
    }
    case TOKEN_LBRACKET:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array literal");
        return parseArrayLiteral(lexer, context, arena, state, globalTable);
    }
    case TOKEN_IDENTIFIER:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier expression");
        return parseIdentifierExpression(lexer, context, arena, state, globalTable);
    }
    case TOKEN_KW_NEW:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing new expression");
        return parseNewExpression(lexer, context, arena, state, globalTable);
    }
    case TOKEN_KW_NULL:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing null expression");
        return parseNullExpression(lexer, context, arena, state, globalTable);
    }
    case TOKEN_KW_TYPEOF:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing typeof expression");
        return parseTypeofIdentifier(lexer, context, arena, state, globalTable);
    }
    case TOKEN_INCREMENT:
    case TOKEN_DECREMENT:
    case TOKEN_MINUS:
    case TOKEN_BANG:
    case TOKEN_STAR:
    case TOKEN_AMPERSAND:
    {
        logMessage(LMI, "INFO", "Parser", "Parsing unary expression");
        return parseUnaryExpression(lexer, context, arena, state, globalTable);
    }
    default:
    {
        NEW_COMPILER_ERROR(state, "ERROR", "Expected an expression or statement.", "parsePrimaryExpression");
        parsingError("Expected an expression", "parsePrimaryExpression", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    }

    // Check for dot notation after the primary expression
    while (lexer->currentToken.type == TOKEN_DOT)
    {
        node = parseDotNotation(lexer, context, arena, state, globalTable);
    }

    // Check for array indexing after an identifier or other primary expression
    while (lexer->currentToken.type == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array indexing");
        node = parseArrayIndexing(lexer, context, NULL, arena, state, globalTable);
    }
    return node;
}
// </parsePrimaryExpression>

ASTNode *parseIdentifierExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    CryoTokenType prevToken = context->lastTokens[0].type;
    char *curToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state).type;

    if (nextToken == TOKEN_KW_NEW)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing new expression");
        return parseNewExpression(lexer, context, arena, state, globalTable);
    }

    // Check for dot notation after the primary expression
    if (nextToken == TOKEN_DOT)
    {
        // This can either be a struct field or a function call.
        return parseDotNotation(lexer, context, arena, state, globalTable);
    }
    // Peek to see if the next token is `[` for array indexing
    if (nextToken == TOKEN_LBRACKET)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing array indexing");
        return parseArrayIndexing(lexer, context, NULL, arena, state, globalTable);
    }
    // Peek to see if the next token is `;` for a statement
    if (nextToken == TOKEN_SEMICOLON)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context, globalTable);
        getNextToken(lexer, arena, state);
        return node;
    }
    // Peek to see if the next token is `)` for a statement
    if (nextToken == TOKEN_RPAREN)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier as a statement");
        ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context, globalTable);
        getNextToken(lexer, arena, state);
        return node;
    }

    // Peek to see if the next token is `=` for assignment
    else if (nextToken == TOKEN_EQUAL)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing assignment");
        return parseAssignment(lexer, context, NULL, arena, state, globalTable);
    }
    // Peek to see if the next token is `(` to start a function call.
    else if (nextToken == TOKEN_LPAREN)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing function call");
        char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        return parseFunctionCall(lexer, context, functionName, arena, state, globalTable);
    }
    // Peek to see if the next token is `::` for a scope call
    else if (nextToken == TOKEN_DOUBLE_COLON)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing scope call");
        return parseScopeCall(lexer, context, arena, state, globalTable);
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Parsing identifier, next token: %s", CryoTokenToString(peekNextUnconsumedToken(lexer, arena, state).type));
    }
    logMessage(LMI, "INFO", "Parser", "Parsing identifier expression");
    // Check to see if it exists in the symbol table as a variable or parameter
    ASTNode *node = createIdentifierNode(strndup(lexer->currentToken.start, lexer->currentToken.length), arena, state, lexer, context, globalTable);
    getNextToken(lexer, arena, state);
    return node;
}

// <parseExpression>
ASTNode *parseExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing expression...");

    return parseBinaryExpression(lexer, context, 1, arena, state, globalTable);
}
// </parseExpression>

// <parseExpressionStatement>
ASTNode *parseExpressionStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing expression statement...");

    ASTNode *expression = parseExpression(lexer, context, arena, state, globalTable);

    logMessage(LMI, "Parser", "Expression parsed: %s", CryoNodeTypeToString(expression->metaData->type));

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon", "parseExpressionStatement", arena, state, context);

    return createExpressionStatement(expression, arena, state, lexer);
}
// </parseExpressionStatement>

// <parseBinaryExpression>
ASTNode *parseBinaryExpression(Lexer *lexer, ParsingContext *context, int minPrecedence, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing binary expression...");
    ASTNode *left = parsePrimaryExpression(lexer, context, arena, state, globalTable);
    if (!left)
    {
        parsingError("Expected an expression.", "parseBinaryExpression", arena, state, lexer, lexer->source, globalTable);
        return NULL;
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

        // Parse the right side with a higher precedence
        ASTNode *right = parseBinaryExpression(lexer, context, precedence + 1, arena, state, globalTable);
        if (!right)
        {
            parsingError("Expected an expression on the right side of the operator.", "parseBinaryExpression", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        // Create a new binary expression node
        CryoOperatorType op = CryoTokenToOperator(opToken);
        if (op == OPERATOR_NA)
        {
            parsingError("Invalid operator.", "parseBinaryExpression", arena, state, lexer, lexer->source, globalTable);
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
ASTNode *parseUnaryExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing unary expression...");
    if (lexer->currentToken.type == TOKEN_AMPERSAND)
    {
        consume(__LINE__, lexer, TOKEN_AMPERSAND, "Expected identifier after &", "parseUnaryExpression", arena, state, context);

        // Parse the operand - must be an lvalue
        ASTNode *operand = parsePrimaryExpression(lexer, context, arena, state, globalTable);

        // Create address-of node
        ASTNode *node = createUnaryExpr(TOKEN_AMPERSAND, operand, arena, state, lexer);

        // Set the result type to be a pointer to operand's type
        DataType *operandType = DTM->astInterface->getTypeofASTNode(operand);
        operandType->setPointer(operandType, true);

        node->data.unary_op->resultType = operandType;
        node->data.unary_op->op = TOKEN_ADDRESS_OF;
        node->data.unary_op->expression = operand;

        return node;
    }
    if (lexer->currentToken.type == TOKEN_STAR)
    {
        consume(__LINE__, lexer, TOKEN_STAR, "Expected identifier after *", "parseUnaryExpression", arena, state, context);

        // Parse the operand - must be a pointer type
        ASTNode *operand = parsePrimaryExpression(lexer, context, arena, state, globalTable);

        // Create dereference node
        ASTNode *node = createUnaryExpr(TOKEN_STAR, operand, arena, state, lexer);

        // Set the result type to be the type pointed to by operand's type
        DataType *operandType = DTM->astInterface->getTypeofASTNode(operand);
        // TODO: implement the code below. Need to fully implement the pointer type system
        // if (!operandType->isPointer)
        // {
        //     parsingError("Dereference operator must be applied to a pointer type", "parseUnaryExpression", arena, state, lexer, lexer->source, globalTable);
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
        opToken = lexer->currentToken.type;
        getNextToken(lexer, arena, state);
        right = parseUnaryExpression(lexer, context, arena, state, globalTable);
        return createUnaryExpr(opToken, right, arena, state, lexer);
    }
    if (lexer->currentToken.type == TOKEN_INCREMENT || lexer->currentToken.type == TOKEN_DECREMENT)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing increment or decrement expression...");
        char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
        opToken = lexer->currentToken.type;

        getNextToken(lexer, arena, state);
        right = parsePrimaryExpression(lexer, context, arena, state, globalTable);
        if (!right)
        {
            parsingError("Expected an operand", "parseUnaryExpression", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }

    return createUnaryExpr(opToken, right, arena, state, lexer);
}
// </parseUnaryExpression>

// <parsePublicDeclaration>
ASTNode *parsePublicDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing public declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PUBLIC, "Expected 'public' keyword.", "parsePublicDeclaration", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state, globalTable);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PUBLIC, arena, state, globalTable);
    case TOKEN_KW_MODULE:
        return parseModuleDeclaration(VISIBILITY_PUBLIC, lexer, context, arena, state, globalTable);
    default:
        parsingError("Expected a declaration.", "parsePublicDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
}
// </parsePublicDeclaration>

// <parsePrivateDeclaration>
ASTNode *parsePrivateDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing private declaration...");
    consume(__LINE__, lexer, TOKEN_KW_PRIVATE, "Expected 'private' keyword.", "parsePrivateDeclaration", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_CONST:
    case TOKEN_KW_MUT:
        return parseVarDeclaration(lexer, context, arena, state, globalTable);
    case TOKEN_KW_FN:
        return parseFunctionDeclaration(lexer, context, VISIBILITY_PRIVATE, arena, state, globalTable);
    case TOKEN_KW_MODULE:
        return parseModuleDeclaration(VISIBILITY_PRIVATE, lexer, context, arena, state, globalTable);
    default:
        parsingError("Expected a declaration.", "parsePrivateDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
}
// </parsePrivateDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Blocks                                              */

// <parseBlock>
ASTNode *parseBlock(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing block...");
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start block.", "parseBlock", arena, state, context);

    context->scopeLevel++;

    ASTNode *block = createBlockNode(arena, state, lexer);
    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *statement = parseStatement(lexer, context, arena, state, globalTable);
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
ASTNode *parseFunctionBlock(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        ASTNode *statement = parseStatement(lexer, context, arena, state, globalTable);
        if (statement)
        {
            logMessage(LMI, "INFO", "Parser", "Adding statement to function block...");
            // DEBUG_ARENA_PRINT(arena);
            // ( statement, arena);
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
ASTNode *parseVarDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        parsingError("[Parser] Expected variable name.", "parseVarDeclaration", arena, state, lexer, lexer->source, globalTable);
    }
    char *var_name = strndup(lexer->currentToken.start, lexer->currentToken.length);
    getNextToken(lexer, arena, state);

    // Parse the variable type
    DataType *dataType = NULL;
    if (lexer->currentToken.type == TOKEN_COLON)
    {
        getNextToken(lexer, arena, state);
        char *varType = strndup(lexer->currentToken.start, lexer->currentToken.length);

        dataType = getCryoDataType(varType, arena, state, lexer, globalTable);
        if (!dataType)
        {
            parsingError("[Parser] Unknown data type.", "parseVarDeclaration", arena, state, lexer, lexer->source, globalTable);
        }
        getNextToken(lexer, arena, state);
    }
    else
    {
        parsingError("[Parser] Expected ':' after variable name.", "parseVarDeclaration", arena, state, lexer, lexer->source, globalTable);
    }

    // Parse the variable initializer
    if (lexer->currentToken.type != TOKEN_EQUAL)
    {
        parsingError("[Parser] Expected '=' after type.", "parseVarDeclaration", arena, state, lexer, lexer->source, globalTable);
    }
    getNextToken(lexer, arena, state);

    // Check if the variable is a reference (Note: This is not yet implemented in the lexer)
    if (lexer->currentToken.type == TOKEN_AMPERSAND)
    {
        isReference = true;
        getNextToken(lexer, arena, state);
    }

    // Parse the initializer expression
    ASTNode *initializer = parseExpression(lexer, context, arena, state, globalTable);
    if (initializer == NULL)
    {
        parsingError("[Parser] Expected expression after '='.", "parseVarDeclaration", arena, state, lexer, lexer->source, globalTable);
    }

    if (initializer->metaData->type == NODE_FUNCTION_CALL)
    {
        logMessage(LMI, "INFO", "Parser", "Function call detected.");
    }
    else
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after variable declaration.", "parseVarDeclaration", arena, state, context);
    }

    logMessage(LMI, "INFO", "Parser", "Variable declaration parsed.");

    ASTNode *varDeclNode = createVarDeclarationNode(var_name, dataType, initializer, isMutable, isConstant, isReference, false, arena, state, lexer);
    if (initializer->metaData->type == NODE_INDEX_EXPR)
    {
        logMessage(LMI, "INFO", "Parser", "Index expression detected.");
        varDeclNode->data.varDecl->indexExpr = initializer->data.indexExpr;
        varDeclNode->data.varDecl->hasIndexExpr = true;
    }

    (varDeclNode, arena);

    AddVariableToSymbolTable(globalTable, varDeclNode, currentScopeID);

    return varDeclNode;
}
// </parseVarDeclaration>

/* ====================================================================== */
/* @ASTNode_Parsing - Functions                                           */

// <parseFunctionDeclaration>
ASTNode *parseFunctionDeclaration(Lexer *lexer, ParsingContext *context, CryoVisibilityType visibility, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword.", "parseFunctionDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionName);

    const char *functionScopeID = Generate64BitHashID(functionName);
    const char *namespaceScopeID = getNamespaceScopeID(context);
    setCurrentFunction(context, functionName, namespaceScopeID); // Context Manager

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
    }

    ASTNode **params = parseParameterList(lexer, context, arena, strdup(functionName), state, globalTable);

    for (int i = 0; params[i] != NULL; i++)
    {
        logMessage(LMI, "INFO", "Parser", "Adding parameter: %s", params[i]->data.varDecl->name);
    }
    int paramCount = 0;
    DataType **paramTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    for (int i = 0; params[i] != NULL; i++)
    {
        DataType *paramType = params[i]->data.varDecl->type;
        logMessage(LMI, "INFO", "Parser", "Parameter type: %s", paramType->debug->toString(paramType));
        paramTypes[i] = params[i]->data.param->type;
        paramCount++;
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage(LMI, "INFO", "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, arena, state, globalTable);
        getNextToken(lexer, arena, state);
    }
    else
    {
        parsingError("Expected `->` for return type.", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
    }

    logMessage(LMI, "INFO", "Parser", "Function Return Type: %s", returnType->debug->toString(returnType));

    // Initialize the function symbol
    if (isGeneric)
    {
        InitGenericFunctionDeclaration(globalTable, functionName, namespaceScopeID, params, paramCount, returnType, genericParams, genericParamCount); // Global Symbol Table
    }
    else
    {
        InitFunctionDeclaration(globalTable, functionName, namespaceScopeID, params, paramCount, returnType); // Global Symbol Table
    }

    // Ensure the next token is `{` for the function block
    if (lexer->currentToken.type != TOKEN_LBRACE)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected `{` to start function block.");
        parsingError("Expected `{` to start function block.", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    // Definition of the function
    ASTNode *functionDefNode = createFunctionNode(visibility, strdup(functionName), params, NULL, returnType, arena, state, lexer);
    if (!functionDefNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create function node.");
        parsingError("Failed to create function node.", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    functionDefNode->data.functionDecl->paramTypes = paramTypes;
    functionDefNode->data.functionDecl->paramCount = paramCount;
    functionDefNode->data.functionDecl->parentScopeID = getNamespaceScopeID(context);
    functionDefNode->data.functionDecl->functionScopeID = Generate64BitHashID(functionName);

    // Parse the function block
    ASTNode *functionBlock = parseFunctionBlock(lexer, context, arena, state, globalTable);
    if (!functionBlock)
    {
        parsingError("Failed to parse function block.", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    ASTNode *functionNode = createFunctionNode(visibility, strdup(functionName), params, functionBlock, returnType, arena, state, lexer);
    DataType *functionType = DTM->functionTypes->createFunctionType(paramTypes, paramCount, returnType);

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

    CompleteFunctionDeclaration(globalTable, functionNode, functionName, namespaceScopeID); // Global Symbol Table
    context->inGenericContext = false;                                                      // Reset generic context flag in ParsingContext
    resetCurrentFunction(context);                                                          // Context Manager

    return functionNode;
}
// </parseFunctionDeclaration>

// <parseExternFunctionDeclaration>
ASTNode *parseExternFunctionDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing extern function declaration...");
    consume(__LINE__, lexer, TOKEN_KW_FN, "Expected `function` keyword", "parseExternFunctionDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseExternFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    const char *namespaceScopeID = getNamespaceScopeID(context);
    char *functionName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionName);

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, context, arena, strdup(functionName), state, globalTable);
    // get length of params
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    DataType *returnType = NULL; // Default return type
    if (lexer->currentToken.type == TOKEN_RESULT_ARROW)
    {
        logMessage(LMI, "INFO", "Parser", "Found return type arrow");
        getNextToken(lexer, arena, state);
        returnType = parseType(lexer, context, arena, state, globalTable);
        getNextToken(lexer, arena, state);
    }
    else
    {
        parsingError("Expected `->` for return type.", "parseFunctionDeclaration", arena, state, lexer, lexer->source, globalTable);
    }

    logMessage(LMI, "INFO", "Parser", "Function Return Type: %s", returnType->debug->toString(returnType));
    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseExternFunctionDeclaration", arena, state, context);

    ASTNode *externFunc = createExternFuncNode(functionName, params, returnType, arena, state, lexer);

    (externFunc, arena);

    AddExternFunctionToTable(globalTable, externFunc, namespaceScopeID);

    return externFunc;
}
// </parseExternFunctionDeclaration>

// <parseFunctionCall>
ASTNode *parseFunctionCall(Lexer *lexer, ParsingContext *context,
                           char *functionName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function call...");

    Token token = lexer->currentToken;

    // Check if the function name is a struct declaration.
    // In this case, we have to create a new struct instance and redirect the parser
    // to the struct declaration.
    if (IsStructSymbol(globalTable, functionName))
    {
        logMessage(LMI, "INFO", "Parser", "Struct declaration detected.");
        return parseNewStructObject(functionName, lexer, context, arena, state, globalTable);
    }

    // Create function call node
    ASTNode *functionCallNode = createFunctionCallNode(arena, state, lexer);
    functionCallNode->data.functionCall->name = strdup(functionName);
    functionCallNode->data.functionCall->argCount = 0;
    functionCallNode->data.functionCall->argCapacity = 8;
    functionCallNode->data.functionCall->args = (ASTNode **)ARENA_ALLOC(arena,
                                                                        functionCallNode->data.functionCall->argCapacity * sizeof(ASTNode *));

    // Look up function in global symbol table (NEW)
    const char *currentScopeID = getCurrentScopeID(context);
    Symbol *funcSymbol = GetFrontendSymbol(globalTable, functionName, currentScopeID, FUNCTION_SYMBOL);
    if (!funcSymbol)
    {
        logMessage(LMI, "ERROR", "Parser",
                   "Function not found: %s", functionName);
        parsingError("Function not found.", "parseFunctionCall", arena, state,
                     lexer, lexer->source, globalTable);
        return NULL;
    }
    TypeOfSymbol typeofSymbol = funcSymbol->symbolType;
    ASTNode *functionNode = NULL;
    int _paramCount = 0;
    if (typeofSymbol == FUNCTION_SYMBOL)
    {
        logMessage(LMI, "INFO", "Parser", "Function Symbol");
        functionNode = funcSymbol->function->node;
        _paramCount = funcSymbol->function->paramCount;
    }
    else if (typeofSymbol == EXTERN_SYMBOL)
    {
        logMessage(LMI, "INFO", "Parser", "Extern Symbol");
        functionNode = funcSymbol->externSymbol->node;
        _paramCount = funcSymbol->externSymbol->paramCount;
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser",
                   "Invalid symbol type: %s", typeofSymbol);
        parsingError("Invalid symbol type.", "parseFunctionCall", arena, state,
                     lexer, lexer->source, globalTable);
        return NULL;
    }

    char *functionNameToken = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Function name: %s", functionNameToken);

    // Consume function name
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.",
            "parseFunctionCall", arena, state, context);

    ASTNode **expectedArgs = (ASTNode **)ARENA_ALLOC(arena, 8 * sizeof(ASTNode *));
    DataType **expectedTypes = (DataType **)ARENA_ALLOC(arena, 8 * sizeof(DataType *));

    // Parse arguments
    switch (typeofSymbol)
    {
    case EXTERN_SYMBOL:
    {
        logMessage(LMI, "INFO", "Parser", "Arguments for extern function...");
        ASTNode *externFuncNode = functionNode;
        logASTNode(externFuncNode);
        ASTNode **params = externFuncNode->data.externFunction->params;
        int paramCount = externFuncNode->data.externFunction->paramCount;
        for (int i = 0; i < paramCount; ++i)
        {
            DataType *expectedType = params[i]->data.param->type;
        }

        break;
    }
    case FUNCTION_SYMBOL:
    {
        logMessage(LMI, "INFO", "Parser", "Arguments for function...");
        ASTNode *funcDeclNode = functionNode;
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
        logMessage(LMI, "ERROR", "Parser",
                   "Invalid function type: ");
        parsingError("Invalid function type.", "parseFunctionCall",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    }

    if (lexer->currentToken.type != TOKEN_LPAREN)
    {
        logMessage(LMI, "ERROR", "Parser",
                   "Expected '(' after function name, got: %s", CryoTokenToString(lexer->currentToken.type));

        parsingError("Expected '(' after function name.", "parseFunctionCall",
                     arena, state, lexer, lexer->source, globalTable);

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
                ASTNode *arg = parseUnaryExpression(lexer, context, arena, state, globalTable);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                continue;
            }
            logMessage(LMI, "INFO", "Parser", "Current Token Type: %s", CryoTokenToString(token));
            switch (token)
            {
            case TOKEN_INT_LITERAL:
            {
                DataType *expectedType = DTM->primitives->createInt();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state, globalTable);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                break;
            }

            case TOKEN_STRING_LITERAL:
            {
                char *stringLiteral = strndup(lexer->currentToken.start, lexer->currentToken.length);
                int stringLength = strlen(stringLiteral);
                DataType *expectedType = DTM->primitives->createString();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state, globalTable);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                break;
            }

            case TOKEN_BOOLEAN_LITERAL:
            {
                bool booleanValue = lexer->currentToken.type == TOKEN_KW_TRUE ? true : false;
                DataType *expectedType = DTM->primitives->createBoolean();
                ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state, globalTable);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
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
                    ASTNode *dotNode = parseDotNotation(lexer, context, arena, state, globalTable);
                    addArgumentToFunctionCall(functionCallNode, dotNode, arena, state, globalTable);
                }
                else if (nextToken.type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing function call...");
                    ASTNode *funcCallNode = parseFunctionCall(lexer, context, identifier, arena, state, globalTable);
                    addArgumentToFunctionCall(functionCallNode, funcCallNode, arena, state, globalTable);
                }
                else
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing identifier...");
                    ASTNode *arg = parsePrimaryExpression(lexer, context, arena, state, globalTable);
                    addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                }
                break;
            }

            case TOKEN_KW_THIS:
            {
                if (!context->thisContext)
                {
                    parsingError("Invalid use of `this` keyword.", "parseFunctionCall",
                                 arena, state, lexer, lexer->source, globalTable);
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
                    parsingError("Expected `.` after `this` keyword.", "parseFunctionCall",
                                 arena, state, lexer, lexer->source, globalTable);
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
                        addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                        break;
                    }

                    if (i == thisContextPropCount - 1)
                    {
                        parsingError("Property not found in `this` context.", "parseFunctionCall",
                                     arena, state, lexer, lexer->source, globalTable);
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
                ASTNode *arg = parseExpression(lexer, context, arena, state, globalTable);
                addArgumentToFunctionCall(functionCallNode, arg, arena, state, globalTable);
                break;
            }

            case TOKEN_SEMICOLON:
            {
                consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected ';' after function call.",
                        "parseFunctionCall", arena, state, context);
                break;
            }

            default:
            {
                const char *tokenStr = CryoTokenToString(token);
                const char *errorStr = "Invalid argument, received: ";
                const char *fullErrorMessage = concatStrings(errorStr, tokenStr);
                parsingError((char *)fullErrorMessage, "parseFunctionCall",
                             arena, state, lexer, lexer->source, globalTable);
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

    // Validate argument count
    if (functionCallNode->data.functionCall->argCount != _paramCount)
    {
        logMessage(LMI, "ERROR", "Parser",
                   "Argument count mismatch. Expected: %d, Got: %d",
                   _paramCount, functionCallNode->data.functionCall->argCount);
        parsingError("Argument count mismatch.", "parseFunctionCall",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    return functionCallNode;
}
// </parseFunctionCall>

// <parseReturnStatement>
ASTNode *parseReturnStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        expression = parseExpression(lexer, context, arena, state, globalTable);
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
ASTNode *parseParameter(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
            DataType *elementType = parseType(lexer, context, arena, state, globalTable);
            if (!elementType)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse element type for variadic parameter");
                parsingError("Failed to parse element type for variadic parameter", "parseParameter", arena, state, lexer, lexer->source, globalTable);
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
                parsingError("Variadic parameter must have array type.", "parseParameter", arena, state, lexer, lexer->source, globalTable);
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
            ASTNode *node = createParamNode("...", functionName, DTM->primitives->createAny(), arena, state, lexer);
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
        parsingError("Expected an identifier.", "parseParameter", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Parameter name: %s", paramName);

    getNextToken(lexer, arena, state);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after parameter name.", "parseParameter", arena, state, context);

    DataType *paramType = parseType(lexer, context, arena, state, globalTable);
    if (!paramType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse parameter type");
        parsingError("Failed to parse parameter type", "parseParameter", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    const char *paramTypeStr = paramType->debug->toString(paramType);
    logMessage(LMI, "INFO", "Parser", "<!> Parameter type: %s", paramTypeStr);

    // Consume data type token
    getNextToken(lexer, arena, state);

    // Create parameter node
    ASTNode *node = createParamNode(strdup(paramName), strdup(functionName), paramType, arena, state, lexer);

    return node;
}
// </parseParameter>

// <parseParameterList>
ASTNode **parseParameterList(Lexer *lexer, ParsingContext *context, Arena *arena, char *functionName, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        ASTNode *param = parseParameter(lexer, context, arena, functionName, state, globalTable);
        if (param)
        {
            if (param->metaData->type == NODE_PARAM)
            {
                logMessage(LMI, "INFO", "Parser", "Adding parameter: %s", param->data.param->name);
                paramListNode[paramCount] = param;
                paramCount++;

                if (globalTable)
                {
                    const char *functionScopeID = Generate64BitHashID(functionName);
                    AddParamToSymbolTable(globalTable, param, functionScopeID);
                }
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
ASTNode *parseArguments(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing arguments...");

    Token currentToken = lexer->currentToken;

    if (currentToken.type != TOKEN_IDENTIFIER &&
        currentToken.type != TOKEN_INT_LITERAL &&
        currentToken.type != TOKEN_STRING_LITERAL &&
        currentToken.type != TOKEN_BOOLEAN_LITERAL &&
        currentToken.type != TOKEN_KW_THIS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected an identifier or literal, received: %s", CryoTokenToString(currentToken.type));
        parsingError("Expected an identifier or literal.", "parseArguments", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    char *argName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    bool isLiteral = false;
    DataType *argType = NULL;
    CryoNodeType nodeType = NODE_UNKNOWN;

    // Resolve the type if it's not a literal
    // Check if `argName` is a literal number
    if (lexer->currentToken.type == TOKEN_INT_LITERAL)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is an integer literal");
        isLiteral = true;
        nodeType = NODE_LITERAL_EXPR;
        argType = DTM->primitives->createInt();
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
        return parseThisContext(lexer, context, arena, state, globalTable);
    }
    else if (lexer->currentToken.type == TOKEN_IDENTIFIER)
    {
        if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Argument is a dot notation");
            return parseDotNotation(lexer, context, arena, state, globalTable);
        }
        logMessage(LMI, "INFO", "Parser", "Argument is an identifier");
        nodeType = NODE_VAR_DECLARATION;
        argType = DTM->primitives->createVoid();
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
        Symbol *sym = GetFrontendSymbol(globalTable, argName, curScopeID, VARIABLE_SYMBOL);
        if (sym)
        {
            logMessage(LMI, "INFO", "Parser", "Symbol found in global table: %s", argName);
            argType = sym->variable->type;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in global table.");
            parsingError("Symbol not found in global table.", "parseArguments", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }
    else
    {
        if (lexer->currentToken.type == TOKEN_INT_LITERAL)
        {
            argType = DTM->primitives->createInt();
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
ASTNode *parseArgumentList(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        ASTNode *arg = parseArguments(lexer, context, arena, state, globalTable);
        if (arg)
        {
            logMessage(LMI, "INFO", "Parser", "Adding argument to list...");
            addArgumentToList(argListNode, arg, arena, state, globalTable);
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
ASTNode *parseArgumentsWithExpectedType(Lexer *lexer, ParsingContext *context, DataType *expectedType, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
        parsingError("Expected an identifier.", "parseArgumentsWithExpectedType", arena, state, lexer, lexer->source, globalTable);
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
        expectedType = DTM->primitives->createString();
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

        if (usingDotNotation)
        {
            logMessage(LMI, "INFO", "Parser", "Dot notation detected.");
            // Parse through the dot notation
            ASTNode *dotExpr = parseDotNotation(lexer, context, arena, state, globalTable);
            if (!dotExpr)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse dot expression.");
                parsingError("Failed to parse dot expression.", "parseArgumentsWithExpectedType", arena, state, lexer, lexer->source, globalTable);
                return NULL;
            }

            logMessage(LMI, "INFO", "Parser", "Dot expression parsed.");
            return dotExpr;
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "Argument is not using dot notation.");
        }

        if (prevToken.type == TOKEN_KW_THIS)
        {
            logMessage(LMI, "INFO", "Parser", "Argument is a 'this' keyword");
            return parseForThisValueProperty(lexer, expectedType, context, arena, state, globalTable);
        }

        nodeType = NODE_VAR_NAME;

        // New Global Symbol Table Lookup
        const char *curScopeID = getCurrentScopeID(context);
        Symbol *sym = GetFrontendSymbol(globalTable, argName, curScopeID, VARIABLE_SYMBOL);
        if (sym)
        {
            logMessage(LMI, "INFO", "Parser", "Symbol found in global table.");
            expectedType = sym->variable->type;
            isLiteral = false;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in global table.");
            parsingError("Symbol not found in global table.", "parseArgumentsWithExpectedType", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
    }
    else if (lexer->currentToken.type == TOKEN_KW_THIS)
    {
        logMessage(LMI, "INFO", "Parser", "Argument is a 'this' keyword");
        return parseExpectedTypeArgWithThisKW(lexer, expectedType, context, arena, state, globalTable);
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Argument is not a literal");
        Symbol *sym = GetFrontendSymbol(globalTable, argName, getCurrentScopeID(context), VARIABLE_SYMBOL);
        if (!sym)
        {
            logMessage(LMI, "ERROR", "Parser", "Symbol not found in the global table.");
            parsingError("Symbol not found in the global table.", "parseArgumentsWithExpectedType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        DataType *symbolType = GetDataTypeFromSymbol(globalTable, sym);
        if (!symbolType)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to get data type from symbol.");
            parsingError("Failed to get data type from symbol.", "parseArgumentsWithExpectedType", arena, state, lexer, lexer->source, globalTable);
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
ASTNode *parseExpectedTypeArgWithThisKW(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
            parsingError("Expected 'this' keyword to be used inside a struct.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
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
            parsingError("Expected identifier after 'this' keyword.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
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
            parsingError("Property not found in struct.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }

        ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, lexer);
        if (!propAccessNode)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
            parsingError("Failed to create property access node.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }

        return propAccessNode;
    }

    // If the this keyword is not using dot notation, then it's just a reference to the struct
    // For now though, the compiler will only support dot notation for accessing struct properties
    logMessage(LMI, "ERROR", "Parser", "Expected dot notation after 'this' keyword.");
    parsingError("Expected dot notation after 'this' keyword.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
}
// </parseExpectedTypeArgWithThisKW>

// <addParameterToList>
void addParameterToList(ASTNode *paramListNode, ASTNode *param, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
void addArgumentToList(ASTNode *argListNode, ASTNode *arg, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
void addArgumentToFunctionCall(ASTNode *functionCallNode, ASTNode *arg, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
void addParameterToExternDecl(ASTNode *externDeclNode, ASTNode *param, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
void importTypeDefinitions(const char *module, const char *subModule, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Importing type definitions...");

    DEBUG_BREAKPOINT;
    return;
}
// </importTypeDefinitions>

// <parseExtern>
ASTNode *parseExtern(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing extern...");
    consume(__LINE__, lexer, TOKEN_KW_EXTERN, "Expected `extern` keyword.", "parseExtern", arena, state, context);

    switch (lexer->currentToken.type)
    {
    case TOKEN_KW_FN:
        return parseExternFunctionDeclaration(lexer, context, arena, state, globalTable);

    default:
        parsingError("Expected an extern declaration.", "parseExtern", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    return NULL;
}
// </parseExtern>

/* ====================================================================== */
/* @ASTNode_Parsing - Conditionals                                        */

// <parseIfStatement>
ASTNode *parseIfStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing if statement...");
    consume(__LINE__, lexer, TOKEN_KW_IF, "Expected `if` keyword.", "parseIfStatement", arena, state, context);
    context->isParsingIfCondition = true;

    ASTNode *condition = parseIfCondition(lexer, context, arena, state, globalTable);
    ASTNode *ifBlock = parseBlock(lexer, context, arena, state, globalTable);
    ASTNode *elseBlock = NULL;

    if (lexer->currentToken.type == TOKEN_KW_ELSE)
    {
        getNextToken(lexer, arena, state);
        if (lexer->currentToken.type == TOKEN_KW_IF)
        {
            elseBlock = parseIfStatement(lexer, context, arena, state, globalTable);
        }
        else
        {
            elseBlock = parseBlock(lexer, context, arena, state, globalTable);
        }
    }

    context->isParsingIfCondition = false;
    return createIfStatement(condition, ifBlock, elseBlock, arena, state, lexer);
}
// </parseIfStatement>

ASTNode *parseIfCondition(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing if condition...");
    char *cur_token = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Current token: %s", cur_token);

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start if condition.", "parseIfCondition", arena, state, context);

    char *cur_token_cpy = strndup(lexer->currentToken.start, lexer->currentToken.length);

    logMessage(LMI, "INFO", "Parser", "Current token: %s", cur_token_cpy);
    ASTNode *condition = parseExpression(lexer, context, arena, state, globalTable);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end if condition.", "parseIfCondition", arena, state, context);

    return condition;
}

// <parseForLoop>
ASTNode *parseForLoop(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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

    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        init = parseVarDeclaration(lexer, context, arena, state, globalTable);
    }
    logASTNode(init);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after for loop initialization.", "parseForLoop", arena, state, context);

    if (lexer->currentToken.type != TOKEN_SEMICOLON)
    {
        condition = parseExpression(lexer, context, arena, state, globalTable);
    }
    logASTNode(condition);

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after for loop condition.", "parseForLoop", arena, state, context);

    if (lexer->currentToken.type != TOKEN_RPAREN)
    {
        increment = parseExpression(lexer, context, arena, state, globalTable);
    }
    logASTNode(increment);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end for loop.", "parseForLoop", arena, state, context);

    DEBUG_BREAKPOINT;
}
// </parseForLoop>

// <parseWhileStatement>
ASTNode *parseWhileStatement(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing while statement...");
    consume(__LINE__, lexer, TOKEN_KW_WHILE, "Expected `while` keyword.", "parseWhileStatement", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start while loop.", "parseWhileStatement", arena, state, context);

    ASTNode *condition = parseExpression(lexer, context, arena, state, globalTable);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end while loop.", "parseWhileStatement", arena, state, context);

    ASTNode *body = parseBlock(lexer, context, arena, state, globalTable);
    return createWhileStatement(condition, body, arena, state, lexer);
}
// </parseWhileStatement>

/* ====================================================================== */
/* @ASTNode_Parsing - Arrays                                              */

// <parseArrayLiteral>
ASTNode *parseArrayLiteral(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing array literal...");
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array literal.", "parseArrayLiteral", arena, state, context);

    ASTNode *elements = createArrayLiteralNode(arena, state, lexer);
    if (elements == NULL)
    {
        fprintf(stderr, "[Parser] [ERROR] Failed to create array literal node\n");
        return NULL;
    }

    int elementCount = 0;
    DataType **elementTypes = (DataType **)calloc(ARRAY_CAPACITY, sizeof(DataType *));

    while (lexer->currentToken.type != TOKEN_RBRACKET)
    {
        ASTNode *element = parseExpression(lexer, context, arena, state, globalTable);
        if (element)
        {
            addElementToArrayLiteral(elements, element, arena, state, globalTable);
            logMessage(LMI, "INFO", "Parser", "Element added to array literal.");

            DataType *elType = DTM->astInterface->getTypeofASTNode(element);
            elementTypes[elementCount] = elType;
            logMessage(LMI, "INFO", "Parser", "Element type: %s", elType->debug->toString(elType));

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

    elements->data.array->elementTypes = elementTypes;
    elements->data.array->elementCount = elementCount;
    elements->data.array->elementCapacity = elementCount;

    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array literal.", "parseArrayLiteral", arena, state, context);
    return elements;
}
// </parseArrayLiteral>

// <addElementToArrayLiteral>
void addElementToArrayLiteral(ASTNode *arrayLiteral, ASTNode *element, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
ASTNode *parseArrayIndexing(Lexer *lexer, ParsingContext *context, char *arrayName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing array indexing...");
    char *arrName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *arrCpyName = strdup(arrName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseArrayIndexing", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LBRACKET, "Expected `[` to start array indexing.", "parseArrayIndexing", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Array name: %s", arrCpyName);
    ASTNode *arrNode = (ASTNode *)ARENA_ALLOC(arena, sizeof(ASTNode));

    // Find the array in the symbol table
    Symbol *sym = FindSymbol(globalTable, arrCpyName, getCurrentScopeID(context));
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Array not found.");
        parsingError("Array not found.", "parseArrayIndexing", arena, state, lexer, lexer->source, globalTable);
        exit(1);
        return NULL;
    }
    else
    {
        logMessage(LMI, "INFO", "Parser", "Array found.");
        arrNode = GetASTNodeFromSymbol(globalTable, sym);
    }

    ASTNode *index = parseExpression(lexer, context, arena, state, globalTable);
    consume(__LINE__, lexer, TOKEN_RBRACKET, "Expected `]` to end array indexing.", "parseArrayIndexing", arena, state, context);
    return createIndexExprNode(strdup(arrCpyName), arrNode, index, arena, state, lexer);
}
// </parseArrayIndexing>

/* ====================================================================== */
/* @ASTNode_Parsing - Assignments                                         */

ASTNode *parseAssignment(Lexer *lexer, ParsingContext *context, char *varName, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing assignment...");
    char *_varName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    char *varNameCpy = strdup(_varName);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseAssignment", arena, state, context);
    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for assignment.", "parseAssignment", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Variable name: %s", varNameCpy);

    Symbol *sym = FindSymbol(globalTable, varNameCpy, getCurrentScopeID(context));
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find symbol.");
        parsingError("Failed to find symbol.", "parseAssignment", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    ASTNode *symbolNode = GetASTNodeFromSymbol(globalTable, sym);
    ASTNode *oldValue = symbolNode;
    ASTNode *newValue = parseExpression(lexer, context, arena, state, globalTable);

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
        parsingError("Old value is not a variable declaration.", "parseAssignment", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    }

    // Check if the symbol is mutable
    if (!isMutable)
    {
        logMessage(LMI, "ERROR", "Parser", "Variable is not mutable.");
        parsingError("Variable is not mutable.", "parseAssignment", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseAssignment", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Creating assignment node...");

    ASTNode *assignment = createVarReassignment(strdup(varNameCpy), oldValue, newValue, arena, state, lexer);
    logMessage(LMI, "INFO", "Parser", "Assignment node created.");
    return assignment;
}

ASTNode *parseThisContext(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing `this` context...");
    consume(__LINE__, lexer, TOKEN_KW_THIS, "Expected `this` keyword.", "parseThisContext", arena, state, context);

    if (context->thisContext == NULL)
    {
        parsingError("This context not in scope.", "parseThisContext", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    if (context->thisContext->isStatic)
    {
        parsingError("Cannot use `this` keyword in static context.", "parseThisContext", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    CryoTokenType currentToken = lexer->currentToken.type;

    ASTNode *thisNode;
    if (currentToken == TOKEN_DOT)
    {
        consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseThisContext", arena, state, context);
        thisNode = parseDotNotation(lexer, context, arena, state, globalTable);
    }
    else
    {
        thisNode = createThisNode(arena, state, lexer);
    }

    // Check if we are setting a property of the `this` context with `=`
    if (lexer->currentToken.type == TOKEN_EQUAL)
    {
        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` for property reassignment.", "parseThisContext", arena, state, context);
        char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);

        ASTNode *newValue = parseExpression(lexer, context, arena, state, globalTable);

        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseThisContext", arena, state, context);

        ASTNode *propReasignment = createPropertyReassignmentNode(thisNode, propName, newValue, arena, state, lexer);
        return propReasignment;
    }

    logMessage(LMI, "INFO", "Parser", "Finished parsing `this` context.");
    return thisNode;
}

ASTNode *parseLHSIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, DataType *typeOfNode, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing LHS identifier...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseLHSIdentifier", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing dot notation...");

    CryoTokenType currentToken = lexer->currentToken.type;

    switch (currentToken)
    {
    case TOKEN_KW_THIS:
    {
        return parseThisContext(lexer, context, arena, state, globalTable);
    }
    case TOKEN_IDENTIFIER:
    {
        return parseIdentifierDotNotation(lexer, context, arena, state, globalTable);
    }
    default:
    {
        parsingError("Expected `this` keyword.", "parseDotNotation", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    }

    DEBUG_BREAKPOINT;
}

ASTNode *parseIdentifierDotNotation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
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
                    logMessage(LMI, "INFO", "Parser", "Property found in struct.");
                    return createPropertyAccessNode(properties[i], thisContext->nodeName, arena, state, lexer);
                }
                else
                {
                    logMessage(LMI, "ERROR", "Parser", "Property not found in struct.");
                }
            }

            parsingError("Property not found in struct.", "parseIdentifierDotNotation", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
        else
        {
            parsingError("Expected a struct declaration.", "parseIdentifierDotNotation", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }
    if (currentToken.type == TOKEN_IDENTIFIER)
    {
        // Get the identifier name
        char *identifierName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Identifier name: %s", identifierName);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseIdentifierDotNotation", arena, state, context);

        Symbol *sym = FindSymbol(globalTable, identifierName, getCurrentScopeID(context));
        if (!sym)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find symbol.");
            parsingError("Failed to find symbol.", "parseIdentifierDotNotation", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        ASTNode *symbolNode = GetASTNodeFromSymbol(globalTable, sym);
        ASTNode *identifierNode = symbolNode;
        DataType *typeOfNode = DTM->astInterface->getTypeofASTNode(identifierNode);
        DataType *typeFromSymbol = GetDataTypeFromSymbol(globalTable, sym);

        if (lexer->currentToken.type == TOKEN_DOT)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing dot notation with identifier...");
            return parseDotNotationWithType(identifierNode, typeFromSymbol, lexer, context, arena, state, globalTable);
        }

        DEBUG_BREAKPOINT;
    }

    char *unexpectedTokenStr = (char *)malloc(256);
    char *tokenStr = CryoTokenToString(lexer->currentToken.type);
    sprintf(unexpectedTokenStr, "Unexpected token in dot notation. Received: %s", tokenStr);
    parsingError(unexpectedTokenStr, "parseIdentifierDotNotation", arena, state, lexer, lexer->source, globalTable);
    return NULL;
}

ASTNode *parseDotNotationWithType(ASTNode *object, DataType *typeOfNode, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing dot notation with type...");
    consume(__LINE__, lexer, TOKEN_DOT, "Expected `.` for property access.", "parseDotNotationWithType", arena, state, context);
    // The first identifier is the type of the node we are accessing
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier after `.`.", "parseDotNotationWithType", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    char *propName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Prop name: %s", propName);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseDotNotationWithType", arena, state, context);

    if (typeOfNode->container->typeOf == OBJECT_TYPE)
    {
        logMessage(LMI, "INFO", "Parser", "Type of node is a struct.");

        DTStructTy *structType = typeOfNode->container->type.structType;
        const char *structName = typeOfNode->typeName;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(object, propName, typeOfNode, lexer, context, arena, state, globalTable);
            return methodCallNode;
        }

        logMessage(LMI, "INFO", "Parser", "Struct name: %s", structName);
        logMessage(LMI, "INFO", "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        ASTNode *property = DTM->propertyTypes->findStructPropertyNode(structType, propName);
        if (property)
        {
            logMessage(LMI, "INFO", "Parser", "Property found in struct, name: %s", propName);
            return createStructPropertyAccessNode(object, property, (const char *)propName, typeOfNode, arena, state, lexer);
        }
        else
        {

            logMessage(LMI, "ERROR", "Parser", "Property Attempted: %s", propName);
            parsingError("Property not found in struct.", "parseDotNotationWithType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }
    // Return the property access node for *CLASS TYPES*.
    else if (typeOfNode->container->typeOf == OBJECT_TYPE)
    {
        logMessage(LMI, "INFO", "Parser", "Type of node is a class.");

        DTClassTy *classType = typeOfNode->container->type.classType;
        const char *className = typeOfNode->typeName;
        Token nextToken = peekNextUnconsumedToken(lexer, arena, state);
        Token currentToken = lexer->currentToken;

        if (currentToken.type == TOKEN_LPAREN)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing method call...");
            ASTNode *methodCallNode = parseMethodCall(object, propName, typeOfNode, lexer, context, arena, state, globalTable);
            return methodCallNode;
        }

        logMessage(LMI, "INFO", "Parser", "Class name: %s", className);
        logMessage(LMI, "INFO", "Parser", "Next token: %s", CryoTokenToString(nextToken.type));

        ASTNode *property = GetClassProperty(globalTable, (const char *)propName, className);
        if (property)
        {
            logMessage(LMI, "INFO", "Parser", "Property found in class, name: %s", propName);
            return createClassPropertyAccessNode(object, property, (const char *)propName, typeOfNode, arena, state, lexer);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Property Attempted: %s", propName);
            parsingError("Property not found in class.", "parseDotNotationWithType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }
    else
    {
        parsingError("Expected a struct or class type.", "parseDotNotationWithType", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
}

ASTNode *parseForThisValueProperty(Lexer *lexer, DataType *expectedType, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing for this value property...");
    if (context->thisContext == NULL)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected 'this' keyword to be used inside a struct.");
        parsingError("Expected 'this' keyword to be used inside a struct.", "parseForThisValueProperty", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    // This assumes the `this` and `.` tokens have already been consumed,
    ASTNode **accessProperties = context->thisContext->properties;
    int propertyCount = context->thisContext->propertyCount;

    // Check the next token identifier and match it with the properties of the struct
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected identifier after 'this' keyword. Received: %s", CryoTokenToString(lexer->currentToken.type));
        parsingError("Expected identifier after 'this' keyword.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
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

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseExpectedTypeArgWithThisKW", arena, state, context);

    if (!matchedProperty)
    {
        logMessage(LMI, "ERROR", "Parser", "Property not found in struct.");
        logMessage(LMI, "ERROR", "Parser", "Property name: %s", propertyName);

        parsingError("Property not found in struct.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    ASTNode *propAccessNode = createStructPropertyAccessNode(matchedProperty, matchedProperty, propertyName, expectedType, arena, state, lexer);
    if (!propAccessNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
        parsingError("Failed to create property access node.", "parseExpectedTypeArgWithThisKW", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    return propAccessNode;
}

ASTNode *parseNewExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing new expression...");
    consume(__LINE__, lexer, TOKEN_KW_NEW, "Expected `new` keyword.", "parseNewExpression", arena, state, context);

    const char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseNewExpression", arena, state, context);
    int argCount = 0;

    logMessage(LMI, "INFO", "Parser", "Type name: %s", typeName);

    DataType *type = ResolveDataType(globalTable, typeName);
    if (!type)
    {
        logMessage(LMI, "ERROR", "Parser", "Type not found.");
        parsingError("Type not found.", "parseNewExpression", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "Parser", "Type found.");
    type->debug->printType(type);

    ASTNode *args = parseArgumentList(lexer, context, arena, state, globalTable);
    if (!args)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse argument list.");
        parsingError("Failed to parse argument list.", "parseNewExpression", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    logASTNode(args);

    // We need to check if the type is a struct or a class
    if (type->container->typeOf == OBJECT_TYPE)
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
            parsingError("Argument count mismatch.", "parseNewExpression", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
    }

    // TODO: Implement
    // if (type->container->baseType == CLASS_TYPE)
    // {
    //     logMessage(LMI, "INFO", "Parser", "Type is a class.");
    // }

    ASTNode **arguments = args->data.argList->args;

    ASTNode *objectNode = createObject(typeName, type, true, arguments, argCount, arena, state, lexer);
    if (!objectNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create object node.");
        parsingError("Failed to create object node.", "parseNewExpression", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    logASTNode(objectNode);

    return objectNode;
}

ASTNode *parseNullExpression(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing null expression...");
    consume(__LINE__, lexer, TOKEN_KW_NULL, "Expected `null` keyword.", "parseNullExpression", arena, state, context);

    return createNullNode(arena, state, lexer);
}

// The `typeof` keyword is used to get the type of an identifier.
// e.g. `typeof(x)` would return the type of the variable `x` as a string.
ASTNode *parseTypeofIdentifier(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing typeof identifier...");
    consume(__LINE__, lexer, TOKEN_KW_TYPEOF, "Expected `typeof` keyword.", "parseTypeofIdentifier", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start typeof expression.", "parseTypeofIdentifier", arena, state, context);

    ASTNode *identifier = parseExpression(lexer, context, arena, state, globalTable);
    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end typeof expression.", "parseTypeofIdentifier", arena, state, context);

    return createTypeofNode(identifier, arena, state, lexer);
}
