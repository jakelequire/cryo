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
/* @ASTNode_Parsing - Structures                                          */

// <parseStructDeclaration>
ASTNode *parseStructDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing struct declaration...");
    consume(__LINE__, lexer, TOKEN_KW_STRUCT, "Expected `struct` keyword.", "parseStructDeclaration", table, arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseStructDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *structName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Struct name: %s", structName);

    // Setting the context to the struct name
    setThisContext(context, (const char *)structName, NODE_STRUCT_DECLARATION, typeTable);

    getNextToken(lexer, arena, state, typeTable);

    if (lexer->currentToken.type == TOKEN_LESS)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing generic declaration...");
        ASTNode *genericNode = parseGenericDecl(structName, lexer, table, context, arena, state, typeTable);
        DEBUG_BREAKPOINT;
    }

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start struct declaration.", "parseStructDeclaration", table, arena, state, typeTable, context);

    int PROPERTY_CAPACITY = 64;
    ASTNode **properties = (ASTNode **)ARENA_ALLOC(arena, PROPERTY_CAPACITY * sizeof(ASTNode *));
    ASTNode **methods = (ASTNode **)ARENA_ALLOC(arena, PROPERTY_CAPACITY * sizeof(ASTNode *));

    int propertyCount = 0;
    int methodCount = 0;
    bool hasDefaultProperty = false;
    bool hasConstructor = false;
    int defaultPropertyCount = 0;
    ASTNode *constructorNode = NULL;

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        int count = 0;
        ASTNode *field = parseStructField(lexer, table, context, arena, state, typeTable);
        if (field)
        {
            CryoNodeType fieldType = field->metaData->type;
            if (fieldType == NODE_PROPERTY)
            {
                properties[propertyCount] = field;
                propertyCount++;
                addASTNodeSymbol(table, field, arena);
                addPropertyToThisContext(context, field, typeTable);

                if (parsePropertyForDefaultFlag(field) && !hasDefaultProperty)
                {
                    hasDefaultProperty = true;
                    defaultPropertyCount++;
                }
                if (defaultPropertyCount > 1)
                {
                    logMessage("ERROR", __LINE__, "Parser", "Struct can only have one default property.");
                    return NULL;
                }

                count++;
            }
            else
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to parse struct field, received Node Type %s", CryoNodeTypeToString(fieldType));
                parsingError("Failed to parse struct field.", "parseStructDeclaration", table, arena, state, lexer, lexer->source, typeTable);
            }

            if (fieldType == NODE_METHOD)
            {
                logMessage("INFO", __LINE__, "Parser", "Parsing method declaration...");
                DEBUG_BREAKPOINT;
            }

            // End the loop
            // if (lexer->currentToken.type == TOKEN_RBRACE)
            // {
            //     break;
            // }

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                // parse the next field
                continue;
            }
        }

        if (lexer->currentToken.type == TOKEN_KW_CONSTRUCTOR)
        {
            hasConstructor = true;
            ConstructorMetaData *metaData = (ConstructorMetaData *)ARENA_ALLOC(arena, sizeof(ConstructorMetaData));
            metaData->parentName = strdup(structName);
            metaData->parentNodeType = NODE_STRUCT_DECLARATION;
            metaData->hasDefaultFlag = hasDefaultProperty;

            constructorNode = parseConstructor(lexer, table, context, arena, state, metaData, typeTable);
        }

        // This is for the method declarations
        if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
            lexer->nextToken.type == TOKEN_LPAREN &&
            lexer->currentToken.type != TOKEN_KW_CONSTRUCTOR)
        {
            ASTNode *method = parseMethodDeclaration(lexer, table, context, arena, state, typeTable);
            if (method)
            {
                methods[methodCount] = method;
                methodCount++;
                addASTNodeSymbol(table, method, arena);
                addMethodToThisContext(context, method, typeTable);
            }
        }
        else if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }
        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse struct field.");
            CONDITION_FAILED;
        }

        if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }

        else
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse struct field.");
            CONDITION_FAILED;
        }
    }

    logMessage("INFO", __LINE__, "Parser", "Property Count: %d | Method Count: %d", propertyCount, methodCount);
    ASTNode *structNode = createStructNode(structName, properties, propertyCount, constructorNode,
                                           methods, methodCount,
                                           arena, state, typeTable);
    structNode->data.structNode->hasDefaultValue = hasDefaultProperty;
    structNode->data.structNode->hasConstructor = hasConstructor;

    DataType *structDataType = createDataTypeFromStructNode(structNode, properties, propertyCount,
                                                            methods, methodCount,
                                                            state, typeTable);
    structNode->data.structNode->type = structDataType;

    addTypeToTypeTable(typeTable, structName, structDataType);

    // Add the struct to the symbol table
    addASTNodeSymbol(table, structNode, arena);

    // Clear the `this` context after parsing the struct
    clearThisContext(context, typeTable);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end struct declaration.", "parseStructDeclaration", table, arena, state, typeTable, context);
    return structNode;
}

