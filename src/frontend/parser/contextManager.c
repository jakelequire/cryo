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
#include "frontend/parser.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

void ParsingContext_setIntegerContextType(ParsingContext *context, DataType *type)
{
    __STACK_FRAME__
    if (context->integerContextType)
    {
        free(context->integerContextType);
    }
    context->integerContextType = (DataType *)malloc(sizeof(DataType));
    context->integerContextType = type;
}

void ParsingContext_setStringContextType(ParsingContext *context, DataType *type)
{
    __STACK_FRAME__
    if (context->stringContextType)
    {
        free(context->stringContextType);
    }
    context->stringContextType = (DataType *)malloc(sizeof(DataType));
    context->stringContextType = type;
}

void ParsingContext_clearIntegerContextType(ParsingContext *context)
{
    __STACK_FRAME__
    if (context->integerContextType)
    {
        context->integerContextType = NULL;
    }
}

void ParsingContext_clearStringContextType(ParsingContext *context)
{
    __STACK_FRAME__
    if (context->stringContextType)
    {
        context->stringContextType = NULL;
    }
}

void ParsingContext_addPragmaArg(ParsingContext *context, const char *arg)
{
    __STACK_FRAME__
    if (context->pragmaArgCount >= MAX_ARGUMENTS)
    {
        fprintf(stderr, "Error: Too many pragma arguments\n");
        return;
    }
    context->pragmaArgs[context->pragmaArgCount] = strdup(arg);
    context->pragmaArgCount++;
}

bool ParsingContext_doesPragmaExist(ParsingContext *context, const char *arg)
{
    __STACK_FRAME__
    for (int i = 0; i < context->pragmaArgCount; i++)
    {
        if (strcmp(context->pragmaArgs[i], arg) == 0)
        {
            return true;
        }
    }
    return false;
}

ParsingContext *createParsingContext(void)
{
    __STACK_FRAME__
    ParsingContext *context = (ParsingContext *)malloc(sizeof(ParsingContext));
    context->lastTokenCount = 0;
    context->thisContext = NULL;
    context->functionName = (char *)malloc(sizeof(char) * 128);
    context->currentNamespace = (char *)malloc(sizeof(char) * 128);
    context->namespaceScopeID = (char *)malloc(sizeof(char) * 128);
    context->scopeLevel = 0;
    context->lastTokenCount = 0;

    context->pragmaArgCount = 0;
    context->addPragmaArg = ParsingContext_addPragmaArg;
    context->doesPragmaExist = ParsingContext_doesPragmaExist;

    context->programNodePtr = NULL;

    context->isParsingIfCondition = false;
    context->isParsingModuleFile = false;
    context->inGenericContext = false;

    context->integerContextType = NULL;
    context->setIntegerContextType = ParsingContext_setIntegerContextType;
    context->clearIntegerContextType = ParsingContext_clearIntegerContextType;
    context->stringContextType = NULL;
    context->setStringContextType = ParsingContext_setStringContextType;
    context->clearStringContextType = ParsingContext_clearStringContextType;

    context->currentGenericParamCapacity = 16;
    context->currentGenericParamCount = 0;
    context->currentGenericParams = (GenericType **)malloc(sizeof(GenericType *) * 16);

    context->addGenericParam = ParsingContext_addGenericParam;

    return context;
}

void ParsingContext_addGenericParam(ParsingContext *context, const char *name, GenericType *param)
{
    __STACK_FRAME__
    if (context->currentGenericParamCount >= context->currentGenericParamCapacity)
    {
        context->currentGenericParamCapacity *= 2;
        context->currentGenericParams = (GenericType **)realloc(context->currentGenericParams, sizeof(GenericType *) * context->currentGenericParamCapacity);
    }

    context->currentGenericParams[context->currentGenericParamCount] = param;
    context->currentGenericParamCount++;

    return;
}

