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

    DataType *i8_ty = DTM->dataTypes->wrapTypeContainer(container);
    i8_ty->setTypeName(i8_ty, "i8");
    return i8_ty;
}

DataType *DTMPrimitives_createI16(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I16;

    DataType *i16_ty = DTM->dataTypes->wrapTypeContainer(container);
    i16_ty->setTypeName(i16_ty, "i16");
    return i16_ty;
}

DataType *DTMPrimitives_createI32(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I32;

    DataType *i32_ty = DTM->dataTypes->wrapTypeContainer(container);
    i32_ty->setTypeName(i32_ty, "i32");
    return i32_ty;
}

DataType *DTMPrimitives_createI64(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I64;

    DataType *i64_ty = DTM->dataTypes->wrapTypeContainer(container);
    i64_ty->setTypeName(i64_ty, "i64");
    return i64_ty;
}

DataType *DTMPrimitives_createI128(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_I128;

    DataType *i128_ty = DTM->dataTypes->wrapTypeContainer(container);
    i128_ty->setTypeName(i128_ty, "i128");
    return i128_ty;
}

DataType *DTMPrimitives_createInt(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_INT;

    DataType *int_ty = DTM->dataTypes->wrapTypeContainer(container);
    int_ty->setTypeName(int_ty, "int");
    return int_ty;
}

DataType *DTMPrimitives_createFloat(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_FLOAT;

    DataType *float_ty = DTM->dataTypes->wrapTypeContainer(container);
    float_ty->setTypeName(float_ty, "float");
    return float_ty;
}

DataType *DTMPrimitives_createString(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_STRING;

    DataType *string_ty = DTM->dataTypes->wrapTypeContainer(container);
    string_ty->setTypeName(string_ty, "string");
    return string_ty;
}

DataType *DTMPrimitives_createBoolean(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_BOOLEAN;

    DataType *bool_ty = DTM->dataTypes->wrapTypeContainer(container);
    bool_ty->setTypeName(bool_ty, "boolean");
    return bool_ty;
}

DataType *DTMPrimitives_createVoid(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_VOID;

    DataType *void_ty = DTM->dataTypes->wrapTypeContainer(container);
    void_ty->setTypeName(void_ty, "void");
    return void_ty;
}

DataType *DTMPrimitives_createNull(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_NULL;

    DataType *null_ty = DTM->dataTypes->wrapTypeContainer(container);
    null_ty->setTypeName(null_ty, "null");
    return null_ty;
}

DataType *DTMPrimitives_createAny(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_ANY;

    DataType *any_ty = DTM->dataTypes->wrapTypeContainer(container);
    any_ty->setTypeName(any_ty, "any");
    return any_ty;
}

DataType *DTMPrimitives_createUndefined(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_UNDEFINED;

    DataType *undefined_ty = DTM->dataTypes->wrapTypeContainer(container);
    undefined_ty->setTypeName(undefined_ty, "undefined");
    return undefined_ty;
}

DataType *DTMPrimitives_createAutoTy(void)
{
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_AUTO;

    DataType *auto_ty = DTM->dataTypes->wrapTypeContainer(container);
    auto_ty->setTypeName(auto_ty, "auto");
    return auto_ty;
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
    logMessage(LMI, "INFO", "DTM", "Creating primitive type 'string'");
    TypeContainer *container = createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->primitive = PRIM_STRING;

    return DTM->dataTypes->wrapTypeContainer(container);
}

DataType *DTMPrimitives_createPrimBoolean(bool value)
{
    logMessage(LMI, "INFO", "DTM", "Creating primitive type 'boolean'");
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
