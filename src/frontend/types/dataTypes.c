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

TypeTable *initTypeTable(void)
{
    TypeTable *table = (TypeTable *)malloc(sizeof(TypeTable));
    if (!table)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table.\n");
        CONDITION_FAILED;
    }

    table->types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!table->types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type table types.\n");
        CONDITION_FAILED;
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
        fprintf(stderr, "[DataTypes] Error: Failed to allocate TypeContainer\n");
        CONDITION_FAILED;
    }

    container->baseType = UNKNOWN_TYPE;
    container->primitive = PRIM_UNKNOWN;
    container->size = 0;
    container->length = 0;
    container->isArray = false;
    container->arrayDimensions = 0;
    container->custom.name = (char *)malloc(sizeof(char) * 64);
    container->custom.structDef = NULL;
    container->custom.funcDef = NULL;
    container->custom.structDef = NULL;
    container->custom.generic.declaration = NULL;
    container->custom.generic.instantiation = NULL;

    return container;
}

DataType *parseDataType(const char *typeStr, TypeTable *typeTable)
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
        CONDITION_FAILED;
    }

    // Handle primitive types
    if (strcmp(baseTypeStr, "int") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_INT;
    }
    else if (strcmp(baseTypeStr, "i8") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I8;
    }
    else if (strcmp(baseTypeStr, "i16") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I16;
    }
    else if (strcmp(baseTypeStr, "i32") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I32;
    }
    else if (strcmp(baseTypeStr, "i64") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I64;
    }
    else if (strcmp(baseTypeStr, "i128") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_I128;
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
    else if (strcmp(baseTypeStr, "any") == 0)
    {
        container->baseType = PRIMITIVE_TYPE;
        container->primitive = PRIM_ANY;
    }
    else
    {
        // This is where we handle custom types.
        // Get the name of the type:
        char *typeName = strdup(baseTypeStr);
        // Look up the type in the type table
        logMessage("INFO", __LINE__, "DataTypes", "Looking up custom type '%s' in type table", typeName);
        printTypeTable(typeTable);
        TypeContainer *customType = lookupType(typeTable, typeName)->container;
        if (!customType)
        {
            fprintf(stderr, "[TypeTable] Error: Custom type '%s' not found in type table.\n", typeName);
            free(container);
            free(baseTypeStr);
            free(typeName);
            CONDITION_FAILED;
        }

        // Set the custom type
        container->baseType = customType->baseType;
        if (container->baseType == STRUCT_TYPE)
        {
            container->custom.structDef = customType->custom.structDef;
        }
        else if (container->baseType == FUNCTION_TYPE)
        {
            container->custom.funcDef = customType->custom.funcDef;
        }
        else
        {
            fprintf(stderr, "[TypeTable] Error: Invalid custom type '%s' in type table.\n", typeName);
            free(container);
            free(baseTypeStr);
            free(typeName);
            CONDITION_FAILED;
        }
    }

    // Handle array types
    if (isArray)
    {
        logMessage("INFO", __LINE__, "DataTypes", "Creating array type for '%s'", baseTypeStr);
        TypeContainer *arrayContainer = createArrayType(container, 1);
        free(container); // Free the base container since createArrayType makes a copy
        container = arrayContainer;
    }

    // Wrap in DataType
    DataType *dataType = wrapTypeContainer(container);

    return dataType;
}

// Create DataType wrapper
DataType *wrapTypeContainer(TypeContainer *container)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate DataType\n");
        CONDITION_FAILED;
    }

    type->container = container;
    type->isConst = false;
    type->isReference = false;
    type->next = NULL;
    type->genericParam = NULL;

    return type;
}

DataType *lookupType(TypeTable *table, const char *name)
{
    for (int i = 0; i < table->count; i++)
    {
        DataType *type = table->types[i];
        if (type->container->custom.name &&
            strcmp(type->container->custom.name, name) == 0)
        {
            logMessage("INFO", __LINE__, "DataTypes", "Found type '%s' in type table", name);
            return type;
        }
    }

    logMessage("INFO", __LINE__, "DataTypes", "Type '%s' not found in type table", name);
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
    logMessage("INFO", __LINE__, "DataTypes", "Looking up type '%s' in type table", name);
    DataType *existingType = lookupType(table, name);
    if (existingType)
    {
        fprintf(stderr, "[TypeTable] Error: Type '%s' already exists in the type table\n", name);
        // Update the existing type
        updateTypeInTypeTable(table, name, type);
        return;
    }

    logMessage("INFO", __LINE__, "DataTypes", "Adding type '%s' to type table", name);
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

    logMessage("INFO", __LINE__, "DataTypes", "Adding type '%s' to type table, count: %d", name, table->count);
    table->types[table->count++] = type;
    logMessage("INFO", __LINE__, "DataTypes", "Added type '%s' to type table", name);
}

