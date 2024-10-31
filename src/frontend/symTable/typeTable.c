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
#include "frontend/typeTable.h"

TypeTable *initTypeTable(void)
{
    TypeTable *table = (TypeTable *)malloc(sizeof(TypeTable));
    if (!table)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table.\n");
        return NULL;
    }

    table->types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!table->types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table types.\n");
        return NULL;
    }

    table->count = 0;
    table->capacity = 64;

    return table;
}

// # =========================================================================== #
// # Specialized Type Creation Functions

StructType *createStructDataType(const char *name)
{
    StructType *type = (StructType *)malloc(sizeof(StructType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type.\n");
        return NULL;
    }

    type->name = name;
    type->properties = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    if (!type->properties)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type properties.\n");
        return NULL;
    }

    type->propertyCount = 0;
    type->propertyCapacity = 64;

    type->methods = (ASTNode **)malloc(sizeof(ASTNode *) * 64);
    if (!type->methods)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for struct type methods.\n");
        return NULL;
    }

    type->methodCount = 0;
    type->methodCapacity = 64;

    type->hasDefaultValue = false;

    return type;
}

// # =========================================================================== #
// # DataType Creation Functions

DataType *createDataTypeFromPrimitive(PrimitiveDataType type)
{
    DataType *dataType = (DataType *)malloc(sizeof(DataType));
    if (!dataType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    dataType->typeOf = PRIMITIVE_TYPE;
    dataType->primitiveType = type;

    return dataType;
}

DataType *createDataTypeFromStruct(ASTNode *structNode, CompilerState *state, TypeTable *typeTable)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = STRUCT_TYPE;

    const char *structName = structNode->data.structNode->name;
    StructType *structType = createStructDataType(structName);
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create struct type.\n");
        return NULL;
    }

    structType->properties = structNode->data.structNode->properties;
    structType->propertyCount = structNode->data.structNode->propertyCount;
    structType->propertyCapacity = structNode->data.structNode->propertyCapacity;

    structType->methods = structNode->data.structNode->methods;
    structType->methodCount = structNode->data.structNode->methodCount;
    structType->methodCapacity = structNode->data.structNode->methodCapacity;

    structType->hasDefaultValue = structNode->data.structNode->hasDefaultValue;
    structType->hasConstructor = structNode->data.structNode->hasConstructor;

    type->structType = structType;

    return type;
}

DataType *createDataTypeFromEnum(Arena *arena, CompilerState *state, ASTNode *enumNode)
{
    DataType *type = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = ENUM_TYPE;
    type->enumType = enumNode;

    return type;
}

DataType *createDataTypeFromFunction(Arena *arena, CompilerState *state, ASTNode *functionNode)
{
    DataType *type = (DataType *)ARENA_ALLOC(arena, sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = FUNCTION_TYPE;
    type->functionType = functionNode;

    return type;
}

DataType *createDataTypeFromUnknown(void)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for data type.\n");
        return NULL;
    }

    type->typeOf = UNKNOWN_TYPE;

    return type;
}

// # =========================================================================== #
// # Add Type to Type Table
void addTypeToTypeTable(TypeTable *table, DataType *type)
{
    if (table->count >= table->capacity)
    {
        int newCapacity = table->capacity * 2;
        DataType **newTypes = (DataType **)realloc(table->types, newCapacity * sizeof(DataType *));
        if (!newTypes)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to reallocate memory for type table.\n");
            return;
        }

        table->types = newTypes;
        table->capacity = newCapacity;
    }

    table->types[table->count++] = type;
}

// # =========================================================================== #
// # Utility Functions

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

char *PrimitiveDataTypeToString(PrimitiveDataType type)
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
    case PRIM_UNKNOWN:
        return "<UNKNOWN>";
    default:
        return "<PRIMITIVE UNKNOWN>";
    }
}

void printFormattedStructType(StructType *type)
{
    printf("   ─────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n", type->size, type->propertyCount, type->methodCount);
    printf("   Name: %s | HDV: %s | Has Constructor: %s\n", type->name, type->hasDefaultValue ? "true" : "false", type->hasConstructor ? "true" : "false");
}

void printFormattedType(DataType *type)
{
    switch (type->typeOf)
    {
    case PRIMITIVE_TYPE:
        printf("PRIMITIVE_TYPE %s\n", PrimitiveDataTypeToString(type->primitiveType));
        break;
    case STRUCT_TYPE:
        printFormattedStructType(type->structType);
        break;
    case ENUM_TYPE:
        printf("ENUM_TYPE\n");
        break;
    case FUNCTION_TYPE:
        printf("FUNCTION_TYPE\n");
        break;
    case UNKNOWN_TYPE:
        printf("UNKNOWN_TYPE\n");
        break;
    default:
        printf("<TYPE UNKNOWN>\n");
        break;
    }
}

void printTypeTable(TypeTable *table)
{
    printf("\n");
    printf(BOLD CYAN "╓────────────────────────── Type Table ──────────────────────────╖\n" COLOR_RESET);
    printf("  Type Table: %p\n", (void *)table);
    printf("  Type Count: %d\n", table->count);
    printf("  Type Capacity: %d\n", table->capacity);
    printf(" Namespace: %s\n", table->namespaceName);
    printf(BOLD CYAN "╟────────────────────────────────────────────────────────────────╢\n" COLOR_RESET);

    for (int i = 0; i < table->count; i++)
    {
        DataType *type = table->types[i];
        printFormattedType(type);
    }
    printf("   ─────────────────────────────────────────────────────────────\n");
    printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
    printf("\n");
    if (table->count == 0)
    {
        printf("  No types in the type table.\n");
        printf(BOLD CYAN "╙────────────────────────────────────────────────────────────────╜\n" COLOR_RESET);
        printf("\n");
    }
}
