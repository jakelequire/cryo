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

#define MAX_PARAM_CAPACITY 16
#define MAX_FIELD_CAPACITY 16
#define MAX_METHOD_CAPACITY 16
#define MAX_ENUM_CAPACITY 16
#define MAX_GENERIC_CAPACITY 16
#define MAX_TYPE_CAPACITY 16

#define DYN_GROWTH_FACTOR 2

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Simple Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTSimpleTy *createDTSimpleTy(void)
{
    DTSimpleTy *simpleType = (DTSimpleTy *)malloc(sizeof(DTSimpleTy));
    if (!simpleType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTSimpleTy\n");
        CONDITION_FAILED;
    }

    return simpleType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Array Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTArrayTy_addElement(DTArrayTy *arrayType, DataType *element)
{
    if (arrayType->elementCount >= arrayType->elementCapacity)
    {
        arrayType->elementCapacity *= DYN_GROWTH_FACTOR;
        arrayType->elements = (DataType **)realloc(arrayType->elements, sizeof(DataType *) * arrayType->elementCapacity);
        if (!arrayType->elements)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTArrayTy elements\n");
            CONDITION_FAILED;
        }
    }

    arrayType->elements[arrayType->elementCount++] = element;
}

void DTArrayTy_removeElement(DTArrayTy *arrayType, DataType *element)
{
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        if (arrayType->elements[i] == element)
        {
            for (int j = i; j < arrayType->elementCount - 1; j++)
            {
                arrayType->elements[j] = arrayType->elements[j + 1];
            }

            arrayType->elementCount--;
            break;
        }
    }
}

void DTArrayTy_resize(DTArrayTy *arrayType)
{
    arrayType->elementCapacity *= DYN_GROWTH_FACTOR;
    arrayType->elements = (DataType **)realloc(arrayType->elements, sizeof(DataType *) * arrayType->elementCapacity);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTArrayTy elements\n");
        CONDITION_FAILED;
    }
}

void DTArrayTy_reset(DTArrayTy *arrayType)
{
    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_FIELD_CAPACITY;

    arrayType->freeData(arrayType);
    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * arrayType->elementCapacity);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy elements\n");
        CONDITION_FAILED;
    }
}

void DTArrayTy_free(DTArrayTy *arrayType)
{
    free(arrayType->elements);
    free(arrayType);
}

void DTArrayTy_freeData(DTArrayTy *arrayType)
{
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        arrayType->elements[i]->free(arrayType->elements[i]);
    }
}

void DTArrayTy_printArray(DTArrayTy *arrayType)
{
    printf("[");
    for (int i = 0; i < arrayType->elementCount; i++)
    {
        arrayType->elements[i]->debug->printType(arrayType->elements[i]);
        if (i < arrayType->elementCount - 1)
        {
            printf(", ");
        }
    }
    printf("]");
}

DTArrayTy *createDTArrayTy(void)
{
    DTArrayTy *arrayType = (DTArrayTy *)malloc(sizeof(DTArrayTy));
    if (!arrayType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy\n");
        CONDITION_FAILED;
    }

    arrayType->elements = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!arrayType->elements)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTArrayTy elements\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    arrayType->elementCount = 0;
    arrayType->elementCapacity = MAX_FIELD_CAPACITY;
    arrayType->dimensions = 0;
    arrayType->baseType = NULL;

    // ==================== [ Function Assignments ] ==================== //

    arrayType->addElement = DTArrayTy_addElement;
    arrayType->removeElement = DTArrayTy_removeElement;
    arrayType->resize = DTArrayTy_resize;
    arrayType->reset = DTArrayTy_reset;
    arrayType->free = DTArrayTy_free;
    arrayType->freeData = DTArrayTy_freeData;
    arrayType->printArray = DTArrayTy_printArray;

    return arrayType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Enum Data Types ---------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTEnumTy_addValue(DTEnumTy *enumType, ASTNode *value)
{
    if (enumType->valueCount >= enumType->valueCapacity)
    {
        enumType->valueCapacity *= DYN_GROWTH_FACTOR;
        enumType->values = (ASTNode **)realloc(enumType->values, sizeof(ASTNode *) * enumType->valueCapacity);
        if (!enumType->values)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTEnumTy values\n");
            CONDITION_FAILED;
        }
    }

    enumType->values[enumType->valueCount++] = value;
}

