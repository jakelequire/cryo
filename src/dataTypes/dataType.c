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
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Types Implementation -------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DataTypes_isConst(DataType *type, bool isConst)
{
    type->isConst = isConst;
}

void DataTypes_isPointer(DataType *type, bool isPointer)
{
    type->isPointer = isPointer;
}

void DataTypes_isReference(DataType *type, bool isReference)
{
    type->isReference = isReference;
}

void DataTypes_cast(DataType *fromType, DataType *toType)
{
    logMessage(LMI, "INFO", "DTM", "Casting data type...");
    if (!fromType || !toType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to cast NULL data types\n");
        CONDITION_FAILED;
    }

    if (fromType->container->typeOf != toType->container->typeOf)
    {
        fprintf(stderr, "[Data Type Manager] Error: Cannot cast between different data types\n");
        CONDITION_FAILED;
    }

    if (fromType->container->typeOf == PRIM_TYPE)
    {
        if (fromType->container->primitive != toType->container->primitive)
        {
            fprintf(stderr, "[Data Type Manager] Error: Cannot cast between different primitive types\n");
            CONDITION_FAILED;
        }
    }
}

void DataTypes_unsafeCast(DataType *fromType, DataType *toType)
{
    // This function will cast between different data types without checking
    // if the cast is valid. This is dangerous and should be used with caution.
    logMessage(LMI, "INFO", "DTM", "Unsafe casting data type...");
    if (!fromType || !toType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to unsafe cast NULL data types\n");
        CONDITION_FAILED;
    }

    logMessage(LMI, "INFO", "DTM", "Unsafe cast from %s to %s", fromType->typeName, toType->typeName);
    // Preform the cast without checking
    // This is dangerous and should be used with caution.
    fromType->container = toType->container;
    fromType->typeName = toType->typeName;
    fromType->isConst = toType->isConst;
    fromType->isPointer = toType->isPointer;
    fromType->isReference = toType->isReference;
    fromType->isArray = toType->isArray;

    logMessage(LMI, "INFO", "DTM", "Unsafe cast complete");
    logMessage(LMI, "INFO", "DTM", "New type name: %s", fromType->typeName);

    return;
}

void DataTypes_setTypeName(DataType *type, const char *name)
{
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to set type name on NULL data type\n");
        CONDITION_FAILED;
    }

    type->typeName = name;
}

void DataTypes_free(DataType *type)
{
    logMessage(LMI, "INFO", "DTM", "Freeing data type...");
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to free NULL data type\n");
        CONDITION_FAILED;
    }

    free(type);
}

DataType *DataTypes_clone(DataType *type)
{
    logMessage(LMI, "INFO", "DTM", "Cloning data type...");
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Attempted to clone NULL data type\n");
        CONDITION_FAILED;
    }

    DataType *clonedType = (DataType *)malloc(sizeof(DataType));
    if (!clonedType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to clone data type\n");
        CONDITION_FAILED;
    }

    memcpy(clonedType, type, sizeof(DataType));
    return clonedType;
}

TypeContainer *DTMTypeContainerWrappers_createTypeContainer(void)
{
    logMessage(LMI, "INFO", "DTM", "Creating Type Container...");
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create Type Container\n");
        CONDITION_FAILED;
    }

    return container;
}

DataType *DTMTypeContainerWrappers_wrapTypeContainer(TypeContainer *container)
{
    DataType *type = (DataType *)malloc(sizeof(DataType));
    if (!type)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to wrap Type Container\n");
        CONDITION_FAILED;
    }

    type->container = container;
    type->debug = createDTDebug();

    type->setConst = DataTypes_isConst;
    type->setPointer = DataTypes_isPointer;
    type->setReference = DataTypes_isReference;
    type->cast = DataTypes_cast;
    type->unsafeCast = DataTypes_unsafeCast;
    type->clone = DataTypes_clone;
    type->setTypeName = DataTypes_setTypeName;
    type->free = DataTypes_free;

    return type;
}

