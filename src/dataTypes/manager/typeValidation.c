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

// TODO: Implement a more robust type validation system.
bool DTMTypeValidation_isSameType(DataType *type1, DataType *type2)
{
    if (!type1 || !type2)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to compare NULL data types\n");
        CONDITION_FAILED;
    }

    return type1->container->primitive == type2->container->primitive;
}

bool DTMTypeValidation_isCompatibleType(DataType *type1, DataType *type2)
{
    if (!type1 || !type2)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to compare NULL data types\n");
        CONDITION_FAILED;
    }

    return type1->container->primitive == type2->container->primitive;
}

bool DTMTypeValidation_isStringType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to compare NULL data type\n");
        CONDITION_FAILED;
    }

    return type->container->primitive == PRIM_STRING;
}

bool DTMTypeValidation_isIdentifierDataType(const char *typeStr)
{
    if (!typeStr)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to compare NULL data type\n");
        CONDITION_FAILED;
    }

    if (cStringCompare(typeStr, "i8") == 0 ||
        cStringCompare(typeStr, "i16") == 0 ||
        cStringCompare(typeStr, "i32") == 0 ||
        cStringCompare(typeStr, "i64") == 0 ||
        cStringCompare(typeStr, "str") == 0 ||
        cStringCompare(typeStr, "char") == 0 ||
        cStringCompare(typeStr, "null") == 0 ||
        cStringCompare(typeStr, "boolean") == 0 ||
        cStringCompare(typeStr, "any") == 0 ||
        cStringCompare(typeStr, "float") == 0 ||
        cStringCompare(typeStr, "void") == 0)
    {
        return true;
    }
    return false;
}

DTMTypeValidation *createDTMTypeValidation(void)
{
    DTMTypeValidation *validation = (DTMTypeValidation *)malloc(sizeof(DTMTypeValidation));
    if (!validation)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Type Validation\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    validation->isSameType = DTMTypeValidation_isSameType;
    validation->isCompatibleType = DTMTypeValidation_isCompatibleType;
    validation->isStringType = DTMTypeValidation_isStringType;
    validation->isIdentifierDataType = DTMTypeValidation_isIdentifierDataType;

    return validation;
}
