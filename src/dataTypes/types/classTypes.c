/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
// ------------------------------------ Class Data Types --------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void DTClassTy_addPublicProperty(DTClassTy *classType, DTPropertyTy *property)
{
    if (classType->publicMembers->propertyCount >= classType->publicMembers->propertyCapacity)
    {
        classType->publicMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->publicMembers->properties = (DTPropertyTy **)realloc(classType->publicMembers->properties, sizeof(DTPropertyTy *) * classType->publicMembers->propertyCapacity);
        if (!classType->publicMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy public properties\n");
            CONDITION_FAILED;
        }
    }

    classType->publicMembers->properties[classType->publicMembers->propertyCount++] = property;
}

void DTClassTy_addPublicMethod(DTClassTy *classType, DataType *method)
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

void DTClassTy_addPrivateProperty(DTClassTy *classType, DTPropertyTy *property)
{
    if (classType->privateMembers->propertyCount >= classType->privateMembers->propertyCapacity)
    {
        classType->privateMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->privateMembers->properties = (DTPropertyTy **)realloc(classType->privateMembers->properties, sizeof(DTPropertyTy *) * classType->privateMembers->propertyCapacity);
        if (!classType->privateMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy private properties\n");
            CONDITION_FAILED;
        }
    }

    classType->privateMembers->properties[classType->privateMembers->propertyCount++] = property;
}

void DTClassTy_addPrivateMethod(DTClassTy *classType, DataType *method)
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

void DTClassTy_addProtectedProperty(DTClassTy *classType, DTPropertyTy *property)
{
    if (classType->protectedMembers->propertyCount >= classType->protectedMembers->propertyCapacity)
    {
        classType->protectedMembers->propertyCapacity *= DYN_GROWTH_FACTOR;
        classType->protectedMembers->properties = (DTPropertyTy **)realloc(classType->protectedMembers->properties, sizeof(DTPropertyTy *) * classType->protectedMembers->propertyCapacity);
        if (!classType->protectedMembers->properties)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to resize DTClassTy protected properties\n");
            CONDITION_FAILED;
        }
    }

    classType->protectedMembers->properties[classType->protectedMembers->propertyCount++] = property;
}

void DTClassTy_addProtectedMethod(DTClassTy *classType, DataType *method)
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

void DTClassTy_addPublicProperties(DTClassTy *classType, DTPropertyTy **properties, int propertyCount)
{
    for (int i = 0; i < propertyCount; i++)
    {
        DTClassTy_addPublicProperty(classType, properties[i]);
    }
}

void DTClassTy_addPublicMethods(DTClassTy *classType, DataType **methods, int methodCount)
{
    for (int i = 0; i < methodCount; i++)
    {
        DTClassTy_addPublicMethod(classType, methods[i]);
    }
}

void DTClassTy_addPrivateProperties(DTClassTy *classType, DTPropertyTy **properties, int propertyCount)
{
    for (int i = 0; i < propertyCount; i++)
    {
        DTClassTy_addPrivateProperty(classType, properties[i]);
    }
}

void DTClassTy_addPrivateMethods(DTClassTy *classType, DataType **methods, int methodCount)
{
    for (int i = 0; i < methodCount; i++)
    {
        DTClassTy_addPrivateMethod(classType, methods[i]);
    }
}

void DTClassTy_addProtectedProperties(DTClassTy *classType, DTPropertyTy **properties, int propertyCount)
{
    for (int i = 0; i < propertyCount; i++)
    {
        DTClassTy_addProtectedProperty(classType, properties[i]);
    }
}

void DTClassTy_addProtectedMethods(DTClassTy *classType, DataType **methods, int methodCount)
{
    for (int i = 0; i < methodCount; i++)
    {
        DTClassTy_addProtectedMethod(classType, methods[i]);
    }
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

    classType->properties = (DTPropertyTy **)malloc(sizeof(DTPropertyTy *) * MAX_FIELD_CAPACITY);
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
    classType->constructors = (DataType **)malloc(sizeof(DataType *) * MAX_PARAM_CAPACITY);
    classType->ctorCount = 0;
    classType->ctorCapacity = MAX_PARAM_CAPACITY;

    // ==================== [ Function Assignments ] ==================== //

    classType->addPublicProperty = DTClassTy_addPublicProperty;
    classType->addPublicProperties = DTClassTy_addPublicProperties;
    classType->addPublicMethod = DTClassTy_addPublicMethod;
    classType->addPublicMethods = DTClassTy_addPublicMethods;

    classType->addPrivateProperty = DTClassTy_addPrivateProperty;
    classType->addPrivateProperties = DTClassTy_addPrivateProperties;
    classType->addPrivateMethod = DTClassTy_addPrivateMethod;
    classType->addPrivateMethods = DTClassTy_addPrivateMethods;

    classType->addProtectedProperty = DTClassTy_addProtectedProperty;
    classType->addProtectedProperties = DTClassTy_addProtectedProperties;
    classType->addProtectedMethod = DTClassTy_addProtectedMethod;
    classType->addProtectedMethods = DTClassTy_addProtectedMethods;

    return classType;
}
