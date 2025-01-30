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

ASTNode *parseClassDeclaration(bool isStatic,
                               Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Parsing class declaration");
    consume(__LINE__, lexer, TOKEN_KW_CLASS, "Expected `class` keyword.", "parseclassNodearation", arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier for class name.", "parseclassNodearation", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    const char *className = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage(LMI, "INFO", "Parser", "Class name: %s", className);

    InitClassDeclaration(globalTable, className);
    createClassScope(context, className);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier for class name.", "parseclassNodearation", arena, state, typeTable, context);

    ASTNode *classNode = createClassDeclarationNode(className, arena, state, typeTable, lexer);
    if (!classNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create class declaration node.");
        parsingError("Failed to create class declaration node.", "parseclassNodearation", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    clearThisContext(context, typeTable);
    setThisContext(context, className, NODE_CLASS, typeTable);

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start class body.", "parseclassNodearation", arena, state, typeTable, context);

    ASTNode *classBody = parseClassBody(classNode, className, isStatic, lexer, context, arena, state, typeTable, globalTable);
    if (!classBody)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to parse class body.");
        parsingError("Failed to parse class body.", "parseclassNodearation", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    DataType *classType = createClassDataType(className, classNode->data.classNode);
    if (!classType)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create class data type.");
        parsingError("Failed to create class data type.", "parseclassNodearation", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }
    classNode->data.classNode->type = classType;
    classNode->data.classNode->type->container->custom.name = strdup(className);
    classNode->data.classNode->type->container->primitive = PRIM_CUSTOM;
    classNode->data.classNode->type->container->custom.classDef->classNode = classNode;

    CompleteClassDeclaration(globalTable, classNode, className); // Global Symbol Table

    // Clear the context
    clearThisContext(context, typeTable);
    // Clear Scope
    clearScopeContext(context);

    return classNode;
}

ASTNode *parseClassBody(ASTNode *classNode, const char *className, bool isStatic,
                        Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
            ASTNode *constructor = parseConstructor(lexer, context, arena, state, constructorMetaData, typeTable, globalTable);
            if (!constructor)
            {
                logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                CONDITION_FAILED;
            }

            logASTNode(constructor);

            addConstructorToClass(classNode, constructor, arena, state, typeTable);
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
            consume(__LINE__, lexer, lexer->currentToken.type, "Expected visibility keyword.", "parseClassBody", arena, state, typeTable, context);
            if (lexer->currentToken.type == TOKEN_KW_STATIC)
            {
                logMessage(LMI, "INFO", "Parser", "Parsing static method or property...");
                isStatic = true;
                consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected `static` keyword.", "parseClassBody", arena, state, typeTable, context);
            }

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);

                // For Properties
                if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_COLON)
                {
                    // Consume the identifier
                    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseClassBody", arena, state, typeTable, context);
                    // Move past the `:` token
                    getNextToken(lexer, arena, state, typeTable);

                    logMessage(LMI, "INFO", "Parser", "Parsing property declaration...");
                    DataType *type = parseType(lexer, context, arena, state, typeTable, globalTable);
                    type->container->custom.name = strdup(identifier);

                    // Move past the data type token
                    getNextToken(lexer, arena, state, typeTable);

                    // Check for initializer
                    ASTNode *initializer = NULL;
                    if (lexer->currentToken.type == TOKEN_EQUAL)
                    {
                        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected '=' for property initializer.", "parseClassBody", arena, state, typeTable, context);
                        initializer = parseExpression(lexer, context, arena, state, typeTable, globalTable);
                    }

                    // Consume the semicolon
                    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseClassBody", arena, state, typeTable, context);

                    ASTNode *propNode = createFieldNode(identifier, type, className, NODE_CLASS, initializer, arena, state, typeTable, lexer);
                    addPropertyToClass(classNode, propNode, visibility, arena, state, typeTable, context, globalTable);

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
                else if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing method declaration...");
                    ASTNode *methodNode = parseMethodDeclaration(isStatic, className, lexer, context, arena, state, typeTable, globalTable);
                    if (!methodNode)
                    {
                        logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                        parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                        CONDITION_FAILED;
                    }
                    logMessage(LMI, "INFO", "Parser", "Method node created.");
                    addMethodToClass(classNode, methodNode, visibility, arena, state, typeTable, context, globalTable);

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
                    parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                    CONDITION_FAILED;
                }
            }
            else
            {
                printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                CONDITION_FAILED;
            }

            break;
        }

        case TOKEN_KW_STATIC:
        {
            // Handle static properties and methods
            bool isStatic = true;
            consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected `static` keyword.", "parseClassBody", arena, state, typeTable, context);

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);

                // For Properties
                if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_COLON)
                {
                    // Consume the identifier
                    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseClassBody", arena, state, typeTable, context);
                    // Move past the `:` token
                    getNextToken(lexer, arena, state, typeTable);

                    logMessage(LMI, "INFO", "Parser", "Parsing static property declaration...");
                    DataType *type = parseType(lexer, context, arena, state, typeTable, globalTable);
                    type->container->custom.name = strdup(identifier);

                    // Move past the data type token
                    getNextToken(lexer, arena, state, typeTable);

                    // Check for initializer
                    ASTNode *initializer = NULL;
                    if (lexer->currentToken.type == TOKEN_EQUAL)
                    {
                        consume(__LINE__, lexer, TOKEN_EQUAL, "Expected '=' for property initializer.", "parseClassBody", arena, state, typeTable, context);
                        initializer = parseExpression(lexer, context, arena, state, typeTable, globalTable);
                    }

                    // Consume the semicolon
                    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseClassBody", arena, state, typeTable, context);

                    ASTNode *propNode = createFieldNode(identifier, type, className, NODE_CLASS, initializer, arena, state, typeTable, lexer);
                    addPropertyToClass(classNode, propNode, VISIBILITY_PUBLIC, arena, state, typeTable, context, globalTable);

                    logMessage(LMI, "INFO", "Parser", "Static property added to class.");

                    publicPropertyCount++;

                    break;
                }

                // For Methods
                else if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_LPAREN)
                {
                    logMessage(LMI, "INFO", "Parser", "Parsing static method declaration...");
                    ASTNode *methodNode = parseMethodDeclaration(isStatic, className, lexer, context, arena, state, typeTable, globalTable);
                    if (!methodNode)
                    {
                        logMessage(LMI, "ERROR", "Parser", "Failed to parse method declaration.");
                        parsingError("Failed to parse method declaration.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                        CONDITION_FAILED;
                    }
                    logMessage(LMI, "INFO", "Parser", "Static method node created.");
                    addMethodToClass(classNode, methodNode, VISIBILITY_PUBLIC, arena, state, typeTable, context, globalTable);

                    publicMethodCount++;

                    break;
                }
                // Unknown
                else
                {
                    printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                    parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
                    CONDITION_FAILED;
                }
            }
            else
            {
                printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
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
            parsingError("Unexpected token in class body.", "parseClassBody", arena, state, lexer, lexer->source, typeTable, globalTable);
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

    logASTNode(classNode);

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end class body.", "parseClassBody", arena, state, typeTable, context);
    logMessage(LMI, "INFO", "Parser", "Finished parsing class body.");
    return classNode;
}