void DTEnumTy_setValues(DTEnumTy *enumType, ASTNode **values, int valueCount)
{
    if (enumType->valueCount == 0)
    {
        enumType->values = values;
        enumType->valueCount = valueCount;
    }
}

DTEnumTy *createDTEnumTy(void)
{
    DTEnumTy *enumType = (DTEnumTy *)malloc(sizeof(DTEnumTy));
    if (!enumType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTEnumTy\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    enumType->values = (ASTNode **)malloc(sizeof(ASTNode *) * MAX_ENUM_CAPACITY);
    if (!enumType->values)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTEnumTy values\n");
        CONDITION_FAILED;
    }
    enumType->name = NULL;
    enumType->valueCount = 0;
    enumType->valueCapacity = MAX_ENUM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    enumType->addValue = DTEnumTy_addValue;
    enumType->setValues = DTEnumTy_setValues;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Function Data Types ------------------------------------------ //
// --------------------------------------------------------------------------------------------------- //

void DTFunctionTy_addParam(DTFunctionTy *function, DataType *paramType)
{
    if (function->paramCount >= function->paramCapacity)
    {
        function->paramCapacity *= DYN_GROWTH_FACTOR;
        function->paramTypes = (DataType **)realloc(function->paramTypes, sizeof(DataType *) * function->paramCapacity);
        if (!function->paramTypes)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTFunctionTy paramTypes\n");
            CONDITION_FAILED;
        }
    }

    function->paramTypes[function->paramCount++] = paramType;
}

void DTFunctionTy_setParams(DTFunctionTy *function, DataType **paramTypes, int paramCount)
{
    if (function->paramCount == 0)
    {
        function->paramTypes = paramTypes;
        function->paramCount = paramCount;
    }
}

void DTFunctionTy_setReturnType(DTFunctionTy *function, DataType *returnType)
{
    function->returnType = returnType;
}

DTFunctionTy *createDTFunctionTy(void)
{
    DTFunctionTy *function = (DTFunctionTy *)malloc(sizeof(DTFunctionTy));
    if (!function)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTFunctionTy\n");
        CONDITION_FAILED;
    }

    function->paramTypes = (DataType **)malloc(sizeof(DataType *) * MAX_PARAM_CAPACITY);
    if (!function->paramTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTFunctionTy paramTypes\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    function->returnType = NULL;
    function->paramCount = 0;
    function->paramCapacity = MAX_PARAM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    function->addParam = DTFunctionTy_addParam;
    function->setParams = DTFunctionTy_setParams;
    function->setReturnType = DTFunctionTy_setReturnType;

    return function;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Struct Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTStructTy_addProperty(DTStructTy *structType, DataType *property)
{
    if (structType->propertyCount >= structType->propertyCapacity)
    {
        structType->propertyCapacity *= DYN_GROWTH_FACTOR;
        structType->properties = (DataType **)realloc(structType->properties, sizeof(DataType *) * structType->propertyCapacity);
        if (!structType->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy properties\n");
            CONDITION_FAILED;
        }
    }

    structType->properties[structType->propertyCount++] = property;
}

void DTStructTy_addMethod(DTStructTy *structType, DataType *method)
{
    if (structType->methodCount >= structType->methodCapacity)
    {
        structType->methodCapacity *= DYN_GROWTH_FACTOR;
        structType->methods = (DataType **)realloc(structType->methods, sizeof(DataType *) * structType->methodCapacity);
        if (!structType->methods)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy methods\n");
            CONDITION_FAILED;
        }
    }

    structType->methods[structType->methodCount++] = method;
}

void DTStructTy_addCtorParam(DTStructTy *structType, DataType *param)
{
    if (structType->ctorParamCount >= structType->ctorParamCapacity)
    {
        structType->ctorParamCapacity *= DYN_GROWTH_FACTOR;
        structType->ctorParams = (DataType **)realloc(structType->ctorParams, sizeof(DataType *) * structType->ctorParamCapacity);
        if (!structType->ctorParams)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy ctorParams\n");
            CONDITION_FAILED;
        }
    }

    structType->ctorParams[structType->ctorParamCount++] = param;
}

