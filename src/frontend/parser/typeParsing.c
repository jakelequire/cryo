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
#include "dataTypes/dataTypeManager.h"

/* ====================================================================== */
/* @ASTNode_Parsing - Structures                                          */

// <parseStructDeclaration>

ASTNode *parseStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct declaration...");
    consume(__LINE__, lexer, TOKEN_KW_STRUCT, "Expected `struct` keyword.", "parseStructDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseStructDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    char *_structName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    const char *structName = (const char *)_structName;
    logMessage(LMI, "INFO", "Parser", "Struct name: %s", structName);

    const char *parentNamespaceNameID = getNamespaceScopeID(context);

    // Setting the context to the struct name
    setThisContext(context, structName, NODE_STRUCT_DECLARATION);

    getNextToken(lexer, arena, state);

    // Check if the next token is a `<` character to determine if it is a generic struct declaration
    if (lexer->currentToken.type == TOKEN_LESS)
    {
        return parseGenericStructDeclaration(lexer, context, arena, state, globalTable, structName, parentNamespaceNameID);
    }

    return parseNonGenericStructDeclaration(lexer, context, arena, state, globalTable, structName, parentNamespaceNameID);
}

ASTNode *parseGenericStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, const char *parentNamespaceNameID)
{
    logMessage(LMI, "INFO", "Parser", "Parsing generic struct declaration...");
    InitGenericStructDeclaration(globalTable, structName, parentNamespaceNameID); // GlobalSymbolTable
    printGlobalSymbolTable(globalTable);
    // ASTNode *genericStruct = parseGenericStructDeclaration(structName, lexer, context, arena, state, globalTable, parentNamespaceNameID);
    // if (genericStruct)
    // {
    //     return genericStruct;
    // }
    DEBUG_BREAKPOINT;
    return NULL;
}

ASTNode *parseNonGenericStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, const char *parentNamespaceNameID)
{
    logMessage(LMI, "INFO", "Parser", "Parsing struct declaration...");
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start struct declaration.", "parseStructDeclaration", arena, state, context);

    ASTNode **properties = (ASTNode **)malloc(PROPERTY_CAPACITY * sizeof(ASTNode *));
    ASTNode **methods = (ASTNode **)malloc(METHOD_CAPACITY * sizeof(ASTNode *));

    int defaultPropertyCount = 0;
    ASTNode *constructorNode = NULL;

    // Add the struct name to the type table
    DataType *structDefinition = createStructDefinition(structName);
    InitStructDeclaration(globalTable, structName, parentNamespaceNameID, structDefinition); // GlobalSymbolTable

    int propertyCount = structDefinition->container->custom.structDef->propertyCount;
    int methodCount = structDefinition->container->custom.structDef->methodCount;
    bool hasDefaultProperty = false;
    bool hasConstructor = false;
    int ctorArgCount = structDefinition->container->custom.structDef->ctorParamCount;
    DataType **ctorArgs = (DataType **)malloc(sizeof(DataType *) * ARG_CAPACITY);

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        if (!parseStructFieldOrMethod(structDefinition, lexer, context, arena, state, globalTable, structName, properties, &propertyCount, methods, &methodCount, &hasDefaultProperty, &defaultPropertyCount, &hasConstructor, &constructorNode, ctorArgs, &ctorArgCount))
        {
            return NULL;
        }
    }

    return finalizeStructDeclaration(lexer, context, arena, state, globalTable, structName, properties, propertyCount, methods, methodCount, hasDefaultProperty, hasConstructor, constructorNode, ctorArgs, ctorArgCount);
}