bool parsePropertyForDefaultFlag(ASTNode *propertyNode)
{
    if (propertyNode->metaData->type == NODE_PROPERTY)
    {
        PropertyNode *property = propertyNode->data.property;
        return property->defaultProperty;
    }
    return false;
}

// <parseStructField>
ASTNode *parseStructField(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing struct field...");

    int defaultCount = 0; // This should never be more than 1
    CryoTokenType currentToken = lexer->currentToken.type;
    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state, typeTable).type;
    if (currentToken == TOKEN_KW_DEFAULT)
    {
        defaultCount++;
        // Consume the `default` keyword
        getNextToken(lexer, arena, state, typeTable);
    }

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseStructField", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    if (nextToken == TOKEN_LPAREN && currentToken != TOKEN_KW_CONSTRUCTOR)
    {
        logMessage("INFO", __LINE__, "Parser", "Parsing method declaration...");
        return parseMethodDeclaration(lexer, table, context, arena, state, typeTable);
    }

    printf("Default Count: %d\n", defaultCount);

    char *fieldName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Field name: %s", strdup(fieldName));

    getNextToken(lexer, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after field name.", "parseStructField", table, arena, state, typeTable, context);

    DataType *fieldType = parseType(lexer, context, table, arena, state, typeTable);
    getNextToken(lexer, arena, state, typeTable);

    // This is where we could add support for values in the struct fields
    // For now, this is just going to be a type declaration

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseStructField", table, arena, state, typeTable, context);

    const char *parentName = context->thisContext->nodeName;
    // Find the parent node in the symbol table
    CryoNodeType parentNodeType = context->thisContext->nodeType;

    ASTNode *propertyNode = createFieldNode(strdup(fieldName), fieldType, parentName, parentNodeType, NULL, arena, state, typeTable);
    if (defaultCount > 0)
    {
        propertyNode->data.property->defaultProperty = true;
    }

    return propertyNode;
}
// </parseStructField>

