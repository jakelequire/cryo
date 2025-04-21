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

    DTM->symbolTable->addProtoType(
        DTM->symbolTable,
        parentNamespaceNameID,
        structName,
        PRIM_OBJECT,
        OBJECT_TYPE,
        STRUCT_OBJ);

    DataType *protoType = DTM->symbolTable->getProtoType(
        DTM->symbolTable,
        parentNamespaceNameID,
        structName);
    setTypePtrToContext(context, protoType);

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
    DataType *structDefinition = DTM->structTypes->createStructTemplate();

    int propertyCount = structDefinition->container->type.structType->propertyCount;
    int methodCount = structDefinition->container->type.structType->methodCount;
    bool hasDefaultProperty = false;
    bool hasConstructor = false;
    int ctorArgCount = structDefinition->container->type.structType->ctorParamCount;
    DataType **ctorArgs = (DataType **)malloc(sizeof(DataType *) * ARG_CAPACITY);

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        if (!parseStructFieldOrMethod(structDefinition, lexer, context, arena, state, globalTable, structName, properties, &propertyCount, methods, &methodCount, &hasDefaultProperty, &defaultPropertyCount, &hasConstructor, &constructorNode, ctorArgs, &ctorArgCount))
        {
            return NULL;
        }
    }

    return finalizeStructDeclaration(lexer, context, arena, state, globalTable, structName, properties, propertyCount, methods, structDefinition->container->type.structType->methodCount, hasDefaultProperty, hasConstructor, constructorNode, ctorArgs, ctorArgCount);
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
            DataType *fieldType = DTM->astInterface->getTypeofASTNode(field);
            DTPropertyTy *propertyTy = field->data.property->type->container->type.propertyType;
            dataType->container->type.structType->addProperty(dataType->container->type.structType, propertyTy);

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
            DataType *methodType = DTM->astInterface->getTypeofASTNode(field);
            dataType->container->type.structType->addMethod(dataType->container->type.structType, methodType);
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
                    ctorArgs[argCount] = DTM->astInterface->getTypeofASTNode(arg);
                }
            }
            dataType->container->type.structType->ctorParamCount = argCount;
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
            DataType *methodType = DTM->astInterface->getTypeofASTNode(method);
            dataType->container->type.structType->addMethod(dataType->container->type.structType, methodType);
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

    DTPropertyTy **propertyTypes = DTM->astInterface->createPropertyArrayFromAST(properties, propertyCount);
    DataType **methodTypes = DTM->astInterface->createTypeArrayFromASTArray(methods, methodCount);
    DataType *structDataType = DTM->structTypes->createCompleteStructType(structName, propertyTypes, propertyCount, methodTypes, methodCount, hasConstructor, ctorArgs, &ctorArgCount);
    structDataType->container->objectType = STRUCT_OBJ;
    structDataType->container->primitive = PRIM_OBJECT;
    structDataType->container->typeOf = OBJECT_TYPE;
    logMessage(LMI, "INFO", "Parser::TypeParsing", "Created struct data type:");

    const char *scopeName = getNamespaceScopeID(context);
    DTM->symbolTable->addEntry(DTM->symbolTable, scopeName, structName, structDataType);

    structDataType->debug->printType(structDataType);

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
    DTM->symbolTable->printTable(DTM->symbolTable);
    // Get the DataType from the `parentName`
    const char *scopeName = getNamespaceScopeID(context);
    DataType *parentDataType = DTM->symbolTable->getEntry(DTM->symbolTable, scopeName, parentName);
    if (!parentDataType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to resolve parent data type.");
        return NULL;
    }

    bool isGenericType = parentDataType->container->type.structType->generic.isGeneric;
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

    const char *methodID = Generate64BitHashID(strdup(methodName));
    setCurrentMethod(context, strdup(methodName), parentName); // ParsingContext

    getNextToken(lexer, arena, state);

    ASTNode **params = parseParameterList(lexer, context, arena, strdup(methodName), state, globalTable);
    int paramCount = 0;
    while (params[paramCount] != NULL)
    {
        paramCount++;
    }

    // Get the return type `-> <type>`
    consume(__LINE__, lexer, TOKEN_RESULT_ARROW, "Expected `->` for return type.", "parseMethodDeclaration", arena, state, context);
    DataType *returnType = parseType(lexer, context, arena, state, globalTable);
    returnType->typeName = strdup(methodName);
    getNextToken(lexer, arena, state);

    // Check if the next token is `;` to determine if it is a forward declaration
    if (lexer->currentToken.type == TOKEN_SEMICOLON)
    {
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` to end method declaration.", "parseMethodDeclaration", arena, state, context);
        ASTNode *methodNode = createMethodNode(returnType, NULL, strdup(methodName), params, paramCount, parentName, isStatic,
                                               arena, state, lexer);
        DataType **paramTypes = (DataType **)malloc(paramCount * sizeof(DataType *) + 1);
        for (int i = 0; i < paramCount; i++)
        {
            paramTypes[i] = params[i]->data.param->type;
        }
        DataType *methodType = DTM->functionTypes->createMethodType(strdup(methodName), returnType, paramTypes, paramCount);
        methodType->container->primitive = PRIM_FUNCTION;
        methodType->container->typeOf = FUNCTION_TYPE;
        methodNode->data.method->type = methodType;
        methodNode->data.method->paramTypes = paramTypes;

        // Remove the static identifier from the context
        addStaticIdentifierToContext(context, false);

        AddMethodToStruct(globalTable, parentName, methodNode); // GlobalSymbolTable

        resetCurrentMethod(context); // ParsingContext

        return methodNode;
    }

    // Create the method body
    ASTNode *methodBody = parseBlock(lexer, context, arena, state, globalTable);
    // Create the method node
    ASTNode *methodNode = createMethodNode(returnType, methodBody, strdup(methodName), params, paramCount, parentName, isStatic,
                                           arena, state, lexer);

    DataType **paramTypes = (DataType **)malloc(paramCount * sizeof(DataType *));
    for (int i = 0; i < paramCount; i++)
    {
        paramTypes[i] = params[i]->data.param->type;
    }

    // Create the method type
    DataType *methodType = DTM->functionTypes->createMethodType(strdup(methodName), returnType, paramTypes, paramCount);
    methodNode->data.method->type = methodType;
    methodNode->data.method->paramTypes = paramTypes;

    // Remove the static identifier from the context
    addStaticIdentifierToContext(context, false);

    AddMethodToStruct(globalTable, parentName, methodNode); // GlobalSymbolTable

    resetCurrentMethod(context); // ParsingContext

    return methodNode;
}

ASTNode *parseMethodCall(ASTNode *accessorObj, const char *methodName, DataType *instanceType,
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
        typeOfSymbol = OBJECT_TYPE;
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
        if (varType->container->typeOf == OBJECT_TYPE)
        {
            logMessage(LMI, "INFO", "Parser", "Found class type.");
            varType->debug->printType(varType);
            const char *className = varType->typeName;
            logMessage(LMI, "INFO", "Parser", "Class name: %s", strdup(className));
            strcpy(instanceTypeName, className);
        }
        else if (varType->container->typeOf == OBJECT_TYPE)
        {
            logMessage(LMI, "INFO", "Parser", "Found struct type.");
            varType->debug->printType(varType);
            const char *structName = varType->typeName;
            const char *__structName = varType->typeName;
            logMessage(LMI, "INFO", "Parser", "Struct name: %s", __structName);
            strcpy(instanceTypeName, __structName);
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Invalid instance type, received: %s", varType->debug->toString(varType));
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

    return NULL;
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

    int expectedParamCount = baseType->container->type.genericType->paramCount;
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

    return NULL;
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

// | =========================================================================================================================================== | //
// |                                                                                                                                             | //
// |                                                    Type Declaration Parsing Functions                                                       | //
// |                                                                                                                                             | //
// | =========================================================================================================================================== | //

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

    // Check for the `extends` keyword
    if (lexer->currentToken.type == TOKEN_KW_EXTENDS)
    {
        consume(__LINE__, lexer, TOKEN_KW_EXTENDS, "Expected `extends` keyword.", "parseTypeDeclaration", arena, state, context);
        char *parentName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Parent name: %s", parentName);
        consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected parent name.", "parseTypeDeclaration", arena, state, context);
        DataType *parentType = DTM->symbolTable->getEntry(DTM->symbolTable, getNamespaceScopeID(context), parentName);
        if (!parentType)
        {
            parsingError("Failed to resolve parent type.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        // If the statement is followed by a `;`, it is a forward declaration / type alias
        if (lexer->currentToken.type == TOKEN_SEMICOLON)
        {
            consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after type name.", "parseTypeDeclaration", arena, state, context);
            DataType *typeAlias = DTM->dataTypes->createTypeAlias(typeName, parentType);
            if (!typeAlias)
            {
                parsingError("Failed to create type alias.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
                return NULL;
            }

            logMessage(LMI, "INFO", "Parser", "Type alias: %s", typeAlias->typeName);
            // Add the type alias to the symbol table
            DTM->symbolTable->addEntry(DTM->symbolTable, getNamespaceScopeID(context), typeName, typeAlias);
            ASTNode *aliasTypeNode = createTypeDeclNode(typeName, typeAlias, arena, state, lexer);
            if (!aliasTypeNode)
            {
                parsingError("Failed to create type alias node.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
                return NULL;
            }
            return aliasTypeNode;
        }
    }

    consume(__LINE__, lexer, TOKEN_EQUAL, "Expected `=` after type name.", "parseTypeDeclaration", arena, state, context);
    DataType *typeDefinition = parseTypeDefinition(typeName, lexer, context, arena, state, globalTable);
    if (!typeDefinition)
    {
        parsingError("Failed to parse type definition.", "parseTypeDeclaration", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    DTM->symbolTable->addEntry(DTM->symbolTable, getNamespaceScopeID(context), typeName, typeDefinition);

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
DataType *parseTypeDefinition(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing type definition...");

    // Check if the current token is a primitive type
    DataType *primitiveType = parseForPrimitive(typeName, lexer, context, arena, state, globalTable);
    if (primitiveType)
    {
        return primitiveType;
    }

    // Check if the definition is a function type
    if (lexer->currentToken.type == TOKEN_LPAREN)
    {
        return parseFunctionType(typeName, lexer, context, arena, state, globalTable);
    }
    else if (lexer->currentToken.type == TOKEN_LBRACE)
    {
        // This is an object type
        return parseObjectType(typeName, lexer, context, arena, state, globalTable);
    }

    DEBUG_BREAKPOINT;
}

// This function looks at the current token and determines if it is a primitive type.
// If not, then it will return NULL.
DataType *parseForPrimitive(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "@parseForPrimitive Parsing for primitive type...");
    logMessage(LMI, "INFO", "Parser", "@parseForPrimitive Type name: %s", typeName);
    logMessage(LMI, "INFO", "Parser", "@parseForPrimitive Current token: %s", CryoTokenToString(lexer->currentToken.type));

    char *currentTokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);

    if (cStringCompare("int", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_KW_INT, "Expected `int` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createInt();
    }
    else if (cStringCompare("i8", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I8, "Expected `i8` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI8();
    }
    else if (cStringCompare("i16", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I16, "Expected `i16` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI16();
    }
    else if (cStringCompare("i32", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I32, "Expected `i32` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI32();
    }
    else if (cStringCompare("i64", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I64, "Expected `i64` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI64();
    }
    else if (cStringCompare("i128", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_TYPE_I128, "Expected `i128` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createI128();
    }
    else if (cStringCompare("float", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_KW_FLOAT, "Expected `float` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createFloat();
    }
    else if (cStringCompare("boolean", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_KW_BOOLEAN, "Expected `boolean` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createBoolean();
    }
    else if (cStringCompare("string", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_KW_STRING, "Expected `string` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createString();
    }
    else if (cStringCompare("void", currentTokenStr))
    {
        consume(__LINE__, lexer, TOKEN_KW_VOID, "Expected `void` keyword.", "parseForPrimitive", arena, state, context);
        return DTM->primitives->createVoid();
    }
    else if (cStringCompare("null", currentTokenStr))
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

DataType *parseFunctionType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing function type...");

    consume(__LINE__, lexer, TOKEN_LPAREN, "Expected `(` to start function type.", "parseFunctionType", arena, state, context);

    DTMDynamicTypeArray *argArray = DTM->helpers->dynTypeArray;

    // Parse function arguments
    while (lexer->currentToken.type != TOKEN_RPAREN)
    {
        DataType *argType = parseType(lexer, context, arena, state, globalTable);
        if (!argType)
        {
            parsingError("Failed to parse function argument type.", "parseFunctionType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
        logMessage(LMI, "INFO", "Parser", "Function argument completed");
        argArray->add(argArray, argType);

        getNextToken(lexer, arena, state);

        if (lexer->currentToken.type == TOKEN_COMMA)
        {
            getNextToken(lexer, arena, state);
            continue;
        }

        if (lexer->currentToken.type != TOKEN_RPAREN)
        {
            parsingError("Expected ',' or ')' in function argument list.", "parseFunctionType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }
    }
    logMessage(LMI, "INFO", "Parser", "Function arguments parsed.");
    argArray->printArray(argArray);

    consume(__LINE__, lexer, TOKEN_RPAREN, "Expected `)` to end function argument list.", "parseFunctionType", arena, state, context);
    consume(__LINE__, lexer, TOKEN_RESULT_ARROW, "Expected `->` for return type.", "parseFunctionType", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Parsing function return type...");
    DataType *returnType = parseType(lexer, context, arena, state, globalTable);
    if (!returnType)
    {
        parsingError("Failed to parse function return type.", "parseFunctionType", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }
    logMessage(LMI, "INFO", "Parser", "Function return type parsed.");

    char *curTok = strndup(lexer->currentToken.start, lexer->currentToken.length);
    getNextToken(lexer, arena, state);

    logMessage(LMI, "INFO", "Parser", "Creating Function Type...");

    DataType *functionType = DTM->functionTypes->createFunctionType(argArray->data, argArray->count, returnType);
    if (!functionType)
    {
        parsingError("Failed to create function type.", "parseFunctionType", arena, state, lexer, lexer->source, globalTable);
        return NULL;
    }

    functionType->setTypeName(functionType, typeName);

    logMessage(LMI, "INFO", "Parser", "Function type created.");

    return functionType;
}

DataType *parseObjectType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing object type...");

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start object type.", "parseObjectType", arena, state, context);

    DTMDynamicTuple *tuple = DTM->helpers->dynTuple;

    // Parse object properties
    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        DataType *propertyType = parseType(lexer, context, arena, state, globalTable);
        if (!propertyType)
        {
            parsingError("Failed to parse object property type.", "parseObjectType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        getNextToken(lexer, arena, state);

        if (lexer->currentToken.type != TOKEN_IDENTIFIER)
        {
            parsingError("Expected an identifier for object property.", "parseObjectType", arena, state, lexer, lexer->source, globalTable);
            return NULL;
        }

        char *propertyName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Object property name: %s", propertyName);

        getNextToken(lexer, arena, state);

        consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after object property name.", "parseObjectType", arena, state, context);
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected `;` after object property type.", "parseObjectType", arena, state, context);

        tuple->add(tuple, propertyName, propertyType);
    }

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end object type.", "parseObjectType", arena, state, context);

    return NULL;
}

DataType *parseStructType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    DEBUG_BREAKPOINT;
}

DataType *parseClassType(const char *typeName, Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    DEBUG_BREAKPOINT;
}

// | =========================================================================================================================================== | //
// |                                                                                                                                             | //
// |                                                   Class Declaration Parsing                                                                 | //
// |                                                                                                                                             | //
// | =========================================================================================================================================== | //

ASTNode *parseClassDeclaration(bool isStatic,
                               Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing class declaration");
    consume(__LINE__, lexer, TOKEN_KW_CLASS, "Expected `class` keyword.", "parseclassNodearation", arena, state, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier for class name.", "parseclassNodearation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    const char *className = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Class name: %s", className);

    InitClassDeclaration(globalTable, className);
    createClassScope(context, className);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier for class name.", "parseclassNodearation", arena, state, context);

    ASTNode *classNode = createClassDeclarationNode(className, arena, state, lexer);
    if (!classNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create class declaration node.");
        parsingError("Failed to create class declaration node.", "parseclassNodearation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    clearThisContext(context);
    setThisContext(context, className, NODE_CLASS);

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start class body.", "parseclassNodearation", arena, state, context);

    ASTNode *classBody = parseClassBody(classNode, className, isStatic, lexer, context, arena, state, globalTable);
    if (!classBody)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse class body.");
        parsingError("Failed to parse class body.", "parseclassNodearation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Parser", "Class body parsed.");

    ClassNode *classBodyNode = classBody->data.classNode;
    DataType *classDataType = DTM->classTypes->createClassTypeWithMembers(
        className,
        classBodyNode->publicMembers->properties,
        classBodyNode->publicMembers->propertyCount,

        classBodyNode->privateMembers->properties,
        classBodyNode->privateMembers->propertyCount,

        classBodyNode->protectedMembers->properties,
        classBodyNode->protectedMembers->propertyCount,

        classBodyNode->publicMembers->methods,
        classBodyNode->publicMembers->methodCount,

        classBodyNode->privateMembers->methods,
        classBodyNode->privateMembers->methodCount,

        classBodyNode->protectedMembers->methods,
        classBodyNode->protectedMembers->methodCount,

        classBodyNode->hasConstructor,
        classBodyNode->constructors,
        classBodyNode->constructorCount);

    printf("Class data type: %s\n", classDataType->debug->toString(classDataType));

    logMessage(LMI, "INFO", "Parser", "Class methods added to class type.");
    classNode->data.classNode->type = classDataType;

    CompleteClassDeclaration(globalTable, classNode, className); // Global Symbol Table
    classNode->data.classNode->type = classDataType;

    DTM->symbolTable->addEntry(DTM->symbolTable, getNamespaceScopeID(context), className, classDataType);

    // Clear the context
    clearThisContext(context);
    // Clear Scope
    clearScopeContext(context);

    logMessage(LMI, "INFO", "Parser", "Class declaration completed.");

    return classNode;
}

ASTNode *parseClassBody(ASTNode *classNode, const char *className, bool isStatic,
                        Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing class body...");

    int privateMethodCount = 0;
    int publicMethodCount = 0;
    int protectedMethodCount = 0;

    int privatePropertyCount = 0;
    int publicPropertyCount = 0;
    int protectedPropertyCount = 0;

    bool hasConstructor = false;

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        switch (lexer->currentToken.type)
        {
        case TOKEN_KW_CONSTRUCTOR:
        {
            ConstructorMetaData *constructorMetaData = createConstructorMetaData(className, NODE_CLASS, false);
            ASTNode *constructor = parseConstructor(lexer, context, arena, state, constructorMetaData, globalTable);
            if (!constructor)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                CONDITION_FAILED;
            }

            logASTNode(constructor);

            addConstructorToClass(classNode, constructor, arena, state);
            hasConstructor = true;

            break;
        }

        // Public, Private, and Protected identifiers can be for methods or properties
        // Properties or Methods can also prefix with the `static` keyword before or after the visibility keyword
        // Properties: { public, private, protected } {?static} <identifier>: <type> = <?value>; (Optional)
        // Methods: { public, private, protected } {?static} <identifier>(<params>) { <body> }
        case TOKEN_KW_PUBLIC:
        case TOKEN_KW_PRIVATE:
        case TOKEN_KW_PROTECTED:
        {
            CryoVisibilityType visibility;
            if (lexer->currentToken.type == TOKEN_KW_PUBLIC)
                visibility = VISIBILITY_PUBLIC;
            else if (lexer->currentToken.type == TOKEN_KW_PRIVATE)
                visibility = VISIBILITY_PRIVATE;
            else
                visibility = VISIBILITY_PROTECTED;

            bool isStatic = false;
            consume(__LINE__, lexer, lexer->currentToken.type, "Expected visibility keyword.", "parseClassBody", arena, state, context);
            if (lexer->currentToken.type == TOKEN_KW_STATIC)
            {
                logMessage(LMI, "INFO", "Parser", "Parsing static method or property...");
                isStatic = true;
                consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected `static` keyword.", "parseClassBody", arena, state, context);
            }

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);

                // For Properties
                if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_COLON)
                {
                    // Consume the identifier
                    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseClassBody", arena, state, context);
                    // Move past the `:` token
                    getNextToken(lexer, arena, state);

                    logMessage(LMI, "INFO", "Parser", "Parsing property declaration...");
                    DataType *type = parseType(lexer, context, arena, state, globalTable);
                    type->typeName = strdup(identifier);

                    // Move past the data type token
                    getNextToken(lexer, arena, state);

                    // Check for initializer
                    ASTNode *initializer = NULL;
                    if (lexer->currentToken.type == TOKEN_EQUAL)
                    {
                        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected '=' for property initializer.", "parseClassBody", arena, state, context);
                        initializer = parseExpression(lexer, context, arena, state, globalTable);
                    }

                    // Consume the semicolon
                    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseClassBody", arena, state, context);

                    ASTNode *propNode = createFieldNode(identifier, type, className, NODE_CLASS, initializer, arena, state, lexer);
                    addPropertyToClass(classNode, propNode, visibility, arena, state, context, globalTable);

                    logMessage(LMI, "INFO", "Parser", "Property added to class.");

                    if (visibility == VISIBILITY_PUBLIC)
                        publicPropertyCount++;
                    else if (visibility == VISIBILITY_PRIVATE)
                        privatePropertyCount++;
                    else
                        protectedPropertyCount++;

                    break;
                }

                // For Methods
                else if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
                    ASTNode *methodNode = parseMethodDeclaration(isStatic, className, lexer, context, arena, state, globalTable);
                    if (!methodNode)
                    {
                        logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                        parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                        CONDITION_FAILED;
                    }
                    logMessage(LMI, "INFO", "Parser", "Method node created.");
                    addMethodToClass(classNode, methodNode, visibility, arena, state, context, globalTable);

                    if (visibility == VISIBILITY_PUBLIC)
                        publicMethodCount++;
                    else if (visibility == VISIBILITY_PRIVATE)
                        privateMethodCount++;
                    else
                        protectedMethodCount++;

                    break;
                }
                // Unknown
                else
                {
                    printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                    parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                    CONDITION_FAILED;
                }
            }
            else
            {
                printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                CONDITION_FAILED;
            }

            break;
        }

        case TOKEN_KW_STATIC:
        {
            // Handle static properties and methods
            bool isStatic = true;
            consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected `static` keyword.", "parseClassBody", arena, state, context);

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);

                // For Properties
                if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_COLON)
                {
                    // Consume the identifier
                    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseClassBody", arena, state, context);
                    // Move past the `:` token
                    getNextToken(lexer, arena, state);

                    logMessage(LMI, "INFO", "Parser", "Parsing static property declaration...");
                    DataType *type = parseType(lexer, context, arena, state, globalTable);
                    type->typeName = strdup(identifier);

                    // Move past the data type token
                    getNextToken(lexer, arena, state);

                    // Check for initializer
                    ASTNode *initializer = NULL;
                    if (lexer->currentToken.type == TOKEN_EQUAL)
                    {
                        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected '=' for property initializer.", "parseClassBody", arena, state, context);
                        initializer = parseExpression(lexer, context, arena, state, globalTable);
                    }

                    // Consume the semicolon
                    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseClassBody", arena, state, context);

                    ASTNode *propNode = createFieldNode(identifier, type, className, NODE_CLASS, initializer, arena, state, lexer);
                    addPropertyToClass(classNode, propNode, VISIBILITY_PUBLIC, arena, state, context, globalTable);

                    logMessage(LMI, "INFO", "Parser", "Static property added to class.");

                    publicPropertyCount++;

                    break;
                }

                // For Methods
                else if (peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing static method declaration...");
                    ASTNode *methodNode = parseMethodDeclaration(isStatic, className, lexer, context, arena, state, globalTable);
                    if (!methodNode)
                    {
                        logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                        parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                        CONDITION_FAILED;
                    }
                    logMessage(LMI, "INFO", "Parser", "Static method node created.");
                    addMethodToClass(classNode, methodNode, VISIBILITY_PUBLIC, arena, state, context, globalTable);

                    publicMethodCount++;

                    break;
                }
                // Unknown
                else
                {
                    printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                    parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                    CONDITION_FAILED;
                }
            }
            else
            {
                printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
                CONDITION_FAILED;
            }

            break;
        }

        case TOKEN_IDENTIFIER:
        {
            // TODO: Will implement later
        }

        default:
        {
            printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
            parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
        }
    }

    int propertyCount = privatePropertyCount + publicPropertyCount + protectedPropertyCount;
    int methodCount = privateMethodCount + publicMethodCount + protectedMethodCount;

    logMessage(LMI, "INFO", "Parser", "Property count: %i", propertyCount);
    logMessage(LMI, "INFO", "Parser", "Method count: %i", methodCount);

    // Update the constructor flag with the local variable flag
    classNode->data.classNode->hasConstructor = hasConstructor;

    classNode->data.classNode->privateMembers->propertyCount = privatePropertyCount;
    classNode->data.classNode->privateMembers->methodCount = privateMethodCount;

    classNode->data.classNode->protectedMembers->propertyCount = protectedPropertyCount;
    classNode->data.classNode->protectedMembers->methodCount = protectedMethodCount;

    classNode->data.classNode->publicMembers->propertyCount = publicPropertyCount;
    classNode->data.classNode->publicMembers->methodCount = publicMethodCount;

    classNode->data.classNode->propertyCount = propertyCount;
    classNode->data.classNode->methodCount = methodCount;

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end class body.", "parseClassBody", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Finished parsing class body.");
    return classNode;
}

// Helper function to ensure capacity for members
static void ensureCapacity(ASTNode **array, int *capacity, int count, int increment)
{
    __STACK_FRAME__
    if (count >= *capacity)
    {
        *capacity = *capacity == 0 ? 4 : *capacity * 2;
        ASTNode **newArray = (ASTNode **)realloc(array, *capacity * sizeof(ASTNode *));
        if (!newArray)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to reallocate memory for class members");
            return;
        }
        array = newArray;
    }
}

void addConstructorToClass(ASTNode *classNode, ASTNode *constructorNode, Arena *arena, CompilerState *state)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding constructor to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->constructors)
    {
        classData->constructors = (ASTNode **)malloc(sizeof(ASTNode *) * 12);
        if (!classData->constructors)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate constructor members");
            return;
        }
    }

    ensureCapacity(classData->constructors, &classData->constructorCapacity, classData->constructorCount, 1);

    classData->constructors[classData->constructorCount++] = constructorNode;
    classData->hasConstructor = true;
    logMessage(LMI, "INFO", "Parser", "Constructor added to class.");
    return;
}

void addMethodToClass(ASTNode *classNode, ASTNode *methodNode, CryoVisibilityType visibility,
                      Arena *arena, CompilerState *state, ParsingContext *context, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    switch (visibility)
    {
    case VISIBILITY_PRIVATE:
        logMessage(LMI, "INFO", "Parser", "Adding private method to class...");
        addPrivateMethod(classNode, methodNode, arena, state, context);
        break;
    case VISIBILITY_PUBLIC:
        logMessage(LMI, "INFO", "Parser", "Adding public method to class...");
        addPublicMethod(classNode, methodNode, arena, state, context);
        break;
    case VISIBILITY_PROTECTED:
        logMessage(LMI, "INFO", "Parser", "Adding protected method to class...");
        addProtectedMethod(classNode, methodNode, arena, state, context);
        break;
    default:
        logMessage(LMI, "ERROR", "Parser", "Invalid visibility type");
    }

    logMessage(LMI, "INFO", "Parser", "Method added to class.");
    return;
}

void addPropertyToClass(ASTNode *classNode, ASTNode *propNode, CryoVisibilityType visibility,
                        Arena *arena, CompilerState *state, ParsingContext *context, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    switch (visibility)
    {
    case VISIBILITY_PRIVATE:
        addPrivateProperty(classNode, propNode, arena, state, context);
        break;
    case VISIBILITY_PUBLIC:
        addPublicProperty(classNode, propNode, arena, state, context);
        break;
    case VISIBILITY_PROTECTED:
        addProtectedProperty(classNode, propNode, arena, state, context);
        break;
    default:
        logMessage(LMI, "ERROR", "Parser", "Invalid visibility type");
    }

    const char *className = classNode->data.classNode->name;
    AddPropertyToClass(globalTable, className, propNode); // Update Global Symbol Table

    logMessage(LMI, "INFO", "Parser", "Property added to class.");
    return;
}

void addPrivateMethod(ASTNode *classNode, ASTNode *methodNode,
                      Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding private method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (struct PrivateMembers *)malloc(sizeof(struct PrivateMembers));
        if (!classData->privateMembers)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate private members");
            return;
        }
    }

    ensureCapacity(classData->privateMembers->methods,
                   &classData->privateMembers->methodCapacity,
                   classData->privateMembers->methodCount, 1);

    classData->privateMembers->methods[classData->privateMembers->methodCount++] = methodNode;
}

void addPublicMethod(ASTNode *classNode, ASTNode *methodNode,
                     Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding public method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    logMessage(LMI, "INFO", "Parser", "Setting public method to class...");
    classNode->data.classNode->publicMembers->methods[classNode->data.classNode->publicMembers->methodCount++] = methodNode;

    addMethodToThisContext(context, methodNode);
    logMessage(LMI, "INFO", "Parser", "Public method added to class.");
    return;
}

void addProtectedMethod(ASTNode *classNode, ASTNode *methodNode,
                        Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding protected method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (struct ProtectedMembers *)malloc(sizeof(struct ProtectedMembers));
        if (!classData->protectedMembers)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate protected members");
            return;
        }
    }

    ensureCapacity(classData->protectedMembers->methods,
                   &classData->protectedMembers->methodCapacity,
                   classData->protectedMembers->methodCount, 1);

    classData->protectedMembers->methods[classData->protectedMembers->methodCount++] = methodNode;
}

void addPublicProperty(ASTNode *classNode, ASTNode *propNode,
                       Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding public property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    addPropertyToThisContext(context, propNode);
    classData->publicMembers->properties[classData->publicMembers->propertyCount++] = propNode;
}

void addPrivateProperty(ASTNode *classNode, ASTNode *propNode,
                        Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding private property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (struct PrivateMembers *)malloc(sizeof(struct PrivateMembers));
        if (!classData->privateMembers)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate private members");
            return;
        }
    }

    classData->privateMembers->properties[classData->privateMembers->propertyCount++] = propNode;
}

void addProtectedProperty(ASTNode *classNode, ASTNode *propNode,
                          Arena *arena, CompilerState *state, ParsingContext *context)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Adding protected property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }
    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (struct ProtectedMembers *)malloc(sizeof(struct ProtectedMembers));
        if (!classData->protectedMembers)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate protected members");
            return;
        }
    }

    ensureCapacity(classData->protectedMembers->properties,
                   &classData->protectedMembers->propertyCapacity,
                   classData->protectedMembers->propertyCount, 1);

    classData->protectedMembers->properties[classData->protectedMembers->propertyCount++] = propNode;
}

ASTNode *parseMethodScopeResolution(const char *scopeName,
                                    Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing method scope resolution...");

    const char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseMethodScopeResolution", arena, state, context);

    logMessage(LMI, "INFO", "Parser", "Method name: %s", methodName);

    Symbol *symbol = FindMethodSymbol(globalTable, methodName, strdup(scopeName), OBJECT_TYPE);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find method symbol.");
        parsingError("Failed to find method symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    ASTNode *methodNode = GetASTNodeFromSymbol(globalTable, symbol);
    if (!methodNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to get method node from symbol.");
        parsingError("Failed to get method node from symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    DataType *methodType = methodNode->data.method->type;

    // Check if this is a static method
    if (methodNode->metaData->type == NODE_METHOD)
    {
        bool isStaticMethod = methodNode->data.method->isStatic;
        ASTNode *argList = parseArgumentList(lexer, context, arena, state, globalTable);
        if (!argList)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse method arguments.");
            parsingError("Failed to parse method arguments.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
        ASTNode **args = argList->data.argList->args;
        int argCount = argList->data.argList->argCount;

        // We need to get the AST Node of the class as well as its type
        DataType *classType = ResolveDataType(globalTable, scopeName);
        if (!classType)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find class type.");
            parsingError("Failed to find class type.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }

        // Create the method call node
        ASTNode *methodCall = createMethodCallNode(NULL, methodType, classType, methodName, args, argCount, isStaticMethod,
                                                   arena, state, lexer);
        if (!methodCall)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create method call node.");
            parsingError("Failed to create method call node.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }

        const char *classScopeID = Generate64BitHashID(scopeName);
        Symbol *classSym = FindSymbol(globalTable, scopeName, classScopeID);
        if (!classSym)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find class symbol.");
            parsingError("Failed to find class symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
        ASTNode *classASTNode = GetASTNodeFromSymbol(globalTable, classSym);
        methodCall->data.methodCall->accessorObj = classASTNode;

        logASTNode(methodCall);

        return methodCall;
    }
    DEBUG_BREAKPOINT;
}

ASTNode *createClassPropertyAccessNode(ASTNode *object, ASTNode *property, const char *propName, DataType *typeOfNode,
                                       Arena *arena, CompilerState *state, Lexer *lexer)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Creating class property access node...");

    ASTNode *propAccess = createPropertyAccessNode(object, propName, arena, state, lexer);
    if (!propAccess)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
        parsingError("Failed to create property access node.", "createClassPropertyAccessNode", arena, state, lexer, lexer->source, NULL);
        CONDITION_FAILED;
    }

    logASTNode(propAccess);
    return propAccess;
}

// | =========================================================================================================================================== | //
// |                                                                                                                                             | //
// |                                                    Implementation Parsing Functions                                                         | //
// |                                                                                                                                             | //
// | =========================================================================================================================================== | //

// The `implementation` keyword is used to define the implementation of a class or struct.
// The syntax is as follows:
// implement <struct|class> <typeName> { <body> }
ASTNode *parseImplementation(Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing implementation...");

    consume(__LINE__, lexer, TOKEN_KW_IMPLEMENT, "Expected `implementation` keyword.", "parseImplementation", arena, state, context);

    if (lexer->currentToken.type != TOKEN_KW_CLASS &&
        lexer->currentToken.type != TOKEN_KW_STRUCT)
    {
        parsingError("Expected `class` or `struct` keyword.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    bool isClass = false;
    bool isStruct = false;

    if (lexer->currentToken.type == TOKEN_KW_CLASS)
    {
        consume(__LINE__, lexer, TOKEN_KW_CLASS, "Expected `class` keyword.", "parseImplementation", arena, state, context);
        isClass = true;
    }
    else if (lexer->currentToken.type == TOKEN_KW_STRUCT)
    {
        consume(__LINE__, lexer, TOKEN_KW_STRUCT, "Expected `struct` keyword.", "parseImplementation", arena, state, context);
        isStruct = true;
    }
    else
    {
        parsingError("Expected `class` or `struct` keyword.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    const char *typeName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    if (!typeName)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to allocate memory for type name.");
        parsingError("Failed to allocate memory for type name.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "Parser", "Implementation of <%s> %s", isClass ? "class" : "struct", typeName);

    DTM->symbolTable->printTable(DTM->symbolTable);
    DataType *interfaceType = DTM->symbolTable->lookup(DTM->symbolTable, typeName);
    if (!interfaceType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find interface type.");
        parsingError("Failed to find interface type.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier for implementation name.", "parseImplementation", arena, state, context);
    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start implementation body.", "parseImplementation", arena, state, context);

    ASTNode **methods = (ASTNode **)malloc(sizeof(ASTNode *) * MAX_METHOD_CAPACITY);
    if (!methods)
    {
        logMessage(LMI, "ERROR", "Memory", "Failed to allocate memory for methods.");
        parsingError("Failed to allocate memory for methods.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }
    ASTNode **properties = (ASTNode **)malloc(sizeof(ASTNode *) * MAX_FIELD_CAPACITY);
    if (!properties)
    {
        logMessage(LMI, "ERROR", "Memory", "Failed to allocate memory for properties.");
        parsingError("Failed to allocate memory for properties.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    int methodCount = 0;
    int propertyCount = 0;

    while (lexer->currentToken.type != TOKEN_RBRACE)
    {
        ASTNode *fieldNode = parseImplementationBody(interfaceType, lexer, context, arena, state, globalTable);
        if (!fieldNode)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse field declaration.");
            parsingError("Failed to parse field declaration.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }

        if (fieldNode->metaData->type == NODE_METHOD)
        {
            methods[methodCount++] = fieldNode;
        }
        else if (fieldNode->metaData->type == NODE_PROPERTY)
        {
            properties[propertyCount++] = fieldNode;
        }
        else
        {
            logMessage(LMI, "ERROR", "Parser", "Unknown node type.");
            parsingError("Unknown node type.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
            CONDITION_FAILED;
        }
    }
    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end implementation body.", "parseImplementation", arena, state, context);
    logMessage(LMI, "INFO", "Parser", "Finished parsing implementation body.");

    ASTNode *implementationNode = createImplementationNode(typeName, properties, propertyCount,
                                                           methods, methodCount,
                                                           arena, state, lexer);
    if (!implementationNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create implementation node.");
        parsingError("Failed to create implementation node.", "parseImplementation", arena, state, lexer, lexer->source, globalTable);
        CONDITION_FAILED;
    }

    implementationNode->print(implementationNode);
    logMessage(LMI, "INFO", "Parser", "Implementation node created.");

    return implementationNode;
}

ASTNode *parseImplementationBody(DataType *interfaceType,
                                 Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, CryoGlobalSymbolTable *globalTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Parser", "Parsing implementation body...");

    // Handle property declarations (identifier followed by colon)
    if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
        peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_COLON)
    {

        const char *fieldName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Implementation property: %s", fieldName);

        // Consume the field name
        getNextToken(lexer, arena, state);
        // Consume the colon
        consume(__LINE__, lexer, TOKEN_COLON, "Expected `:` after field name.", "parseImplementationBody", arena, state, context);

        // Parse the type
        DataType *fieldType = parseType(lexer, context, arena, state, globalTable);
        getNextToken(lexer, arena, state);

        fieldType->debug->printType(fieldType);

        // Check for initializer
        ASTNode *initializer = NULL;
        if (lexer->currentToken.type == TOKEN_EQUAL)
        {
            consume(__LINE__, lexer, TOKEN_EQUAL, "Expected '=' for property initializer.", "parseImplementationBody", arena, state, context);
            initializer = parseExpression(lexer, context, arena, state, globalTable);
        }

        // Consume the semicolon
        consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseImplementationBody", arena, state, context);
        const char *parentName = interfaceType->debug->toString(interfaceType);
        CryoNodeType parentNodeType = interfaceType->container->objectType == OBJECT_TYPE ? NODE_CLASS : NODE_STRUCT_DECLARATION;

        return createFieldNode(fieldName, fieldType, parentName, parentNodeType, initializer, arena, state, lexer);
    }

    // Handle method declarations (identifier followed by parentheses)
    if (lexer->currentToken.type == TOKEN_IDENTIFIER &&
        peekNextUnconsumedToken(lexer, arena, state).type == TOKEN_LPAREN)
    {
        const char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
        logMessage(LMI, "INFO", "Parser", "Implementation method: %s", methodName);

        // Since we're in an implementation, we need to determine the parent type
        const char *parentName = interfaceType->debug->toString(interfaceType);
        bool isStatic = false; // Implementation methods are instance methods by default

        logMessage(LMI, "INFO", "Parser", "Method parent name: %s", parentName);

        // Parse the method declaration
        return parseMethodDeclaration(isStatic, parentName, lexer, context, arena, state, globalTable);
    }

    // If we reach here, we couldn't parse a valid implementation member
    char *tokenStr = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "ERROR", "Parser", "Unexpected token in implementation body: %s", tokenStr);
    parsingError("Unexpected token in implementation body.", "parseImplementationBody", arena, state, lexer, lexer->source, globalTable);

    return NULL;
}
