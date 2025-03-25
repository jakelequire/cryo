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
// ------------------------------------- DTM Debug Functions ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDebug_printDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    const char *typeName = type->typeName;
    if (!typeName || typeName == NULL)
    {
        // Check to see if the DataType is a primitive type
        if (type->container->typeOf == PRIM_TYPE)
        {
            switch (type->container->primitive)
            {
            case PRIM_INT:
                typeName = "int";
                break;
            case PRIM_FLOAT:
                typeName = "float";
                break;
            case PRIM_STRING:
                typeName = "string";
                break;
            case PRIM_BOOLEAN:
                typeName = "boolean";
                break;
            case PRIM_VOID:
                typeName = "void";
                break;
            case PRIM_NULL:
                typeName = "null";
                break;
            case PRIM_ANY:
                typeName = "any";
                break;
            case PRIM_UNDEFINED:
                typeName = "undefined";
                break;
            case PRIM_AUTO:
                typeName = "auto";
                break;
            default:
                fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type\n");
                CONDITION_FAILED;
            }
        }
        else
        {
            fprintf(stderr, "[Data Type Manager] Error: Unknown data type\n");
            CONDITION_FAILED;
        }
    }

    printf("%s\n", typeName);
}

const char *DTMDebug_typeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIM_TYPE:
        return "Primitive";
    case ARRAY_TYPE:
        return "Array";
    case ENUM_TYPE:
        return "Enum";
    case FUNCTION_TYPE:
        return "Function";
    case GENERIC_TYPE:
        return "Generic";
    case OBJECT_TYPE:
        return "Object";
    case TYPE_DEF:
        return "Type Def";
    case UNKNOWN_TYPE:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_typeofObjectTypeToString(TypeofObjectType type)
{
    switch (type)
    {
    case STRUCT_OBJ:
        return "Struct Object";
    case CLASS_OBJ:
        return "Class Object";
    case INTERFACE_OBJ:
        return "Interface Object";
    case TRAIT_OBJ:
        return "Trait Object";
    case MODULE_OBJ:
        return "Module Object";
    case OBJECT_OBJ:
        return "Object Object";
    case NON_OBJECT:
        return "Non-Object";
    case UNKNOWN_OBJECT:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_primitiveDataTypeToString(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
    case PRIM_I8:
        return "i8";
    case PRIM_I16:
        return "i16";
    case PRIM_I32:
        return "i32";
    case PRIM_I64:
        return "i64";
    case PRIM_I128:
        return "i128";

    case PRIM_FLOAT:
        return "float";
    case PRIM_STRING:
        return "string";
    case PRIM_BOOLEAN:
        return "boolean";
    case PRIM_VOID:
        return "void";
    case PRIM_NULL:
        return "null";
    case PRIM_ANY:
        return "any";
    case PRIM_OBJECT:
        return "prim_object";
    case PRIM_AUTO:
        return "auto";
    case PRIM_UNDEFINED:
        return "undefined";
    case PRIM_UNKNOWN:
        return "unknown";
    default:
        return "<DEFAULTED>";
    }
}

const char *DTMDebug_primitiveDataTypeToCType(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
    case PRIM_FLOAT:
        return "float";
    case PRIM_STRING:
        return "char *";
    case PRIM_BOOLEAN:
        return "bool";
    case PRIM_VOID:
        return "void";
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<UNKNOWN>";
    }
}

const char *DTMDebug_dataTypeToString(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type\n");
        CONDITION_FAILED;
    }

    const char *typeName = type->typeName;
    if (!typeName || typeName == NULL)
    {
        return "<ERROR>";
    }

    // Check to see if the DataType is a primitive type
    if (type->container->typeOf == PRIM_TYPE)
    {
        switch (type->container->primitive)
        {
        case PRIM_INT:
            return "int";
            break;
        case PRIM_I8:
            return "i8";
            break;
        case PRIM_I16:
            return "i16";
            break;
        case PRIM_I32:
            return "i32";
            break;
        case PRIM_I64:
            return "i64";
            break;
        case PRIM_I128:
            return "i128";
            break;
        case PRIM_FLOAT:
            return "float";
            break;
        case PRIM_STRING:
            return "string";
            break;
        case PRIM_BOOLEAN:
            return "boolean";
            break;
        case PRIM_VOID:
            return "void";
            break;
        case PRIM_NULL:
            return "null";
            break;
        case PRIM_ANY:
            return "any";
            break;
        case PRIM_UNDEFINED:
            return "undefined";
            break;
        case PRIM_AUTO:
            return "auto";
            break;
        case PRIM_OBJECT:
            return "object";
            break;
        default:
            const char *typeOfPrim = DTM->debug->primitiveDataTypeToString(type->container->primitive);
            fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type: %s\n", typeOfPrim);
            CONDITION_FAILED;
        }
    }
    else if (type->container->typeOf == ARRAY_TYPE)
    {
        return "Array";
    }
    else if (type->container->typeOf == ENUM_TYPE)
    {
        return "Enum";
    }
    else if (type->container->typeOf == FUNCTION_TYPE)
    {
        DTFunctionTy *functionType = type->container->type.functionType;
        if (!functionType)
        {
            fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL function type\n");
        }
        const char *functionSig = functionType->signatureToString(functionType);
        if (!functionSig)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to get function signature\n");
            CONDITION_FAILED;
        }
        return functionSig;
    }
    else if (type->container->typeOf == GENERIC_TYPE)
    {
        typeName = "Generic";
    }
    else if (type->container->typeOf == OBJECT_TYPE)
    {
        typeName = "Object";
    }
    else if (type->container->typeOf == TYPE_DEF)
    {
        typeName = "Type Definition";
    }
    else
    {
        fprintf(stderr, "[Data Type Manager] Error: Unknown data type\n");
        CONDITION_FAILED;
    }

    return typeName;
}

DTMDebug *createDTMDebug(void)
{
    DTMDebug *debug = (DTMDebug *)malloc(sizeof(DTMDebug));
    if (!debug)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Debug\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    debug->printDataType = DTMDebug_printDataType;

    debug->typeofDataTypeToString = DTMDebug_typeofDataTypeToString;
    debug->typeofObjectTypeToString = DTMDebug_typeofObjectTypeToString;
    debug->primitiveDataTypeToString = DTMDebug_primitiveDataTypeToString;
    debug->primitiveDataTypeToCType = DTMDebug_primitiveDataTypeToCType;
    debug->dataTypeToString = DTMDebug_dataTypeToString;

    return debug;
}