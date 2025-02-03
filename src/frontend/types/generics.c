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

TypeContainer *createGenericTypeContainer(void)
{
    __STACK_FRAME__
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[DataTypes] Error: Failed to allocate TypeContainer\n");
        CONDITION_FAILED;
    }

    container->baseType = GENERIC_TYPE;
    container->primitive = PRIM_CUSTOM;
    container->size = 0;
    container->length = 0;
    container->isArray = false;
    container->boolValue = false;
    container->isGeneric = true;
    container->arrayDimensions = 0;
    container->custom.name = (char *)malloc(sizeof(char) * 64);
    container->custom.structDef = NULL;
    container->custom.funcDef = NULL;
    container->custom.structDef = NULL;
    container->custom.generic.declaration = (GenericDeclType *)malloc(sizeof(GenericDeclType));
    container->custom.generic.declaration->genericDef = NULL;
    container->custom.generic.instantiation = (GenericInstType *)malloc(sizeof(GenericInstType));

    return container;
}

void initGenericType(GenericType *type, const char *name)
{
    __STACK_FRAME__
    type->name = strdup(name);
    type->constraint = NULL;
    type->next = NULL;
}

// Function to create a new generic parameter
GenericType *createGenericParameter(const char *name)
{
    __STACK_FRAME__
    GenericType *param = (GenericType *)malloc(sizeof(GenericType));
    initGenericType(param, name);
    return param;
}

TypeContainer *createGenericArrayType(DataType *genericParam)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    GenericDeclType *decl = (GenericDeclType *)malloc(sizeof(GenericDeclType));
    decl->genericDef = structDef;
    decl->params = genericParam;
    decl->paramCount = paramCount;
    return decl;
}
GenericInstType *createGenericInstanceContainer(StructType *structDef, DataType **typeArgs, int argCount, TypeContainer *baseDef)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
    type->constraint = constraint;
}

// Function to link multiple generic parameters
void linkGenericParameter(GenericType *base, GenericType *next)
{
    __STACK_FRAME__
    GenericType *current = base;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = next;
}

TypeContainer *createGenericStructInstance(TypeContainer *genericDef, DataType *concreteType)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    return type->container->isGeneric;
}

bool isGenericInstance(TypeContainer *type)
{
    __STACK_FRAME__
    return type->baseType == STRUCT_TYPE &&
           strstr(type->custom.name, "<") != NULL;
}

DataType *getGenericParameter(TypeContainer *type, int index)
{
    __STACK_FRAME__
    if (index >= 0 && index < type->custom.generic.declaration->paramCount)
    {
        return type->custom.generic.declaration->params[index]->container->custom.generic.declaration->params[0];
    }
}

int getGenericParameterCount(TypeContainer *type)
{
    __STACK_FRAME__
    return type->custom.generic.declaration->paramCount;
}

const char *getGenericTypeName(DataType *type)
{
    __STACK_FRAME__
    return type->container->custom.name;
}

// Function to substitute generic types in a struct definition
StructType *substituteGenericType(StructType *structDef, DataType *genericParam, DataType *concreteType)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
        logMessage(LMI, "ERROR", "DataTypes", "Failed to clone and substitute generic statement, received node type: %s",
                   CryoNodeTypeToString(statement->metaData->type));
        break;
    }

    return newStatement;
}

bool validateGenericInstantiation(ASTNode *node)
{
    __STACK_FRAME__
    if (node->metaData->type != NODE_GENERIC_INST)
        return false;

    GenericInstNode *inst = node->data.genericInst;

    // Look up the generic declaration
    DEBUG_BREAKPOINT;

    return true;
}

bool validateGenericType(DataType *type, DataType *concreteType)
{
    __STACK_FRAME__
    if (type->container->baseType == GENERIC_TYPE)
    {
        DataType *paramDataType = type->container->custom.generic.declaration->params[0];
        return isTypeCompatible(paramDataType, concreteType);
    }

    return true;
}

bool isTypeCompatible(DataType *type, DataType *other)
{
    __STACK_FRAME__
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

/*
typedef struct GenericType
{
    const char *name;     // Name of the generic type (e.g., "T")
    DataType *constraint; // Optional constraint on the generic type
    bool isType;          // Whether this is a type parameter (T) or a concrete type (int)
    union
    {
        struct
        {                            // For type parameters (when isType is false)
            ASTNode **genericParams; // Array of generic parameter nodes
            int genericParamCount;
            int genericParamCapacity;
            struct
            {
                bool isArray;
                int arrayDimensions;
            } arrayInfo;
        } parameter;

        struct
        {                           // For concrete types (when isType is true)
            DataType *concreteType; // The actual type used in instantiation
        } concrete;
    };
    struct GenericType *next; // For linking multiple generic params (e.g., <T, U>)
} GenericType;
*/
void addGenericTypeParam(TypeContainer *container, DataType *param)
{
    __STACK_FRAME__
    if (container->baseType != GENERIC_TYPE)
    {
        fprintf(stderr, "[TypeTable] Error: Attempted to add generic type parameter to non-generic type.\n");
        return;
    }

    GenericDeclType *decl = container->custom.generic.declaration;
    if (!decl)
    {
        fprintf(stderr, "[TypeTable] Error: Generic type declaration not found.\n");
        return;
    }

    if (decl->paramCount == 0)
    {
        decl->params = (DataType **)malloc(sizeof(DataType *));
    }
    else
    {
        decl->params = (DataType **)realloc(decl->params, sizeof(DataType *) * (decl->paramCount + 1));
    }

    decl->params[decl->paramCount] = param;
    decl->paramCount++;
}

DataType *createGenericDataTypeInstance(DataType *genericType, DataType **concreteTypes, int paramCount)
{
    __STACK_FRAME__
    if (!genericType || !concreteTypes || paramCount <= 0)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid generic type or concrete types.\n");
        return NULL;
    }

    TypeContainer *container = createTypeContainer();
    container->baseType = STRUCT_TYPE;

    // Create a new struct type
    StructType *instance = (StructType *)malloc(sizeof(StructType));
    instance->name = (char *)malloc(strlen(genericType->container->custom.name) + 1);
    strcpy((char *)instance->name, (char *)genericType->container->custom.name);

    // Initialize arrays
    instance->propertyCapacity = 0;
    instance->methodCapacity = 0;
    instance->properties = NULL;
    instance->methods = NULL;

    // Deep copy and substitute generic types in properties
    instance->propertyCount = 0;

    // Deep copy and substitute generic types in methods
    instance->methodCount = 0;

    instance->hasDefaultValue = false;
    instance->hasConstructor = false;
    instance->size = 0;

    container->custom.structDef = instance;
    container->custom.name = strdup(instance->name);

    // Set up the generic parameter
    container->custom.generic.instantiation = createGenericInstanceContainer(
        genericType->container->custom.structDef,
        concreteTypes,
        paramCount,
        genericType->container);

    return wrapTypeContainer(container);
}
