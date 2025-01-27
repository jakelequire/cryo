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
#include "frontend/dataTypes.h"

// createMethodType(returnType, paramTypes, paramCount, arena, state, typeTable);

FunctionType *createFunctionTypeContainer(void)
{
    FunctionType *funcType = (FunctionType *)malloc(sizeof(FunctionType));
    if (!funcType)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate FunctionType\n");
        CONDITION_FAILED;
    }

    funcType->returnType = NULL;
    funcType->paramCount = 0;
    funcType->paramTypes = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!funcType->paramTypes)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for function type parameters\n");
        CONDITION_FAILED;
    }
    funcType->name = (char *)malloc(sizeof(char) * 64);
    funcType->body = (ASTNode *)malloc(sizeof(ASTNode));

    return funcType;
}

DataType *createMethodType(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    DataType *methodType = wrapTypeContainer(createTypeContainer());
    if (!methodType)
    {
        logMessage(LMI, "ERROR", "DataTypes", "Failed to allocate memory for method type");
        return NULL;
    }

    methodType->container->baseType = FUNCTION_TYPE;
    methodType->container->custom.name = strdup(methodName);
    methodType->container->custom.funcDef = createFunctionTypeContainer();

    methodType->container->custom.funcDef->returnType = returnType;
    methodType->container->custom.funcDef->paramCount = paramCount;
    methodType->container->custom.funcDef->name = strdup(methodName);
    methodType->container->custom.funcDef->body = NULL;

    methodType->container->primitive = PRIM_CUSTOM;

    for (int i = 0; i < paramCount; i++)
    {
        methodType->container->custom.funcDef->paramTypes[i] = paramTypes[i];
    }

    return methodType;
}

DataType *createFunctionType(const char *functionName, DataType *returnType, DataType **paramTypes, int paramCount, Arena *arena, CompilerState *state, TypeTable *typeTable)
{
    DataType *functionType = wrapTypeContainer(createTypeContainer());
    if (!functionType)
    {
        logMessage(LMI, "ERROR", "DataTypes", "Failed to allocate memory for function type");
        return NULL;
    }

    functionType->container->baseType = FUNCTION_TYPE;
    functionType->container->custom.funcDef = createFunctionTypeContainer();
    functionType->container->custom.name = strdup(functionName);

    functionType->container->custom.funcDef->returnType = returnType;
    functionType->container->custom.funcDef->paramCount = paramCount;
    functionType->container->custom.funcDef->name = strdup(functionName);

    functionType->container->primitive = PRIM_CUSTOM;

    for (int i = 0; i < paramCount; i++)
    {
        functionType->container->custom.funcDef->paramTypes[i] = paramTypes[i];
    }

    return functionType;
}
