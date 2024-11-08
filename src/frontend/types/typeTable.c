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

// Create DataType wrapper
DataType *wrapTypeContainer(TypeContainer *container)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate DataType\n");
        return NULL;
    }

    type->container = container;
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
        if (type->container->custom.name &&
            strcmp(type->container->custom.name, name) == 0)
        {
            return type->container;
        }
    }
    return NULL;
}

// # =========================================================================== #
// # Add Type to Type Table
void addTypeToTypeTable(TypeTable *table, const char *name, DataType *type)
{
    if (!name || !type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid name or type in addTypeToTypeTable\n");
        return;
    }

    // Check if the type already exists
    TypeContainer *existingType = lookupType(table, name);
    if (existingType)
    {
        fprintf(stderr, "[TypeTable] Error: Type '%s' already exists in the type table\n", name);
        return;
    }

    // Add the type to the table
    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->types = (DataType **)realloc(table->types, table->capacity * sizeof(DataType *));
        if (!table->types)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to reallocate memory for type table\n");
            return;
        }
    }

    table->types[table->count++] = type;
}

DataType *getDataTypeFromASTNode(ASTNode *node, CompilerState *state, TypeTable *typeTable)
{
    if (!node)
        return NULL;

    switch (node->metaData->type)
    {
    case NODE_PROPERTY_ACCESS:
    {
        // Get the object type
        DataType *objectType = getDataTypeFromASTNode(node->data.propertyAccess->object, state, typeTable);
        if (!objectType)
            return NULL;

        // Get the property name
        const char *propertyName = node->data.propertyAccess->property;

        // Get the struct type
        if (objectType->container->baseType == STRUCT_TYPE)
        {
            StructType *structType = objectType->container->custom.structDef;
            if (!structType)
            {
                fprintf(stderr, "[TypeTable] Error: Failed to get struct type from property access.\n");
                return NULL;
            }

            // Find the property in the struct
            for (int i = 0; i < structType->propertyCount; i++)
            {
                ASTNode *property = structType->properties[i];
                if (strcmp(property->data.property->name, propertyName) == 0)
                {
                    return property->data.property->type;
                }
            }

            fprintf(stderr, "[TypeTable] Error: Property '%s' not found in struct '%s'.\n", propertyName, structType->name);
            return NULL;
        }
        else
        {
            fprintf(stderr, "[TypeTable] Error: Property access on non-struct type.\n");
            return NULL;
        }
    }

    default:
        return NULL;
    }
}

ASTNode *findStructProperty(StructType *structType, const char *propertyName)
{
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid struct type.\n");
        fprintf(stderr, "[TypeTable] Error: Failed to find property '%s' in struct.\n", propertyName);
        return NULL;
    }

    logMessage("INFO", __LINE__, "TypeTable", "Finding property '%s' in struct '%s'", propertyName, structType->name);
    logStructType(structType);

    for (int i = 0; i < structType->propertyCount; i++)
    {
        ASTNode *property = structType->properties[i];
        if (strcmp(property->data.property->name, propertyName) == 0)
        {
            return property;
        }
    }

    return NULL;
}

// # =========================================================================== #
// # Utility Functions

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

DataType *DataTypeFromNode(ASTNode *node)
{
    CryoNodeType nodeType = node->metaData->type;

    switch (nodeType)
    {
    case NODE_LITERAL_EXPR:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from literal expression");
        return node->data.literal->type;
    case NODE_VAR_NAME:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from var name");
        return node->data.varName->type;
    case NODE_STRUCT_DECLARATION:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from struct declaration");
        return node->data.structNode->type;
    case NODE_FUNCTION_DECLARATION:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from function declaration");
        return node->data.functionDecl->type;
    case NODE_PROPERTY:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from property");
        return node->data.property->type;
    case NODE_PARAM:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from param");
        return node->data.param->type;
    case NODE_VAR_DECLARATION:
        logMessage("INFO", __LINE__, "TypeTable", "Getting data type from var declaration");
        return node->data.varDecl->type;
    default:
    {
        fprintf(stderr, "[TypeTable] Error: Failed to get data type from node.\n");
        printf("Received node type: %s\n", CryoNodeTypeToString(nodeType));
        CONDITION_FAILED;
    }
    }

    return NULL;
}
