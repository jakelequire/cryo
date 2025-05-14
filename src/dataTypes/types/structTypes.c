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
#include "tools/logger/logger_config.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Struct Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTStructTy_addProperty(DTStructTy *structType, DTPropertyTy *property)
{
    if (structType->propertyCount >= structType->propertyCapacity)
    {
        structType->propertyCapacity *= DYN_GROWTH_FACTOR;
        structType->properties = (struct DTPropertyTy_t **)realloc(structType->properties, sizeof(struct DTPropertyTy_t *) * structType->propertyCapacity);
        if (!structType->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy properties\n");
            CONDITION_FAILED;
        }
    }

    structType->properties[structType->propertyCount++] = property;
}

void DTStructTy_addMethod(DTStructTy *structType, DataType *method)
{
    if (structType->methodCount >= structType->methodCapacity)
    {
        structType->methodCapacity *= DYN_GROWTH_FACTOR;
        structType->methods = (DataType **)realloc(structType->methods, sizeof(DataType *) * structType->methodCapacity);
        if (!structType->methods)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy methods\n");
            fprintf(stderr, "Method Count: %d, Method Capacity: %d\n", structType->methodCount, structType->methodCapacity);
            fprintf(stderr, "Struct Name: %s\n", structType->name);
            DEBUG_PRINT_FILTER({
                DTM->symbolTable->printTable(DTM->symbolTable);
            });
            CONDITION_FAILED;
        }
    }

    structType->methods[structType->methodCount++] = method;
}

void DTStructTy_addCtorParam(DTStructTy *structType, DataType *param)
{
    if (structType->ctorParamCount >= structType->ctorParamCapacity)
    {
        structType->ctorParamCapacity *= DYN_GROWTH_FACTOR;
        structType->ctorParams = (DataType **)realloc(structType->ctorParams, sizeof(DataType *) * structType->ctorParamCapacity);
        if (!structType->ctorParams)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy ctorParams\n");
            CONDITION_FAILED;
        }
    }

    structType->ctorParams[structType->ctorParamCount++] = param;
}

void DTStructTy_addGenericParam(DTStructTy *structType, struct DTGenericTy_t *param)
{
    if (structType->generic.paramCount >= structType->generic.paramCapacity)
    {
        structType->generic.paramCapacity *= DYN_GROWTH_FACTOR;
        structType->generic.params = (struct DTGenericTy_t **)realloc(structType->generic.params, sizeof(struct DTGenericTy_t *) * structType->generic.paramCapacity);
        if (!structType->generic.params)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy generic params\n");
            CONDITION_FAILED;
        }
    }

    structType->generic.params[structType->generic.paramCount++] = param;
}

struct DTStructTy_t *DTStructTy_substituteGenericType(struct DTStructTy_t *structType, struct DTGenericTy_t *DTGenericType, struct DataType_t *substituteType)
{
    for (int i = 0; i < structType->propertyCount; i++)
    {
        if (structType->properties[i]->type->container->typeOf == GENERIC_TYPE)
        {
            struct DTGenericTy_t *generic = structType->properties[i]->type->container->type.genericType;
            if (generic == DTGenericType)
            {
                structType->properties[i]->type = substituteType;
            }
        }
    }

    for (int i = 0; i < structType->methodCount; i++)
    {
        if (structType->methods[i]->container->typeOf == GENERIC_TYPE)
        {
            struct DTGenericTy_t *generic = structType->methods[i]->container->type.genericType;
            if (generic == DTGenericType)
            {
                structType->methods[i] = substituteType;
            }
        }
    }

    return structType;
}

struct DTStructTy_t *DTStructTy_cloneAndSubstituteGenericMethod(struct DTStructTy_t *structType, struct DTGenericTy_t *DTGenericType, struct DataType_t *substituteType)
{
    for (int i = 0; i < structType->methodCount; i++)
    {
        if (structType->methods[i]->container->typeOf == GENERIC_TYPE)
        {
            struct DTGenericTy_t *generic = structType->methods[i]->container->type.genericType;
            if (generic == DTGenericType)
            {
                structType->methods[i] = substituteType;
            }
        }
    }

    return structType;
}

struct DTStructTy_t *DTStructTy_cloneAndSubstituteGenericParam(struct DTStructTy_t *structType, struct DTGenericTy_t *DTGenericType, struct DataType_t *substituteType)
{
    for (int i = 0; i < structType->propertyCount; i++)
    {
        if (structType->properties[i]->type->container->typeOf == GENERIC_TYPE)
        {
            struct DTGenericTy_t *generic = structType->properties[i]->type->container->type.genericType;
            if (generic == DTGenericType)
            {
                structType->properties[i]->type = substituteType;
            }
        }
    }

    return structType;
}

DTStructTy *createDTStructTy(void)
{
    DTStructTy *structType = (DTStructTy *)malloc(sizeof(DTStructTy));
    if (!structType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTStructTy\n");
        CONDITION_FAILED;
    }

    structType->properties = (DTPropertyTy **)malloc(sizeof(DTPropertyTy *) * MAX_FIELD_CAPACITY);
    if (!structType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTStructTy properties\n");
        CONDITION_FAILED;
    }

    structType->methods = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!structType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTStructTy methods\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    structType->name = NULL;
    structType->propertyCount = 0;
    structType->propertyCapacity = MAX_FIELD_CAPACITY;

    structType->methodCount = 0;
    structType->methodCapacity = MAX_FIELD_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    structType->addProperty = DTStructTy_addProperty;
    structType->addMethod = DTStructTy_addMethod;
    structType->addCtorParam = DTStructTy_addCtorParam;

    // Generic Struct Methods
    structType->addGenericParam = DTStructTy_addGenericParam;
    structType->substituteGenericType = DTStructTy_substituteGenericType;
    structType->cloneAndSubstituteGenericMethod = DTStructTy_cloneAndSubstituteGenericMethod;
    structType->cloneAndSubstituteGenericParam = DTStructTy_cloneAndSubstituteGenericParam;

    return structType;
}
