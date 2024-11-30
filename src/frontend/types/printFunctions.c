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
#include "frontend/dataTypes.h"

void printTypeTable(TypeTable *table)
{
    printf("\n");
    printf(BOLD CYAN "╓────────────────────────── Type Table ──────────────────────────╖\n" COLOR_RESET);
    printf("  Type Table: %p\n", (void *)table);
    printf("  Type Count: %d\n", table->count);
    printf("  Type Capacity: %d\n", table->capacity);
    printf(BOLD CYAN "╟────────────────────────────────────────────────────────────────╢\n" COLOR_RESET);

    for (int i = 0; i < table->count; i++)
    {
        DataType *type = table->types[i];
        printFormattedType(type);
    }
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
    printf("\n");
    if (table->count == 0)
    {
        printf("  No types in the type table.\n");
        printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
        printf("\n");
    }
}

char *TypeofDataTypeToString(TypeofDataType type)
{
    switch (type)
    {
    case PRIMITIVE_TYPE:
        return "PRIMITIVE_TYPE";
    case ARRAY_TYPE:
        return "ARRAY_TYPE";
    case STRUCT_TYPE:
        return "STRUCT_TYPE";
    case ENUM_TYPE:
        return "ENUM_TYPE";
    case FUNCTION_TYPE:
        return "FUNCTION_TYPE";
    case GENERIC_TYPE:
        return "GENERIC_TYPE";
    case CLASS_TYPE:
        return "CLASS_TYPE";
    case UNKNOWN_TYPE:
        return "UNKNOWN_TYPE";
    default:
        return "<TYPE UNKNOWN>";
    }
}

char *PrimitiveDataTypeToString(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return LIGHT_CYAN BOLD "int" COLOR_RESET;
    case PRIM_FLOAT:
        return LIGHT_CYAN BOLD "float" COLOR_RESET;
    case PRIM_STRING:
        return LIGHT_CYAN BOLD "string" COLOR_RESET;
    case PRIM_BOOLEAN:
        return LIGHT_CYAN BOLD "boolean" COLOR_RESET;
    case PRIM_VOID:
        return LIGHT_CYAN BOLD "void" COLOR_RESET;
    case PRIM_NULL:
        return LIGHT_CYAN BOLD "null" COLOR_RESET;
    case PRIM_ANY:
        return LIGHT_CYAN BOLD "any" COLOR_RESET;
    case PRIM_CUSTOM:
        return LIGHT_CYAN BOLD "Custom Type" COLOR_RESET;
    case PRIM_UNKNOWN:
        return LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET;
    default:
        return LIGHT_RED BOLD "<PRIMITIVE UNKNOWN>" COLOR_RESET;
    }

    return "<UNKNOWN>";
}

char *PrimitiveDataTypeToString_UF(PrimitiveDataType type)
{
    switch (type)
    {
    case PRIM_INT:
        return "int";
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
    case PRIM_CUSTOM:
        return "Custom Type";
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<PRIMITIVE UNKNOWN>";
    }

    return "<UNKNOWN>";
}

char *DataTypeToStringUnformatted(DataType *type)
{
    if (!type)
        return "<NULL DATATYPE>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    switch (type->container->baseType)
    {
    case PRIMITIVE_TYPE:
        if (type->container->isArray)
        {
            sprintf(typeString, "%s[]", PrimitiveDataTypeToString_UF(type->container->primitive));
            return typeString;
        }
        sprintf(typeString, "%s", PrimitiveDataTypeToString_UF(type->container->primitive));
        break;

    case STRUCT_TYPE:
        sprintf(typeString, "%s", type->container->custom.structDef->name);
        break;

    case CLASS_TYPE:
        sprintf(typeString, "%s", type->container->custom.classDef->name);

    default:
        sprintf(typeString, "<UNKNOWN>");
        break;
    }

    return typeString;
}