bool parseStructFieldOrMethod(DataType *dataType, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, ASTNode **properties, int *propertyCount, ASTNode **methods, int *methodCount, bool *hasDefaultProperty, int *defaultPropertyCount, bool *hasConstructor, ASTNode **constructorNode, DataType **ctorArgs, int *ctorArgCount)
{
    ASTNode *field = parseStructField(structName, lexer, context, arena, state, globalTable);
    if (field)
    {
        CryoNodeType fieldType = field->metaData->type;
        if (fieldType == NODE_PROPERTY)
        {
            properties[*propertyCount] = field;
            (*propertyCount)++;
            addPropertyToThisContext(context, field);
            dataType->container->custom.structDef->addProperty(dataType->container->custom.structDef, field);

            if (parsePropertyForDefaultFlag(field) && !(*hasDefaultProperty))
            {
                *hasDefaultProperty = true;
                (*defaultPropertyCount)++;
            }
            if (*defaultPropertyCount > 1)
            {
                logMessage(LMI, "ERROR", "Parser", "Struct can only have one default property.");
                return false;
            }
            return true;
        }
        else if (fieldType == NODE_METHOD)
        {
            logMessage(LMI, "INFO", "Parser", "Adding method to struct data type.");
            methods[*methodCount] = field;
            (*methodCount)++;
            addMethodToThisContext(context, field);
            dataType->container->custom.structDef->addMethod(dataType->container->custom.structDef, field);
            return true;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse struct field, received Node Type %s", CryoNodeTypeToString(fieldType));
            parsingError("Failed to parse struct field.", "parseStructDeclaration", arena, state, lexer, lexer->source, globalTable);
            DEBUG_BREAKPOINT;
            return false;
        }
    }

    if (lexer->currentToken.type == TOKEN_KW_CONSTRUCTOR)
    {
        *hasConstructor = true;
        ConstructorMetaData *constructorMetaData = createConstructorMetaData(structName, NODE_STRUCT_DECLARATION, *hasDefaultProperty);
        *constructorNode = parseConstructor(lexer, context, arena, state, constructorMetaData, globalTable);
        if (*constructorNode)
        {
            int argCount = (*constructorNode)->data.structConstructor->argCount;
            for (int i = 0; i < argCount; i++)
            {
                logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                ASTNode *arg = (*constructorNode)->data.structConstructor->args[i];
                if (arg)
                {
                    logMessage(LMI, "INFO", "Parser", "Adding constructor argument to struct data type.");
                    ctorArgs[argCount] = getDataTypeFromASTNode(arg);
                }
            }
            dataType->container->custom.structDef->ctorParamCount = argCount;
            ctorArgCount = &argCount;
        }
        return true;
    }

    if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
        lexer->nextToken.type == TOKEN_LPAREN &&
        lexer->currentToken.type != TOKEN_KW_CONSTRUCTOR)
    {
        ASTNode *method = parseMethodDeclaration(false, structName, lexer, context, arena, state, globalTable);
        if (method)
        {
            methods[*methodCount] = method;
            (*methodCount)++;
            addMethodToThisContext(context, method);
            dataType->container->custom.structDef->addMethod(dataType->container->custom.structDef, method);
        }
        return true;
    }

    if (lexer->currentToken.type == TOKEN_RBRACE)
    {
        return true;
    }

    char *currentTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
    printf("Current Token: %s, Token Str: %s\n", CryoTokenToString(lexer->currentToken.type), currentTokenStr);
    logMessage(LMI, "ERROR", "Parser::TypeParsing", "Failed to parse struct field.");
    CONDITION_FAILED;
    return false;
}

