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
#include "diagnostics/diagnostics.h"

// # ========================================================= #
// # Structs

// Create an empty struct definition
DataType *createStructDefinition(const char *structName)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    structType->properties = (ASTNode **)malloc(sizeof(ASTNode *) * PROPERTY_CAPACITY);
    structType->methods = (ASTNode **)malloc(sizeof(ASTNode *) * METHOD_CAPACITY);
    structType->hasDefaultValue = structNode->data.structNode->hasDefaultValue;

    structType->hasConstructor = structNode->data.structNode->hasConstructor;
    structType->ctorParamCount = structNode->data.structNode->ctorArgCount;
    structType->ctorParamCapacity = structNode->data.structNode->ctorArgCapacity;
    // structType->ctorParams = getTypeArrayFromASTNode(structNode->data.structNode->ctorArgs);
    structType->ctorParams = NULL;

    return structType;
}

DataType *createDataTypeFromStructNode(
    ASTNode *structNode, ASTNode **properties, int propCount,
    ASTNode **methods, int methodCount,
    CompilerState *state, TypeTable *typeTable)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "DataTypes", "Creating data type from struct node: %s", structNode->data.structNode->name);
    StructType *structType = createStructTypeFromStructNode(structNode, state, typeTable);
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create struct type from node.\n");
        CONDITION_FAILED;
    }
    logMessage(LMI, "INFO", "DataTypes", "Created struct type from node: %s", structType->name);

    addPropertiesToStruct(properties, propCount, structType);
    logMessage(LMI, "INFO", "DataTypes", "Added properties to struct: %s", structType->name);

    addMethodsToStruct(methods, methodCount, structType);
    logMessage(LMI, "INFO", "DataTypes", "Added methods to struct: %s", structType->name);

    logMessage(LMI, "INFO", "DataTypes", "Creating data type from struct node: %s", structType->name);
    const char *typeName = structType->name;
    logMessage(LMI, "INFO", "DataTypes", "Type name: %s", typeName);
    TypeContainer *structContainer = createStructType(typeName, structType);
    logMessage(LMI, "INFO", "DataTypes", "Created struct type: %s", structType->name);

    DataType *dataType = wrapTypeContainer(structContainer);
    dataType->container->baseType = STRUCT_TYPE;
    dataType->container->primitive = PRIM_CUSTOM;

    return dataType;
}

void addPropertiesToStruct(ASTNode **properties, int propCount, StructType *structType)
{
    __STACK_FRAME__
    if (!properties || propCount <= 0)
    {
        logMessage(LMI, "INFO", "DataTypes", "No properties to add to struct: %s", structType->name);
        return;
    }
    if (structType->propertyCount + propCount >= structType->propertyCapacity)
    {
        // Grow properties array
        logMessage(LMI, "INFO", "DataTypes", "Growing properties array for struct: %s", structType->name);
        int newCapacity = structType->propertyCapacity * 2;
        ASTNode **newProperties = (ASTNode **)realloc(structType->properties, newCapacity * sizeof(ASTNode *));
        if (!newProperties)
            return;

        structType->properties = newProperties;
        structType->propertyCapacity = newCapacity;
        logMessage(LMI, "INFO", "DataTypes", "Grew properties array for struct: %s", structType->name);
    }

    // Add properties to struct
    logASTNode(properties[0]);
    for (int i = 0; i < propCount; i++)
    {
        logMessage(LMI, "INFO", "DataTypes", "Added property to struct: %s", properties[i]->data.property->name);
        structType->properties[structType->propertyCount++] = properties[i];
    }

    logMessage(LMI, "INFO", "DataTypes", "Added properties to struct: %s", structType->name);

    return;
}

void addMethodsToStruct(ASTNode **methods, int methodCount, StructType *structType)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    DataType *type = lookupType(table, name);
    if (!type)
        return false;

    return type->container->baseType == STRUCT_TYPE;
}

bool isStructType(DataType *type)
{
    __STACK_FRAME__
    return type->container->baseType == STRUCT_TYPE;
}