void setDefaultThisContext(const char *currentNamespace, ParsingContext *context)
{
    __STACK_FRAME__
    ThisContext *thisContext = (ThisContext *)malloc(sizeof(ThisContext));
    thisContext->nodeName = currentNamespace;
    thisContext->nodeType = NODE_NAMESPACE;
    thisContext->properties = NULL;
    thisContext->propertyCount = 0;
    thisContext->methods = NULL;
    thisContext->methodCount = 0;
    thisContext->isStatic = false;
    thisContext->type = DTM->primitives->createUndefined();
    context->thisContext = thisContext;
}

void setThisContext(ParsingContext *context, const char *nodeName, CryoNodeType nodeType)
{
    __STACK_FRAME__
    ThisContext *thisContext = (ThisContext *)malloc(sizeof(ThisContext));
    thisContext->nodeName = nodeName;
    thisContext->nodeType = nodeType;
    thisContext->properties = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    thisContext->propertyCount = 0;
    thisContext->methods = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    thisContext->methodCount = 0;
    thisContext->isStatic = false;
    thisContext->type = DTM->primitives->createUndefined();
    context->thisContext = thisContext;
}

void setTypePtrToContext(ParsingContext *context, DataType *type)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        if (context->thisContext->type)
        {
            context->thisContext->type = NULL;
            context->thisContext->type = type;
        }
        else
        {
            context->thisContext->type = type;
        }
    }
    else
    {
        logMessage(LMI, "ERROR", "Parser", "This Context is NULL");
        return;
    }
}

void setCurrentFunction(ParsingContext *context, const char *functionName, const char *namespaceScopeID)
{
    __STACK_FRAME__
    clearScopeContext(context);
    context->functionName = functionName;
    createFunctionScope(context, functionName, namespaceScopeID);
    return;
}

void setCurrentMethod(ParsingContext *context, const char *methodName, const char *className)
{
    __STACK_FRAME__
    clearScopeContext(context);
    context->functionName = methodName;
    createMethodScope(context, methodName, className);
    return;
}

void resetCurrentMethod(ParsingContext *context)
{
    __STACK_FRAME__
    if (context->scopeContext)
    {
        ScopeParsingContext *parent = context->scopeContext->parent;
        context->scopeContext = parent;
    }
}

void resetCurrentFunction(ParsingContext *context)
{
    __STACK_FRAME__
    clearScopeContext(context);
    context->functionName = NULL;
    return;
}

const char *getCurrentScopeID(ParsingContext *context)
{
    __STACK_FRAME__
    if (!context)
    {
        fprintf(stderr, "getCurrentScopeID: Parsing Context is NULL\n");
        return NULL;
    }
    if (context->scopeContext)
    {
        logMessage(LMI, "INFO", "Parser", "Scope ID: %s", context->scopeContext->scopeID);
        return context->scopeContext->scopeID;
    }

    fprintf(stderr, "getCurrentScopeID: Scope Context is NULL\n");
    return NULL;
}

void clearThisContext(ParsingContext *context)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        free(context->thisContext);
        context->thisContext = NULL;
    }
}

void addPropertyToThisContext(ParsingContext *context, ASTNode *propertyNode)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        int index = context->thisContext->propertyCount;
        context->thisContext->properties[index] = propertyNode;
        context->thisContext->propertyCount++;
    }
}

void addMethodToThisContext(ParsingContext *context, ASTNode *methodNode)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        int index = context->thisContext->methodCount;
        context->thisContext->methods[index] = methodNode;
        context->thisContext->methodCount++;
    }
}

void setModuleFileParsingFlag(ParsingContext *context, bool value)
{
    __STACK_FRAME__
    context->isParsingModuleFile = value;
}

ASTNode *getPropertyByName(ParsingContext *context, const char *name)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        for (int i = 0; i < context->thisContext->propertyCount; i++)
        {
            ASTNode *property = context->thisContext->properties[i];
            if (strcmp(property->data.property->name, name) == 0)
            {
                return property;
            }
        }
    }
    return NULL;
}

