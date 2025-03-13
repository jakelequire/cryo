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
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Primitive Data Types ---------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMPrimitives_createI8(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_I8;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI16(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_I16;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI32(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_I32;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI64(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_I64;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI128(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_I128;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createInt(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createFloat(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createString(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createBoolean(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createVoid(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createNull(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAny(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_ANY;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createUndefined(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_UNDEFINED;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAutoTy(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIMITIVE_TYPE;
    container->primitive = PRIM_AUTO;

    return wrapTypeContainer(container);
}

DTMPrimitives *createDTMPrimitives(void)
{
    DTMPrimitives *primitives = (DTMPrimitives *)malloc(sizeof(DTMPrimitives));
    if (!primitives)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Primitives\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    primitives->createI8 = DTMPrimitives_createI8;
    primitives->createI16 = DTMPrimitives_createI16;
    primitives->createI32 = DTMPrimitives_createI32;
    primitives->createI64 = DTMPrimitives_createI64;
    primitives->createI128 = DTMPrimitives_createI128;
    primitives->createInt = DTMPrimitives_createInt;
    primitives->createFloat = DTMPrimitives_createFloat;
    primitives->createString = DTMPrimitives_createString;
    primitives->createBoolean = DTMPrimitives_createBoolean;
    primitives->createVoid = DTMPrimitives_createVoid;
    primitives->createNull = DTMPrimitives_createNull;
    primitives->createAny = DTMPrimitives_createAny;
    primitives->createUndefined = DTMPrimitives_createUndefined;
    primitives->createAutoTy = DTMPrimitives_createAutoTy;

    return primitives;
}
