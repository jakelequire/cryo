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
// ------------------------------------ Simple Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTSimpleTy *createDTSimpleTy(void)
{
    DTSimpleTy *simpleType = (DTSimpleTy *)malloc(sizeof(DTSimpleTy));
    if (!simpleType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTSimpleTy\n");
        CONDITION_FAILED;
    }

    return simpleType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Array Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTArrayTy_addElement(struct DTArrayTy_t *arrayType, DataType *element)
{
    if (arrayType->elementCount >= arrayType->elementCapacity)
    {
        arrayType->elementCapacity *= DYN_GROWTH_FACTOR;
        arrayType->elements = (DataType **)realloc(arrayType->elements, sizeof(DataType *) * arrayType->elementCapacity);
        if (!arrayType->elements)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTArrayTy elements\n");
            CONDITION_FAILED;
        }
    }

    arrayType->elements[arrayType->elementCount++] = element;
}

void DTArrayTy_removeElement(struct DTArrayTy_t *arrayType, DataType *element)
{
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        if (arrayType->elements[i] == element)
        {
            for (int j = i; j < arrayType->elementCount - 1; j++)
            {
                arrayType->elements[j] = arrayType->elements[j + 1];
            }

            arrayType->elementCount--;
            break;
        }
    }
}

void DTArrayTy_resize(struct DTArrayTy_t *arrayType)
{
    arrayType->elementCapacity *= DYN_GROWTH_FACTOR;
    arrayType->elements = (DataType **)realloc(arrayType->elements, sizeof(DataType *) * arrayType->elementCapacity);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTArrayTy elements\n");
        CONDITION_FAILED;
    }
}

void DTArrayTy_reset(struct DTArrayTy_t *arrayType)
{
    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_FIELD_CAPACITY;

    arrayType->freeData(arrayType);
    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * arrayType->elementCapacity);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy elements\n");
        CONDITION_FAILED;
    }
}

void DTArrayTy_free(struct DTArrayTy_t *arrayType)
{
    free(arrayType->elements);
    free(arrayType);
}

void DTArrayTy_freeData(struct DTArrayTy_t *arrayType)
{
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        arrayType->elements[i]->free(arrayType->elements[i]);
    }
}

void DTArrayTy_printArray(struct DTArrayTy_t *arrayType)
{
    printf("[");
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        arrayType->elements[i]->debug->printType(arrayType->elements[i]);
        if (i < arrayType->elementCount - 1)
        {
            printf(", ");
        }
    }
    printf("]");
}

DTArrayTy *createDTArrayTy(void)
{
    DTArrayTy *arrayType = (DTArrayTy *)malloc(sizeof(DTArrayTy));
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy\n");
        CONDITION_FAILED;
    }

    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy elements\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_FIELD_CAPACITY;
    arrayType->dimensions = 0;
    arrayType->baseType = NULL;

    // ==================== [ Function Assignments ] ==================== //

    arrayType->addElement = DTArrayTy_addElement;
    arrayType->removeElement = DTArrayTy_removeElement;
    arrayType->resize = DTArrayTy_resize;
    arrayType->reset = DTArrayTy_reset;
    arrayType->free = DTArrayTy_free;
    arrayType->freeData = DTArrayTy_freeData;
    arrayType->printArray = DTArrayTy_printArray;

    return arrayType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Enum Data Types ---------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTEnumTy_addValue(DTEnumTy *enumType, ASTNode *value)
{
    if (enumType->valueCount >= enumType->valueCapacity)
    {
        enumType->valueCapacity *= DYN_GROWTH_FACTOR;
        enumType->values = (ASTNode **)realloc(enumType->values, sizeof(ASTNode *) * enumType->valueCapacity);
        if (!enumType->values)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTEnumTy values\n");
            CONDITION_FAILED;
        }
    }

    enumType->values[enumType->valueCount++] = value;
}

void DTEnumTy_setValues(DTEnumTy *enumType, ASTNode **values, int valueCount)
{
    if (enumType->valueCount == 0)
    {
        enumType->values = values;
        enumType->valueCount = valueCount;
    }
}

DTEnumTy *createDTEnumTy(void)
{
    DTEnumTy *enumType = (DTEnumTy *)malloc(sizeof(DTEnumTy));
    if (!enumType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTEnumTy\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    enumType->values = (ASTNode **)malloc(sizeof(ASTNode *) * MAX_ENUM_CAPACITY);
    if (!enumType->values)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTEnumTy values\n");
        CONDITION_FAILED;
    }
    enumType->name = NULL;
    enumType->valueCount = 0;
    enumType->valueCapacity = MAX_ENUM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    enumType->addValue = DTEnumTy_addValue;
    enumType->setValues = DTEnumTy_setValues;
}

// ------------------------------------------------------------------------------------------- //
// ----------------------------------- Property Data Type ------------------------------------ //
// ------------------------------------------------------------------------------------------- //

void DTProperty_setStatic(DTPropertyTy *property, bool isStatic)
{
    property->isStatic = isStatic;
}

void DTProperty_setConst(DTPropertyTy *property, bool isConst)
{
    property->isConst = isConst;
}

void DTProperty_setPublic(DTPropertyTy *property, bool isPublic)
{
    property->isPublic = isPublic;
}

void DTProperty_setPrivate(DTPropertyTy *property, bool isPrivate)
{
    property->isPrivate = isPrivate;
}

void DTProperty_setProtected(DTPropertyTy *property, bool isProtected)
{
    property->isProtected = isProtected;
}

void DTProperty_setType(DTPropertyTy *property, DataType *type)
{
    property->type = type;
}

void DTProperty_setName(DTPropertyTy *property, const char *name)
{
    property->name = name;
}

DTPropertyTy *createDTProperty(void)
{
    DTPropertyTy *property = (DTPropertyTy *)malloc(sizeof(DTPropertyTy));
    if (!property)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTProperty\n");
        CONDITION_FAILED;
    }

    property->name = NULL;
    property->type = DTM->primitives->createUndefined();
    property->node = (ASTNode *)malloc(sizeof(ASTNode));
    property->isStatic = false;
    property->isConst = false;
    property->isPublic = false;
    property->isPrivate = false;
    property->isProtected = false;

    // ==================== [ Function Assignments ] ==================== //

    property->setStatic = DTProperty_setStatic;
    property->setConst = DTProperty_setConst;
    property->setPublic = DTProperty_setPublic;
    property->setPrivate = DTProperty_setPrivate;
    property->setProtected = DTProperty_setProtected;
    property->setType = DTProperty_setType;
    property->setName = DTProperty_setName;

    return property;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Type Container ----------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

TypeContainer *createTypeContainer(void)
{
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate TypeContainer\n");
        CONDITION_FAILED;
    }

    return container;
}