void DTStructTy_addGenericParam(DTStructTy *structType, GenericType *param)
{
    if (structType->generic.paramCount >= structType->generic.paramCapacity)
    {
        structType->generic.paramCapacity *= DYN_GROWTH_FACTOR;
        structType->generic.params = (GenericType **)realloc(structType->generic.params, sizeof(GenericType *) * structType->generic.paramCapacity);
        if (!structType->generic.params)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTStructTy generic params\n");
            CONDITION_FAILED;
        }
    }

    structType->generic.params[structType->generic.paramCount++] = param;
}

void DTStructTy_substituteGenericType(DTStructTy *structType, GenericType *genericType, DataType *substituteType)
{
    for (int i = 0; i < structType->propertyCount; i++)
    {
        if (structType->properties[i]->container->typeOf == GENERIC_TYPE)
        {
            DTGenericType *generic = structType->properties[i]->container->type.genericType;
            if (generic == genericType)
            {
                structType->properties[i] = substituteType;
            }
        }
    }

    for (int i = 0; i < structType->methodCount; i++)
    {
        if (structType->methods[i]->container->typeOf == GENERIC_TYPE)
        {
            DTGenericType *generic = structType->methods[i]->container->type.genericType;
            if (generic == genericType)
            {
                structType->methods[i] = substituteType;
            }
        }
    }
}

void DTStructTy_cloneAndSubstituteGenericMethod(DTStructTy *structType, GenericType *genericType, DataType *substituteType)
{
    for (int i = 0; i < structType->methodCount; i++)
    {
        if (structType->methods[i]->container->typeOf == GENERIC_TYPE)
        {
            DTGenericType *generic = structType->methods[i]->container->type.genericType;
            if (generic == genericType)
            {
                structType->methods[i] = substituteType;
            }
        }
    }
}

void DTStructTy_cloneAndSubstituteGenericParam(DTStructTy *structType, GenericType *genericType, DataType *substituteType)
{
    for (int i = 0; i < structType->propertyCount; i++)
    {
        if (structType->properties[i]->container->typeOf == GENERIC_TYPE)
        {
            DTGenericType *generic = structType->properties[i]->container->type.genericType;
            if (generic == genericType)
            {
                structType->properties[i] = substituteType;
            }
        }
    }
}

DTStructTy *createDTStructTy(void)
{
    DTStructTy *structType = (DTStructTy *)malloc(sizeof(DTStructTy));
    if (!structType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTStructTy\n");
        CONDITION_FAILED;
    }

    structType->properties = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!structType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTStructTy properties\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    structType->name = NULL;
    structType->propertyCount = 0;
    structType->propertyCapacity = MAX_FIELD_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    structType->addProperty = DTStructTy_addProperty;
    structType->addMethod = DTStructTy_addMethod;
    structType->addCtorParam = DTStructTy_addCtorParam;
    structType->addGenericParam = DTStructTy_addGenericParam;
    structType->substituteGenericType = DTStructTy_substituteGenericType;
    structType->cloneAndSubstituteGenericMethod = DTStructTy_cloneAndSubstituteGenericMethod;
    structType->cloneAndSubstituteGenericParam = DTStructTy_cloneAndSubstituteGenericParam;

    return structType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Class Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTClassTy_addPublicProperty(DTClassTy *classType, ASTNode *property)
{
    if (classType->publicMembers->propertyCount >= classType->publicMembers->propertyCapacity)
    {
        classType->publicMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->publicMembers->properties = (DataType **)realloc(classType->publicMembers->properties, sizeof(DataType *) * classType->publicMembers->propertyCapacity);
        if (!classType->publicMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy public properties\n");
            CONDITION_FAILED;
        }
    }

    classType->publicMembers->properties[classType->publicMembers->propertyCount++] = property;
}

void DTClassTy_addPublicMethod(DTClassTy *classType, ASTNode *method)
{
    if (classType->publicMembers->methodCount >= classType->publicMembers->methodCapacity)
    {
        classType->publicMembers->methodCapacity *= DYN_GROWTH_FACTOR;
        classType->publicMembers->methods = (DataType **)realloc(classType->publicMembers->methods, sizeof(DataType *) * classType->publicMembers->methodCapacity);
        if (!classType->publicMembers->methods)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy public methods\n");
            CONDITION_FAILED;
        }
    }

    classType->publicMembers->methods[classType->publicMembers->methodCount++] = method;
}

