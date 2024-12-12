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

ASTNode *parseClassDeclaration(bool isStatic,
                               Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing class declaration");
    consume(__LINE__, lexer, TOKEN_KW_CLASS, "Expected `class` keyword.", "parseclassNodearation", table, arena, state, typeTable, context);

    if (lexer->currentToken.type != TOKEN_IDENTIFIER)
    {
        parsingError("Expected an identifier for class name.", "parseclassNodearation", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    char *className = strndup(lexer->currentToken.start, lexer->currentToken.length);
    logMessage("INFO", __LINE__, "Parser", "Class name: %s", className);

    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier for class name.", "parseclassNodearation", table, arena, state, typeTable, context);

    ASTNode *classNode = createClassDeclarationNode(className, arena, state, typeTable, lexer);
    if (!classNode)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create class declaration node.");
        parsingError("Failed to create class declaration node.", "parseclassNodearation", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    clearThisContext(context, typeTable);
    setThisContext(context, className, NODE_CLASS, typeTable);

    consume(__LINE__, lexer, TOKEN_LBRACE, "Expected `{` to start class body.", "parseclassNodearation", table, arena, state, typeTable, context);

    ASTNode *classBody = parseClassBody(classNode, className, isStatic, lexer, table, context, arena, state, typeTable);
    if (!classBody)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to parse class body.");
        parsingError("Failed to parse class body.", "parseclassNodearation", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    DataType *classType = createClassDataType(className, classNode->data.classNode);
    if (!classType)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to create class data type.");
        parsingError("Failed to create class data type.", "parseclassNodearation", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }
    classNode->data.classNode->type = classType;
    classNode->data.classNode->type->container->custom.name = strdup(className);
    classNode->data.classNode->type->container->primitive = PRIM_CUSTOM;

    // Add to the symbol table
    addASTNodeSymbol(table, classNode, arena);
    // Add to the TypeTable
    addTypeToTypeTable(typeTable, className, classType);
    // Clear the context
    clearThisContext(context, typeTable);

    return classNode;
}

ASTNode *parseClassBody(ASTNode *classNode, const char *className, bool isStatic,
                        Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing class body...");

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
            ASTNode *constructor = parseConstructor(lexer, table, context, arena, state, constructorMetaData, typeTable);
            if (!constructor)
            {
                logMessage("ERROR", __LINE__, "Parser", "Failed to parse method declaration.");
                parsingError("Failed to parse method declaration.", "parseClassBody", table, arena, state, lexer, lexer->source, typeTable);
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
        {
            bool isStatic = false;
            consume(__LINE__, lexer, TOKEN_KW_PUBLIC, "Expected `public` keyword.", "parseClassBody", table, arena, state, typeTable, context);
            if (lexer->currentToken.type == TOKEN_KW_STATIC)
            {
                logMessage("INFO", __LINE__, "Parser", "Parsing static method or property...");
                isStatic = true;
                consume(__LINE__, lexer, TOKEN_KW_STATIC, "Expected `static` keyword.", "parseClassBody", table, arena, state, typeTable, context);
            }

            if (lexer->currentToken.type == TOKEN_IDENTIFIER)
            {
                char *identifier = strndup(lexer->currentToken.start, lexer->currentToken.length);

                // For Properties
                if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_COLON)
                {
                    // Consume the identifier
                    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseClassBody", table, arena, state, typeTable, context);
                    // Move past the `:` token
                    getNextToken(lexer, arena, state, typeTable);

                    logMessage("INFO", __LINE__, "Parser", "Parsing property declaration...");
                    DataType *type = parseType(lexer, context, table, arena, state, typeTable);
                    type->container->custom.name = strdup(identifier);

                    // Move past the data type token
                    getNextToken(lexer, arena, state, typeTable);
                    // Consume the semicolon
                    consume(__LINE__, lexer, TOKEN_SEMICOLON, "Expected a semicolon.", "parseClassBody", table, arena, state, typeTable, context);

                    ASTNode *propNode = createFieldNode(identifier, type, className, NODE_CLASS, NULL, arena, state, typeTable, lexer);
                    addPropertyToClass(classNode, propNode, VISIBILITY_PUBLIC, arena, state, typeTable, context);

                    logMessage("INFO", __LINE__, "Parser", "Property added to class.");

                    publicPropertyCount++;

                    break;
                }

                // For Methods
                else if (peekNextUnconsumedToken(lexer, arena, state, typeTable).type == TOKEN_LPAREN)
                {
                    logMessage("INFO", __LINE__, "Parser", "Parsing method declaration...");
                    ASTNode *methodNode = parseMethodDeclaration(isStatic, lexer, table, context, arena, state, typeTable);
                    if (!methodNode)
                    {
                        logMessage("ERROR", __LINE__, "Parser", "Failed to parse method declaration.");
                        parsingError("Failed to parse method declaration.", "parseClassBody", table, arena, state, lexer, lexer->source, typeTable);
                        CONDITION_FAILED;
                    }
                    // Add the method to the symbol table
                    addASTNodeSymbol(table, methodNode, arena);
                    logMessage("INFO", __LINE__, "Parser", "Method node created.");
                    addMethodToClass(classNode, methodNode, VISIBILITY_PUBLIC, arena, state, typeTable, context, table);

                    publicMethodCount++;

                    break;
                }

                // Unknown
                else
                {
                    printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                    parsingError("Unexpected token in class body.", "parseClassBody", table, arena, state, lexer, lexer->source, typeTable);
                    CONDITION_FAILED;
                }
            }
            else
            {
                printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
                parsingError("Unexpected token in class body.", "parseClassBody", table, arena, state, lexer, lexer->source, typeTable);
                CONDITION_FAILED;
            }

            break;
        }

        case TOKEN_KW_PRIVATE:
        {
            // TODO: Will implement later
        }

        case TOKEN_KW_PROTECTED:
        {
            // TODO: Will implement later
        }

        case TOKEN_KW_STATIC:
        {
            // TODO: Will implement later
        }

        case TOKEN_IDENTIFIER:
        {
            // TODO: Will implement later
        }

        default:
        {
            printf("Unexpected token: %s @Line: %i\n", CryoTokenToString(lexer->currentToken.type), __LINE__);
            parsingError("Unexpected token in class body.", "parseClassBody", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }
        }
    }

    int propertyCount = privatePropertyCount + publicPropertyCount + protectedPropertyCount;
    int methodCount = privateMethodCount + publicMethodCount + protectedMethodCount;

    logMessage("INFO", __LINE__, "Parser", "Property count: %i", propertyCount);
    logMessage("INFO", __LINE__, "Parser", "Method count: %i", methodCount);

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

    consume(__LINE__, lexer, TOKEN_RBRACE, "Expected `}` to end class body.", "parseClassBody", table, arena, state, typeTable, context);
    logMessage("INFO", __LINE__, "Parser", "Finished parsing class body.");
    return classNode;
}

// Helper function to ensure capacity for members
static void ensureCapacity(ASTNode **array, int *capacity, int count, int increment)
{
    if (count >= *capacity)
    {
        *capacity = *capacity == 0 ? 4 : *capacity * 2;
        ASTNode **newArray = (ASTNode **)realloc(array, *capacity * sizeof(ASTNode *));
        if (!newArray)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to reallocate memory for class members");
            return;
        }
        array = newArray;
    }
}

void addConstructorToClass(ASTNode *classNode, ASTNode *constructorNode, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Adding constructor to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->constructor)
    {
        classData->constructor = (ASTNode *)calloc(1, sizeof(ASTNode));
        if (!classData->constructor)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to allocate constructors");
            return;
        }
    }
    classData->constructor = constructorNode;

    logMessage("INFO", __LINE__, "Parser", "Constructor added to class.");
}

