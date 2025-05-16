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
#include "tools/logger/logger_config.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- DTM Debug Functions ----------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTMDebug_printDataType(DataType *type)
{
    DEBUG_PRINT_FILTER({
        if (!type)
        {
            fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type (DTMDebug_printDataType)\n");
        }

        // Check to see if the DataType is a primitive type
        if (type->container->typeOf == PRIM_TYPE)
        {
            switch (type->container->primitive)
            {
            case PRIM_INT:
                printf("int\n");
                break;
            case PRIM_I8:
                printf("i8\n");
                break;
            case PRIM_I16:
                printf("i16\n");
                break;
            case PRIM_I32:
                printf("i32\n");
                break;
            case PRIM_I64:
                printf("i64\n");
                break;
            case PRIM_I128:
                printf("i128\n");
                break;
            case PRIM_FLOAT:
                printf("float\n");
                break;
            case PRIM_STRING:
                printf("string\n");
                break;
            case PRIM_CHAR:
                printf("char\n");
                break;
            case PRIM_STR:
                printf("str\n");
                break;
            case PRIM_BOOLEAN:
                printf("boolean\n");
                break;
            case PRIM_VOID:
                printf("void\n");
                break;
            case PRIM_NULL:
                printf("null\n");
                break;
            case PRIM_ANY:
                printf("any\n");
                break;
            case PRIM_OBJECT:
                printf("object\n");
                break;
            case PRIM_FUNCTION:
                printf("function\n");
                break;
            case PRIM_UNDEFINED:
                printf("undefined\n");
                break;
            case PRIM_UNKNOWN:
                printf("unknown\n");
                break;
            case PRIM_AUTO:
                printf("auto\n");
                break;
            default:
                fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type\n");
                CONDITION_FAILED;
            }
        }
        else if (type->container->typeOf == OBJECT_TYPE)
        {
            switch (type->container->objectType)
            {
            case STRUCT_OBJ:
            {
                char *structName = (char *)malloc(sizeof(char) * 1024);
                if (!structName)
                {
                    fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for struct name\n");
                    CONDITION_FAILED;
                }
                if (type->container->type.structType->isProtoType)
                {
                    strcpy(structName, "@proto ");
                }
                strcpy(structName, "struct ");
                strcat(structName, type->container->type.structType->name);

                printf("%s\n", structName);
                free(structName);
                break;
            }
            case CLASS_OBJ:
            {
                char *className = (char *)malloc(sizeof(char) * 1024);
                if (!className)
                {
                    fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for class name\n");
                    CONDITION_FAILED;
                }
                strcpy(className, "class ");
                strcat(className, type->container->type.classType->name);
                printf("%s\n", className);
                free(className);
                break;
            }
            case INTERFACE_OBJ:
                printf("Interface\n");
                break;
            case OBJECT_OBJ:
                printf("Object\n");
                break;
            case VA_ARGS_OBJ:
                printf("VA_ARGS\n");
                break;
            case NON_OBJECT:
                printf("Non-Object\n");
                break;
            case UNKNOWN_OBJECT:
                printf("Unknown Object\n");
                break;
            default:
                fprintf(stderr, "[Data Type Manager] Error: Unknown object type\n");
                CONDITION_FAILED;
            }
        }
        else if (type->container->typeOf == ARRAY_TYPE)
        {
            int arraySize = type->container->type.arrayType->size;
            int arrayCount = type->container->type.arrayType->elementCount;
            char *arrayTypeName = (char *)malloc(sizeof(char) * 1024);
            if (!arrayTypeName)
            {
                fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for array type name\n");
                CONDITION_FAILED;
            }
            strcpy(arrayTypeName, "Array<");
            strcat(arrayTypeName, type->container->type.arrayType->baseType->debug->toString(type->container->type.arrayType->baseType));
            strcat(arrayTypeName, ">");
            strcat(arrayTypeName, "[");
            strcat(arrayTypeName, intToSafeString(arraySize));
            strcat(arrayTypeName, "]");
            printf("%s\n", arrayTypeName);
            free(arrayTypeName);
        }
        else if (type->container->typeOf == ENUM_TYPE)
        {
            printf("Enum\n");
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

            printf("Function: %s\n", functionSig);
            free((char *)functionSig);
        }
        else if (type->container->typeOf == GENERIC_TYPE)
        {
            printf("Generic\n");
        }
        else if (type->container->typeOf == TYPE_DEF)
        {
            printf("Type Definition\n");
        }
        else if (type->container->typeOf == POINTER_TYPE)
        {
            printf("Pointer\n");
        }
        else
        {
            fprintf(stderr, "[Data Type Manager] Error: Unknown data type @DTMDebug_printDataType\n");
            CONDITION_FAILED;
        }
    });
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
    case POINTER_TYPE:
        return "Pointer";
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
        return "Struct";
    case CLASS_OBJ:
        return "Class";
    case INTERFACE_OBJ:
        return "Interface";
    case OBJECT_OBJ:
        return "Object";
    case VA_ARGS_OBJ:
        return "VA_ARGS";
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
    case PRIM_CHAR:
        return "char";
    case PRIM_STR:
        return "str";
    case PRIM_BOOLEAN:
        return "boolean";
    case PRIM_VOID:
        return "void";
    case PRIM_NULL:
        return "null";
    case PRIM_ANY:
        return "any";
    case PRIM_ENUM:
        return "enum";
    case PRIM_ARRAY:
        return "array";
    case PRIM_OBJECT:
        return "object";
    case PRIM_FUNCTION:
        return "function";
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
        fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL data type (DTMDebug_dataTypeToString)\n");
    }

    // Check to see if the DataType is a primitive type
    if (type->container->typeOf == PRIM_TYPE)
    {
        switch (type->container->primitive)
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
        case PRIM_CHAR:
            return "char";
        case PRIM_STR:
            return "str";
        case PRIM_BOOLEAN:
            return "boolean";
        case PRIM_VOID:
            return "void";
        case PRIM_NULL:
            return "null";
        case PRIM_ANY:
            return "any";
        case PRIM_UNDEFINED:
            return "undefined";
        case PRIM_AUTO:
            return "auto";
        case PRIM_ENUM:
            return "enum";
        case PRIM_ARRAY:
            return "array";
        case PRIM_OBJECT:
            return "object";
        case PRIM_FUNCTION:
            return "function";
        case PRIM_UNKNOWN:
            return "unknown";
        default:
            const char *typeOfPrim = DTM->debug->primitiveDataTypeToString(type->container->primitive);
            fprintf(stderr, "[Data Type Manager] Error: Unknown primitive type: %s\n", typeOfPrim);
            CONDITION_FAILED;
        }
    }
    else if (type->container->typeOf == ARRAY_TYPE)
    {
        int arraySize = type->container->type.arrayType->size;
        int arrayCount = type->container->type.arrayType->elementCount;
        char *arrayTypeName = (char *)malloc(sizeof(char) * 1024);
        if (!arrayTypeName)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for array type name\n");
            CONDITION_FAILED;
        }
        strcpy(arrayTypeName, "Array<");
        strcat(arrayTypeName, type->container->type.arrayType->baseType->debug->toString(type->container->type.arrayType->baseType));
        strcat(arrayTypeName, ">");
        strcat(arrayTypeName, "[");
        strcat(arrayTypeName, intToSafeString(arraySize));
        strcat(arrayTypeName, "]");
        return arrayTypeName;
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
        return "Generic";
    }
    else if (type->container->typeOf == OBJECT_TYPE)
    {
        switch (type->container->objectType)
        {
        case STRUCT_OBJ:
        {
            char *structName = (char *)malloc(sizeof(char) * 1024);
            if (!structName)
            {
                fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for struct name\n");
                CONDITION_FAILED;
            }
            if (type->container->type.structType->isProtoType)
            {
                strcpy(structName, "@proto ");
            }
            strcpy(structName, "struct ");
            strcat(structName, type->container->type.structType->name);

            return structName;
        }
        case CLASS_OBJ:
        {
            return type->container->type.classType->name;
        }
        case INTERFACE_OBJ:
            return "interface";
        case OBJECT_OBJ:
            return "object";
        case VA_ARGS_OBJ:
            return "VA_ARGS";
        case NON_OBJECT:
            return "non-object";
        case UNKNOWN_OBJECT:
            return "unknown";
        default:
            fprintf(stderr, "[Data Type Manager] Error: Unknown object type\n");
            CONDITION_FAILED;
        }
    }
    else if (type->container->typeOf == TYPE_DEF)
    {
        return "Type Definition";
    }
    else if (type->container->typeOf == POINTER_TYPE)
    {
        DataType *baseType = type->container->type.pointerType->baseType;
        if (!baseType)
        {
            fprintf(stderr, "[Data Type Manager] Error: Attempted to print NULL base type\n");
            CONDITION_FAILED;
        }

        const char *baseTypeStr = DTM->debug->dataTypeToString(baseType);
        if (!baseTypeStr)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to get base type string\n");
            CONDITION_FAILED;
        }

        char *pointerTypeStr = (char *)malloc(sizeof(char) * 1024);
        if (!pointerTypeStr)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for pointer type string\n");
            CONDITION_FAILED;
        }

        snprintf(pointerTypeStr, 1024, "%s *", baseTypeStr);

        return pointerTypeStr;
    }
    else if (type->container->typeOf == UNKNOWN_TYPE)
    {
        return "<UNKNOWN>";
    }
    else
    {
        fprintf(stderr, "[Data Type Manager] Error: Unknown data type @DTMDebug_dataTypeToString\n");
        CONDITION_FAILED;
    }

    return "<UNKNOWN>";
}

const char *DTMDebug_literalTypeToString(LiteralType type)
{
    switch (type)
    {
    case LITERAL_INT:
        return "int";
    case LITERAL_FLOAT:
        return "float";
    case LITERAL_STRING:
        return "string";
    case LITERAL_BOOLEAN:
        return "boolean";
    case LITERAL_NULL:
        return "null";
    default:
        return "<UNKNOWN>";
    }
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
    debug->literalTypeToString = DTMDebug_literalTypeToString;

    return debug;
}