// Helper function to ensure capacity for members
static void ensureCapacity(ASTNode **array, int *capacity, int count, int increment)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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

void addConstructorToClass(ASTNode *classNode, ASTNode *constructorNode, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding constructor to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->constructor)
    {
        classData->constructor = (ASTNode *)calloc(1, sizeof(ASTNode));
        if (!classData->constructor)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate constructors");
            return;
        }
    }
    classData->constructor = constructorNode;

    logMessage(LMI, "INFO", "Parser", "Constructor added to class.");
}

void addMethodToClass(ASTNode *classNode, ASTNode *methodNode, CryoVisibilityType visibility,
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoGlobalSymbolTable *globalTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
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
        addPrivateMethod(classNode, methodNode, arena, state, typeTable, context);
        break;
    case VISIBILITY_PUBLIC:
        logMessage(LMI, "INFO", "Parser", "Adding public method to class...");
        addPublicMethod(classNode, methodNode, arena, state, typeTable, context);
        break;
    case VISIBILITY_PROTECTED:
        logMessage(LMI, "INFO", "Parser", "Adding protected method to class...");
        addProtectedMethod(classNode, methodNode, arena, state, typeTable, context);
        break;
    default:
        logMessage(LMI, "ERROR", "Parser", "Invalid visibility type");
    }

    logMessage(LMI, "INFO", "Parser", "Method added to class.");
    return;
}

