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
// ----------------------------------- Debugging Functions ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTDebug_printType(struct DataType_t *type)
{
    DTM->debug->printDataType(type);
}

void DTDebug_printVerbosType(struct DataType_t *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    printf(">------------------------- [ Verbose Data Type ] -------------------------<\n");
    printf("Data Type Name: %s\n", type->typeName);
    printf("Const: %s\n", type->isConst ? "true" : "false");
    printf("Pointer: %s\n", type->isPointer ? "true" : "false");
    printf("Reference: %s\n", type->isReference ? "true" : "false");
    printf(">-------------------------------------------------------------------------<\n");
}

const char *DTDebug_toString(struct DataType_t *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
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