ASTNode *findStructProperty(StructType *structType, const char *propertyName)
{
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid struct type.\n");
        fprintf(stderr, "[TypeTable] Error: Failed to find property '%s' in struct.\n", propertyName);
        CONDITION_FAILED;
    }

    logMessage("INFO", __LINE__, "DataTypes", "Finding property '%s' in struct '%s'", propertyName, structType->name);

    for (int i = 0; i < structType->propertyCount; i++)
    {
        ASTNode *property = structType->properties[i];
        if (strcmp(property->data.property->name, propertyName) == 0)
        {
            return property;
        }
    }

    CONDITION_FAILED;
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
        return createPrimitiveStringType(0);
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
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from literal expression");
        return node->data.literal->type;
    case NODE_VAR_NAME:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from var name");
        return node->data.varName->type;
    case NODE_STRUCT_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from struct declaration");
        return node->data.structNode->type;
    case NODE_FUNCTION_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from function declaration");
        return node->data.functionDecl->type;
    case NODE_PROPERTY:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from property");
        return node->data.property->type;
    case NODE_PARAM:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from param");
        return node->data.param->type;
    case NODE_VAR_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from var declaration");
        return node->data.varDecl->type;
    case NODE_CLASS:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from class");
        return node->data.classNode->type;
    default:
    {
        fprintf(stderr, "[TypeTable] Error: Failed to get data type from node.\n");
        printf("Received node type: %s\n", CryoNodeTypeToString(nodeType));
        CONDITION_FAILED;
    }
    }

    CONDITION_FAILED;
}

const char *getDataTypeName(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid data type. @getDataTypeName\n");
        CONDITION_FAILED;
    }

    if (type->container->baseType == PRIMITIVE_TYPE)
    {
        return PrimitiveDataTypeToString_UF(type->container->primitive);
    }
    else if (type->container->baseType == STRUCT_TYPE)
    {
        return type->container->custom.structDef->name;
    }
    else if (type->container->baseType == CLASS_TYPE)
    {
        return type->container->custom.classDef->name;
    }
    else if (type->container->baseType == FUNCTION_TYPE)
    {
        return type->container->custom.funcDef->name;
    }
    else
    {
        return "unknown";
    }
}

// Very unsafe function. Will only allocate memory for the first 64 types.
DataType **getTypeArrayFromASTNode(ASTNode **node)
{
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        CONDITION_FAILED;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type array.\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < 64; i++)
    {
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from AST node");
        if (!node[i])
        {
            char *nodeType = CryoNodeTypeToString(node[i]->metaData->type);
            logMessage("INFO", __LINE__, "DataTypes", "Node type: %s", nodeType);
            logMessage("INFO", __LINE__, "DataTypes", "Node is NULL, breaking loop");
            break;
        }
        types[i] = DataTypeFromNode(node[i]);
        logMessage("INFO", __LINE__, "DataTypes", "Got data type from AST node");
    }

    return types;
}

DataType *getDataTypeFromASTNode(ASTNode *node)
{
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid AST node.\n");
        CONDITION_FAILED;
    }

    switch (node->metaData->type)
    {
    case NODE_LITERAL_EXPR:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from literal expression");
        return node->data.literal->type;
    case NODE_VAR_NAME:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from var name");
        return node->data.varName->type;
    case NODE_STRUCT_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from struct declaration");
        return node->data.structNode->type;
    case NODE_FUNCTION_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from function declaration");
        return node->data.functionDecl->type;
    case NODE_PROPERTY:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from property");
        return node->data.property->type;
    case NODE_PARAM:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from param");
        return node->data.param->type;
    case NODE_VAR_DECLARATION:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from var declaration");
        return node->data.varDecl->type;
    case NODE_METHOD:
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from method");
        return node->data.method->type;
    case NODE_PROPERTY_ACCESS:
    {
        logMessage("INFO", __LINE__, "DataTypes", "Getting data type from property access");
        // Get the object type
        DataType *objectType = getDataTypeFromASTNode(node->data.propertyAccess->object);
        if (!objectType)
            CONDITION_FAILED;

        // Get the property name
        const char *propertyName = node->data.propertyAccess->propertyName;

        // Get the struct type
        if (objectType->container->baseType == STRUCT_TYPE)
        {
            StructType *structType = objectType->container->custom.structDef;
            if (!structType)
            {
                fprintf(stderr, "[TypeTable] Error: Failed to get struct type from property access.\n");
                CONDITION_FAILED;
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
            CONDITION_FAILED;
        }
        else
        {
            fprintf(stderr, "[TypeTable] Error: Property access on non-struct type.\n");
            CONDITION_FAILED;
        }
    }
    default:
        logMessage("ERROR", __LINE__, "DataTypes", "Failed to get data type from AST node, received node type: %s",
                   CryoNodeTypeToString(node->metaData->type));
        CONDITION_FAILED;
    }
}

