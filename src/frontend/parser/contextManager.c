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

void setThisContext(ParsingContext *context, const char *nodeName, CryoNodeType nodeType)
{
    ThisContext *thisContext = (ThisContext *)malloc(sizeof(ThisContext));
    thisContext->nodeName = nodeName;
    thisContext->nodeType = nodeType;
    thisContext->properties = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    thisContext->propertyCount = 0;
    thisContext->methods = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    thisContext->methodCount = 0;
    context->thisContext = thisContext;
}

void clearThisContext(ParsingContext *context)
{
    if (context->thisContext)
    {
        free(context->thisContext);
        context->thisContext = NULL;
    }
}

void addPropertyToThisContext(ParsingContext *context, ASTNode *propertyNode)
{
    if (context->thisContext)
    {
        int index = context->thisContext->propertyCount;
        context->thisContext->properties[index] = propertyNode;
        context->thisContext->propertyCount++;
    }
}

void addMethodToThisContext(ParsingContext *context, ASTNode *methodNode)
{
    if (context->thisContext)
    {
        int index = context->thisContext->methodCount;
        context->thisContext->methods[index] = methodNode;
        context->thisContext->methodCount++;
    }
}

ASTNode *getPropertyByName(ParsingContext *context, const char *name)
{
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
// # ============================================================ #

void logThisContext(ParsingContext *context)
{
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
        logMessage("INFO", __LINE__, "Parser", "No this context set.");
    }
}
