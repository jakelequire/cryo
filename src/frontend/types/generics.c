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
    container->custom.generic.declaration = createGenericDeclarationContainer(
        NULL,
        &genericParam,
        1);

    return container;
}

TypeContainer *createGenericStructType(const char *name, StructType *structDef, DataType *genericParam)
{
    TypeContainer *container = createTypeContainer();

    container->baseType = STRUCT_TYPE;
    container->custom.name = strdup(name); // "Array"
    container->custom.structDef = structDef;

    // Set up the generic parameter
    container->custom.generic.declaration = createGenericDeclarationContainer(
        structDef,
        &genericParam,
        1);

    return container;
}

GenericDeclType *createGenericDeclarationContainer(StructType *structDef, DataType **genericParam, int paramCount)
{
    GenericDeclType *decl = (GenericDeclType *)malloc(sizeof(GenericDeclType));
    decl->genericDef = structDef;
    decl->params = genericParam;
    decl->paramCount = paramCount;
    return decl;
}
GenericInstType *createGenericInstanceContainer(StructType *structDef, DataType **typeArgs, int argCount, TypeContainer *baseDef)
{
    GenericInstType *inst = (GenericInstType *)malloc(sizeof(GenericInstType));
    inst->structDef = structDef;
    inst->typeArgs = typeArgs;
    inst->argCount = argCount;
    inst->baseDef = baseDef;
    return inst;
}

