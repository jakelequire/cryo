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
#include "frontend/parser.h"
#include "diagnostics/diagnostics.h"

/* ====================================================================== */
/* @ASTNode_Parsing - Structures                                          */

// <parseStructDeclaration>
ASTNode *parseStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct declaration...");
    consume(__LINE__, lexer, TOKEN_KW_STRUCT, "Expected `struct` keyword.", "parseStructDeclaration", arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseStructDeclaration", arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    char *_structName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    const char *structName = (const char *)_structName;
    logMessage(LMI, "INFO", "Parser", "Struct name: %s", structName);

    const char *parentNamespaceNameID = getNamespaceScopeID(context);
    InitStructDeclaration(globalTable, structName, parentNamespaceNameID); // GlobalSymbolTable

    // Setting the context to the struct name
    setThisContext(context, structName, NODE_STRUCT_DECLARATION, typeTable);

    getNextToken(lexer, arena, state, typeTable);

    // Check if the next token is a `<` character to determine if it is a generic struct declaration
    if (lexer->currentToken.type == TOKEN_LESS)
    {
        ASTNode *genericStruct = parseGenericStructDeclaration(structName, lexer, context, arena, state, typeTable, globalTable);
        if (genericStruct)
        {
            return genericStruct;
        }
        CONDITION_FAILED;
    }

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start struct declaration.", "parseStructDeclaration", arena, state, typeTable, context);

    ASTNode **properties = (ASTNode **)ARENA_ALLOC(arena, PROPERTY_CAPACITY * sizeof(ASTNode *));
    ASTNode **methods = (ASTNode **)ARENA_ALLOC(arena, METHOD_CAPACITY * sizeof(ASTNode *));

    int propertyCount = 0;
    int methodCount = 0;
    bool hasDefaultProperty = false;
    bool hasConstructor = false;
    int ctorArgCount = 0;
    DataType **ctorArgs = (DataType **)ARENA_ALLOC(arena, sizeof(DataType *) * ARG_CAPACITY);

    int defaultPropertyCount = 0;
    ASTNode *constructorNode = NULL;

    // Add the struct name to the type table
    DataType *structDefinition = createStructDefinition(structName);

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        int count = 0;
        ASTNode *field = parseStructField(structName, lexer, context, arena, state, typeTable, globalTable);
        if (field)
        {
            CryoNodeType fieldType = field->metaData->type;
            if (fieldType == NODE_PROPERTY)
            {
                properties[propertyCount] = field;
                propertyCount++;
                addPropertyToThisContext(context, field, typeTable);
                AddPropertyToStruct(globalTable, structName, field); // GlobalSymbolTable

                if (parsePropertyForDefaultFlag(field) && !hasDefaultProperty)
                {
                    hasDefaultProperty = true;
                    defaultPropertyCount++;
                }
                if (defaultPropertyCount > 1)
                {
                    logMessage(LMI, "ERROR", "Parser", "Struct can only have one default property.");
                    return NULL;
                }

                count++;
            }
            else if (fieldType == NODE_METHOD)
            {
                methods[methodCount] = field;
                methodCount++;
                addMethodToThisContext(context, field, typeTable);
                count++;

                // Go to the next field
                continue;
            }
            else
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse struct field, received Node Type %s", CryoNodeTypeToString(fieldType));
                parsingError("Failed to parse struct field.", "parseStructDeclaration", arena, state, lexer, lexer->source, typeTable, globalTable);
            }
            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                // parse the next field
                continue;
            }
        }

        if (lexer->currentToken.type == TOKEN_KW_CONSTRUCTOR)
        {
            hasConstructor = true;
            ConstructorMetaData *constructorMetaData = createConstructorMetaData(structName, NODE_STRUCT_DECLARATION, hasDefaultProperty);
            constructorNode = parseConstructor(lexer, context, arena, state, constructorMetaData, typeTable, globalTable);
            if (constructorNode)
            {
                // Get the constructor arguments and add them to the struct data type
                int argCount = constructorNode->data.structConstructor->argCount;
                for (int i = 0; i < argCount; i++)
                {
                    logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                    ASTNode *arg = constructorNode->data.structConstructor->args[i];
                    if (arg)
                    {
                        logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                        ctorArgs[ctorArgCount] = getDataTypeFromASTNode(arg);
                        ctorArgCount++;
                    }
                }
            }
        }

        // This is for the method declarations
        if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
            lexer->nextToken.type == TOKEN_LPAREN &&
            lexer->currentToken.type != TOKEN_KW_CONSTRUCTOR)
        {
            ASTNode *method = parseMethodDeclaration(false, structName, lexer, context, arena, state, typeTable, globalTable);
            if (method)
            {
                methods[methodCount] = method;
                methodCount++;
                addMethodToThisContext(context, method, typeTable);
            }
        }

        // If we reach the end of the struct declaration with `}`
        else if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }
        // If we fail to parse a struct field
        else
        {
            char *currentTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
            printf("Current Token: %s, Token Str: %s\n", CryoTokenToString(lexer->currentToken.type), currentTokenStr);
            logMessage(LMI, "ERROR", "Parser::TypeParsing", "Failed to parse struct field.");
            CONDITION_FAILED;
        }
    }

    logMessage(LMI, "INFO", "Parser", "Property Count: %d | Method Count: %d", propertyCount, methodCount);
    ASTNode *structNode = createStructNode(structName, properties, propertyCount, constructorNode,
                                           methods, methodCount,
                                           arena, state, typeTable, lexer);
    structNode->data.structNode->hasDefaultValue = hasDefaultProperty;
    structNode->data.structNode->hasConstructor = hasConstructor;

    DataType *structDataType = createDataTypeFromStructNode(structNode, properties, propertyCount,
                                                            methods, methodCount,
                                                            state, typeTable);
    structNode->data.structNode->type = structDataType;
    structNode->data.structNode->type->container->primitive = PRIM_CUSTOM;

    StructType *structDef = structNode->data.structNode->type->container->custom.structDef;
    structDef->properties = properties;
    structDef->propertyCount = propertyCount;
    structDef->methods = methods;
    structDef->methodCount = methodCount;
    structDef->ctorParamCount = ctorArgCount;
    structDef->ctorParams = ctorArgs;
    structNode->data.structNode->type = structDataType;

    logMessage(LMI, "INFO", "Parser::TypeParsing", "Created struct data type:");

    logVerboseDataType(structDataType);

    CompleteStructDeclaration(globalTable, structNode, structName);

    // Clear the `this` context after parsing the struct
    clearThisContext(context, typeTable);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end struct declaration.", "parseStructDeclaration", arena, state, typeTable, context);
    return structNode;
}

