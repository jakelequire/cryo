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
#include "dataTypes/dataTypes.h"
#include "diagnostics/diagnostics.h"

TypeContainer *createArrayType(TypeContainer *baseType, int dimensions)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    // Copy base type info
    container->baseType = baseType->baseType;
    container->primitive = baseType->primitive;
    container->custom = baseType->custom;

    // Add array info
    container->isArray = true;
    container->arrayDimensions = dimensions;

    return container;
}

/*
typedef struct ArrayType
{
    DataType *baseType;
    int dimensions;
    int length;
    DataType **elements;
    int elementCount;
} ArrayType;
*/

ArrayType *createArrayTypeContainer(DataType *baseType, DataType **elementTypes, int length, int dimensions)
{
    __STACK_FRAME__
    ArrayType *arrayType = (ArrayType *)malloc(sizeof(ArrayType));
    if (!arrayType)
    {
        logMessage(LMI, "ERROR", "DataTypes", "Failed to allocate memory for ArrayType");
        CONDITION_FAILED;
    }
    arrayType->baseType = baseType;
    arrayType->dimensions = dimensions;
    arrayType->length = 0;
    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!arrayType->elements)
    {
        logMessage(LMI, "ERROR", "DataTypes", "Failed to allocate memory for ArrayType elements");
        CONDITION_FAILED;
    }
    arrayType->elementCount = 0;

    return arrayType;
}

DataType *wrapArrayType(ArrayType *arrayType)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    if (!container)
    {
        logMessage(LMI, "ERROR", "DataTypes", "Failed to create type container for array type");
        CONDITION_FAILED;
    }

    container->baseType = ARRAY_TYPE;
    container->custom.arrayDef = arrayType;
    container->isArray = true;
    container->arrayDimensions = arrayType->dimensions;
    container->length = arrayType->length;

    return wrapTypeContainer(container);
}