void addPropertyToClass(ASTNode *classNode, ASTNode *propNode, CryoVisibilityType visibility,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoGlobalSymbolTable *globalTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    switch (visibility)
    {
    case VISIBILITY_PRIVATE:
        addPrivateProperty(classNode, propNode, arena, state, typeTable, context);
        break;
    case VISIBILITY_PUBLIC:
        addPublicProperty(classNode, propNode, arena, state, typeTable, context);
        break;
    case VISIBILITY_PROTECTED:
        addProtectedProperty(classNode, propNode, arena, state, typeTable, context);
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
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding private method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (PrivateMembers *)calloc(1, sizeof(PrivateMembers));
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
                     Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding public method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    logMessage(LMI, "INFO", "Parser", "Setting public method to class...");
    classNode->data.classNode->publicMembers->methods[classNode->data.classNode->publicMembers->methodCount++] = methodNode;

    addMethodToThisContext(context, methodNode, typeTable);
    logMessage(LMI, "INFO", "Parser", "Public method added to class.");
    return;
}

void addProtectedMethod(ASTNode *classNode, ASTNode *methodNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding protected method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (ProtectedMembers *)calloc(1, sizeof(ProtectedMembers));
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
                       Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding public property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    addPropertyToThisContext(context, propNode, typeTable);
    classData->publicMembers->properties[classData->publicMembers->propertyCount++] = propNode;
}

void addPrivateProperty(ASTNode *classNode, ASTNode *propNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding private property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (PrivateMembers *)calloc(1, sizeof(PrivateMembers));
        if (!classData->privateMembers)
        {
            logMessage(LMI, "ERROR", "Memory", "Failed to allocate private members");
            return;
        }
    }

    classData->privateMembers->properties[classData->privateMembers->propertyCount++] = propNode;
}

void addProtectedProperty(ASTNode *classNode, ASTNode *propNode,
                          Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Adding protected property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage(LMI, "ERROR", "Parser", "Expected class declaration node.");
        return;
    }
    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (ProtectedMembers *)calloc(1, sizeof(ProtectedMembers));
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
                                    Lexer *lexer, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable, CryoGlobalSymbolTable *globalTable)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Parsing method scope resolution...");

    const char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseMethodScopeResolution", arena, state, typeTable, context);

    logMessage(LMI, "INFO", "Parser", "Method name: %s", methodName);

    Symbol *symbol = FindMethodSymbol(globalTable, methodName, strdup(scopeName), CLASS_TYPE);
    if (!symbol)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to find method symbol.");
        parsingError("Failed to find method symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    ASTNode *methodNode = GetASTNodeFromSymbol(globalTable, symbol);
    if (!methodNode)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to get method node from symbol.");
        parsingError("Failed to get method node from symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
        CONDITION_FAILED;
    }

    DataType *methodType = methodNode->data.method->type;

    // Check if this is a static method
    if (methodNode->metaData->type == NODE_METHOD)
    {
        bool isStaticMethod = methodNode->data.method->isStatic;
        ASTNode *argList = parseArgumentList(lexer, context, arena, state, typeTable, globalTable);
        if (!argList)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to parse method arguments.");
            parsingError("Failed to parse method arguments.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
            CONDITION_FAILED;
        }
        ASTNode **args = argList->data.argList->args;
        int argCount = argList->data.argList->argCount;

        // We need to get the AST Node of the class as well as its type
        DataType *classType = findClassTypeFromName(scopeName, typeTable);
        if (!classType)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find class type.");
            parsingError("Failed to find class type.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
            CONDITION_FAILED;
        }

        // Create the method call node
        ASTNode *methodCall = createMethodCallNode(NULL, methodType, classType, methodName, args, argCount, isStaticMethod,
                                                   arena, state, typeTable, lexer);
        if (!methodCall)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to create method call node.");
            parsingError("Failed to create method call node.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
            CONDITION_FAILED;
        }

        const char *classScopeID = Generate64BitHashID(scopeName);
        Symbol *classSym = FindSymbol(globalTable, scopeName, classScopeID);
        if (!classSym)
        {
            logMessage(LMI, "ERROR", "Parser", "Failed to find class symbol.");
            parsingError("Failed to find class symbol.", "parseMethodScopeResolution", arena, state, lexer, lexer->source, typeTable, globalTable);
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
                                       Arena *arena, CompilerState *state, TypeTable *typeTable, Lexer *lexer)
{
    GDM->createStackFrame(GDM, (char *)__func__, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Parser", "Creating class property access node...");

    ASTNode *propAccess = createPropertyAccessNode(object, propName, arena, state, typeTable, lexer);
    if (!propAccess)
    {
        logMessage(LMI, "ERROR", "Parser", "Failed to create property access node.");
        parsingError("Failed to create property access node.", "createClassPropertyAccessNode", arena, state, lexer, lexer->source, typeTable, NULL);
        CONDITION_FAILED;
    }

    logASTNode(propAccess);
    return propAccess;
}