// Function to add constraint to generic type
void addGenericConstraint(GenericType *type, DataType *constraint)
{
    type->constraint = constraint;
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

TypeContainer *createGenericStructInstance(TypeContainer *genericDef, DataType *concreteType)
{
    TypeContainer *container = createTypeContainer();

    container->baseType = STRUCT_TYPE;

    // Create name like "Array<int>"
    char *instanceName = (char *)malloc(strlen(genericDef->custom.name) +
                                        strlen(concreteType->container->custom.name) + 3);
    sprintf((char *)instanceName, "%s<%s>", genericDef->custom.name,
            concreteType->container->custom.name);
    container->custom.name = instanceName;

    // Create concrete struct definition where T is replaced with int
    container->custom.structDef = substituteGenericType(
        genericDef->custom.structDef,
        genericDef->custom.structDef->properties[0]->data.param->type,
        concreteType);

    container->custom.generic.declaration = createGenericDeclarationContainer(
        genericDef->custom.structDef,
        &concreteType,
        1);
    return container;
}

TypeContainer *createGenericInstance(StructType *baseStruct, DataType *concreteType)
{
    TypeContainer *container = createTypeContainer();
    container->baseType = STRUCT_TYPE;

    // Create a new struct type
    StructType *instance = (StructType *)malloc(sizeof(StructType));
    instance->name = (char *)malloc(strlen(baseStruct->name) + strlen(concreteType->container->custom.name) + 3);
    sprintf((char *)instance->name, "%s<%s>", baseStruct->name, concreteType->container->custom.name);

    // Initialize arrays
    instance->propertyCapacity = baseStruct->propertyCapacity;
    instance->methodCapacity = baseStruct->methodCapacity;
    instance->properties = (ASTNode **)malloc(sizeof(ASTNode *) * instance->propertyCapacity);
    instance->methods = (ASTNode **)malloc(sizeof(ASTNode *) * instance->methodCapacity);

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

bool isGenericType(DataType *type)
{
    if (type->container->baseType == STRUCT_TYPE)
    {
        StructType *structDef = type->container->custom.structDef;
        return structDef->propertyCount == 1 &&
               structDef->methodCount == 0 &&
               structDef->properties[0]->metaData->type == NODE_PARAM &&
               structDef->properties[0]->data.param->type->container->baseType == GENERIC_TYPE;
    }
}

bool isGenericInstance(TypeContainer *type)
{
    return type->baseType == STRUCT_TYPE &&
           strstr(type->custom.name, "<") != NULL;
}

DataType *getGenericParameter(TypeContainer *type, int index)
{
    if (index >= 0 && index < type->custom.generic.declaration->paramCount)
    {
        return type->custom.generic.declaration->params[index]->container->custom.generic.declaration->params[0];
    }
}

int getGenericParameterCount(TypeContainer *type)
{
    return type->custom.generic.declaration->paramCount;
}

const char *getGenericTypeName(DataType *type)
{
    return type->container->custom.name;
}

// Function to substitute generic types in a struct definition
StructType *substituteGenericType(StructType *structDef, DataType *genericParam, DataType *concreteType)
{
    StructType *instance = (StructType *)malloc(sizeof(StructType));
    instance->name = (char *)malloc(strlen(structDef->name) + strlen(concreteType->container->custom.name) + 3);
    sprintf((char *)instance->name, "%s<%s>", structDef->name, concreteType->container->custom.name);

    // Initialize arrays
    instance->propertyCapacity = structDef->propertyCapacity;
    instance->methodCapacity = structDef->methodCapacity;
    instance->properties = (ASTNode **)malloc(sizeof(ASTNode *) * instance->propertyCapacity);
    instance->methods = (ASTNode **)malloc(sizeof(ASTNode *) * instance->methodCapacity);

    // Deep copy and substitute generic types in properties
    instance->propertyCount = structDef->propertyCount;
    for (int i = 0; i < structDef->propertyCount; i++)
    {
        // Create new AST node for the property
        ASTNode *originalProp = structDef->properties[i];
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
    instance->methodCount = structDef->methodCount;
    for (int i = 0; i < structDef->methodCount; i++)
    {
        // Similar process for methods, but we also need to:
        // 1. Update parameter types that use T
        // 2. Update return types that use T
        // 3. Update any local variables in the method body that use T
        instance->methods[i] = cloneAndSubstituteGenericMethod(
            structDef->methods[i],
            concreteType);
    }

    instance->hasDefaultValue = structDef->hasDefaultValue;
    instance->hasConstructor = structDef->hasConstructor;
    instance->size = calculateStructSize(instance); // Recalculate size with concrete types

    return instance;
}

// Function to clone and substitute generic types in a method definition
ASTNode *cloneAndSubstituteGenericMethod(ASTNode *method, DataType *concreteType)
{
    ASTNode *newMethod = (ASTNode *)malloc(sizeof(ASTNode));
    *newMethod = *method; // Shallow copy first

    // Deep copy the method name
    newMethod->data.method->name = strdup(method->data.method->name);

    // Deep copy the method parameters
    newMethod->data.method->paramCapacity = method->data.method->paramCapacity;
    newMethod->data.method->params = (ASTNode **)malloc(sizeof(ASTNode *) * newMethod->data.method->paramCapacity);
    newMethod->data.method->paramCount = method->data.method->paramCount;
    for (int i = 0; i < method->data.method->paramCount; i++)
    {
        newMethod->data.method->params[i] = cloneAndSubstituteGenericParam(
            method->data.method->params[i],
            concreteType);
    }

    // Deep copy the method body
    newMethod->data.method->body = cloneAndSubstituteGenericBody(
        method->data.method->body,
        concreteType);

    return newMethod;
}

// Function to clone and substitute generic types in a method parameter
ASTNode *cloneAndSubstituteGenericParam(ASTNode *param, DataType *concreteType)
{
    ASTNode *newParam = (ASTNode *)malloc(sizeof(ASTNode));
    *newParam = *param; // Shallow copy first

    // Deep copy the parameter name
    newParam->data.param->name = strdup(param->data.param->name);

    // Deep copy the parameter type
    newParam->data.param->type = cloneDataType(getDataTypeFromASTNode(param));

    // Substitute the generic type if it's being used
    if (isGenericType(newParam->data.param->type))
    {
        setNewDataTypeForNode(newParam, concreteType);
    }

    return newParam;
}

// Function to clone and substitute generic types in a method body
ASTNode *cloneAndSubstituteGenericBody(ASTNode *body, DataType *concreteType)
{
    ASTNode *newBody = (ASTNode *)malloc(sizeof(ASTNode));
    *newBody = *body; // Shallow copy first

    // Deep copy the body statements
    newBody->data.block->statementCapacity = body->data.block->statementCapacity;
    newBody->data.block->statements = (ASTNode **)malloc(sizeof(ASTNode *) * newBody->data.block->statementCapacity);
    newBody->data.block->statementCount = body->data.block->statementCount;
    for (int i = 0; i < body->data.block->statementCount; i++)
    {
        newBody->data.block->statements[i] = cloneAndSubstituteGenericStatement(
            body->data.block->statements[i],
            concreteType);
    }

    return newBody;
}

// Function to clone and substitute generic types in a method statement
ASTNode *cloneAndSubstituteGenericStatement(ASTNode *statement, DataType *concreteType)
{
    ASTNode *newStatement = (ASTNode *)malloc(sizeof(ASTNode));
    *newStatement = *statement; // Shallow copy first

    // Deep copy the statement
    switch (statement->metaData->type)
    {
    case NODE_VAR_DECLARATION:
        newStatement->data.varDecl->name = strdup(statement->data.varDecl->name);
        newStatement->data.varDecl->type = cloneDataType(getDataTypeFromASTNode(statement));
        if (isGenericType(newStatement->data.varDecl->type))
        {
            setNewDataTypeForNode(newStatement, concreteType);
        }
        break;
    case NODE_VAR_REASSIGN:
        newStatement->data.varReassignment->existingVarName = strdup(statement->data.varReassignment->existingVarName);
        newStatement->data.varReassignment->type = cloneDataType(getDataTypeFromASTNode(statement));
        if (isGenericType(newStatement->data.varReassignment->type))
        {
            setNewDataTypeForNode(newStatement, concreteType);
        }
        break;
    case NODE_PROPERTY_ACCESS:
        newStatement->data.propertyAccess->propertyName = strdup(statement->data.propertyAccess->propertyName);
        break;
    case NODE_METHOD_CALL:
        newStatement->data.methodCall->name = strdup(statement->data.methodCall->name);
        break;
    default:
        logMessage("ERROR", __LINE__, "DataTypes", "Failed to clone and substitute generic statement, received node type: %s",
                   CryoNodeTypeToString(statement->metaData->type));
        break;
    }

    return newStatement;
}

bool validateGenericInstantiation(ASTNode *node, TypeTable *typeTable)
{
    if (node->metaData->type != NODE_GENERIC_INST)
        return false;

    GenericInstNode *inst = node->data.genericInst;

    // Look up the generic declaration
    DataType *baseType = lookupType(typeTable, inst->baseName);
    if (!baseType || !isGenericType(baseType))
    {
        return false; // Base type must exist and be generic
    }

    // Validate argument count matches parameter count
    GenericDeclType *decl = baseType->container->custom.generic.declaration;
    int paramCount = baseType->container->custom.generic.declaration->paramCount;
    for (int i = 0; i < paramCount; i++)
    {
        DataType *param = decl->params[i];
        if (!isTypeCompatible(param, inst->typeArguments[i]))
        {
            return false;
        }
    }

    return true;
}

bool validateGenericType(DataType *type, DataType *concreteType)
{
    if (type->container->baseType == GENERIC_TYPE)
    {
        DataType *paramDataType = type->container->custom.generic.declaration->params[0];
        return isTypeCompatible(paramDataType, concreteType);
    }

    return true;
}

bool isTypeCompatible(DataType *type, DataType *other)
{
    if (type->container->baseType == STRUCT_TYPE && other->container->baseType == STRUCT_TYPE)
    {
        StructType *structType = type->container->custom.structDef;
        StructType *otherType = other->container->custom.structDef;

        if (structType->propertyCount != otherType->propertyCount)
        {
            return false;
        }

        for (int i = 0; i < structType->propertyCount; i++)
        {
            if (!isTypeCompatible(structType->properties[i]->data.property->type, otherType->properties[i]->data.property->type))
            {
                return false;
            }
        }

        return true;
    }

    return type->container->baseType == other->container->baseType;
}