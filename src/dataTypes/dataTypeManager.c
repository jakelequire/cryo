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

// The global Data Type Manager instance. This is a static instance that is used to manage data types in the compiler.
// This is initialized in the `main` function of the compiler.
DataTypeManager *globalDataTypeManager = NULL;

// ----------------- Data Type Manager Implementation ----------------- //

// ------------------ DTM Primitives Implementation ------------------ //

DataType *DTMPrimitives_createI8(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I8;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI16(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I16;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI32(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I32;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI64(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I64;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createI128(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_I128;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createInt(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createFloat(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createString(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createBoolean(bool b)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;
    container->boolValue = b;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createVoid(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createNull(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

DataType *DTMPrimitives_createAny(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_ANY;

    return wrapTypeContainer(container);
}

// --------------------- DTM Debug Implementation --------------------- //

void DTMDebug_printDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    printf("Data Type: %s\n", type->container->custom.name);
}

// -------------------- Function Implementations --------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
}

DataTypeManager *createDataTypeManager(void)
{
    DataTypeManager *manager = (DataTypeManager *)malloc(sizeof(DataTypeManager));
    if (!manager)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Data Type Manager\n");
        CONDITION_FAILED;
    }

    // ===================== [ Property Assignments ] ===================== //

    manager->symbolTable = createDTMSymbolTable();
    manager->primitives = createDTMPrimitives();
    manager->debug = createDTMDebug();
    manager->structTypes = createDTMStructTypes();
    manager->classTypes = createDTMClassTypes();
    manager->functionTypes = createDTMFunctionTypes();
    manager->generics = createDTMGenerics();
    manager->enums = createDTMEnums();

    return manager;
}

DTMDebug *createDTMDebug(void)
{
    DTMDebug *debug = (DTMDebug *)malloc(sizeof(DTMDebug));
    if (!debug)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Debug\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    debug->printDataType = DTMDebug_printDataType;

    return debug;
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

    return primitives;
}

DTMStructTypes *createDTMStructTypes(void)
{
    DTMStructTypes *structTypes = (DTMStructTypes *)malloc(sizeof(DTMStructTypes));
    if (!structTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Struct Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return structTypes;
}

DTMClassTypes *createDTMClassTypes(void)
{
    DTMClassTypes *classTypes = (DTMClassTypes *)malloc(sizeof(DTMClassTypes));
    if (!classTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Class Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return classTypes;
}

DTMFunctionTypes *createDTMFunctionTypes(void)
{
    DTMFunctionTypes *functionTypes = (DTMFunctionTypes *)malloc(sizeof(DTMFunctionTypes));
    if (!functionTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return functionTypes;
}

DTMGenerics *createDTMGenerics(void)
{
    DTMGenerics *generics = (DTMGenerics *)malloc(sizeof(DTMGenerics));
    if (!generics)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generics\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return generics;
}

DTMEnums *createDTMEnums(void)
{
    DTMEnums *enums = (DTMEnums *)malloc(sizeof(DTMEnums));
    if (!enums)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Enums\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    return enums;
}