void setNewDataTypeForNode(ASTNode *node, DataType *type)
{
    if (!node || !type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node or type.\n");
        return;
    }

    switch (node->metaData->type)
    {
    case NODE_LITERAL_EXPR:
        node->data.literal->type = type;
        break;
    case NODE_VAR_NAME:
        node->data.varName->type = type;
        break;
    case NODE_STRUCT_DECLARATION:
        node->data.structNode->type = type;
        break;
    case NODE_FUNCTION_DECLARATION:
        node->data.functionDecl->type = type;
        break;
    case NODE_PROPERTY:
        node->data.property->type = type;
        break;
    case NODE_PARAM:
        node->data.param->type = type;
        break;
    case NODE_VAR_DECLARATION:
        node->data.varDecl->type = type;
        break;
    default:
        fprintf(stderr, "[TypeTable] Error: Failed to set data type for node, received node type: %s\n",
                CryoNodeTypeToString(node->metaData->type));
        break;
    }
}

DataType *cloneDataType(DataType *type)
{
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid data type. @cloneDataType\n");
        CONDITION_FAILED;
    }

    TypeContainer *container = type->container;
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid type container.\n");
        CONDITION_FAILED;
    }

    TypeContainer *newContainer = createTypeContainer();
    if (!newContainer)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create new type container.\n");
        CONDITION_FAILED;
    }

    newContainer->baseType = container->baseType;
    newContainer->primitive = container->primitive;
    newContainer->size = container->size;
    newContainer->length = container->length;
    newContainer->isArray = container->isArray;
    newContainer->arrayDimensions = container->arrayDimensions;
    newContainer->custom.name = container->custom.name;
    newContainer->custom.structDef = container->custom.structDef;
    newContainer->custom.funcDef = container->custom.funcDef;

    DataType *newType = wrapTypeContainer(newContainer);
    if (!newType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to wrap new type container.\n");
        CONDITION_FAILED;
    }

    newType->isConst = type->isConst;
    newType->isReference = type->isReference;
    newType->next = type->next;
    newType->genericParam = type->genericParam;

    return newType;
}

void updateTypeInTypeTable(TypeTable *table, const char *name, DataType *type)
{
    if (!table || !name || !type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid table, name, or type.\n");
        return;
    }

    for (int i = 0; i < table->count; i++)
    {
        DataType *existingType = table->types[i];
        if (strcmp(existingType->container->custom.name, name) == 0)
        {
            table->types[i] = type;
            return;
        }
    }

    fprintf(stderr, "[TypeTable] Error: Type '%s' not found in type table.\n", name);
}

void importTypesFromRootNode(TypeTable *typeTable, ASTNode *root)
{
    if (!root)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid root node.\n");
        return;
    }

    if (root->metaData->type != NODE_PROGRAM)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid root node type.\n");
        return;
    }

    for (int i = 0; i < root->data.program->statementCount; i++)
    {
        ASTNode *node = root->data.program->statements[i];
        if (node->metaData->type == NODE_STRUCT_DECLARATION)
        {
            const char *name = node->data.structNode->name;
            DataType *type = node->data.structNode->type;
            addTypeToTypeTable(typeTable, name, type);
        }
    }
}

DataType *findClassType(ASTNode *node, TypeTable *typeTable)
{
    if (!node || !typeTable)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node or type table.\n");
        CONDITION_FAILED;
    }

    if (node->metaData->type != NODE_CLASS)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node type.\n");
        CONDITION_FAILED;
    }

    const char *name = node->data.classNode->name;
    DataType *type = lookupType(typeTable, name);

    return type;
}

DataType *findClassTypeFromName(const char *name, TypeTable *typeTable)
{
    if (!name || !typeTable)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid name or type table.\n");
        CONDITION_FAILED;
    }

    DataType *type = lookupType(typeTable, name);

    if (type->container->baseType != CLASS_TYPE)
    {
        fprintf(stderr, "[TypeTable] Error: Type '%s' is not a class type.\n", name);
        CONDITION_FAILED;
    }

    return type;
}

DataType **getParamTypeArray(ASTNode **node)
{
    if (!node)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid node.\n");
        CONDITION_FAILED;
    }

    DataType **types = (DataType **)malloc(sizeof(DataType *) * 64);
    if (!types)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate memory for type array.\n");
        CONDITION_FAILED;
    }

    for (int i = 0; i < 64; i++)
    {
        printf("Viewing Node Type: %s\n", CryoNodeTypeToString(node[i]->metaData->type));
        if (!node[i])
        {
            logMessage("INFO", __LINE__, "DataTypes", "Node is NULL, breaking loop");
            break;
        }
        types[i] = node[i]->data.param->type;
        logMessage("INFO", __LINE__, "DataTypes", "Got data type from AST node");
    }

    logMessage("INFO", __LINE__, "DataTypes", "Returning param type array");
    return types;
}