DataType *DTMTypeContainerWrappers_wrapSimpleType(struct DTSimpleTy_t *simpleTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = PRIM_TYPE;
    container->objectType = NON_OBJECT;
    container->type.simpleType = simpleTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapArrayType(struct DTArrayTy_t *arrayTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ARRAY_TYPE;
    container->objectType = NON_OBJECT;
    container->type.arrayType = arrayTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapEnumType(struct DTEnumTy_t *enumTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ENUM_TYPE;
    container->objectType = NON_OBJECT;
    container->primitive = PRIM_ENUM;
    container->type.enumType = enumTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapFunctionType(struct DTFunctionTy_t *functionTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = FUNCTION_TYPE;
    container->primitive = PRIM_FUNCTION;
    container->objectType = NON_OBJECT;
    container->type.functionType = functionTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapStructType(struct DTStructTy_t *structTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = STRUCT_OBJ;
    container->primitive = PRIM_OBJECT;
    container->type.structType = structTy;
    container->type.structType->generic.isGeneric = false;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapClassType(struct DTClassTy_t *classTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = CLASS_OBJ;
    container->primitive = PRIM_OBJECT;
    container->type.classType = classTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapObjectType(struct DTObjectTy_t *objectTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = OBJECT_TYPE;
    container->objectType = OBJECT_OBJ;
    container->primitive = PRIM_OBJECT;
    container->type.objectType = objectTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapGenericType(struct DTGenericTy_t *genericTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = GENERIC_TYPE;
    container->type.genericType = genericTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_wrapPointerType(struct DTPointerTy_t *pointerTy)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->type.pointerType = pointerTy;

    return DTMTypeContainerWrappers_wrapTypeContainer(container);
}

DataType *DTMTypeContainerWrappers_createProtoType(const char *name, PrimitiveDataType primitive, TypeofDataType typeOf, TypeofObjectType objectType)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->primitive = primitive;
    container->typeOf = typeOf;
    container->objectType = objectType;

    if (objectType == STRUCT_OBJ)
    {
        container->type.structType = DTM->structTypes->createStructTemplate()->container->type.structType;
        container->type.structType->name = name;
    }
    else if (objectType == CLASS_OBJ)
    {
        container->type.classType = DTM->classTypes->createClassTemplate()->container->type.classType;
        container->type.classType->name = name;
    }

    DataType *protoType = DTMTypeContainerWrappers_wrapTypeContainer(container);
    protoType->setTypeName(protoType, name);

    return protoType;
}

DataType *DTMTypeContainerWrappers_createTypeAlias(const char *name, DataType *type)
{
    PrimitiveDataType primitive = type->container->primitive;
    TypeofDataType typeOf = type->container->typeOf;
    TypeofObjectType objectType = type->container->objectType;

    DataType *clonedType = type->clone(type);
    if (!clonedType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to clone data type for alias\n");
        CONDITION_FAILED;
    }
    clonedType->setTypeName(clonedType, name);
    clonedType->container->primitive = primitive;
    clonedType->container->typeOf = typeOf;
    clonedType->container->objectType = objectType;

    return clonedType;
}

DataType *DTMTypeContainerWrappers_createPointerType(const char *typeName, DataType *baseType, bool isConst)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = POINTER_TYPE;
    container->objectType = baseType->container->objectType;
    container->primitive = baseType->container->primitive;

    container->type.pointerType = (DTPointerTy *)malloc(sizeof(DTPointerTy));
    if (!container->type.pointerType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create pointer type\n");
        CONDITION_FAILED;
    }

    container->type.pointerType->baseType = baseType;
    container->type.pointerType->isConst = isConst;

    DataType *pointerType = DTMTypeContainerWrappers_wrapTypeContainer(container);
    pointerType->setTypeName(pointerType, typeName);

    return pointerType;
}

DataType *DTMTypeContainerWrappers_createArrayType(DataType *baseType, int dimensions)
{
    TypeContainer *container = DTMTypeContainerWrappers_createTypeContainer();
    container->typeOf = ARRAY_TYPE;
    container->objectType = baseType->container->objectType;
    container->primitive = baseType->container->primitive;

    container->type.arrayType = (DTArrayTy *)malloc(sizeof(DTArrayTy));
    if (!container->type.arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to create array type\n");
        CONDITION_FAILED;
    }

    container->type.arrayType->baseType = baseType;
    container->type.arrayType->dimensions = dimensions;
    container->type.arrayType->elementCount = 0;
    container->type.arrayType->elementCapacity = MAX_FIELD_CAPACITY;
    container->type.arrayType->elements = (DataType **)malloc(sizeof(DataType *) * container->type.arrayType->elementCapacity);
    if (!container->type.arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate array type elements\n");
        CONDITION_FAILED;
    }
    container->type.arrayType->isConst = baseType->isConst;
    container->type.arrayType->sizeType = NULL;

    DataType *arrayType = DTMTypeContainerWrappers_wrapTypeContainer(container);
    arrayType->setTypeName(arrayType, baseType->typeName);

    return arrayType;
}

DTMDataTypes *createDTMDataTypes(void)
{
    DTMDataTypes *dataTypes = (DTMDataTypes *)malloc(sizeof(DTMDataTypes));
    if (!dataTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Data Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    dataTypes->createTypeContainer = DTMTypeContainerWrappers_createTypeContainer;
    dataTypes->wrapTypeContainer = DTMTypeContainerWrappers_wrapTypeContainer;

    dataTypes->wrapSimpleType = DTMTypeContainerWrappers_wrapSimpleType;
    dataTypes->wrapArrayType = DTMTypeContainerWrappers_wrapArrayType;
    dataTypes->wrapEnumType = DTMTypeContainerWrappers_wrapEnumType;
    dataTypes->wrapFunctionType = DTMTypeContainerWrappers_wrapFunctionType;
    dataTypes->wrapStructType = DTMTypeContainerWrappers_wrapStructType;
    dataTypes->wrapClassType = DTMTypeContainerWrappers_wrapClassType;
    dataTypes->wrapObjectType = DTMTypeContainerWrappers_wrapObjectType;
    dataTypes->wrapGenericType = DTMTypeContainerWrappers_wrapGenericType;
    dataTypes->wrapPointerType = DTMTypeContainerWrappers_wrapPointerType;

    dataTypes->createProtoType = DTMTypeContainerWrappers_createProtoType;
    dataTypes->createTypeAlias = DTMTypeContainerWrappers_createTypeAlias;
    dataTypes->createPointerType = DTMTypeContainerWrappers_createPointerType;
    dataTypes->createArrayType = DTMTypeContainerWrappers_createArrayType;

    return dataTypes;
}
