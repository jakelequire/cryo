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
#include "frontend/typeTable.h"

// Primitive int Type `(default/baseline)`
DataType *createPrimitiveIntType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

// Primitive float Type creation `(default/baseline)`
DataType *createPrimitiveFloatType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

// Primitive string Type creation `(default/baseline)`
DataType *createPrimitiveStringType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;

    return wrapTypeContainer(container);
}

// Primitive boolean Type creation `(default/baseline)`
DataType *createPrimitiveBooleanType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;

    return wrapTypeContainer(container);
}

// Primitive void Type creation `(default/baseline)`
DataType *createPrimitiveVoidType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

// Primitive null Type creation `(default/baseline)`
DataType *createPrimitiveNullType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

// Unknown Type `(will be resolved later in the compiler)`
DataType *createUnknownType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = UNKNOWN_TYPE;

    return wrapTypeContainer(container);
}

// Create primitive type
TypeContainer *createPrimitiveType(PrimitiveDataType primType)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = PRIMITIVE_TYPE;
    container->primitive = primType;

    return container;
}