char *DataTypeToString(DataType *dataType)
{
    if (!dataType)
        return "<NULL DATATYPE>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    switch (dataType->container->baseType)
    {
    case PRIMITIVE_TYPE:
        if (dataType->container->isArray)
        {
            sprintf(typeString, LIGHT_CYAN BOLD "%s[]" COLOR_RESET,
                    PrimitiveDataTypeToString(dataType->container->primitive));
            return typeString;
        }
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, PrimitiveDataTypeToString(dataType->container->primitive));
        break;

    case STRUCT_TYPE:
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, dataType->container->custom.structDef->name);
        break;

    case CLASS_TYPE:
        break;

    default:
        sprintf(typeString, LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET);
        break;
    }

    return typeString;
}

char *VerboseStructTypeToString(StructType *type)
{
    if (!type)
        return "<NULL STRUCT>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, (char *)type->name);

    if (type->propertyCount > 0)
    {
        for (int i = 0; i < type->propertyCount; i++)
        {
            ASTNode *property = type->properties[i];
            char *propType = DataTypeToString(property->data.property->type);
            sprintf(typeString, "%s %s: %s", typeString, property->data.property->name, propType);
        }
    }

    if (type->methodCount > 0)
    {
        for (int i = 0; i < type->methodCount; i++)
        {
            ASTNode *method = type->methods[i];
            char *methodType = DataTypeToString(method->data.method->type);
            sprintf(typeString, "%s %s →  %s", typeString, method->data.method->name, methodType);
            if (method->data.method->paramCount > 0)
            {
                for (int j = 0; j < method->data.method->paramCount; j++)
                {
                    ASTNode *param = method->data.method->params[j];
                    char *paramType = DataTypeToString(param->data.param->type);
                    sprintf(typeString, "%s %s: %s", typeString, param->data.param->name, paramType);
                }
            }
        }
    }

    return typeString;
}

void logDataType(DataType *type)
{
    if (!type)
        return;

    printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
    printf(BOLD GREEN "   DATATYPE" COLOR_RESET " | Const: %s | Ref: %s\n", type->isConst ? "true" : "false", type->isReference ? "true" : "false");
    printTypeContainer(type->container);
    printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
    printf(COLOR_RESET);
}

void logVerboseDataType(DataType *type)
{
    if (!type)
        return;

    printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
    printf(BOLD GREEN "   (v)DATATYPE" COLOR_RESET " | Const: %s | Ref: %s\n", type->isConst ? "true" : "false", type->isReference ? "true" : "false");
    printVerboseTypeContainer(type->container);
    printf(BOLD CYAN "───────────────────────────────────────────────────────────────\n" COLOR_RESET);
    printf(COLOR_RESET);
}

void logStructType(StructType *type)
{
    if (!type)
    {
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | <NULL>\n");
        return;
    }

    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n",
           type->size, type->propertyCount, type->methodCount);

    printf("   Name: %s | HDV: %s | Has Constructor: %s\n",
           type->name, type->hasDefaultValue ? "true" : "false",
           type->hasConstructor ? "true" : "false");

    printf(COLOR_RESET);
}

void printFormattedStructType(StructType *type)
{
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n", type->size, type->propertyCount, type->methodCount);
    printf("   Name: %s | HDV: %s | Has Constructor: %s\n", type->name, type->hasDefaultValue ? "true" : "false", type->hasConstructor ? "true" : "false");

    if (type->propertyCount > 0)
    {
        printf("   Properties:\n");
        for (int i = 0; i < type->propertyCount; i++)
        {
            ASTNode *property = type->properties[i];
            printf("     %s: %s\n", property->data.property->name, DataTypeToString(property->data.property->type));
        }
    }

    if (type->methodCount > 0)
    {
        printf("   Methods:\n");
        for (int i = 0; i < type->methodCount; i++)
        {
            ASTNode *method = type->methods[i];
            printf("     %s →  %s\n", method->data.method->name, DataTypeToString(method->data.method->type));
            if (method->data.method->paramCount > 0)
            {
                for (int j = 0; j < method->data.method->paramCount; j++)
                {
                    ASTNode *param = method->data.method->params[j];
                    printf("       %s: %s\n", param->data.param->name, DataTypeToString(param->data.param->type));
                }
            }
        }
    }

    printf(COLOR_RESET);
}

