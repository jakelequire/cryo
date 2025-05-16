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
#include "frontend/frontendSymbolTable.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Function Data Types ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMFunctionTypes_createFunctionTemplate(void)
{
    DTFunctionTy *function = createDTFunctionTy();
    return DTM->dataTypes->wrapFunctionType(function);
}

DataType *DTMFunctionTypes_createFunctionType(DataType **paramTypes, int paramCount, DataType *returnType)
{
    DTFunctionTy *function = createDTFunctionTy();
    if (!function)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Type\n");
        CONDITION_FAILED;
    }

    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);

    DataType *functionType = DTM->dataTypes->wrapFunctionType(function);
    functionType->debug->printType(functionType);

    return functionType;
}

DataType *DTMFunctionTypes_createMethodType(const char *methodName, DataType *returnType, DataType **paramTypes, int paramCount)
{
    DTFunctionTy *function = createDTFunctionTy();
    function->setParams(function, paramTypes, paramCount);
    function->setReturnType(function, returnType);
    DataType *methodType = DTM->dataTypes->wrapFunctionType(function);
    methodType->setTypeName(methodType, methodName);

    return methodType;
}

DTMFunctionTypes *createDTMFunctionTypes(void)
{
    DTMFunctionTypes *functionTypes = (DTMFunctionTypes *)malloc(sizeof(DTMFunctionTypes));
    if (!functionTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Function Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    functionTypes->createFunctionTemplate = DTMFunctionTypes_createFunctionTemplate;
    functionTypes->createFunctionType = DTMFunctionTypes_createFunctionType;
    functionTypes->createMethodType = DTMFunctionTypes_createMethodType;

    return functionTypes;
}
