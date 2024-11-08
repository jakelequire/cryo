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

// # ========================================================= #
// # Structs

// Create struct type
TypeContainer *createStructType(const char *name, StructType *structDef)
{
    TypeContainer *container = createTypeContainer();
    if (!container)
        return NULL;

    container->baseType = STRUCT_TYPE;
    container->custom.name = name;
    container->custom.structDef = structDef;

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
    structType->properties = NULL;
    structType->methods = NULL;
    structType->hasDefaultValue = false;
    structType->hasConstructor = false;

    return structType;
}

DataType *createDataTypeFromStructNode(ASTNode *structNode, ASTNode **properties, int propCount, CompilerState *state, TypeTable *typeTable)
{
    StructType *structType = createStructTypeFromStructNode(structNode, state, typeTable);
    if (!structType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create struct type from node.\n");
        CONDITION_FAILED;
    }

    addPropertiesToStruct(properties, propCount, structType);

    return wrapTypeContainer(createStructType(structType->name, structType));
}

void addPropertiesToStruct(ASTNode **properties, int propCount, StructType *structType)
{
    if (!properties || propCount <= 0)
        return;

    if (structType->propertyCount + propCount >= structType->propertyCapacity)
    {
        // Grow properties array
        int newCapacity = structType->propertyCapacity * 2;
        ASTNode **newProperties = (ASTNode **)realloc(structType->properties, newCapacity * sizeof(ASTNode *));
        if (!newProperties)
            return;

        structType->properties = newProperties;
        structType->propertyCapacity = newCapacity;
    }

    // Add properties to struct
    for (int i = 0; i < propCount; i++)
    {
        structType->properties[structType->propertyCount++] = properties[i];
    }
}
