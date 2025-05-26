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
#include "frontend/frontendSymbolTable.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Generic Data Types ------------------------------------------ //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMGenerics_createGenericTypeInstance(DataType *genericType, DataType **paramTypes, int paramCount)
{
    DTGenericTy *generic = DTM->generics->createEmptyGenericType();
    generic->paramCount = paramCount;
    generic->dimensions = 0;
    generic->constraint = NULL;
    generic->next = NULL;
    return DTM->dataTypes->wrapGenericType(generic);
}

DTGenericTy *DTMGenerics_createEmptyGenericType(void)
{
    DTGenericTy *generic = (DTGenericTy *)malloc(sizeof(DTGenericTy));
    if (!generic)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generic Type\n");
        CONDITION_FAILED;
    }
    return generic;
}

DTMGenerics *createDTMGenerics(void)
{
    DTMGenerics *generics = (DTMGenerics *)malloc(sizeof(DTMGenerics));
    if (!generics)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Generics\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    generics->createGenericTypeInstance = DTMGenerics_createGenericTypeInstance;
    generics->createEmptyGenericType = DTMGenerics_createEmptyGenericType;

    return generics;
}
