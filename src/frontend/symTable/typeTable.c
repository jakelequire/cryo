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

// Create new TypeContainer
TypeContainer *createTypeContainer(void)
{
    TypeContainer *container = malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate TypeContainer\n");
        return NULL;
    }

    container->baseType = UNKNOWN_TYPE;
    container->primitive = PRIM_UNKNOWN;
    container->isArray = false;
    container->arrayDimensions = 0;
    container->custom.name = NULL;
    container->custom.structDef = NULL;
    container->custom.funcDef = NULL;
    container->custom.extraData = NULL;

    return container;
}

// # =========================================================================== #
// # Specialized Type Creation Functions

// Create primitive type
TypeContainer *createPrimitiveType(PrimitiveDataType primType)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = PRIMITIVE_TYPE;
    container->primitive = primType;

    return container;
}

// Create struct type
TypeContainer *createStructType(const char *name, StructType *structDef)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = STRUCT_TYPE;
    container->custom.name = strdup(name);
    container->custom.structDef = structDef;

    return container;
}

TypeContainer *createArrayType(TypeContainer *baseType, int dimensions)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    // Copy base type info
    container->baseType = baseType->baseType;
    container->primitive = baseType->primitive;
    container->custom = baseType->custom;

    // Add array info
    container->isArray = true;
    container->arrayDimensions = dimensions;

    return container;
}

// # =========================================================================== #
// # DataType Creation Functions

// Create DataType wrapper
DataType *wrapTypeContainer(TypeContainer *container)
{
    DataType *type = malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate DataType\n");
        return NULL;
    }

    type->container = *container;
    type->isConst = false;
    type->isReference = false;
    type->next = NULL;

    return type;
}

TypeContainer *lookupType(TypeTable *table, const char *name)
{
    for (int i = 0; i < table->count; i++)
    {
        DataType *type = table->types[i];
        if (type->container.custom.name &&
            strcmp(type->container.custom.name, name) == 0)
        {
            return &type->container;
        }
    }
    return NULL;
}

// # =========================================================================== #
// # Add Type to Type Table
void addTypeToTypeTable(TypeTable *table, const char *name, TypeContainer *type)
{
    if (table->count >= table->capacity)
    {
        // Grow table
        int newCapacity = table->capacity * 2;
        DataType **newTypes = realloc(table->types, newCapacity * sizeof(DataType *));
        if (!newTypes)
            return;

        table->types = newTypes;
        table->capacity = newCapacity;
    }

    DataType *newType = malloc(sizeof(DataType));
    newType->container = *type;
    newType->isConst = false;
    newType->isReference = false;
    newType->next = NULL;

    table->types[table->count++] = newType;
}

bool isValidType(TypeContainer *type, TypeTable *typeTable)
{
    if (!type)
        return false;

    switch (type->baseType)
    {
    case PRIMITIVE_TYPE:
        return type->primitive != PRIM_UNKNOWN;

    case STRUCT_TYPE:
        return type->custom.structDef != NULL;

    case FUNCTION_TYPE:
        return type->custom.funcDef != NULL;

    default:
        return false;
    }
}

bool areTypesCompatible(TypeContainer *left, TypeContainer *right)
{
    if (!left || !right)
        return false;

    // Check base type match
    if (left->baseType != right->baseType)
        return false;

    // Check array dimensions
    if (left->isArray != right->isArray)
        return false;
    if (left->isArray && left->arrayDimensions != right->arrayDimensions)
        return false;

    // Check specific type details
    switch (left->baseType)
    {
    case PRIMITIVE_TYPE:
        return left->primitive == right->primitive;

    case STRUCT_TYPE:
        return strcmp(left->custom.name, right->custom.name) == 0;

    default:
        return false;
    }
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
    case UNSET_TYPE:
        return "UNSET_TYPE";
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
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n", type->size, type->propertyCount, type->methodCount);
    printf("   Name: %s | HDV: %s | Has Constructor: %s\n", type->name, type->hasDefaultValue ? "true" : "false", type->hasConstructor ? "true" : "false");
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
        printf(" (%s)", type->custom.name);
        break;

    default:
        break;
    }
    printf("\n");
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
