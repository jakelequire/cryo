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

#define MAX_PARAM_CAPACITY 16
#define MAX_FIELD_CAPACITY 16
#define MAX_METHOD_CAPACITY 16
#define MAX_ENUM_CAPACITY 16
#define MAX_GENERIC_CAPACITY 16
#define MAX_TYPE_CAPACITY 16

#define DYN_GROWTH_FACTOR 2

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Array Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTArrayTy *createDTArrayTy(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Enum Data Types ---------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTEnumTy *createDTEnumTy(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Function Data Types ------------------------------------------ //
// --------------------------------------------------------------------------------------------------- //

void DTFunctionTy_addParam(DTFunctionTy *function, DataType *paramType)
{
    if (function->paramCount >= function->paramCapacity)
    {
        function->paramCapacity *= DYN_GROWTH_FACTOR;
        function->paramTypes = (DataType **)realloc(function->paramTypes, sizeof(DataType *) * function->paramCapacity);
        if (!function->paramTypes)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTFunctionTy paramTypes\n");
            CONDITION_FAILED;
        }
    }

    function->paramTypes[function->paramCount++] = paramType;
}

void DTFunctionTy_setParams(DTFunctionTy *function, DataType **paramTypes, int paramCount)
{
    function->paramTypes = paramTypes;
    function->paramCount = paramCount;
}

void DTFunctionTy_setReturnType(DTFunctionTy *function, DataType *returnType)
{
    function->returnType = returnType;
}

DTFunctionTy *createDTFunctionTy(void)
{
    DTFunctionTy *function = (DTFunctionTy *)malloc(sizeof(DTFunctionTy));
    if (!function)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTFunctionTy\n");
        CONDITION_FAILED;
    }

    function->paramTypes = (DataType **)malloc(sizeof(DataType *) * MAX_PARAM_CAPACITY);
    if (!function->paramTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTFunctionTy paramTypes\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    function->returnType = NULL;
    function->paramCount = 0;
    function->paramCapacity = MAX_PARAM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    function->addParam = DTFunctionTy_addParam;
    function->setParams = DTFunctionTy_setParams;
    function->setReturnType = DTFunctionTy_setReturnType;

    return function;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Simple Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTSimpleTy *createDTSimpleTy(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Struct Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTStructTy *createDTStructTy(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Class Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTClassTy *createDTClassTy(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Object Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTObjectType *createDTObjectType(void)
{
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Type Container ----------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

TypeContainer *createTypeContainer(void)
{
}
