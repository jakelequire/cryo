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

void initGenericType(GenericType *type, const char *name)
{
    type->name = strdup(name);
    type->constraint = NULL;
    type->genericParams = NULL;
    type->genericParamCount = 0;
    type->genericParamCapacity = 0;
    type->arrayInfo.isArray = false;
    type->arrayInfo.arrayDimensions = 0;
    type->next = NULL;
}

// Function to create a new generic parameter
GenericType *createGenericParameter(const char *name)
{
    GenericType *param = (GenericType *)malloc(sizeof(GenericType));
    initGenericType(param, name);
    return param;
}

TypeContainer *createGenericArrayType(DataType *genericParam)
{
    TypeContainer *container = createTypeContainer();

    // This is an array type
    container->isArray = true;
    container->arrayDimensions = 1;

    // Store the generic parameter T
    container->custom.genericParamCount = 1;
    container->custom.genericParams = (DataType **)malloc(sizeof(DataType *));
    container->custom.genericParams[0] = genericParam;

    return container;
}

TypeContainer *createGenericStructType(const char *name, StructType *structDef, DataType *genericParam)
{
    TypeContainer *container = createTypeContainer();

    container->baseType = STRUCT_TYPE;
    container->custom.name = strdup(name); // "Array"
    container->custom.structDef = structDef;

    // Set up the generic parameter
    container->custom.genericParamCount = 1;
    container->custom.genericParams = (DataType **)malloc(sizeof(DataType *));
    container->custom.genericParams[0] = genericParam; // T

    return container;
}

// Function to add constraint to generic type
void addGenericConstraint(GenericType *type, DataType *constraint)
{
    type->constraint = constraint;
}

// Function to handle array types of generic parameters
void setGenericArrayInfo(GenericType *type, int dimensions)
{
    type->arrayInfo.isArray = true;
    type->arrayInfo.arrayDimensions = dimensions;
}

// Function to link multiple generic parameters
void linkGenericParameter(GenericType *base, GenericType *next)
{
    GenericType *current = base;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = next;
}

// Function to validate generic type usage
bool validateGenericType(GenericType *type, DataType *concrete_type)
{
    if (type->constraint != NULL)
    {
        // Check if concrete_type satisfies the constraint
        // This would need to be implemented based on your type system rules
        return areTypesCompatible(type->constraint->container, concrete_type->container);
    }
    return true;
}

TypeContainer *createGenericStructInstance(TypeContainer *genericDef, DataType *concreteType)
{
    TypeContainer *container = createTypeContainer();

    container->baseType = STRUCT_TYPE;

    // Create name like "Array<int>"
    char *instanceName = malloc(strlen(genericDef->custom.name) +
                                strlen(concreteType->container->custom.name) + 3);
    sprintf(instanceName, "%s<%s>", genericDef->custom.name,
            concreteType->container->custom.name);
    container->custom.name = instanceName;

    // Create concrete struct definition where T is replaced with int
    container->custom.structDef = substituteGenericType(
        genericDef->custom.structDef,
        genericDef->custom.genericParams[0],
        concreteType);

    // No generic parameters in concrete instance
    container->custom.genericParamCount = 0;
    container->custom.genericParams = NULL;

    return container;
}

TypeContainer *createGenericInstance(StructType *baseStruct, DataType *concreteType)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = STRUCT_TYPE;

    // Create a new struct type
    StructType *instance = malloc(sizeof(StructType));
    instance->name = (char *)malloc(strlen(baseStruct->name) + strlen(concreteType->container->custom.name) + 3);
    sprintf(instance->name, "%s<%s>", baseStruct->name, concreteType->container->custom.name);

    // Initialize arrays
    instance->propertyCapacity = baseStruct->propertyCapacity;
    instance->methodCapacity = baseStruct->methodCapacity;
    instance->properties = malloc(sizeof(ASTNode *) * instance->propertyCapacity);
    instance->methods = malloc(sizeof(ASTNode *) * instance->methodCapacity);

    // Deep copy and substitute generic types in properties
    instance->propertyCount = baseStruct->propertyCount;
    for (int i = 0; i < baseStruct->propertyCount; i++)
    {
        // Create new AST node for the property
        ASTNode *originalProp = baseStruct->properties[i];
        ASTNode *newProp = (ASTNode *)malloc(sizeof(ASTNode));

        // Copy basic property info
        *newProp = *originalProp; // Shallow copy first

        // If the property type is our generic type, substitute it
        if (isGenericType(getDataTypeFromASTNode(originalProp)) &&
            strcmp(getGenericTypeName(getDataTypeFromASTNode(originalProp)), "T") == 0)
        {
            setNewDataTypeForNode(newProp, concreteType);
        }
        else
        {
            // Deep copy the data type if it's not being substituted
            setNewDataTypeForNode(newProp, cloneDataType(getDataTypeFromASTNode(originalProp)));
        }

        instance->properties[i] = newProp;
    }

    // Deep copy and substitute generic types in methods
    instance->methodCount = baseStruct->methodCount;
    for (int i = 0; i < baseStruct->methodCount; i++)
    {
        // Similar process for methods, but we also need to:
        // 1. Update parameter types that use T
        // 2. Update return types that use T
        // 3. Update any local variables in the method body that use T
        instance->methods[i] = cloneAndSubstituteGenericMethod(
            baseStruct->methods[i],
            concreteType);
    }

    instance->hasDefaultValue = baseStruct->hasDefaultValue;
    instance->hasConstructor = baseStruct->hasConstructor;
    instance->size = calculateStructSize(instance); // Recalculate size with concrete types

    container->custom.structDef = instance;
    container->custom.name = strdup(instance->name);

    return container;
}

bool isGenericType(TypeContainer *type)
{
    return type->custom.genericParamCount > 0;
}

bool isGenericInstance(TypeContainer *type)
{
    return type->baseType == STRUCT_TYPE &&
           strstr(type->custom.name, "<") != NULL;
}

DataType *getGenericParameter(TypeContainer *type, int index)
{
    if (index >= 0 && index < type->custom.genericParamCount)
    {
        return type->custom.genericParams[index];
    }
    return NULL;
}