void printFormattedPrimitiveType(PrimitiveDataType type)
{
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   PRIMITIVE_TYPE" COLOR_RESET " | %s\n", PrimitiveDataTypeToString(type));
    printf(COLOR_RESET);
}

void printFormattedType(DataType *type)
{
    if (!type)
        return;

    switch (type->container->baseType)
    {
    case PRIMITIVE_TYPE:
        if (type->container->isArray)
        {
            printf("   ────────────────────────────────────────────────────────────\n");
            printf(BOLD GREEN "   PRIMITIVE_TYPE" COLOR_RESET " | %s[%d]\n", PrimitiveDataTypeToString(type->container->primitive), type->container->arrayDimensions);
        }
        else
        {
            printFormattedPrimitiveType(type->container->primitive);
        }
        break;

    case STRUCT_TYPE:
        printFormattedStructType(type->container->custom.structDef);
        break;

    default:
        printf("  ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "  UNKNOWN_TYPE" COLOR_RESET "\n");
        break;
    }

    printf(COLOR_RESET);
}

void printTypeContainer(TypeContainer *type)
{
    if (!type)
        return;

    printf("Type: %s", TypeofDataTypeToString(type->baseType));

    if (type->isArray)
    {
        printf("[%d]", type->arrayDimensions);
    }

    switch (type->baseType)
    {
    case PRIMITIVE_TYPE:
        printf(" (%s)", PrimitiveDataTypeToString(type->primitive));
        break;

    case STRUCT_TYPE:
        printf(" (%s)", type->custom.structDef->name);
        break;

    default:
        break;
    }
    printf("\n");
}

void printVerboseTypeContainer(TypeContainer *type)
{
    if (!type)
    {
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   TYPE_CONTAINER" COLOR_RESET " | <NULL>\n");
        return;
    }

    printf("Type: %s", TypeofDataTypeToString(type->baseType));

    if (type->isArray)
    {
        printf("[%d]", type->arrayDimensions);
    }

    switch (type->baseType)
    {
    case PRIMITIVE_TYPE:
        printf(" (%s)", PrimitiveDataTypeToString(type->primitive));
        break;

    case STRUCT_TYPE:
        printf(" (%s)", VerboseStructTypeToString(type->custom.structDef));
        break;

    default:
        printf(" <UNKNOWN>");
        break;
    }
    printf("\n");
}

#define PRINT_MEMBERS_FN(TYPE)                                                                     \
    if (type->TYPE##Members)                                                                       \
    {                                                                                              \
        printf("   " #TYPE " Members:\n");                                                         \
        for (int i = 0; i < type->TYPE##Members->propertyCount; i++)                               \
        {                                                                                          \
            DataType *property = type->TYPE##Members->properties[i];                               \
            printf("     %s: %s\n", property->container->custom.name, DataTypeToString(property)); \
        }                                                                                          \
    }

void printClassType(ClassType *type)
{
    if (!type)
    {
        printf("   ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "   CLASS_TYPE" COLOR_RESET " | <NULL>\n");
        return;
    }

    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   CLASS_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n",
           sizeof(ClassType), type->propertyCount, type->methodCount);

    printf("   Name: %s | Static: %s | Has Constructor: %s\n",
           type->name, type->isStatic ? "true" : "false",
           type->hasConstructor ? "true" : "false");

    // Print constructor if exists
    if (type->hasConstructor && type->constructor)
    {
        printf("   Constructor: %s\n", type->constructor->data.method->name);
    }

    // Use the macro to print members for each access level
    PRINT_MEMBERS_FN(public);
    PRINT_MEMBERS_FN(private);
    PRINT_MEMBERS_FN(protected);

    printf(COLOR_RESET);
}
#undef PRINT_MEMBERS_FN
