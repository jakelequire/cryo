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
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
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

DataType *parseDataType(const char *typeStr)
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

    // First create the base type container
    TypeContainer *container = createTypeContainer();
    if (!container)
    {
        free(baseTypeStr);
        return NULL;
    }

    // Handle primitive types
    if (strcmp(baseTypeStr, "int") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_INT;
    }
    else if (strcmp(baseTypeStr, "float") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_FLOAT;
    }
    else if (strcmp(baseTypeStr, "string") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_STRING;
    }
    else if (strcmp(baseTypeStr, "boolean") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_BOOLEAN;
    }
    else if (strcmp(baseTypeStr, "void") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_VOID;
    }
    else if (strcmp(baseTypeStr, "null") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_NULL;
    }
    else
    {
        // Handle struct/custom types here
        container->baseType = STRUCT_TYPE;
        container->custom.name = strdup(baseTypeStr);
    }

    // Handle array types
    if (isArray)
    {
        TypeContainer *arrayContainer = createArrayType(container, 1);
        free(container); // Free the base container since createArrayType makes a copy
        container = arrayContainer;
    }

    // Wrap in DataType
    DataType *dataType = wrapTypeContainer(container);
    free(container); // wrapTypeContainer makes a copy
    free(baseTypeStr);

    return dataType;
}

PrimitiveDataType getPrimativeTypeFromString(const char *typeStr)
{
    if (strcmp(typeStr, "int") == 0)
    {
        return PRIM_INT;
    }
    else if (strcmp(typeStr, "float") == 0)
    {
        return PRIM_FLOAT;
    }
    else if (strcmp(typeStr, "string") == 0)
    {
        return PRIM_STRING;
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        return PRIM_BOOLEAN;
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        return PRIM_VOID;
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        return PRIM_NULL;
    }
    else
    {
        return PRIM_UNKNOWN;
    }
}

bool isPrimitiveType(const char *typeStr)
{
    return getPrimativeTypeFromString(typeStr) != PRIM_UNKNOWN;
}

// # =========================================================================== #
// # Primitive Type Creation Functions
// # =========================================================================== #

// Primitive int Type `(default/baseline)`
DataType *createPrimitiveIntType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_INT;

    return wrapTypeContainer(container);
}

// Primitive float Type creation `(default/baseline)`
DataType *createPrimitiveFloatType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_FLOAT;

    return wrapTypeContainer(container);
}

// Primitive string Type creation `(default/baseline)`
DataType *createPrimitiveStringType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_STRING;

    return wrapTypeContainer(container);
}

// Primitive boolean Type creation `(default/baseline)`
DataType *createPrimitiveBooleanType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_BOOLEAN;

    return wrapTypeContainer(container);
}

// Primitive void Type creation `(default/baseline)`
DataType *createPrimitiveVoidType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_VOID;

    return wrapTypeContainer(container);
}

// Primitive null Type creation `(default/baseline)`
DataType *createPrimitiveNullType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = PRIMITIVE_TYPE;
    container->primitive = PRIM_NULL;

    return wrapTypeContainer(container);
}

// Unknown Type `(will be resolved later in the compiler)`
DataType *createUnknownType(void)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = UNKNOWN_TYPE;

    return wrapTypeContainer(container);
}

// # =========================================================================== #
// # Specialized Type Creation Functions
// # =========================================================================== #

// # ========================================================= #
// # Primitive Types

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

// # ========================================================= #
// # Structs

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

// Creates a struct type from an ASTNode. `This will also add the struct to the type table.`
StructType *createStructTypeFromStructNode(ASTNode *structNode, CompilerState *state, TypeTable *typeTable)
{
    StructType *structType = (StructType *)malloc(sizeof(StructType));
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate StructType\n");
        return NULL;
    }

    structType->name = strdup(structNode->data.structNode->name);
    structType->size = 0;
    structType->propertyCount = 0;
    structType->methodCount = 0;
    structType->properties = NULL;
    structType->methods = NULL;
    structType->hasDefaultValue = false;
    structType->hasConstructor = false;

    // Add struct to type table
    addTypeToTypeTable(typeTable, structType->name, createStructType(structType->name, structType));

    return structType;
}

DataType *createDataTypeFromStruct(StructType *structType, CompilerState *state, TypeTable *typeTable)
{
    return wrapTypeContainer(createStructType(structType->name, structType));
}

// # ========================================================= #
// # Arrays

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
    DataType *type = (DataType *)malloc(sizeof(DataType));
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
        DataType **newTypes = (DataType **)realloc(table->types, newCapacity * sizeof(DataType *));
        if (!newTypes)
            return;

        table->types = newTypes;
        table->capacity = newCapacity;
    }

    DataType *newType = (DataType *)malloc(sizeof(DataType));
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
    case PRIM_UNKNOWN:
        return LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET;
    default:
        return LIGHT_RED BOLD "<PRIMITIVE UNKNOWN>" COLOR_RESET;
    }
}

char *DataTypeToString(DataType *dataType)
{
    if (!dataType)
        return "<NULL DATATYPE>";

    char *typeString = (char *)malloc(128);
    if (!typeString)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type string.\n");
        return NULL;
    }

    switch (dataType->container.baseType)
    {
    case PRIMITIVE_TYPE:
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, PrimitiveDataTypeToString(dataType->container.primitive));
        break;

    case STRUCT_TYPE:
        sprintf(typeString, LIGHT_CYAN BOLD "%s" COLOR_RESET, dataType->container.custom.name);
        break;

    default:
        sprintf(typeString, LIGHT_RED BOLD "<UNKNOWN>" COLOR_RESET);
        break;
    }

    return typeString;
}

void printFormattedStructType(StructType *type)
{
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   STRUCT_TYPE" COLOR_RESET " | Size: %d | Prop Count: %d | Method Count: %d\n", type->size, type->propertyCount, type->methodCount);
    printf("   Name: %s | HDV: %s | Has Constructor: %s\n", type->name, type->hasDefaultValue ? "true" : "false", type->hasConstructor ? "true" : "false");
}

void printFormattedPrimitiveType(PrimitiveDataType type)
{
    printf("   ────────────────────────────────────────────────────────────\n");
    printf(BOLD GREEN "   PRIMITIVE_TYPE" COLOR_RESET " | %s\n", PrimitiveDataTypeToString(type));
}

void printFormattedType(DataType *type)
{
    if (!type)
        return;

    switch (type->container.baseType)
    {
    case PRIMITIVE_TYPE:
        printFormattedPrimitiveType(type->container.primitive);
        break;

    case STRUCT_TYPE:
        printFormattedStructType(type->container.custom.structDef);
        break;

    default:
        printf("  ────────────────────────────────────────────────────────────\n");
        printf(BOLD GREEN "  UNKNOWN_TYPE" COLOR_RESET "\n");
        break;
    }
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

DataType *CryoDataTypeStringToType(const char *typeStr)
{
    if (strcmp(typeStr, "int") == 0)
    {
        return createPrimitiveIntType();
    }
    else if (strcmp(typeStr, "float") == 0)
    {
        return createPrimitiveFloatType();
    }
    else if (strcmp(typeStr, "string") == 0)
    {
        return createPrimitiveStringType();
    }
    else if (strcmp(typeStr, "boolean") == 0)
    {
        return createPrimitiveBooleanType();
    }
    else if (strcmp(typeStr, "void") == 0)
    {
        return createPrimitiveVoidType();
    }
    else if (strcmp(typeStr, "null") == 0)
    {
        return createPrimitiveNullType();
    }
    else
    {
        return createUnknownType();
    }
}