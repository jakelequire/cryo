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
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I8;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI16(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I16;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI32(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I32;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI64(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I64;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI128(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I128;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createInt(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_INT;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createFloat(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_FLOAT;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createString(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_STRING;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createBoolean(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_BOOLEAN;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createVoid(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_VOID;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createNull(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_NULL;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAny(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_ANY;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createUndefined(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_UNDEFINED;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAutoTy(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_AUTO;

    return DTM->dataTypes->wrapTypeContainer(container);
}

PrimitiveDataType DTMPrimitives_getPrimitiveType(const char *typeStr)
{
    if (cStringCompare(typeStr, "i8"))
    {
        return PRIM_I8;
    }
    else if (cStringCompare(typeStr, "i16"))
    {
        return PRIM_I16;
    }
    else if (cStringCompare(typeStr, "i32"))
    {
        return PRIM_I32;
    }
    else if (cStringCompare(typeStr, "i64"))
    {
        return PRIM_I64;
    }
    else if (cStringCompare(typeStr, "i128"))
    {
        return PRIM_I128;
    }
    else if (cStringCompare(typeStr, "int"))
    {
        return PRIM_INT;
    }
    else if (cStringCompare(typeStr, "float"))
    {
        return PRIM_FLOAT;
    }
    else if (cStringCompare(typeStr, "string"))
    {
        return PRIM_STRING;
    }
    else if (cStringCompare(typeStr, "boolean"))
    {
        return PRIM_BOOLEAN;
    }
    else if (cStringCompare(typeStr, "void"))
    {
        return PRIM_VOID;
    }
    else if (cStringCompare(typeStr, "null"))
    {
        return PRIM_NULL;
    }
    else if (cStringCompare(typeStr, "any"))
    {
        return PRIM_ANY;
    }
    else if (cStringCompare(typeStr, "undefined"))
    {
        return PRIM_UNDEFINED;
    }
    else if (cStringCompare(typeStr, "auto"))
    {
        return PRIM_AUTO;
    }
    else
    {
        fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type '%s'\n", typeStr);
        CONDITION_FAILED;
    }
}

DataType *DTMPrimitives_createPrimString(const char *str)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_STRING;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createPrimBoolean(bool value)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_BOOLEAN;

    return DTM->dataTypes->wrapTypeContainer(container);
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

    primitives->createPrimString = DTMPrimitives_createPrimString;
    primitives->createPrimBoolean = DTMPrimitives_createPrimBoolean;

    primitives->getPrimitiveType = DTMPrimitives_getPrimitiveType;

    return primitives;
}