bool parsePropertyForDefaultFlag(ASTNode *propertyNode)
{
    __STACK_FRAME__
    if (propertyNode->metaData->type == NODE_PROPERTY)
    {
        PropertyNode *property = propertyNode->data.property;
        return property->defaultProperty;
    }
    return false;
}

// <parseStructField>
ASTNode *parseStructField(const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct field...");

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
        parsingError("Expected an identifier.", "parseStructField", arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    if (nextToken == TOKEN_LPAREN && currentToken != TOKEN_KW_CONSTRUCTOR)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
        return parseMethodDeclaration(false, parentName, lexer, context, arena, state, typeTable, globalTable);
    }

    const char *fieldName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Field name: %s", fieldName);

    getNextToken(lexer, arena, state, typeTable);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after field name.", "parseStructField", arena, state, typeTable, context);

    DataType *fieldType = parseType(lexer, context, arena, state, typeTable, globalTable);
    getNextToken(lexer, arena, state, typeTable);

    // This is where we could add support for values in the struct fields
    // For now, this is just going to be a type declaration

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseStructField", arena, state, typeTable, context);

    // Find the parent node in the symbol table
    CryoNodeType parentNodeType = context->thisContext->nodeType;

    ASTNode *propertyNode = createFieldNode(fieldName, fieldType, parentName, parentNodeType, NULL, arena, state, typeTable, lexer);
    if (defaultCount > 0)
    {
        propertyNode->data.property->defaultProperty = true;
    }

    return propertyNode;
}
// </parseStructField>