void addMethodToClass(ASTNode *classNode, ASTNode *methodNode, CryoVisibilityType visibility,
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoSymbolTable *table)
{
    logMessage("INFO", __LINE__, "Parser", "Adding method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    switch (visibility)
    {
    case VISIBILITY_PRIVATE:
        logMessage("INFO", __LINE__, "Parser", "Adding private method to class...");
        addPrivateMethod(classNode, methodNode, arena, state, typeTable, context, table);
        break;
    case VISIBILITY_PUBLIC:
        logMessage("INFO", __LINE__, "Parser", "Adding public method to class...");
        addPublicMethod(classNode, methodNode, arena, state, typeTable, context, table);
        break;
    case VISIBILITY_PROTECTED:
        logMessage("INFO", __LINE__, "Parser", "Adding protected method to class...");
        addProtectedMethod(classNode, methodNode, arena, state, typeTable, context, table);
        break;
    default:
        logMessage("ERROR", __LINE__, "Parser", "Invalid visibility type");
    }

    logMessage("INFO", __LINE__, "Parser", "Method added to class.");
    return;
}

void addPropertyToClass(ASTNode *classNode, ASTNode *propNode, CryoVisibilityType visibility,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    logMessage("INFO", __LINE__, "Parser", "Adding property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
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
        logMessage("ERROR", __LINE__, "Parser", "Invalid visibility type");
    }
}

void addPrivateMethod(ASTNode *classNode, ASTNode *methodNode,
                      Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoSymbolTable *table)
{
    logMessage("INFO", __LINE__, "Parser", "Adding private method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (PrivateMembers *)calloc(1, sizeof(PrivateMembers));
        if (!classData->privateMembers)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to allocate private members");
            return;
        }
    }

    ensureCapacity(classData->privateMembers->methods,
                   &classData->privateMembers->methodCapacity,
                   classData->privateMembers->methodCount, 1);

    classData->privateMembers->methods[classData->privateMembers->methodCount++] = methodNode;
}

