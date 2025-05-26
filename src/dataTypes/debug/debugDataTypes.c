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
#include "tools/logger/logger_config.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ----------------------------------- Debugging Functions ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTDebug_printType(struct DataType_t *type)
{
    DEBUG_PRINT_FILTER({
        DTM->debug->printDataType(type);
    });
}

void DTDebug_printArrayTypeInfo(struct DataType_t *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type (DTDebug_printArrayTypeInfo)\n");
    }

    DTArrayTy *arrayType = type->container->type.arrayType;
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL array type (DTDebug_printArrayTypeInfo)\n");
    }

    /*
        struct DataType_t **elements;
    int elementCount;
    int elementCapacity;
    int dimensions;

    size_t size;
    bool isConst;
    bool isDynamic;
    bool isMonomorphic;
    */

    bool isDynamic = arrayType->isDynamic;
    bool isMonomorphic = arrayType->isMonomorphic;
    int elementCount = arrayType->elementCount;
    int dimensions = arrayType->dimensionCount;
    size_t size = arrayType->size;
    const char *baseTypeStr = DTM->debug->dataTypeToString(arrayType->baseType);

    printf("isDynamic: %s | isMonomorphic: %s\n", isDynamic ? "true" : "false", isMonomorphic ? "true" : "false");
    printf("Element Count: %d | Dimensions: %d | Size: %zu\n", elementCount, dimensions, size);
    printf("Base Type: %s\n", baseTypeStr);
}

void DTDebug_printVerbosType(struct DataType_t *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type (DTDebug_printVerbosType)\n");
    }
    DEBUG_PRINT_FILTER({
        printf(">------------------------- [ Verbose Data Type ] -------------------------<\n");
        printf("Data Type Name: %s\n", type->typeName);
        printf("Const: %s\n", type->isConst ? "true" : "false");
        printf("Pointer: %s\n", type->isPointer ? "true" : "false");
        printf("Reference: %s\n", type->isReference ? "true" : "false");
        printf("Array: %s\n", type->isArray ? "true" : "false");
        printf("TypeOf: %s | Primitive: %s | ObjectType: %s\n",
               DTM->debug->typeofDataTypeToString(type->container->typeOf),
               DTM->debug->primitiveDataTypeToString(type->container->primitive),
               DTM->debug->typeofObjectTypeToString(type->container->objectType));
        printf("isPrototype: %s\n", type->isPrototype ? "true" : "false");
        printf("isGeneric: %s\n", type->isGeneric ? "true" : "false");
        if (type->isArray)
        {
            DTDebug_printArrayTypeInfo(type);
        }
        printf(">-------------------------------------------------------------------------<\n");
    });
}

const char *DTDebug_toString(struct DataType_t *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type (DTDebug_toString)\n");
    }

    const char *typeStr = DTM->debug->dataTypeToString(type);
    return typeStr;
}

DTDebug *createDTDebug(void)
{
    DTDebug *debug = (DTDebug *)malloc(sizeof(DTDebug));
    if (!debug)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTDebug\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    debug->printType = DTDebug_printType;
    debug->printVerbosType = DTDebug_printVerbosType;
    debug->toString = DTDebug_toString;

    return debug;
}
