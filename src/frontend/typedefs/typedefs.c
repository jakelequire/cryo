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
#include "frontend/typedefs.h"

char *TypeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIMITIVE_TYPE:
        return "PRIMITIVE_TYPE";
    case STRUCT_TYPE:
        return "STRUCT_TYPE";
    case ENUM_TYPE:
        return "ENUM_TYPE";
    case FUNCTION_TYPE:
        return "FUNCTION_TYPE";
    case UNKNOWN_TYPE:
        return "UNKNOWN_TYPE";
    default:
        return "<TYPE UNKNOWN>";
    }
}

CryoDataType parseDataType(const char *typeStr)
{
    printf("Parsing data type: %s\n", typeStr);

    // Check if `[]` is at the end of the type string
    size_t len = strlen(typeStr);
    bool isArray = (len >= 2 && typeStr[len - 2] == '[' && typeStr[len - 1] == ']');

    // Create a copy of typeStr without the '[]' if it's an array
    char *baseTypeStr = strdup(typeStr);
    if (isArray)
    {
        baseTypeStr[len - 2] = '\0';
    }

    // Check the primitive base type
    CryoDataType baseType = getPrimativeTypeFromString(baseTypeStr); // Use baseTypeStr instead of typeStr

    if (baseType == DATA_TYPE_UNKNOWN && !isArray)
    {
        logMessage("ERROR", __LINE__, "TypeDefs", "Unknown data type: %s", typeStr);
        free(baseTypeStr);
        CONDITION_FAILED;
    }

    // Check if it has an array after base type
    if (isArray)
    {
        printf("Data type is an array\n");
        printf("Data Type: %s\n", CryoDataTypeToString(baseType));
        free(baseTypeStr);
        switch (baseType)
        {
        case DATA_TYPE_INT:
            return DATA_TYPE_INT_ARRAY;
        case DATA_TYPE_FLOAT:
            return DATA_TYPE_FLOAT_ARRAY;
        case DATA_TYPE_STRING:
            return DATA_TYPE_STRING_ARRAY;
        case DATA_TYPE_BOOLEAN:
            return DATA_TYPE_BOOLEAN_ARRAY;
        case DATA_TYPE_VOID:
            return DATA_TYPE_VOID_ARRAY;
        default:
            logMessage("ERROR", __LINE__, "TypeDefs", "Unknown data type: %s", typeStr);
            CONDITION_FAILED;
        }
    }

    // If it's a dyn_vec, we need to parse the `<>` to get the type
    if (baseType == DATA_TYPE_DYN_VEC)
    {
        // Find the first `<` and the last `>`
        const char *start = strchr(baseTypeStr, '<');
        const char *end = strrchr(baseTypeStr, '>');

        // Check if the start and end are valid
        if (start == NULL || end == NULL || start >= end)
        {
            logMessage("ERROR", __LINE__, "TypeDefs", "Invalid dyn_vec type: %s", typeStr);
            free(baseTypeStr);
            CONDITION_FAILED;
        }

        // Get the length of the type
        size_t length = end - start - 1;

        // Create a new string to hold the type
        char *type = (char *)malloc(sizeof(char) * (length + 1));
        strncpy(type, start + 1, length);
        type[length] = '\0';

        // Parse the type
        CryoDataType result = parseDataType(type);
        free(type);
        free(baseTypeStr);
        return result;
    }

    free(baseTypeStr);
    return baseType;
}

CryoDataType getPrimativeTypeFromString(const char *typeStr)
{
    if (strcmp(typeStr, "int") == 0)
    {
        return DATA_TYPE_INT;
    }
    else if (strcmp(typeStr, "float") == 0)
    {
        return DATA_TYPE_FLOAT;
    }
    else if (strcmp(typeStr, "string") == 0)
    {
        return DATA_TYPE_STRING;
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        return DATA_TYPE_BOOLEAN;
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        return DATA_TYPE_VOID;
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        return DATA_TYPE_NULL;
    }
    else if (strcmp(typeStr, "int8") == 0)
    {
        return DATA_TYPE_SINT8;
    }
    else if (strcmp(typeStr, "int16") == 0)
    {
        return DATA_TYPE_SINT16;
    }
    else if (strcmp(typeStr, "int32") == 0)
    {
        return DATA_TYPE_SINT32;
    }
    else if (strcmp(typeStr, "int64") == 0)
    {
        return DATA_TYPE_SINT64;
    }
    else if (strcmp(typeStr, "u_int8") == 0)
    {
        return DATA_TYPE_UINT8;
    }
    else if (strcmp(typeStr, "u_int16") == 0)
    {
        return DATA_TYPE_UINT16;
    }
    else if (strcmp(typeStr, "u_int32") == 0)
    {
        return DATA_TYPE_UINT32;
    }
    else if (strcmp(typeStr, "u_int64") == 0)
    {
        return DATA_TYPE_UINT64;
    }
    else if (strcmp(typeStr, "dyn_vec") == 0)
    {
        return DATA_TYPE_DYN_VEC;
    }
    else if (strcmp(typeStr, "Any") == 0)
    {
        return DATA_TYPE_ANY;
    }
    else
    {
        logMessage("ERROR", __LINE__, "TypeDefs", "Unknown data type: %s", typeStr);
        return DATA_TYPE_UNKNOWN;
    }
}
