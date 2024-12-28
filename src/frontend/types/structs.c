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

// # ========================================================= #
// # Structs

// Create an empty struct definition
DataType *createStructDefinition(const char *structName)
{
    StructType *structDef = (StructType *)malloc(sizeof(StructType));
    if (!structDef)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate StructType\n");
        return NULL;
    }

    structDef->name = structName;
    structDef->size = 0;
    structDef->propertyCount = 0;
    structDef->methodCount = 0;
    structDef->properties = NULL;
    structDef->methods = NULL;
    structDef->hasDefaultValue = false;
    structDef->hasConstructor = false;

    return wrapStructType(structDef);
}

// Create struct type
TypeContainer *createStructType(const char *name, StructType *structDef)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = STRUCT_TYPE;
    container->custom.name = name;
    container->custom.structDef = structDef;
    container->primitive = PRIM_CUSTOM;

    return container;
}

// Creates a struct type from an ASTNode.
StructType *createStructTypeFromStructNode(ASTNode *structNode, CompilerState *state, TypeTable *typeTable)
{
    StructType *structType = (StructType *)malloc(sizeof(StructType));
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate StructType\n");
        return NULL;
    }

    printf("Creating struct type from node: %s\n", structNode->data.structNode->name);

    structType->name = strdup(structNode->data.structNode->name);
    structType->size = 0;
    structType->propertyCount = 0;
    structType->methodCount = 0;
    structType->properties = (ASTNode **)malloc(sizeof(ASTNode *) * PROPERTY_CAPACITY);
    structType->methods = (ASTNode **)malloc(sizeof(ASTNode *) * METHOD_CAPACITY);
    structType->hasDefaultValue = structNode->data.structNode->hasDefaultValue;

    structType->hasConstructor = structNode->data.structNode->hasConstructor;
    structType->ctorParamCount = structNode->data.structNode->ctorArgCount;
    structType->ctorParamCapacity = structNode->data.structNode->ctorArgCapacity;
    // structType->ctorParams = getTypeArrayFromASTNode(structNode->data.structNode->ctorArgs);
    structType->ctorParams = NULL;

    printf("Struct Created, name: %s\n", structType->name);

    return structType;
}

DataType *createDataTypeFromStructNode(
    ASTNode *structNode, ASTNode **properties, int propCount,
    ASTNode **methods, int methodCount,
    CompilerState *state, TypeTable *typeTable)
{
    logMessage("INFO", __LINE__, "DataTypes", "Creating data type from struct node: %s", structNode->data.structNode->name);
    StructType *structType = createStructTypeFromStructNode(structNode, state, typeTable);
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create struct type from node.\n");
        CONDITION_FAILED;
    }
    logMessage("INFO", __LINE__, "DataTypes", "Created struct type from node: %s", structType->name);

    addPropertiesToStruct(properties, propCount, structType);
    logMessage("INFO", __LINE__, "DataTypes", "Added properties to struct: %s", structType->name);

    addMethodsToStruct(methods, methodCount, structType);
    logMessage("INFO", __LINE__, "DataTypes", "Added methods to struct: %s", structType->name);

    logMessage("INFO", __LINE__, "DataTypes", "Creating data type from struct node: %s", structType->name);
    const char *typeName = structType->name;
    logMessage("INFO", __LINE__, "DataTypes", "Type name: %s", typeName);
    TypeContainer *structContainer = createStructType(typeName, structType);
    logMessage("INFO", __LINE__, "DataTypes", "Created struct type: %s", structType->name);

    DataType *dataType = wrapTypeContainer(structContainer);
    dataType->container->baseType = STRUCT_TYPE;
    dataType->container->primitive = PRIM_CUSTOM;

    return dataType;
}

void addPropertiesToStruct(ASTNode **properties, int propCount, StructType *structType)
{
    if (!properties || propCount <= 0)
    {
        logMessage("INFO", __LINE__, "DataTypes", "No properties to add to struct: %s", structType->name);
        return;
    }
    if (structType->propertyCount + propCount >= structType->propertyCapacity)
    {
        // Grow properties array
        logMessage("INFO", __LINE__, "DataTypes", "Growing properties array for struct: %s", structType->name);
        int newCapacity = structType->propertyCapacity * 2;
        ASTNode **newProperties = (ASTNode **)realloc(structType->properties, newCapacity * sizeof(ASTNode *));
        if (!newProperties)
            return;

        structType->properties = newProperties;
        structType->propertyCapacity = newCapacity;
        logMessage("INFO", __LINE__, "DataTypes", "Grew properties array for struct: %s", structType->name);
    }

    // Add properties to struct
    printf("Property Count: %d\n", propCount);
    logASTNode(properties[0]);
    for (int i = 0; i < propCount; i++)
    {
        logMessage("INFO", __LINE__, "DataTypes", "Added property to struct: %s", properties[i]->data.property->name);
        structType->properties[structType->propertyCount++] = properties[i];
    }

    logMessage("INFO", __LINE__, "DataTypes", "Added properties to struct: %s", structType->name);

    return;
}

void addMethodsToStruct(ASTNode **methods, int methodCount, StructType *structType)
{
    if (!methods || methodCount <= 0)
        return;

    if (structType->methodCount + methodCount >= structType->methodCapacity)
    {
        // Grow methods array
        int newCapacity = structType->methodCapacity * 2;
        ASTNode **newMethods = (ASTNode **)realloc(structType->methods, newCapacity * sizeof(ASTNode *));
        if (!newMethods)
            return;

        structType->methods = newMethods;
        structType->methodCapacity = newCapacity;
    }

    // Add methods to struct
    for (int i = 0; i < methodCount; i++)
    {
        structType->methods[structType->methodCount++] = methods[i];
    }
}

int getPropertyAccessIndex(DataType *type, const char *propertyName)
{
    if (!type)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid data type.\n");
        return -1;
    }

    if (type->container->baseType == STRUCT_TYPE)
    {
        StructType *structType = type->container->custom.structDef;
        if (!structType)
        {
            fprintf(stderr, "[TypeTable] Error: Invalid struct type.\n");
            return -1;
        }

        for (int i = 0; i < structType->propertyCount; i++)
        {
            ASTNode *property = structType->properties[i];
            if (strcmp(property->data.property->name, propertyName) == 0)
            {
                return i;
            }
        }

        fprintf(stderr, "[TypeTable] Error: Property '%s' not found in struct '%s'.\n", propertyName, structType->name);
        return -1;
    }
    else
    {
        fprintf(stderr, "[TypeTable] Error: Property access on non-struct type.\n");
        return -1;
    }
}

int calculateStructSize(StructType *structType)
{
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid struct type.\n");
        return -1;
    }

    int size = 0;
    for (int i = 0; i < structType->propertyCount; i++)
    {
        ASTNode *property = structType->properties[i];
        DataType *propertyType = getDataTypeFromASTNode(property);
        if (!propertyType)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type for property.\n");
            return -1;
        }

        size += propertyType->container->size;
    }

    return size;
}

DataType *wrapStructType(StructType *structDef)
{
    TypeContainer *container = createStructType(structDef->name, structDef);
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to wrap struct type.\n");
        return NULL;
    }

    return wrapTypeContainer(container);
}

bool isStructDeclaration(TypeTable *table, const char *name)
{
    DataType *type = lookupType(table, name);
    if (!type)
        return false;

    return type->container->baseType == STRUCT_TYPE;
}

bool isStructType(DataType *type)
{
    return type->container->baseType == STRUCT_TYPE;
}
