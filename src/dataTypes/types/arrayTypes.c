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

// Add a new dimension to the ArrayDimensions structure
void ArrayDimensions_addDimension(ArrayDimensions *self, int dimensionSize)
{
    if (self == NULL)
    {
        return;
    }

    // Initialize arrays if this is the first dimension
    if (self->dimensionCount == 0)
    {
        int initialCapacity = 4; // Start with space for 4 dimensions

        self->dimensionSizes = (int *)malloc(sizeof(int) * initialCapacity);
        self->dimensions = (int *)malloc(sizeof(int) * initialCapacity);
        self->dimensionCapacities = (int *)malloc(sizeof(int) * initialCapacity);
        self->baseTypes = (DataType **)malloc(sizeof(DataType *) * initialCapacity);

        // Initialize all capacity values
        for (int i = 0; i < initialCapacity; i++)
        {
            self->dimensionCapacities[i] = 0;
        }
    }
    else if (self->dimensionCount >= self->dimensionCapacities[0])
    {
        // Need to resize
        self->resizeDimensions(self);
    }

    // Add the new dimension
    self->dimensionSizes[self->dimensionCount] = dimensionSize;
    self->dimensions[self->dimensionCount] = 0;   // Current index in this dimension
    self->baseTypes[self->dimensionCount] = NULL; // To be set later

    self->dimensionCount++;
}

// Resize the dimension arrays when needed
void ArrayDimensions_resizeDimensions(ArrayDimensions *self)
{
    if (self == NULL || self->dimensionCount == 0)
    {
        return;
    }

    int newCapacity = self->dimensionCapacities[0] == 0 ? 4 : self->dimensionCapacities[0] * 2;

    // Reallocate all arrays
    self->dimensionSizes = (int *)realloc(self->dimensionSizes, sizeof(int) * newCapacity);
    self->dimensions = (int *)realloc(self->dimensions, sizeof(int) * newCapacity);
    self->dimensionCapacities = (int *)realloc(self->dimensionCapacities, sizeof(int) * newCapacity);
    self->baseTypes = (DataType **)realloc(self->baseTypes, sizeof(DataType *) * newCapacity);

    // Update the capacity value for all existing and new entries
    for (int i = 0; i < newCapacity; i++)
    {
        if (i >= self->dimensionCapacities[0])
        {
            self->dimensionCapacities[i] = newCapacity;
        }
    }

    // Update the first capacity entry
    self->dimensionCapacities[0] = newCapacity;
}

ArrayDimensions *createArrayDimensions()
{
    ArrayDimensions *arrayDimensions = (ArrayDimensions *)malloc(sizeof(ArrayDimensions));
    if (!arrayDimensions)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Array Dimensions\n");
        CONDITION_FAILED;
    }

    arrayDimensions->dimensionCount = 0;
    arrayDimensions->baseTypes = NULL;
    arrayDimensions->dimensionSizes = NULL;
    arrayDimensions->dimensions = NULL;
    arrayDimensions->dimensionCapacities = NULL;

    arrayDimensions->addDimension = ArrayDimensions_addDimension;
    arrayDimensions->resizeDimensions = ArrayDimensions_resizeDimensions;

    return arrayDimensions;
}

ArrayDimensions *createArrayDimensionsWithData(DataType **baseTypes, int *sizes, int dimensionCount)
{
    ArrayDimensions *arrayDimensions = createArrayDimensions();
    if (!arrayDimensions)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Array Dimensions\n");
        CONDITION_FAILED;
    }

    arrayDimensions->dimensionCount = dimensionCount;
    arrayDimensions->baseTypes = baseTypes;
    arrayDimensions->dimensionSizes = sizes;
    arrayDimensions->dimensions = (int *)malloc(sizeof(int) * dimensionCount);
    if (!arrayDimensions->dimensions)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Array Dimensions\n");
        CONDITION_FAILED;
    }
    memcpy(arrayDimensions->dimensions, sizes, sizeof(int) * dimensionCount);

    return arrayDimensions;
}

DataType *DTMArrayTypes_createDynArrayType(DataType **arrayTypes, int elementCount)
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

DataType *DTMArrayTypes_createDynamicArrayType(DataType *baseType)
{
    DTArrayTy *arrayType = createDTArrayTy();
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Dynamic Array Type\n");
        CONDITION_FAILED;
    }

    arrayType->baseType = baseType;
    arrayType->dimensions = 0;
    arrayType->size = 0;
    arrayType->isMonomorphic = false;
    arrayType->isDynamic = true;
    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_ARRAY_CAPACITY;

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
    arrayType->isDynamic = true;
    arrayType->elementCapacity = elementCount;
    arrayType->elementCount = elementCount <= MAX_ARRAY_CAPACITY ? elementCount : MAX_ARRAY_CAPACITY;
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
    arrayType->dimensionCount = 1;
    arrayType->size = 0;
    arrayType->isMonomorphic = true;
    arrayType->isDynamic = true;
    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_ARRAY_CAPACITY;
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

DataType *DTMArrayTypes_createStaticArrayType(DataType *baseType, int size)
{
    DTArrayTy *arrayType = createDTArrayTy();
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Static Array Type\n");
        CONDITION_FAILED;
    }

    arrayType->baseType = baseType;
    arrayType->dimensions = 0;
    arrayType->size = size;
    arrayType->isMonomorphic = true;
    arrayType->isDynamic = false;
    arrayType->elementCount = 0;
    arrayType->elementCapacity = size;
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

DataType *DTMArrayTypes_createMultiDimensionalStaticArray(DataType *baseType, int *sizes, int dimensionCount)
{
    if (dimensionCount <= 0 || baseType == NULL || sizes == NULL)
    {
        return NULL;
    }

    // Create array type starting from the innermost dimension (right to left)
    DataType *resultType = baseType;

    for (int i = dimensionCount - 1; i >= 0; i--)
    {
        // Create a static array for this dimension
        resultType = DTM->arrayTypes->createStaticArray(resultType, sizes[i]);

        if (resultType == NULL)
        {
            return NULL;
        }

        // For the outermost dimension, set up the full dimensions information
        if (i == 0 && resultType != NULL && resultType->container != NULL)
        {
            DTArrayTy *arrayTy = resultType->container->type.arrayType;

            // Initialize dimensions array if needed
            if (arrayTy->dimensions == NULL)
            {
                arrayTy->dimensions = createArrayDimensions();
                arrayTy->dimensionCount = dimensionCount;

                // Add each dimension
                for (int j = 0; j < dimensionCount; j++)
                {
                    arrayTy->dimensions->addDimension(arrayTy->dimensions, sizes[j]);
                }
            }
        }
    }

    return resultType;
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

    arrayTypes->createDynArray = DTMArrayTypes_createDynArrayType;
    arrayTypes->createDynamicArray = DTMArrayTypes_createDynamicArrayType;
    arrayTypes->createMonomorphicArray = DTMArrayTypes_createMonomorphicArrayType;
    arrayTypes->createMonomorphicArrayDecl = DTMArrayTypes_createMonomorphicArrayTypeDecl;
    arrayTypes->createStaticArray = DTMArrayTypes_createStaticArrayType;
    arrayTypes->createMultiDimensionalStaticArray = DTMArrayTypes_createMultiDimensionalStaticArray;

    return arrayTypes;
}