ASTNode *getMethodByName(ParsingContext *context, const char *name)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        for (int i = 0; i < context->thisContext->methodCount; i++)
        {
            ASTNode *method = context->thisContext->methods[i];
            if (strcmp(method->data.functionDecl->name, name) == 0)
            {
                return method;
            }
        }
    }
    return NULL;
}

void addStaticIdentifierToContext(ParsingContext *context, bool value)
{
    __STACK_FRAME__
    if (context->thisContext)
    {
        context->thisContext->isStatic = value;
    }
}

// This functions adds a token to the lastTokens array in the parsing context
// If the array is full, it will shift all the tokens to the left by one
void addTokenToContext(ParsingContext *context, Token token)
{
    __STACK_FRAME__
    // Shift all tokens to the right by one
    for (int i = 15; i > 0; i--)
    {
        context->lastTokens[i] = context->lastTokens[i - 1];
    }
    // Add the new token to the first position
    context->lastTokens[0] = token;
    // Increment the last token count
    context->lastTokenCount++;
}

// --------------------------------------------------------------
// Scope Parsing Context Functions

ScopeParsingContext *createScopeParsingContext(const char *name, int level, bool isStatic, CryoNodeType nodeType)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = (ScopeParsingContext *)malloc(sizeof(ScopeParsingContext));
    scopeContext->name = name;
    scopeContext->scopeID = Generate64BitHashID(name);
    scopeContext->level = level;
    scopeContext->isStatic = isStatic;
    scopeContext->nodeType = nodeType;
    scopeContext->parent = NULL;
    return scopeContext;
}

ScopeParsingContext *createClassScopeContext(const char *className, int level, bool isStatic)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = (ScopeParsingContext *)malloc(sizeof(ScopeParsingContext));
    scopeContext->name = className;
    scopeContext->scopeID = Generate64BitHashID(className);
    scopeContext->level = level;
    scopeContext->isStatic = isStatic;
    scopeContext->nodeType = NODE_CLASS;
    scopeContext->parent = NULL;
    return scopeContext;
}

ScopeParsingContext *createMethodScopeContext(const char *methodName, int level, bool isStatic, ScopeParsingContext *parent)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = (ScopeParsingContext *)malloc(sizeof(ScopeParsingContext));
    scopeContext->name = methodName;
    scopeContext->scopeID = Generate64BitHashID(methodName);
    scopeContext->level = level;
    scopeContext->isStatic = isStatic;
    scopeContext->nodeType = NODE_METHOD;
    scopeContext->parent = parent;
    return scopeContext;
}

ScopeParsingContext *createFunctionScopeContext(const char *functionName, int level, ScopeParsingContext *parent)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = (ScopeParsingContext *)malloc(sizeof(ScopeParsingContext));
    scopeContext->name = functionName;
    scopeContext->scopeID = Generate64BitHashID(functionName);
    scopeContext->level = level;
    scopeContext->isStatic = false;
    scopeContext->nodeType = NODE_FUNCTION_DECLARATION;
    scopeContext->parent = parent;
    return scopeContext;
}

ScopeParsingContext *createNamespaceScopeContext(const char *namespaceName)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = (ScopeParsingContext *)malloc(sizeof(ScopeParsingContext));
    scopeContext->name = namespaceName;
    scopeContext->scopeID = Generate64BitHashID(namespaceName);
    scopeContext->level = 0;
    scopeContext->isStatic = false;
    scopeContext->nodeType = NODE_NAMESPACE;
    scopeContext->parent = NULL;
    return scopeContext;
}

void createNamespaceScope(ParsingContext *context, const char *namespaceName)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = createNamespaceScopeContext(namespaceName);
    context->scopeContext = scopeContext;
    context->namespaceScopeID = Generate64BitHashID(namespaceName);
    return;
}

const char *getNamespaceScopeID(ParsingContext *context)
{
    __STACK_FRAME__
    return context->namespaceScopeID;
}

void createFunctionScope(ParsingContext *context, const char *functionName, const char *namespaceScopeID)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = createFunctionScopeContext(
        functionName,
        context->scopeLevel,
        context->scopeContext);

    context->scopeContext = scopeContext;

    return;
}