void DTClassTy_addPrivateProperty(DTClassTy *classType, ASTNode *property)
{
    if (classType->privateMembers->propertyCount >= classType->privateMembers->propertyCapacity)
    {
        classType->privateMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->privateMembers->properties = (DataType **)realloc(classType->privateMembers->properties, sizeof(DataType *) * classType->privateMembers->propertyCapacity);
        if (!classType->privateMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy private properties\n");
            CONDITION_FAILED;
        }
    }

    classType->privateMembers->properties[classType->privateMembers->propertyCount++] = property;
}

void DTClassTy_addPrivateMethod(DTClassTy *classType, ASTNode *method)
{
    if (classType->privateMembers->methodCount >= classType->privateMembers->methodCapacity)
    {
        classType->privateMembers->methodCapacity *= DYN_GROWTH_FACTOR;
        classType->privateMembers->methods = (DataType **)realloc(classType->privateMembers->methods, sizeof(DataType *) * classType->privateMembers->methodCapacity);
        if (!classType->privateMembers->methods)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy private methods\n");
            CONDITION_FAILED;
        }
    }

    classType->privateMembers->methods[classType->privateMembers->methodCount++] = method;
}

void DTClassTy_addProtectedProperty(DTClassTy *classType, ASTNode *property)
{
    if (classType->protectedMembers->propertyCount >= classType->protectedMembers->propertyCapacity)
    {
        classType->protectedMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->protectedMembers->properties = (DataType **)realloc(classType->protectedMembers->properties, sizeof(DataType *) * classType->protectedMembers->propertyCapacity);
        if (!classType->protectedMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy protected properties\n");
            CONDITION_FAILED;
        }
    }

    classType->protectedMembers->properties[classType->protectedMembers->propertyCount++] = property;
}

void DTClassTy_addProtectedMethod(DTClassTy *classType, ASTNode *method)
{
    if (classType->protectedMembers->methodCount >= classType->protectedMembers->methodCapacity)
    {
        classType->protectedMembers->methodCapacity *= DYN_GROWTH_FACTOR;
        classType->protectedMembers->methods = (DataType **)realloc(classType->protectedMembers->methods, sizeof(DataType *) * classType->protectedMembers->methodCapacity);
        if (!classType->protectedMembers->methods)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy protected methods\n");
            CONDITION_FAILED;
        }
    }

    classType->protectedMembers->methods[classType->protectedMembers->methodCount++] = method;
}

DTPublicMembersTypes *createDTPublicMembersType(void)
{
    DTPublicMembersTypes *membersType = (DTPublicMembersTypes *)malloc(sizeof(DTPublicMembersTypes));
    if (!membersType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTPublicMembersTypes\n");
        CONDITION_FAILED;
    }

    return membersType;
}

DTPrivateMembersTypes *createDTPrivateMembersType(void)
{
    DTPrivateMembersTypes *membersType = (DTPrivateMembersTypes *)malloc(sizeof(DTPrivateMembersTypes));
    if (!membersType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTPrivateMembersTypes\n");
        CONDITION_FAILED;
    }

    return membersType;
}

DTProtectedMembersTypes *createDTProtectedMembersType(void)
{
    DTProtectedMembersTypes *membersType = (DTProtectedMembersTypes *)malloc(sizeof(DTProtectedMembersTypes));
    if (!membersType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTProtectedMembersTypes\n");
        CONDITION_FAILED;
    }

    return membersType;
}