ASTNode *parseConstructor(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, ConstructorMetaData *metaData, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing constructor...");
    consume(__LINE__, lexer, TOKEN_KW_CONSTRUCTOR, "Expected `constructor` keyword.", "parseConstructor", table, arena, state, typeTable, context);

    char *consturctorName = (char *)calloc(strlen(metaData->parentName) + strlen("::") + strlen("constructor") + 1, sizeof(char));
    strcat(consturctorName, (char *)metaData->parentName);
    strcat(consturctorName, "::constructor");

    ASTNode **params = parseParameterList(lexer, table, context, arena, consturctorName, state, typeTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    ASTNode *constructorBody = parseBlock(lexer, table, context, arena, state, typeTable);

    ASTNode *constructorNode = createConstructorNode(consturctorName, constructorBody, params, paramCount, arena, state, typeTable);

    return constructorNode;
}

ASTNode *parseMethodDeclaration(Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing method declaration...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseMethodDeclaration", table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Method name: %s", methodName);

    getNextToken(lexer, arena, state, typeTable);

    ASTNode **params = parseParameterList(lexer, table, context, arena, methodName, state, typeTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    // Get the return type `-> <type>`
    consume(__LINE__, lexer, TOKEN_RESULT_ARROW, "Expected `->` for return type.", "parseMethodDeclaration", table, arena, state, typeTable, context);
    DataType *returnType = parseType(lexer, context, table, arena, state, typeTable);
    getNextToken(lexer, arena, state, typeTable);

    // Create the method body
    ASTNode *methodBody = parseBlock(lexer, table, context, arena, state, typeTable);
    // Create the method node
    ASTNode *methodNode = createMethodNode(returnType, methodBody, methodName, params, paramCount, arena, state, typeTable);

    // Add the method to the symbol table
    addASTNodeSymbol(table, methodNode, arena);

    return methodNode;
}

ASTNode *parseMethodCall(ASTNode *accessorObj, char *methodName, DataType *instanceType,
                         Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing method call...");

    ASTNode **params = parseParameterList(lexer, table, context, arena, methodName, state, typeTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    logMessage("INFO", __LINE__, "Parser", "Method name: %s", methodName);
    logMessage("INFO", __LINE__, "Parser", "Param count: %d", paramCount);

    // Find the method in the symbol table
    CryoSymbol *symbol = findSymbol(table, methodName, arena);
    if (!symbol)
    {
        logMessage("ERROR", __LINE__, "Parser", "Method not found.");
        parsingError("Method not found.", "parseMethodCall", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }
    printSymbolTable(table);
    DataType *returnType = symbol->type;
    VALIDATE_TYPE(returnType);

    ASTNode *methodCall = createMethodCallNode(accessorObj, returnType, instanceType, methodName,
                                               params, paramCount, arena, state, typeTable);
    if (!methodCall)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create method call node.");
        parsingError("Failed to create method call node.", "parseMethodCall", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    logASTNode(methodCall);

    return methodCall;
}

// Enhanced parseGenericDecl implementation
ASTNode *parseGenericDecl(const char *typeName, Lexer *lexer, CryoSymbolTable *table,
                          ParsingContext *context, Arena *arena, CompilerState *state,
                          TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing generic declaration...");

    // Create a list to store generic parameters
    int genericParamCapacity = 8;
    GenericType **genericParams = (GenericType **)ARENA_ALLOC(arena,
                                                              genericParamCapacity * sizeof(GenericType *));
    int genericParamCount = 0;

    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` to start generic declaration.",
            "parseGenericDecl", table, arena, state, typeTable, context);

    // Parse generic parameters
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            parsingError("Expected generic type identifier.",
                         "parseGenericDecl", table, arena, state, lexer,
                         lexer->source, typeTable);
            return NULL;
        }

        // Get generic parameter name
        char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        getNextToken(lexer, arena, state, typeTable);

        // Create generic parameter
        GenericType *genericParam = createGenericParameter(paramName);

        // Check for constraints (e.g., T extends Number)
        if (lexer->currentToken.type == TOKEN_KW_EXTENDS)
        {
            getNextToken(lexer, arena, state, typeTable);

            // Parse constraint type
            DataType *constraint = parseType(lexer, context, table, arena, state, typeTable);
            addGenericConstraint(genericParam, constraint);
            getNextToken(lexer, arena, state, typeTable);
        }

        // Add to generic parameters list
        genericParams[genericParamCount++] = genericParam;

        // Handle comma-separated list
        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in generic parameter list.",
                         "parseGenericDecl", table, arena, state, lexer,
                         lexer->source, typeTable);
            return NULL;
        }
    }

    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` to end generic declaration.",
            "parseGenericDecl", table, arena, state, typeTable, context);

    // Create generic type container
    TypeContainer *container = createTypeContainer();
    container->baseType = GENERIC_TYPE;
    container->custom.name = strdup(typeName);
    container->custom.generic.declaration->genericDef = NULL;
    container->custom.generic.declaration->paramCount = genericParamCount;

    // Convert GenericType to DataType for each parameter
    for (int i = 0; i < genericParamCount; i++)
    {
        DataType *paramType = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
        paramType->container = createTypeContainer();
        paramType->container->baseType = GENERIC_TYPE;
        paramType->container->custom.name = strdup(genericParams[i]->name);
        // Transfer constraints if any
        if (genericParams[i]->constraint)
        {
            paramType->genericParam = genericParams[i]->constraint;
        }
        container->custom.generic.declaration->params[i] = paramType->container->custom.generic.declaration->params[0];
    }

    DataType *genericType = wrapTypeContainer(container);
    ASTNode *genericDeclNode = createGenericDeclNode(genericType, typeName, genericParams, genericParamCount, NULL, false,
                                                     arena, state, typeTable);

    // Add to type table
    addTypeToTypeTable(typeTable, typeName, genericType);

    return genericDeclNode;
}

// Helper function to parse generic type instantiation
ASTNode *parseGenericInstantiation(const char *baseName, Lexer *lexer,
                                   CryoSymbolTable *table, ParsingContext *context,
                                   Arena *arena, CompilerState *state,
                                   TypeTable *typeTable)
{
    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` in generic instantiation.",
            "parseGenericInstantiation", table, arena, state, typeTable, context);

    // Find base generic type
    DataType *baseType = lookupType(typeTable, baseName);
    if (!baseType || !isGenericType(baseType))
    {
        parsingError("Type is not generic.", "parseGenericInstantiation",
                     table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    int expectedParamCount = baseType->container->custom.generic.instantiation->argCount;
    DataType **concreteTypes = (DataType **)malloc(expectedParamCount * sizeof(DataType *));
    int paramCount = 0;

    // Parse concrete type arguments
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (paramCount >= expectedParamCount)
        {
            parsingError("Too many type arguments.", "parseGenericInstantiation",
                         table, arena, state, lexer, lexer->source, typeTable);
            return NULL;
        }

        DataType *concreteType = parseType(lexer, context, table, arena, state, typeTable);

        // Validate against constraints
        if (baseType->container->custom.generic.declaration->params[paramCount]->constraint)
        {
            if (!validateGenericType(baseType->container->custom.generic.declaration->params[paramCount]->constraint,
                                     concreteType))
            {
                parsingError("Type argument does not satisfy constraints.",
                             "parseGenericInstantiation", table, arena, state,
                             lexer, lexer->source, typeTable);
                return NULL;
            }
        }

        concreteTypes[paramCount++] = concreteType;

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in type argument list.",
                         "parseGenericInstantiation", table, arena, state,
                         lexer, lexer->source, typeTable);
            return NULL;
        }
    }

    if (paramCount != expectedParamCount)
    {
        parsingError("Wrong number of type arguments.", "parseGenericInstantiation",
                     table, arena, state, lexer, lexer->source, typeTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` after type arguments.",
            "parseGenericInstantiation", table, arena, state, typeTable, context);

    // Create instantiated type
    TypeContainer *instantiatedContainer = createGenericStructInstance(baseType->container,
                                                                       concreteTypes[0]);

    ASTNode *genericInstNode = createGenericInstNode(baseName, concreteTypes, paramCount,
                                                     wrapTypeContainer(instantiatedContainer), arena, state, typeTable);

    return genericInstNode;
}
