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
#include "dataTypes/dataTypeManager.h"

DataType *createObjectType(DataType **properties, const char **propNames, int count)
{
    __STACK_FRAME__
    if (!properties || count <= 0)
    {
        logMessage(LMI, "INFO", "DataTypes", "No properties to add to object.");
        return NULL;
    }

    DataType *objectType = (DataType *)malloc(sizeof(DataType));
    if (!objectType)
    {
        logMessage(LME, "ERROR", "DataTypes", "Failed to allocate object type.");
        return NULL;
    }

    objectType->container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!objectType->container)
    {
        logMessage(LME, "ERROR", "DataTypes", "Failed to allocate object type container.");
        return NULL;
    }

    objectType->container->properties = (DTMDynamicTuple *)malloc(sizeof(DTMDynamicTuple));
    if (!objectType->container->properties)
    {
        logMessage(LME, "ERROR", "DataTypes", "Failed to allocate object type properties.");
        return NULL;
    }

    objectType->container->properties->count = 0;
    objectType->container->properties->capacity = MAX_DYNAMIC_ARRAY_CAPACITY;
    objectType->container->properties->values = (DataType **)malloc(sizeof(DataType *) * objectType->container->properties->capacity);
    objectType->container->properties->keys = (const char **)malloc(sizeof(const char *) * objectType->container->properties->capacity);

    for (int i = 0; i < count; i++)
    {
        objectType->container->properties->add(objectType->container->properties, properties[i], propNames[i]);
    }

    objectType->container->baseType = OBJECT_TYPE;
    objectType->container->primitive = PRIM_OBJECT;

    return objectType;
}