DTClassTy *createDTClassTy(void)
{
    DTClassTy *classType = (DTClassTy *)malloc(sizeof(DTClassTy));
    if (!classType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTClassTy\n");
        CONDITION_FAILED;
    }

    classType->publicMembers = createDTPublicMembersType();
    classType->privateMembers = createDTPrivateMembersType();
    classType->protectedMembers = createDTProtectedMembersType();

    classType->properties = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!classType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTClassTy properties\n");
        CONDITION_FAILED;
    }

    classType->methods = (DataType **)malloc(sizeof(DataType *) * MAX_METHOD_CAPACITY);
    if (!classType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTClassTy methods\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    classType->name = NULL;
    classType->propertyCount = 0;
    classType->propertyCapacity = MAX_FIELD_CAPACITY;
    classType->methodCount = 0;
    classType->methodCapacity = MAX_METHOD_CAPACITY;
    classType->hasConstructor = false;
    classType->ctorParamCount = 0;
    classType->ctorParamCapacity = MAX_PARAM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    classType->addPublicProperty = DTClassTy_addPublicProperty;
    classType->addPublicMethod = DTClassTy_addPublicMethod;
    classType->addPrivateProperty = DTClassTy_addPrivateProperty;
    classType->addPrivateMethod = DTClassTy_addPrivateMethod;
    classType->addProtectedProperty = DTClassTy_addProtectedProperty;
    classType->addProtectedMethod = DTClassTy_addProtectedMethod;

    return classType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Object Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTObjectType_addProperty(DTObjectType *objectType, DataType *property)
{
    if (objectType->propertyCount >= objectType->propertyCapacity)
    {
        objectType->_resizeProperties(objectType);
    }

    objectType->properties[objectType->propertyCount++] = property;
}

void DTObjectType_addMethod(DTObjectType *objectType, DataType *method)
{
    if (objectType->methodCount >= objectType->methodCapacity)
    {
        objectType->_resizeMethods(objectType);
    }

    objectType->methods[objectType->methodCount++] = method;
}

void DTObjectType_resizeMethods(DTObjectType *objectType)
{
    objectType->methodCapacity *= DYN_GROWTH_FACTOR;
    objectType->methods = (DataType **)realloc(objectType->methods, sizeof(DataType *) * objectType->methodCapacity);
    if (!objectType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTObjectType methods\n");
        CONDITION_FAILED;
    }
}

void DTObjectType_resizeProperties(DTObjectType *objectType)
{
    objectType->propertyCapacity *= DYN_GROWTH_FACTOR;
    objectType->properties = (DataType **)realloc(objectType->properties, sizeof(DataType *) * objectType->propertyCapacity);
    if (!objectType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTObjectType properties\n");
        CONDITION_FAILED;
    }
}

DTObjectType *createDTObjectType(void)
{
    DTObjectType *objectType = (DTObjectType *)malloc(sizeof(DTObjectType));
    if (!objectType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType\n");
        CONDITION_FAILED;
    }

    // ==================== [ Property Assignments ] ==================== //

    objectType->properties = (DataType **)malloc(sizeof(DataType *) * MAX_FIELD_CAPACITY);
    if (!objectType->properties)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType properties\n");
        CONDITION_FAILED;
    }

    objectType->methods = (DataType **)malloc(sizeof(DataType *) * MAX_METHOD_CAPACITY);
    if (!objectType->methods)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTObjectType methods\n");
        CONDITION_FAILED;
    }

    objectType->name = NULL;
    objectType->propertyCount = 0;
    objectType->propertyCapacity = MAX_FIELD_CAPACITY;
    objectType->methodCount = 0;
    objectType->methodCapacity = MAX_METHOD_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    objectType->addProperty = DTObjectType_addProperty;
    objectType->addMethod = DTObjectType_addMethod;
    
    objectType->_resizeMethods = DTObjectType_resizeMethods;
    objectType->_resizeProperties = DTObjectType_resizeProperties;

    return objectType;
}

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------ Type Container ----------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

TypeContainer *createTypeContainer(void)
{
    TypeContainer *container = (TypeContainer *)malloc(sizeof(TypeContainer));
    if (!container)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate TypeContainer\n");
        CONDITION_FAILED;
    }

    return container;
}
