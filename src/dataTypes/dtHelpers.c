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
// ------------------------------------ Dynamic Array Implementation --------------------------------- //
// --------------------------------------------------------------------------------------------------- //

#define MAX_DYNAMIC_ARRAY_CAPACITY 16

void DTMDynamicTypeArray_add(DTMDynamicTypeArray *array, DataType *type)
{
    if (array->count >= array->capacity)
    {
        array->resize(array);
    }

    array->data[array->count++] = type;
}

void DTMDynamicTypeArray_remove(DTMDynamicTypeArray *array, DataType *type)
{
    for (int i = 0; i < array->count; i++)
    {
        if (array->data[i] == type)
        {
            for (int j = i; j < array->count - 1; j++)
            {
                array->data[j] = array->data[j + 1];
            }

            array->count--;
            break;
        }
    }
}

void DTMDynamicTypeArray_resize(DTMDynamicTypeArray *array)
{
    array->capacity *= 2;
    array->data = (DataType **)realloc(array->data, sizeof(DataType *) * array->capacity);
    if (!array->data)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTMDynamicTypeArray\n");
        CONDITION_FAILED;
    }
}

void DTMDynamicTypeArray_reset(DTMDynamicTypeArray *array)
{
    array->count = 0;
    array->capacity = MAX_DYNAMIC_ARRAY_CAPACITY;

    array->freeData(array);
    array->data = (DataType **)malloc(sizeof(DataType *) * array->capacity);
    if (!array->data)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTypeArray data\n");
        CONDITION_FAILED;
    }
}

void DTMDynamicTypeArray_freeData(DTMDynamicTypeArray *array)
{
    free(array->data);
}

void DTMDynamicTypeArray_free(DTMDynamicTypeArray *array)
{
    free(array->data);
    free(array);
}

void DTMDynamicTypeArray_printArray(DTMDynamicTypeArray *array)
{
    char *typeStr = (char *)malloc(1024);
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for type string\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < array->count; i++)
    {
        DataType *type = array->data[i];
        sprintf(typeStr, "%s%s", typeStr, type->debug->toString(type));
        if (i < array->count - 1)
        {
            sprintf(typeStr, "%s, ", typeStr);
        }
    }

    printf("%s\n", typeStr);
    free(typeStr);
}

DTMDynamicTypeArray *createDTMDynamicTypeArray(void)
{
    DTMDynamicTypeArray *array = (DTMDynamicTypeArray *)malloc(sizeof(DTMDynamicTypeArray));
    if (!array)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTypeArray\n");
        CONDITION_FAILED;
    }

    array->data = (DataType **)malloc(sizeof(DataType *) * 16);
    if (!array->data)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTypeArray data\n");
        CONDITION_FAILED;
    }

    array->count = 0;
    array->capacity = MAX_DYNAMIC_ARRAY_CAPACITY;

    array->add = DTMDynamicTypeArray_add;
    array->remove = DTMDynamicTypeArray_remove;
    array->resize = DTMDynamicTypeArray_resize;
    array->reset = DTMDynamicTypeArray_reset;
    array->free = DTMDynamicTypeArray_freeData;
    array->freeData = DTMDynamicTypeArray_free;

    array->printArray = DTMDynamicTypeArray_printArray;

    return array;
}

// --------------------------------------------------------------------------------------------------- //
// ----------------------------------- Dynamic Tuple Implementation ---------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDynamicTuple_add(DTMDynamicTuple *tuple, const char *key, DataType *value)
{
    if (tuple->count >= tuple->capacity)
    {
        tuple->resize(tuple);
    }

    tuple->keys[tuple->count] = key;
    tuple->values[tuple->count++] = value;
}

void DTMDynamicTuple_remove(DTMDynamicTuple *tuple, const char *key)
{
    for (int i = 0; i < tuple->count; i++)
    {
        if (strcmp(tuple->keys[i], key) == 0)
        {
            for (int j = i; j < tuple->count - 1; j++)
            {
                tuple->keys[j] = tuple->keys[j + 1];
                tuple->values[j] = tuple->values[j + 1];
            }

            tuple->count--;
            break;
        }
    }
}

void DTMDynamicTuple_resize(DTMDynamicTuple *tuple)
{
    tuple->capacity *= 2;
    tuple->values = (DataType **)realloc(tuple->values, sizeof(DataType *) * tuple->capacity);
    if (!tuple->values)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTMDynamicTuple values\n");
        CONDITION_FAILED;
    }

    tuple->keys = (const char **)realloc(tuple->keys, sizeof(const char *) * tuple->capacity);
    if (!tuple->keys)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTMDynamicTuple keys\n");
        CONDITION_FAILED;
    }
}

void DTMDynamicTuple_reset(DTMDynamicTuple *tuple)
{
    tuple->count = 0;
    tuple->capacity = MAX_DYNAMIC_ARRAY_CAPACITY;

    tuple->free(tuple);
    tuple->values = (DataType **)malloc(sizeof(DataType *) * tuple->capacity);
    if (!tuple->values)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTuple values\n");
        CONDITION_FAILED;
    }

    tuple->keys = (const char **)malloc(sizeof(const char *) * tuple->capacity);
    if (!tuple->keys)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTuple keys\n");
        CONDITION_FAILED;
    }
}

void DTMDynamicTuple_free(DTMDynamicTuple *tuple)
{
    free(tuple->values);
    free(tuple->keys);
    free(tuple);
}

void DTMDynamicTuple_printTuple(DTMDynamicTuple *tuple)
{
    char *typeStr = (char *)malloc(1024);
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for type string\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < tuple->count; i++)
    {
        DataType *type = tuple->values[i];
        sprintf(typeStr, "%s%s: %s", typeStr, tuple->keys[i], type->debug->toString(type));
        if (i < tuple->count - 1)
        {
            sprintf(typeStr, "%s, ", typeStr);
        }
    }

    printf("%s\n", typeStr);
    free(typeStr);
}

DTMDynamicTuple *createDTMDynamicTuple(void)
{
    DTMDynamicTuple *tuple = (DTMDynamicTuple *)malloc(sizeof(DTMDynamicTuple));
    if (!tuple)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTuple\n");
        CONDITION_FAILED;
    }

    tuple->values = (DataType **)malloc(sizeof(DataType *) * MAX_DYNAMIC_ARRAY_CAPACITY);
    if (!tuple->values)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTuple values\n");
        CONDITION_FAILED;
    }

    tuple->keys = (const char **)malloc(sizeof(const char *) * MAX_DYNAMIC_ARRAY_CAPACITY);
    if (!tuple->keys)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTMDynamicTuple keys\n");
        CONDITION_FAILED;
    }

    tuple->count = 0;
    tuple->capacity = MAX_DYNAMIC_ARRAY_CAPACITY;

    tuple->add = DTMDynamicTuple_add;
    tuple->remove = DTMDynamicTuple_remove;
    tuple->resize = DTMDynamicTuple_resize;
    tuple->reset = DTMDynamicTuple_reset;
    tuple->free = DTMDynamicTuple_free;

    tuple->printTuple = DTMDynamicTuple_printTuple;

    return tuple;
}

DTMHelpers *createDTMHelpers(void)
{
    DTMHelpers *helpers = (DTMHelpers *)malloc(sizeof(DTMHelpers));
    if (!helpers)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Helpers\n");
        CONDITION_FAILED;
    }

    helpers->dynTypeArray = createDTMDynamicTypeArray();
    helpers->dynTuple = createDTMDynamicTuple();

    return helpers;
}