void createClassScope(ParsingContext *context, const char *className)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = createScopeParsingContext(
        className,
        context->scopeLevel,
        false,
        NODE_CLASS);

    context->scopeContext = scopeContext;
    return;
}

// A methods scope is a little different from a functions scope. It is a child of the class scope,
// which means that properties of the class are accessible from the method.
void createMethodScope(ParsingContext *context, const char *methodName, const char *className)
{
    __STACK_FRAME__
    ScopeParsingContext *scopeContext = createScopeParsingContext(
        methodName,
        context->scopeLevel,
        false,
        NODE_METHOD);

    ScopeParsingContext *parent = createClassScopeContext(className, context->scopeLevel - 1, false);

    scopeContext->parent = parent;
    context->scopeContext = scopeContext;

    return;
}

void clearScopeContext(ParsingContext *context)
{
    __STACK_FRAME__
    if (context->scopeContext)
    {
        free(context->scopeContext);
        context->scopeContext = NULL;
    }
}

// # ============================================================ #

void logThisContext(ParsingContext *context)
{
    __STACK_FRAME__
    DEBUG_PRINT_FILTER({
        if (context->thisContext)
        {
            ThisContext *thisContext = context->thisContext;
            printf(BOLD MAGENTA "\n┌───────────────── This Context ─────────────────┐\n" COLOR_RESET);
            printf("This Context: %s\n", thisContext->nodeName);
            printf("Node Type: %s\n", CryoNodeTypeToString(thisContext->nodeType));
            printf("Properties:\n");
            for (int i = 0; i < thisContext->propertyCount; i++)
            {
                logASTNodeDebugView(thisContext->properties[i]);
            }
            printf("Methods: \n");
            for (int i = 0; i < thisContext->methodCount; i++)
            {
                logASTNodeDebugView(thisContext->methods[i]);
            }
            printf(BOLD MAGENTA "└────────────────────────────────────────────────┘\n" COLOR_RESET);
        }
        else
        {
            logMessage(LMI, "INFO", "Parser", "No this context set.");
        }
    });
}

void logTokenArray(ParsingContext *context)
{
    __STACK_FRAME__
    DEBUG_PRINT_FILTER({
        printf(BOLD YELLOW "\n┌────────────────────────── Last Tokens ──────────────────────────┐\n" COLOR_RESET);
        for (int i = 0; i < 16; i++)
        {
            Token token = context->lastTokens[i];
            printf(" %-3d %s\n", i, TokenToString(token));
        }
        printf(BOLD YELLOW "└─────────────────────────────────────────────────────────────────┘\n" COLOR_RESET);
    });
}

void logParsingContext(ParsingContext *context)
{
    __STACK_FRAME__
    DEBUG_PRINT_FILTER({
        printf(BOLD CYAN "\n╔══════════════════════════════ Parsing Context ══════════════════════════════╗\n" COLOR_RESET);
        logThisContext(context);
        //  logTokenArray(context);
        printf(BOLD CYAN "╚═════════════════════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    });
}

void logScopeInformation(ParsingContext *context)
{
    __STACK_FRAME__
    DEBUG_PRINT_FILTER({
        printf(BOLD GREEN "\n┌───────────────── Scope Information ─────────────────┐\n" COLOR_RESET);
        printf("Scope ID: %s\n", context->scopeContext->scopeID);
        printf("Scope Name: %s\n", context->scopeContext->name);
        printf("Scope Level: %d\n", context->scopeContext->level);
        printf("Scope Type: %s\n", CryoNodeTypeToString(context->scopeContext->nodeType));
        printf("Scope Static: %s\n", context->scopeContext->isStatic ? "true" : "false");
        printf(BOLD GREEN "└────────────────────────────────────────────────────┘\n" COLOR_RESET);
    });
}

const char *getScopeID(const char *name)
{
    __STACK_FRAME__
    return Generate64BitHashID(name);
}
