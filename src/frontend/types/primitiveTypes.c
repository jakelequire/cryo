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
#include "diagnostics/diagnostics.h"

// Primitive int Type `(default/baseline)`
DataType *createPrimitiveIntType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

DataType *createPrimitiveI8Type(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I8;

    return wrapTypeContainer(container);
}

DataType *createPrimitiveI16Type(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I16;

    return wrapTypeContainer(container);
}

DataType *createPrimitiveI32Type(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I32;

    return wrapTypeContainer(container);
}

DataType *createPrimitiveI64Type(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I64;

    return wrapTypeContainer(container);
}

DataType *createPrimitiveI128Type(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I128;

    return wrapTypeContainer(container);
}

// Primitive float Type creation `(default/baseline)`
DataType *createPrimitiveFloatType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

// Primitive string Type creation `(default/baseline)`
DataType *createPrimitiveStringType(int length)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;
    container->length = length;

    return wrapTypeContainer(container);
}

// Primitive boolean Type creation `(default/baseline)`
DataType *createPrimitiveBooleanType(bool booleanValue)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;
    container->boolValue = booleanValue;

    return wrapTypeContainer(container);
}

// Primitive void Type creation `(default/baseline)`
DataType *createPrimitiveVoidType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

// Primitive null Type creation `(default/baseline)`
DataType *createPrimitiveNullType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

// Primitive any Type creation `(default/baseline)`
DataType *createPrimitiveAnyType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_ANY;

    return wrapTypeContainer(container);
}

// Unknown Type `(will be resolved later in the compiler)`
DataType *createUnknownType(void)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    container->baseType = UNKNOWN_TYPE;

    return wrapTypeContainer(container);
}

// Create primitive type
TypeContainer *createPrimitiveType(PrimitiveDataType primType)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = PRIMITIVE_TYPE;
    container->primitive = primType;

    return container;
}

PrimitiveDataType getPrimativeTypeFromString(const char *typeStr)
{
    __STACK_FRAME__
    if (strcmp(typeStr, "int") == 0)
    {
        return PRIM_INT;
    }
    else if (strcmp(typeStr, "float") == 0)
    {
        return PRIM_FLOAT;
    }
    else if (strcmp(typeStr, "string") == 0)
    {
        return PRIM_STRING;
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        return PRIM_BOOLEAN;
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        return PRIM_VOID;
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        return PRIM_NULL;
    }
    else
    {
        return PRIM_UNKNOWN;
    }
}

bool isPrimitiveType(const char *typeStr)
{
    __STACK_FRAME__
    return getPrimativeTypeFromString(typeStr) != PRIM_UNKNOWN;
}

bool isStringType(DataType *type)
{
    __STACK_FRAME__
    return type->container->baseType == PRIMITIVE_TYPE && type->container->primitive == PRIM_STRING;
}