void addPublicMethod(ASTNode *classNode, ASTNode *methodNode,
                     Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoSymbolTable *table)
{
    logMessage("INFO", __LINE__, "Parser", "Adding public method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    logMessage("INFO", __LINE__, "Parser", "Setting public method to class...");
    classNode->data.classNode->publicMembers->methods[classNode->data.classNode->publicMembers->methodCount++] = methodNode;

    addMethodToThisContext(context, methodNode, typeTable);
    logMessage("INFO", __LINE__, "Parser", "Public method added to class.");
    return;
}

void addProtectedMethod(ASTNode *classNode, ASTNode *methodNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context, CryoSymbolTable *table)
{
    logMessage("INFO", __LINE__, "Parser", "Adding protected method to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (ProtectedMembers *)calloc(1, sizeof(ProtectedMembers));
        if (!classData->protectedMembers)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to allocate protected members");
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
    logMessage("INFO", __LINE__, "Parser", "Adding public property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    addPropertyToThisContext(context, propNode, typeTable);
    classData->publicMembers->properties[classData->publicMembers->propertyCount++] = propNode;
}

void addPrivateProperty(ASTNode *classNode, ASTNode *propNode,
                        Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    logMessage("INFO", __LINE__, "Parser", "Adding private property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }

    ClassNode *classData = classNode->data.classNode;
    if (!classData->privateMembers)
    {
        classData->privateMembers = (PrivateMembers *)calloc(1, sizeof(PrivateMembers));
        if (!classData->privateMembers)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to allocate private members");
            return;
        }
    }

    classData->privateMembers->properties[classData->privateMembers->propertyCount++] = propNode;
}

void addProtectedProperty(ASTNode *classNode, ASTNode *propNode,
                          Arena *arena, CompilerState *state, TypeTable *typeTable, ParsingContext *context)
{
    logMessage("INFO", __LINE__, "Parser", "Adding protected property to class...");
    if (classNode->metaData->type != NODE_CLASS)
    {
        logMessage("ERROR", __LINE__, "Parser", "Expected class declaration node.");
        return;
    }
    ClassNode *classData = classNode->data.classNode;
    if (!classData->protectedMembers)
    {
        classData->protectedMembers = (ProtectedMembers *)calloc(1, sizeof(ProtectedMembers));
        if (!classData->protectedMembers)
        {
            logMessage("ERROR", __LINE__, "Memory", "Failed to allocate protected members");
            return;
        }
    }

    ensureCapacity(classData->protectedMembers->properties,
                   &classData->protectedMembers->propertyCapacity,
                   classData->protectedMembers->propertyCount, 1);

    classData->protectedMembers->properties[classData->protectedMembers->propertyCount++] = propNode;
}

ASTNode *parseMethodScopeResolution(const char *scopeName,
                                    Lexer *lexer, CryoSymbolTable *table, ParsingContext *context, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "Parser", "Parsing method scope resolution...");

    const char *methodName = strndup(lexer->currentToken.start, lexer->currentToken.length);
    consume(__LINE__, lexer, TOKEN_IDENTIFIER, "Expected an identifier.", "parseMethodScopeResolution", table, arena, state, typeTable, context);

    logMessage("INFO", __LINE__, "Parser", "Method name: %s", methodName);

    // Look up the method in the symbol table
    CryoSymbol *sym = resolveModuleSymbol(scopeName, methodName, table, arena);
    if (!sym)
    {
        logMessage("ERROR", __LINE__, "Parser", "Failed to resolve method symbol.");
        parsingError("Failed to resolve method symbol.", "parseMethodScopeResolution", table, arena, state, lexer, lexer->source, typeTable);
        CONDITION_FAILED;
    }

    logASTNode(sym->node);

    // Check if this is a static method
    if (sym->node->metaData->type == NODE_METHOD)
    {
        bool isStaticMethod = sym->node->data.method->isStatic;
        ASTNode *argList = parseArgumentList(lexer, table, context, arena, state, typeTable);
        if (!argList)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to parse method arguments.");
            parsingError("Failed to parse method arguments.", "parseMethodScopeResolution", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }
        ASTNode **args = argList->data.argList->args;
        int argCount = argList->data.argList->argCount;

        // We need to get the AST Node of the class as well as its type
        DataType *classType = findClassTypeFromName(scopeName, typeTable);
        if (!classType)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to find class type.");
            parsingError("Failed to find class type.", "parseMethodScopeResolution", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        // Create the method call node
        ASTNode *methodCall = createMethodCallNode(NULL, sym->type, classType, methodName, args, argCount, isStaticMethod,
                                                   arena, state, typeTable, lexer);
        if (!methodCall)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to create method call node.");
            parsingError("Failed to create method call node.", "parseMethodScopeResolution", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        // Look up the class in the symbol table
        CryoSymbol *classSym = findSymbol(table, scopeName, arena);
        if (!classSym)
        {
            logMessage("ERROR", __LINE__, "Parser", "Failed to find class symbol.");
            parsingError("Failed to find class symbol.", "parseMethodScopeResolution", table, arena, state, lexer, lexer->source, typeTable);
            CONDITION_FAILED;
        }

        ASTNode *classASTNode = classSym->node;
        methodCall->data.methodCall->accessorObj = classASTNode;

        logASTNode(methodCall);

        return methodCall;
    }
    DEBUG_BREAKPOINT;
}
