/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
// ------------------------------------ Object Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTObjectType_addProperty(struct DTObjectTy_t *objectType, DataType *property)
{
    if (objectType->propertyCount >= objectType->propertyCapacity)
    {
        objectType->_resizeProperties(objectType);
    }

    objectType->properties[objectType->propertyCount++] = property;
}

void DTObjectType_addMethod(struct DTObjectTy_t *objectType, DataType *method)
{
    if (objectType->methodCount >= objectType->methodCapacity)
    {
        objectType->_resizeMethods(objectType);
    }

    objectType->methods[objectType->methodCount++] = method;
}

void DTObjectType_resizeMethods(struct DTObjectTy_t *objectType)
{
    objectType->methodCapacity *= DYN_GROWTH_FACTOR;
    objectType->methods = (DataType **)realloc(objectType->methods, sizeof(DataType *) * objectType->methodCapacity);
    if (!objectType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTObjectType methods\n");
        CONDITION_FAILED;
    }
}

void DTObjectType_resizeProperties(struct DTObjectTy_t *objectType)
{
    objectType->propertyCapacity *= DYN_GROWTH_FACTOR;
    objectType->properties = (DataType **)realloc(objectType->properties, sizeof(DataType *) * objectType->propertyCapacity);
    if (!objectType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTObjectType properties\n");
        CONDITION_FAILED;
    }
}

struct DTObjectTy_t *createDTObjectType(void)
{
    struct DTObjectTy_t *objectType = (struct DTObjectTy_t *)malloc(sizeof(struct DTObjectTy_t));
    if (!objectType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    objectType->properties = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!objectType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType properties\n");
        CONDITION_FAILED;
    }

    objectType->methods = (DataType **)malloc(sizeof(DataType *) * MAX_METHOD_CAPACITY);
    if (!objectType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType methods\n");
        CONDITION_FAILED;
    }

    objectType->name = NULL;
    objectType->propertyCount = 0;
    objectType->propertyCapacity = MAX_FIELD_CAPACITY;
    objectType->methodCount = 0;
    objectType->methodCapacity = MAX_METHOD_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    objectType->addProperty = DTObjectType_addProperty;
    objectType->addMethod = DTObjectType_addMethod;

    objectType->_resizeMethods = DTObjectType_resizeMethods;
    objectType->_resizeProperties = DTObjectType_resizeProperties;

    return objectType;
}