ASTNode *finalizeStructDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable, const char *structName, ASTNode **properties, int propertyCount, ASTNode **methods, int methodCount, bool hasDefaultProperty, bool hasConstructor, ASTNode *constructorNode, DataType **ctorArgs, int ctorArgCount)
{
    logMessage(LMI, "INFO", "Parser", "Property Count: %d | Method Count: %d", propertyCount, methodCount);
    ASTNode *structNode = createStructNode(structName, properties, propertyCount, constructorNode,
                                           methods, methodCount,
                                           arena, state, lexer);
    structNode->data.structNode->hasDefaultValue = hasDefaultProperty;
    structNode->data.structNode->hasConstructor = hasConstructor;

    DataType *structDataType = createDataTypeFromStructNode(structNode, properties, propertyCount,
                                                            methods, methodCount,
                                                            state);
    structNode->data.structNode->type = structDataType;
    structNode->data.structNode->type->container->primitive = PRIM_OBJECT;

    StructType *structDef = structNode->data.structNode->type->container->custom.structDef;
    structDef->ctorParamCount = ctorArgCount;
    structDef->ctorParams = ctorArgs;
    structNode->data.structNode->type = structDataType;

    logMessage(LMI, "INFO", "Parser::TypeParsing", "Created struct data type:");

    logVerboseDataType(structDataType);

    CompleteStructDeclaration(globalTable, structNode, structName);

    // Clear the `this` context after parsing the struct
    clearThisContext(context);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end struct declaration.", "parseStructDeclaration", arena, state, context);
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
ASTNode *parseStructField(const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct field...");

    // Get the DataType from the `parentName`
    DataType *parentDataType = ResolveDataType(globalTable, parentName);
    if (!parentDataType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to resolve parent data type.");
        return NULL;
    }

    bool isGenericType = parentDataType->container->isGeneric;
    if (isGenericType)
    {
        logMessage(LMI, "INFO", "Parser", "Parent data type is generic.");
        DEBUG_BREAKPOINT;
    }

    int defaultCount = 0; // This should never be more than 1
    CryoTokenType currentToken = lexer->currentToken.type;
    CryoTokenType nextToken = peekNextUnconsumedToken(lexer, arena, state).type;
    if (currentToken == TOKEN_KW_DEFAULT)
    {
        defaultCount++;
        // Consume the `default` keyword
        getNextToken(lexer, arena, state);
    }

    if (lexer->currentToken.type == TOKEN_KW_CONSTRUCTOR)
    {
        // Break from the function and return null. The callee will handle the constructor
        return NULL;
    }

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse struct field, expected an identifier, received %s", CryoTokenToString(lexer->currentToken.type));
        parsingError("Expected an identifier.", "parseStructField", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    if (nextToken == TOKEN_LPAREN && currentToken != TOKEN_KW_CONSTRUCTOR)
    {
        logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
        return parseMethodDeclaration(false, parentName, lexer, context, arena, state, globalTable);
    }

    const char *fieldName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Field name: %s", fieldName);

    getNextToken(lexer, arena, state);

    consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after field name.", "parseStructField", arena, state, context);

    DataType *fieldType = parseType(lexer, context, arena, state, globalTable);
    getNextToken(lexer, arena, state);

    // This is where we could add support for values in the struct fields
    // For now, this is just going to be a type declaration

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseStructField", arena, state, context);

    // Find the parent node in the symbol table
    CryoNodeType parentNodeType = context->thisContext->nodeType;

    ASTNode *propertyNode = createFieldNode(fieldName, fieldType, parentName, parentNodeType, NULL, arena, state, lexer);
    if (defaultCount > 0)
    {
        propertyNode->data.property->defaultProperty = true;
    }

    return propertyNode;
}
// </parseStructField>

ASTNode *parseConstructor(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, ConstructorMetaData *metaData, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing constructor...");
    consume(__LINE__, lexer, TOKEN_KW_CONSTRUCTOR, "Expected `constructor` keyword.", "parseConstructor", arena, state, context);

    char *consturctorName = (char *)calloc(strlen(metaData->parentName) + strlen("::") + strlen("constructor") + 1, sizeof(char));
    strcat(consturctorName, (char *)metaData->parentName);
    strcat(consturctorName, ".constructor");

    ASTNode **params = parseParameterList(lexer, context, arena, consturctorName, state, globalTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    ASTNode *constructorBody = parseBlock(lexer, context, arena, state, globalTable);
    if (!constructorBody)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse constructor body.");
        parsingError("Failed to parse constructor body.", "parseConstructor", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    ASTNode *constructorNode = createConstructorNode(consturctorName, constructorBody, params, paramCount, arena, state, lexer);
    if (!constructorNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create constructor node.");
        parsingError("Failed to create constructor node.", "parseConstructor", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    return constructorNode;
}

ASTNode *parseMethodDeclaration(bool isStatic, const char *parentName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseMethodDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    // Add the static identifier to the context
    addStaticIdentifierToContext(context, isStatic);

    char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Method name: %s", methodName);

    const char *methodID = Generate64BitHashID(methodName);
    setCurrentMethod(context, methodName, parentName); // ParsingContext

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, context, arena, methodName, state, globalTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    // Get the return type `-> <type>`
    consume(__LINE__, lexer, TOKEN_RESULT_ARROW, "Expected `->` for return type.", "parseMethodDeclaration", arena, state, context);
    DataType *returnType = parseType(lexer, context, arena, state, globalTable);
    returnType->container->custom.name = strdup(methodName);
    getNextToken(lexer, arena, state);

    // Create the method body
    ASTNode *methodBody = parseBlock(lexer, context, arena, state, globalTable);
    // Create the method node
    ASTNode *methodNode = createMethodNode(returnType, methodBody, methodName, params, paramCount, parentName, isStatic,
                                           arena, state, lexer);

    DataType **paramTypes = (DataType **)ARENA_ALLOC(arena, paramCount * sizeof(DataType *));
    for (int i = 0; i < paramCount; i++)
    {
        paramTypes[i] = params[i]->data.param->type;
    }

    // Create the method type
    DataType *methodType = createMethodType(strdup(methodName), returnType, paramTypes, paramCount, arena, state);
    methodNode->data.method->type = methodType;
    methodNode->data.method->paramTypes = paramTypes;

    // Remove the static identifier from the context
    addStaticIdentifierToContext(context, false);

    AddMethodToStruct(globalTable, parentName, methodNode); // GlobalSymbolTable

    resetCurrentMethod(context); // ParsingContext

    return methodNode;
}

ASTNode *parseMethodCall(ASTNode *accessorObj, char *methodName, DataType *instanceType,
                         Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing method call...");

    ASTNode *argList = parseArgumentList(lexer, context, arena, state, globalTable);
    if (!argList)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse argument list.");
        parsingError("Failed to parse argument list.", "parseMethodCall", arena, state, lexer, lexer->source, globalTable);
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
            parsingError("Invalid instance type.", "parseMethodCall", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
        break;
    }
    default:
    {
        logMessage(LMI, "ERROR", "Parser", "Invalid instance type, received: %s", CryoNodeTypeToString(accessorObj->metaData->type));
        parsingError("Invalid instance type.", "parseMethodCall", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }
    }

    Symbol *sym = FindMethodSymbol(globalTable, methodName, strdup(instanceTypeName), typeOfSymbol);
    if (!sym)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find instance symbol.");
        parsingError("Failed to find instance symbol.", "parseMethodCall", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Parser", "Found instance symbol.");
    ASTNode *symbolNode = GetASTNodeFromSymbol(globalTable, sym);
    DataType *returnType = GetDataTypeFromSymbol(globalTable, sym);
    VALIDATE_TYPE(returnType);

    bool isStatic = symbolNode->data.method->isStatic;
    ASTNode *methodCall = createMethodCallNode(accessorObj, returnType, instanceType, methodName,
                                               params, paramCount, isStatic,
                                               arena, state, lexer);
    if (!methodCall)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create method call node.");
        parsingError("Failed to create method call node.", "parseMethodCall", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    logASTNode(methodCall);

    return methodCall;
}

// Enhanced parseGenericDecl implementation
ASTNode *parseGenericDecl(const char *typeName, Lexer *lexer,
                          ParsingContext *context, Arena *arena, CompilerState *state,
                          CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    __DUMP_STACK_TRACE__
    logMessage(LMI, "INFO", "Parser", "Parsing generic declaration...");

    // Create a list to store generic parameters
    int genericParamCapacity = 8;
    GenericType **genericParams = (GenericType **)malloc(genericParamCapacity * sizeof(GenericType *));
    int genericParamCount = 0;

    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` to start generic declaration.",
            "parseGenericDecl", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Parsing generic declaration...");

    // Parse generic parameters
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            parsingError("Expected generic type identifier.",
                         "parseGenericDecl", arena, state, lexer,
                         lexer->source, globalTable);
            return NULL;
        }

        logMessage(LMI, "INFO", "Parser", "Parsing generic parameter...");

        // Get generic parameter name
        char *paramName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        getNextToken(lexer, arena, state);

        logMessage(LMI, "INFO", "Parser", "Generic parameter name: %s", paramName);

        // Create generic parameter
        GenericType *genericParam = createGenericParameter(paramName);

        logMessage(LMI, "INFO", "Parser", "Generic parameter created.");

        // Check for constraints (e.g., T extends Number)
        if (lexer->currentToken.type == TOKEN_KW_EXTENDS)
        {
            getNextToken(lexer, arena, state);

            // Parse constraint type
            DataType *constraint = parseType(lexer, context, arena, state, globalTable);
            addGenericConstraint(genericParam, constraint);
            getNextToken(lexer, arena, state);
            logMessage(LMI, "INFO", "Parser", "Generic parameter constraint added.");
        }

        // Add to generic parameters list
        genericParams[genericParamCount++] = genericParam;

        logMessage(LMI, "INFO", "Parser", "Generic parameter added to list.");

        // Handle comma-separated list
        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            logMessage(LMI, "INFO", "Parser", "Parsing next generic parameter...");
            getNextToken(lexer, arena, state);
            continue;
        }

        logMessage(LMI, "INFO", "Parser", "Checking for end of generic declaration...");

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in generic parameter list.",
                         "parseGenericDecl", arena, state, lexer,
                         lexer->source, globalTable);
            return NULL;
        }
    }
    logMessage(LMI, "INFO", "Parser", "End of generic declaration.");
    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` to end generic declaration.",
            "parseGenericDecl", arena, state, context);

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
                                                     arena, state, lexer);

    logMessage(LMI, "INFO", "Parser", "Generic declaration node created.");

    return genericDeclNode;
}

// Helper function to parse generic type instantiation
ASTNode *parseGenericInstantiation(const char *baseName, Lexer *lexer,
                                   ParsingContext *context,
                                   Arena *arena, CompilerState *state,
                                   CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    consume(__LINE__, lexer, TOKEN_LESS, "Expected `<` in generic instantiation.",
            "parseGenericInstantiation", arena, state, context);

    // Find base generic type
    DataType *baseType = NULL;
    DEBUG_BREAKPOINT;

    int expectedParamCount = baseType->container->custom.generic.instantiation->argCount;
    DataType **concreteTypes = (DataType **)malloc(expectedParamCount * sizeof(DataType *));
    int paramCount = 0;

    // Parse concrete type arguments
    while (lexer->currentToken.type != TOKEN_GREATER)
    {
        if (paramCount >= expectedParamCount)
        {
            parsingError("Too many type arguments.", "parseGenericInstantiation",
                         arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        DataType *concreteType = parseType(lexer, context, arena, state, globalTable);

        concreteTypes[paramCount++] = concreteType;

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_GREATER)
        {
            parsingError("Expected ',' or '>' in type argument list.",
                         "parseGenericInstantiation", arena, state,
                         lexer, lexer->source, globalTable);
            return NULL;
        }
    }

    if (paramCount != expectedParamCount)
    {
        parsingError("Wrong number of type arguments.", "parseGenericInstantiation",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_GREATER, "Expected `>` after type arguments.",
            "parseGenericInstantiation", arena, state, context);

    // Create instantiated type
    TypeContainer *instantiatedContainer = createGenericStructInstance(baseType->container,
                                                                       concreteTypes[0]);

    ASTNode *genericInstNode = createGenericInstNode(baseName, concreteTypes, paramCount,
                                                     wrapTypeContainer(instantiatedContainer), arena, state, lexer);

    return genericInstNode;
}

ASTNode *parseStructInstance(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct instance...");

    Token currentToken = lexer->currentToken;

    // Find struct type in type table
    DataType *structType = ResolveDataType(globalTable, structName);
    if (!structType)
    {
        parsingError("Failed to resolve struct type.", "parseStructInstance",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected struct name in struct instance.",
            "parseStructInstance", arena, state, context);

    // Parse struct arguments
    ASTNode *args = parseArgumentList(lexer, context, arena, state, globalTable);
    if (!args)
    {
        parsingError("Failed to parse struct arguments.", "parseStructInstance",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    DEBUG_BREAKPOINT;
}

ASTNode *parseNewStructObject(const char *structName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing struct instance...");

    Token currentToken = lexer->currentToken;

    // Find struct type in type table
    DataType *structType = ResolveDataType(globalTable, structName);
    if (!structType)
    {
        parsingError("Failed to resolve struct type.", "parseStructInstance",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    bool isNew = false;
    if (currentToken.type == TOKEN_KW_NEW)
    {
        isNew = true;
        getNextToken(lexer, arena, state);
    }

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected struct name in struct instance.",
            "parseStructInstance", arena, state, context);

    // Parse struct arguments
    ASTNode *args = parseArgumentList(lexer, context, arena, state, globalTable);
    if (!args)
    {
        parsingError("Failed to parse struct arguments.", "parseStructInstance",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    ASTNode **_args = args->data.argList->args;
    int argCount = args->data.argList->argCount;

    ASTNode *structObject = createObject(structName, structType, isNew, _args, argCount, arena, state, lexer);
    if (!structObject)
    {
        parsingError("Failed to create struct object.", "parseStructInstance",
                     arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    return structObject;
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

ASTNode *parseTypeDeclaration(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing type declaration...");
    consume(__LINE__, lexer, TOKEN_KW_TYPE, "Expected `type` keyword.", "parseTypeDeclaration", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    // The name of the type definition
    char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Type name: %s", typeName);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected type name.", "parseTypeDeclaration", arena, state, context);

    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` after type name.", "parseTypeDeclaration", arena, state, context);

    DataType *typeDefinition = parseTypeDefinition(lexer, context, arena, state, globalTable);
    if (!typeDefinition)
    {
        parsingError("Failed to parse type definition.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    // Create the type declaration node
    ASTNode *typeDeclNode = createTypeDeclNode(typeName, typeDefinition, arena, state, lexer);
    if (!typeDeclNode)
    {
        parsingError("Failed to create type declaration node.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after type declaration.", "parseTypeDeclaration", arena, state, context);

    return typeDeclNode;
}

// This function should be responsible for parsing a type definition
//
// type {TYPE_NAME} <?TypeParams...> = {TYPE_DEFINITION}
// ------------------------------------------- ^ This is the part that should be parsed
//
// Type Definition Syntax:
// - Primitive Type:    int, i8-i128, float, boolean, string
// - Struct Type:       struct {STRUCT_NAME}
// - Class Type:        class {CLASS_NAME}
// - Function Type:     (?args...) -> returnType
//
DataType *parseTypeDefinition(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing type definition...");

    // Check if the current token is a primitive type
    DataType *primitiveType = parseForPrimitive(lexer, context, arena, state, globalTable);
    if (primitiveType)
    {
        return primitiveType;
    }

    // Check if the definition is a function type
    if (lexer->currentToken.type == TOKEN_LPAREN)
    {
        return parseFunctionType(lexer, context, arena, state, globalTable);
    }

    DEBUG_BREAKPOINT;
}

// This function looks at the current token and determines if it is a primitive type.
// If not, then it will return NULL.
DataType *parseForPrimitive(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "@parseForPrimitive Parsing for primitive type...");

    const char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "@parseForPrimitive Type name: %s", typeName);

    if (cStringCompare("int", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_INT, "Expected `int` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createInt();
    }
    else if (cStringCompare("i8", typeName))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I8, "Expected `i8` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI8();
    }
    else if (cStringCompare("i16", typeName))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I16, "Expected `i16` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI16();
    }
    else if (cStringCompare("i32", typeName))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I32, "Expected `i32` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI32();
    }
    else if (cStringCompare("i64", typeName))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I64, "Expected `i64` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI64();
    }
    else if (cStringCompare("i128", typeName))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I128, "Expected `i128` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI128();
    }
    else if (cStringCompare("float", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_FLOAT, "Expected `float` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createFloat();
    }
    else if (cStringCompare("boolean", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_BOOLEAN, "Expected `boolean` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createBoolean();
    }
    else if (cStringCompare("string", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_STRING, "Expected `string` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createString();
    }
    else if (cStringCompare("void", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_VOID, "Expected `void` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createVoid();
    }
    else if (cStringCompare("null", typeName))
    {
        consume(__LINE__, lexer, TOKEN_KW_NULL, "Expected `null` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createNull();
    }
    else
    {
        return NULL;
    }

    return NULL;
}

DataType *parseFunctionType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function type...");

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start function type.", "parseFunctionType", arena, state, context);

    DEBUG_BREAKPOINT;
}

DataType *parseStructType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
}

DataType *parseClassType(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
}