ASTNode *parseConstructor(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, ConstructorMetaData *metaData, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing constructor...");
    consume(__LINE__, lexer, TOKEN_KW_CONSTRUCTOR, "Expected `constructor` keyword.", "parseConstructor", arena, state, typeTable, context);

    char *consturctorName = (char *)calloc(strlen(metaData->parentName) + strlen("::") + strlen("constructor") + 1, sizeof(char));
    strcat(consturctorName, (char *)metaData->parentName);
    strcat(consturctorName, ".constructor");

    ASTNode **params = parseParameterList(lexer, context, arena, consturctorName, state, typeTable, globalTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    ASTNode *constructorBody = parseBlock(lexer, context, arena, state, typeTable, globalTable);

    ASTNode *constructorNode = createConstructorNode(consturctorName, constructorBody, params, paramCount, arena, state, typeTable, lexer);

    return constructorNode;
}

ASTNode *parseMethodDeclaration(bool isStatic, const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseMethodDeclaration", arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    // Add the static identifier to the context
    addStaticIdentifierToContext(context, isStatic);

    char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Method name: %s", methodName);

    const char *methodID = Generate64BitHashID(methodName);
    setCurrentMethod(context, methodName, parentName); // ParsingContext

    getNextToken(lexer, arena, state, typeTable);

    ASTNode **params = parseParameterList(lexer, context, arena, methodName, state, typeTable, globalTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    // Get the return type `-> <type>`
    consume(__LINE__, lexer, TOKEN_RESULT_ARROW, "Expected `->` for return type.", "parseMethodDeclaration", arena, state, typeTable, context);
    DataType *returnType = parseType(lexer, context, arena, state, typeTable, globalTable);
    returnType->container->custom.name = strdup(methodName);
    getNextToken(lexer, arena, state, typeTable);

    // Create the method body
    ASTNode *methodBody = parseBlock(lexer, context, arena, state, typeTable, globalTable);
    // Create the method node
    ASTNode *methodNode = createMethodNode(returnType, methodBody, methodName, params, paramCount, parentName, isStatic,
                                           arena, state, typeTable, lexer);

    DataType **paramTypes = (DataType **)ARENA_ALLOC(arena, paramCount * sizeof(DataType *));
    for (int i = 0; i < paramCount; i++)
    {
        paramTypes[i] = params[i]->data.param->type;
    }

    // Create the method type
    DataType *methodType = createMethodType(strdup(methodName), returnType, paramTypes, paramCount, arena, state, typeTable);
    methodNode->data.method->type = methodType;
    methodNode->data.method->paramTypes = paramTypes;

    // Remove the static identifier from the context
    addStaticIdentifierToContext(context, false);

    AddMethodToStruct(globalTable, parentName, methodNode); // GlobalSymbolTable

    resetCurrentMethod(context); // ParsingContext

    return methodNode;
}

ASTNode *parseMethodCall(ASTNode *accessorObj, char *methodName, DataType *instanceType,
                         Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing method call...");

    ASTNode *argList = parseArgumentList(lexer, context, arena, state, typeTable, globalTable);
    if (!argList)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse argument list.");
        parsingError("Failed to parse argument list.", "parseMethodCall", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }
    int paramCount = argList->data.argList->argCount;
    ASTNode **params = argList->data.argList->args;

    logMessage(LMI, "INFO", "Parser", "Method name: %s", methodName);
    logMessage(LMI, "INFO", "Parser", "Param count: %d", paramCount);

    TypeofDataType typeOfSymbol = UNKNOWN_TYPE;
    char *instanceTypeName = (char *)malloc(sizeof(char) * 128);
    if (!instanceTypeName)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to allocate memory for instance type name.");
        return NULL;
    }

    switch (accessorObj->metaData->type)
    {
    case NODE_CLASS:
    {
        const char *className = accessorObj->data.classNode->name;
        logMessage(LMI, "INFO", "Parser", "Class name: %s", strdup(className));
        strcpy(instanceTypeName, className);
        typeOfSymbol = CLASS_TYPE;
        break;
    }
    case NODE_STRUCT_DECLARATION:
    {
        const char *structName = accessorObj->data.structNode->name;
        logMessage(LMI, "INFO", "Parser", "Struct name: %s", strdup(structName));
        strcpy(instanceTypeName, structName);
        break;
    }
    case NODE_VAR_DECLARATION:
    {
        // This is a variable that is of a class or struct type (e.g., const obj: MyClass = new MyClass())
        // and accessing member methods. We need to get the DataType * from the ASTNode
        DataType *varType = accessorObj->data.varDecl->type;
        if (varType->container->baseType == CLASS_TYPE)
        {
            logMessage(LMI, "INFO", "Parser", "Found class type.");
            logDataType(varType);
            const char *className = varType->container->custom.name;
            logMessage(LMI, "INFO", "Parser", "Class name: %s", strdup(className));
            strcpy(instanceTypeName, className);
        }
        else if (varType->container->baseType == STRUCT_TYPE)
        {
            logMessage(LMI, "INFO", "Parser", "Found struct type.");
            logDataType(varType);
            const char *structName = varType->container->custom.name;
            const char *__structName = getDataTypeName(varType);
            logMessage(LMI, "INFO", "Parser", "Struct name: %s", __structName);
            strcpy(instanceTypeName, __structName);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Invalid instance type, received: %s", DataTypeToString(varType));
            parsingError("Invalid instance type.", "parseMethodCall", arena, state, lexer, lexer->source, typeTable, globalTable);
            CONDITION_FAILED;
        }
        break;
    }
    default:
    {
        logMessage(LMI, "ERROR", "Parser", "Invalid instance type, received: %s", CryoNodeTypeToString(accessorObj->metaData->type));
        parsingError("Invalid instance type.", "parseMethodCall", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }
    }

    Symbol *sym = FindMethodSymbol(globalTable, methodName, strdup(instanceTypeName), typeOfSymbol);
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find instance symbol.");
        parsingError("Failed to find instance symbol.", "parseMethodCall", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Parser", "Found instance symbol.");
    ASTNode *symbolNode = GetASTNodeFromSymbol(globalTable, sym);
    DataType *returnType = GetDataTypeFromSymbol(globalTable, sym);
    VALIDATE_TYPE(returnType);

    bool isStatic = symbolNode->data.method->isStatic;
    ASTNode *methodCall = createMethodCallNode(accessorObj, returnType, instanceType, methodName,
                                               params, paramCount, isStatic,
                                               arena, state, typeTable, lexer);
    if (!methodCall)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create method call node.");
        parsingError("Failed to create method call node.", "parseMethodCall", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    logASTNode(methodCall);

    return methodCall;
}

// Enhanced parseGenericDecl implementation
ASTNode *parseGenericDecl(const char *typeName, Lexer *lexer,
                          ParsingContext *context, Arena *arena, CompilerState *state,
                          TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    __DUMP_STACK_TRACE__
    logMessage(LMI, "INFO", "Parser", "Parsing generic declaration...");

    // Create a list to store generic parameters
    int genericParamCapacity = 8;
    GenericType **genericParams = (GenericType **)malloc(genericParamCapacity * sizeof(GenericType *));
    int genericParamCount = 0;

    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` to start generic declaration.",
            "parseGenericDecl", arena, state, typeTable, context);

    logMessage(LMI, "INFO", "Parser", "Parsing generic declaration...");

    // Parse generic parameters
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            parsingError("Expected generic type identifier.",
                         "parseGenericDecl", arena, state, lexer,
                         lexer->source, typeTable, globalTable);
            return NULL;
        }

        logMessage(LMI, "INFO", "Parser", "Parsing generic parameter...");

        // Get generic parameter name
        char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        getNextToken(lexer, arena, state, typeTable);

        logMessage(LMI, "INFO", "Parser", "Generic parameter name: %s", paramName);

        // Create generic parameter
        GenericType *genericParam = createGenericParameter(paramName);

        logMessage(LMI, "INFO", "Parser", "Generic parameter created.");

        // Check for constraints (e.g., T extends Number)
        if (lexer->currentToken.type == TOKEN_KW_EXTENDS)
        {
            getNextToken(lexer, arena, state, typeTable);

            // Parse constraint type
            DataType *constraint = parseType(lexer, context, arena, state, typeTable, globalTable);
            addGenericConstraint(genericParam, constraint);
            getNextToken(lexer, arena, state, typeTable);
            logMessage(LMI, "INFO", "Parser", "Generic parameter constraint added.");
        }

        // Add to generic parameters list
        genericParams[genericParamCount++] = genericParam;

        logMessage(LMI, "INFO", "Parser", "Generic parameter added to list.");

        // Handle comma-separated list
        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing next generic parameter...");
            getNextToken(lexer, arena, state, typeTable);
            continue;
        }

        logMessage(LMI, "INFO", "Parser", "Checking for end of generic declaration...");

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in generic parameter list.",
                         "parseGenericDecl", arena, state, lexer,
                         lexer->source, typeTable, globalTable);
            return NULL;
        }
    }
    logMessage(LMI, "INFO", "Parser", "End of generic declaration.");
    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` to end generic declaration.",
            "parseGenericDecl", arena, state, typeTable, context);

    logMessage(LMI, "INFO", "Parser", "Creating generic type...");

    // Create generic type container
    TypeContainer *container = createGenericTypeContainer();
    logMessage(LMI, "INFO", "Parser", "Empty Generic type container created.");
    container->baseType = GENERIC_TYPE;
    logMessage(LMI, "INFO", "Parser", "Generic type base type set.");
    container->custom.name = strdup(typeName);
    logMessage(LMI, "INFO", "Parser", "Generic type name set.");
    container->custom.generic.declaration->genericDef = NULL;
    logMessage(LMI, "INFO", "Parser", "Generic type declaration set.");
    container->custom.generic.declaration->paramCount = genericParamCount;
    logMessage(LMI, "INFO", "Parser", "Generic type parameter count set.");

    logMessage(LMI, "INFO", "Parser", "Generic type container created.");

    // Convert GenericType to DataType for each parameter
    for (int i = 0; i < genericParamCount; i++)
    {
        logMessage(LMI, "INFO", "Parser", "Converting generic parameter to DataType...");
        DataType *paramType = (DataType *)malloc(sizeof(DataType));
        paramType->container = createTypeContainer();
        paramType->container->baseType = GENERIC_TYPE;
        paramType->container->custom.name = strdup(genericParams[i]->name);
        // Transfer constraints if any
        if (genericParams[i]->constraint)
        {
            logMessage(LMI, "INFO", "Parser", "Transferring generic parameter constraint...");
            paramType->genericParam = genericParams[i]->constraint;
        }
        logMessage(LMI, "INFO", "Parser", "Generic parameter converted to DataType.");
        addGenericTypeParam(container, paramType);
        logMessage(LMI, "INFO", "Parser", "Generic parameter added to container.");
    }

    logMessage(LMI, "INFO", "Parser", "Creating generic type...");
    DataType *genericType = wrapTypeContainer(container);
    logMessage(LMI, "INFO", "Parser", "Generic type created.");
    ASTNode *genericDeclNode = createGenericDeclNode(genericType, typeName, genericParams, genericParamCount, NULL, false,
                                                     arena, state, typeTable, lexer);

    logMessage(LMI, "INFO", "Parser", "Generic declaration node created.");

    return genericDeclNode;
}

// Helper function to parse generic type instantiation
ASTNode *parseGenericInstantiation(const char *baseName, Lexer *lexer,
                                   ParsingContext *context,
                                   Arena *arena, CompilerState *state,
                                   TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` in generic instantiation.",
            "parseGenericInstantiation", arena, state, typeTable, context);

    // Find base generic type
    DataType *baseType = lookupType(typeTable, baseName);
    if (!baseType || !isGenericType(baseType))
    {
        parsingError("Type is not generic.", "parseGenericInstantiation",
                     arena, state, lexer, lexer->source, typeTable, globalTable);
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
                         arena, state, lexer, lexer->source, typeTable, globalTable);
            return NULL;
        }

        DataType *concreteType = parseType(lexer, context, arena, state, typeTable, globalTable);

        concreteTypes[paramCount++] = concreteType;

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state, typeTable);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in type argument list.",
                         "parseGenericInstantiation", arena, state,
                         lexer, lexer->source, typeTable, globalTable);
            return NULL;
        }
    }

    if (paramCount != expectedParamCount)
    {
        parsingError("Wrong number of type arguments.", "parseGenericInstantiation",
                     arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` after type arguments.",
            "parseGenericInstantiation", arena, state, typeTable, context);

    // Create instantiated type
    TypeContainer *instantiatedContainer = createGenericStructInstance(baseType->container,
                                                                       concreteTypes[0]);

    ASTNode *genericInstNode = createGenericInstNode(baseName, concreteTypes, paramCount,
                                                     wrapTypeContainer(instantiatedContainer), arena, state, typeTable, lexer);

    return genericInstNode;
}

ASTNode *parseStructInstance(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct instance...");

    Token currentToken = lexer->currentToken;

    // Find struct type in type table
    DataType *structType = lookupType(typeTable, structName);
    if (!structType || !isStructType(structType))
    {
        parsingError("Type is not a struct.", "parseStructInstance",
                     arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected struct name in struct instance.",
            "parseStructInstance", arena, state, typeTable, context);

    // Parse struct arguments
    ASTNode *args = parseArgumentList(lexer, context, arena, state, typeTable, globalTable);
    if (!args)
    {
        parsingError("Failed to parse struct arguments.", "parseStructInstance",
                     arena, state, lexer, lexer->source, typeTable, globalTable);
        return NULL;
    }

    DEBUG_BREAKPOINT;
}

ConstructorMetaData *createConstructorMetaData(const char *parentName, CryoNodeType parentNodeType, bool hasDefaultFlag)
{
    __STACK_FRAME__
    ConstructorMetaData *metaData = (ConstructorMetaData *)malloc(sizeof(ConstructorMetaData));
    metaData->parentName = strdup(parentName);
    metaData->parentNodeType = parentNodeType;
    metaData->hasDefaultFlag = hasDefaultFlag;
    return metaData;
}

ASTNode *parseGenericStructDeclaration(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing generic struct declaration...");

    ASTNode *genericDecl = parseGenericDecl(structName, lexer, context, arena, state, typeTable, globalTable);
    if (!genericDecl)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse generic declaration.");
        parsingError("Failed to parse generic declaration.", "parseGenericStructDeclaration", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start generic struct declaration.",
            "parseGenericStructDeclaration", arena, state, typeTable, context);

    ASTNode **properties = (ASTNode **)ARENA_ALLOC(arena, PROPERTY_CAPACITY * sizeof(ASTNode *));
    ASTNode **methods = (ASTNode **)ARENA_ALLOC(arena, METHOD_CAPACITY * sizeof(ASTNode *));
    int propertyCount = 0;
    int methodCount = 0;
    bool hasDefaultProperty = false;
    bool hasConstructor = false;
    int ctorArgCount = 0;
    DataType **ctorArgs = (DataType **)ARENA_ALLOC(arena, sizeof(DataType *) * ARG_CAPACITY);

    int defaultPropertyCount = 0;
    ASTNode *constructorNode = NULL;

    // Add the struct name to the type table
    DataType *structDefinition = createStructDefinition(structName);

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        int count = 0;
        ASTNode *field = parseStructField(structName, lexer, context, arena, state, typeTable, globalTable);
        if (field)
        {
            CryoNodeType fieldType = field->metaData->type;
            if (fieldType == NODE_PROPERTY)
            {
                properties[propertyCount] = field;
                propertyCount++;
                addPropertyToThisContext(context, field, typeTable);
                AddPropertyToStruct(globalTable, structName, field); // GlobalSymbolTable

                if (parsePropertyForDefaultFlag(field) && !hasDefaultProperty)
                {
                    hasDefaultProperty = true;
                    defaultPropertyCount++;
                }
                if (defaultPropertyCount > 1)
                {
                    logMessage(LMI, "ERROR", "Parser", "Struct can only have one default property.");
                    return NULL;
                }

                count++;
            }
            else if (fieldType == NODE_METHOD)
            {
                methods[methodCount] = field;
                methodCount++;
                addMethodToThisContext(context, field, typeTable);
                count++;

                // Go to the next field
                continue;
            }
            else
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse struct field, received Node Type %s", CryoNodeTypeToString(fieldType));
                parsingError("Failed to parse struct field.", "parseStructDeclaration", arena, state, lexer, lexer->source, typeTable, globalTable);
            }
            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                // parse the next field
                continue;
            }
        }

        if (lexer->currentToken.type == TOKEN_KW_CONSTRUCTOR)
        {
            hasConstructor = true;
            ConstructorMetaData *constructorMetaData = createConstructorMetaData(structName, NODE_STRUCT_DECLARATION, hasDefaultProperty);
            constructorNode = parseConstructor(lexer, context, arena, state, constructorMetaData, typeTable, globalTable);
            if (constructorNode)
            {
                // Get the constructor arguments and add them to the struct data type
                int argCount = constructorNode->data.structConstructor->argCount;
                for (int i = 0; i < argCount; i++)
                {
                    logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                    ASTNode *arg = constructorNode->data.structConstructor->args[i];
                    if (arg)
                    {
                        logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                        ctorArgs[ctorArgCount] = getDataTypeFromASTNode(arg);
                        ctorArgCount++;
                    }
                }
            }
        }

        // This is for the method declarations
        if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
            lexer->nextToken.type == TOKEN_LPAREN &&
            lexer->currentToken.type != TOKEN_KW_CONSTRUCTOR)
        {
            ASTNode *method = parseMethodDeclaration(false, structName, lexer, context, arena, state, typeTable, globalTable);
            if (method)
            {
                methods[methodCount] = method;
                methodCount++;
                addMethodToThisContext(context, method, typeTable);
            }
        }

        // If we reach the end of the struct declaration with `}`
        else if (lexer->currentToken.type == TOKEN_RBRACE)
        {
            break;
        }
        // If we fail to parse a struct field
        else
        {
            char *currentTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
            printf("Current Token: %s, Token Str: %s\n", CryoTokenToString(lexer->currentToken.type), currentTokenStr);
            logMessage(LMI, "ERROR", "Parser::TypeParsing", "Failed to parse struct field.");
            CONDITION_FAILED;
        }
    }

    logMessage(LMI, "INFO", "Parser", "Property Count: %d | Method Count: %d", propertyCount, methodCount);
    ASTNode *structNode = createStructNode(structName, properties, propertyCount, constructorNode,
                                           methods, methodCount,
                                           arena, state, typeTable, lexer);
    structNode->data.structNode->hasDefaultValue = hasDefaultProperty;
    structNode->data.structNode->hasConstructor = hasConstructor;

    DataType *structDataType = createDataTypeFromStructNode(structNode, properties, propertyCount,
                                                            methods, methodCount,
                                                            state, typeTable);
    structNode->data.structNode->type = structDataType;
    structNode->data.structNode->type->container->primitive = PRIM_CUSTOM;

    StructType *structDef = structNode->data.structNode->type->container->custom.structDef;
    structDef->properties = properties;
    structDef->propertyCount = propertyCount;
    structDef->methods = methods;
    structDef->methodCount = methodCount;
    structDef->ctorParamCount = ctorArgCount;
    structDef->ctorParams = ctorArgs;
    structNode->data.structNode->type = structDataType;

    logMessage(LMI, "INFO", "Parser::TypeParsing", "Created struct data type:");

    logVerboseDataType(structDataType);

    CompleteStructDeclaration(globalTable, structNode, structName);

    // Clear the `this` context after parsing the struct
    clearThisContext(context, typeTable);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end struct declaration.", "parseStructDeclaration", arena, state, typeTable, context);
    return structNode;
}
