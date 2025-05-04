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

DataType *DTMArrayTypes_createDynamicArrayType(DataType **arrayTypes, int elementCount)
{
    DTArrayTy *arrayType = createDTArrayTy();
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Dynamic Array Type\n");
        CONDITION_FAILED;
    }

    arrayType->baseType = NULL;
    arrayType->elements = arrayTypes;
    arrayType->dimensions = 0;
    arrayType->size = 0;
    arrayType->isMonomorphic = false;
    arrayType->isDynamic = true;
    arrayType->elementCount = elementCount;

    DataType *arrayDataType = DTM->dataTypes->wrapArrayType(arrayType);
    const char *arrayDataTypeName = arrayType->createArrayTypeString(arrayType);
    if (!arrayDataTypeName)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create array type string\n");
        CONDITION_FAILED;
    }
    arrayDataType->setTypeName(arrayDataType, strdup(arrayDataTypeName));
    arrayDataType->isArray = true;
    return arrayDataType;
}

DataType *DTMArrayTypes_createMonomorphicArrayType(DataType *baseType, int elementCount)
{
    DTArrayTy *arrayType = createDTArrayTy();
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Monomorphic Array Type\n");
        CONDITION_FAILED;
    }

    arrayType->baseType = baseType;
    arrayType->dimensions = 0;
    arrayType->size = elementCount;
    arrayType->isMonomorphic = true;
    arrayType->elementCount = elementCount;
    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * elementCount);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Monomorphic Array Type elements\n");
        CONDITION_FAILED;
    }
    for (int i = 0; i < elementCount; i++)
    {
        arrayType->elements[i] = baseType;
    }

    DataType *arrayDataType = DTM->dataTypes->wrapArrayType(arrayType);
    const char *arrayDataTypeName = arrayType->createArrayTypeString(arrayType);
    if (!arrayDataTypeName)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create array type string\n");
        CONDITION_FAILED;
    }
    arrayDataType->setTypeName(arrayDataType, strdup(arrayDataTypeName));
    arrayDataType->isArray = true;
    return arrayDataType;
}

DataType *DTMArrayTypes_createMonomorphicArrayTypeDecl(DataType *baseType)
{
    DTArrayTy *arrayType = createDTArrayTy();
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Monomorphic Array Type Declaration\n");
        CONDITION_FAILED;
    }

    arrayType->baseType = baseType;
    arrayType->dimensions = 0;
    arrayType->size = 0;
    arrayType->isMonomorphic = true;
    arrayType->isDynamic = true;
    arrayType->elementCount = 0;
    arrayType->elements = NULL;

    DataType *arrayDataType = DTM->dataTypes->wrapArrayType(arrayType);
    const char *arrayDataTypeName = arrayType->createArrayTypeString(arrayType);
    if (!arrayDataTypeName)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create array type string\n");
        CONDITION_FAILED;
    }
    arrayDataType->setTypeName(arrayDataType, strdup(arrayDataTypeName));
    arrayDataType->isArray = true;
    return arrayDataType;
}

DTMArrayTypes *createDTMArrayTypes(void)
{
    DTMArrayTypes *arrayTypes = (DTMArrayTypes *)malloc(sizeof(DTMArrayTypes));
    if (!arrayTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Array Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    arrayTypes->createDynArray = DTMArrayTypes_createDynamicArrayType;
    arrayTypes->createMonomorphicArray = DTMArrayTypes_createMonomorphicArrayType;
    arrayTypes->createMonomorphicArrayDecl = DTMArrayTypes_createMonomorphicArrayTypeDecl;

    return arrayTypes;
}
