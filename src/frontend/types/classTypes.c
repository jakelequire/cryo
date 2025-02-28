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

ClassType *createClassType(const char *name, ClassNode *classNode)
{
    __STACK_FRAME__
    ClassType *classType = (ClassType *)malloc(sizeof(ClassType));
    if (!classType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate ClassType\n");
        return NULL;
    }

    classType->name = name;
    classType->constructor = classNode->constructor;
    classType->propertyCount = classNode->propertyCount;
    classType->propertyCapacity = classNode->propertyCapacity;
    classType->methodCount = classNode->methodCount;
    classType->methodCapacity = classNode->methodCapacity;
    classType->hasConstructor = classNode->hasConstructor;
    classType->isStatic = classNode->isStatic;
    classType->hasConstructor = classNode->hasConstructor;
    classType->classNode = (ASTNode *)malloc(sizeof(ASTNode));

    classType->publicMembers = createPublicMembersType();
    if (!classType->publicMembers)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create public members type\n");
        return NULL;
    }
    classType->privateMembers = createPrivateMembersType();
    if (!classType->privateMembers)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create private members type\n");
        return NULL;
    }
    classType->protectedMembers = createProtectedMembersType();
    if (!classType->protectedMembers)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create protected members type\n");
        return NULL;
    }

    // Link public members
    linkPublicMemebers(classType->publicMembers, classNode->publicMembers);
    // Link private members
    linkPrivateMemebers(classType->privateMembers, classNode->privateMembers);
    // Link protected members
    linkProtectedMemebers(classType->protectedMembers, classNode->protectedMembers);

    return classType;
}

TypeContainer *wrapClassInTypeContainer(ClassType *classType)
{
    __STACK_FRAME__
    TypeContainer *container = createTypeContainer();
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate TypeContainer\n");
        return NULL;
    }

    container->baseType = CLASS_TYPE;
    container->custom.classDef = classType;
    container->custom.name = classType->name;

    return container;
}

DataType *createClassDataType(const char *name, ClassNode *classNode)
{
    __STACK_FRAME__
    ClassType *classType = createClassType(name, classNode);
    if (!classType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to create class type\n");
        return NULL;
    }

    TypeContainer *container = wrapClassInTypeContainer(classType);
    if (!container)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to wrap class type in container\n");
        return NULL;
    }

    DataType *dataType = (DataType *)malloc(sizeof(DataType));
    if (!dataType)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate DataType\n");
        return NULL;
    }

    dataType->container = container;

    return dataType;
}

void addPublicPropertyToClassType(ClassType *classType, ASTNode *property)
{
    __STACK_FRAME__
    if (!classType || !property)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or property\n");
        return;
    }

    if (classType->publicMembers->propertyCount + 1 >= classType->publicMembers->propertyCapacity)
    {
        // Grow properties array
        int newCapacity = classType->publicMembers->propertyCapacity * 2;
        DataType **newProperties = (DataType **)realloc(classType->publicMembers->properties, newCapacity * sizeof(DataType *));
        if (!newProperties)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow properties array\n");
            return;
        }

        classType->publicMembers->properties = newProperties;
        classType->publicMembers->propertyCapacity = newCapacity;
    }

    // Add property to class type
    classType->publicMembers->properties[classType->publicMembers->propertyCount++] = getDataTypeFromASTNode(property);
}

void addPublicMethodToClassType(ClassType *classType, ASTNode *method)
{
    __STACK_FRAME__
    if (!classType || !method)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or method\n");
        return;
    }

    if (classType->publicMembers->methodCount + 1 >= classType->publicMembers->methodCapacity)
    {
        // Grow methods array
        int newCapacity = classType->publicMembers->methodCapacity * 2;
        DataType **newMethods = (DataType **)realloc(classType->publicMembers->methods, newCapacity * sizeof(DataType *));
        if (!newMethods)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow methods array\n");
            return;
        }

        classType->publicMembers->methods = newMethods;
        classType->publicMembers->methodCapacity = newCapacity;
    }

    // Add method to class type
    classType->publicMembers->methods[classType->publicMembers->methodCount++] = getDataTypeFromASTNode(method);
}

void addPrivatePropertyToClassType(ClassType *classType, ASTNode *property)
{
    __STACK_FRAME__
    if (!classType || !property)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or property\n");
        return;
    }

    if (classType->privateMembers->propertyCount + 1 >= classType->privateMembers->propertyCapacity)
    {
        // Grow properties array
        int newCapacity = classType->privateMembers->propertyCapacity * 2;
        DataType **newProperties = (DataType **)realloc(classType->privateMembers->properties, newCapacity * sizeof(DataType *));
        if (!newProperties)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow properties array\n");
            return;
        }

        classType->privateMembers->properties = newProperties;
        classType->privateMembers->propertyCapacity = newCapacity;
    }

    // Add property to class type
    classType->privateMembers->properties[classType->privateMembers->propertyCount++] = getDataTypeFromASTNode(property);
}

void addPrivateMethodToClassType(ClassType *classType, ASTNode *method)
{
    __STACK_FRAME__
    if (!classType || !method)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or method\n");
        return;
    }

    if (classType->privateMembers->methodCount + 1 >= classType->privateMembers->methodCapacity)
    {
        // Grow methods array
        int newCapacity = classType->privateMembers->methodCapacity * 2;
        DataType **newMethods = (DataType **)realloc(classType->privateMembers->methods, newCapacity * sizeof(DataType *));
        if (!newMethods)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow methods array\n");
            return;
        }

        classType->privateMembers->methods = newMethods;
        classType->privateMembers->methodCapacity = newCapacity;
    }

    // Add method to class type
    classType->privateMembers->methods[classType->privateMembers->methodCount++] = getDataTypeFromASTNode(method);
}

void addProtectedPropertyToClassType(ClassType *classType, ASTNode *property)
{
    __STACK_FRAME__
    if (!classType || !property)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or property\n");
        return;
    }

    if (classType->protectedMembers->propertyCount + 1 >= classType->protectedMembers->propertyCapacity)
    {
        // Grow properties array
        int newCapacity = classType->protectedMembers->propertyCapacity * 2;
        DataType **newProperties = (DataType **)realloc(classType->protectedMembers->properties, newCapacity * sizeof(DataType *));
        if (!newProperties)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow properties array\n");
            return;
        }

        classType->protectedMembers->properties = newProperties;
        classType->protectedMembers->propertyCapacity = newCapacity;
    }

    // Add property to class type
    classType->protectedMembers->properties[classType->protectedMembers->propertyCount++] = getDataTypeFromASTNode(property);
}

void addProtectedMethodToClassType(ClassType *classType, ASTNode *method)
{
    __STACK_FRAME__
    if (!classType || !method)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid class type or method\n");
        return;
    }

    if (classType->protectedMembers->methodCount + 1 >= classType->protectedMembers->methodCapacity)
    {
        // Grow methods array
        int newCapacity = classType->protectedMembers->methodCapacity * 2;
        DataType **newMethods = (DataType **)realloc(classType->protectedMembers->methods, newCapacity * sizeof(DataType *));
        if (!newMethods)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to grow methods array\n");
            return;
        }

        classType->protectedMembers->methods = newMethods;
        classType->protectedMembers->methodCapacity = newCapacity;
    }

    // Add method to class type
    classType->protectedMembers->methods[classType->protectedMembers->methodCount++] = getDataTypeFromASTNode(method);
}

PublicMembersTypes *createPublicMembersType(void)
{
    __STACK_FRAME__
    PublicMembersTypes *members = (PublicMembersTypes *)malloc(sizeof(PublicMembersTypes));
    if (!members)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate PublicMembersTypes\n");
        return NULL;
    }

    members->propertyCount = 0;
    members->propertyCapacity = PROPERTY_CAPACITY;
    members->properties = (DataType **)malloc(members->propertyCapacity * sizeof(DataType *));
    if (!members->properties)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate properties array\n");
        return NULL;
    }

    members->methodCount = 0;
    members->methodCapacity = METHOD_CAPACITY;
    members->methods = (DataType **)malloc(members->methodCapacity * sizeof(DataType *));
    if (!members->methods)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate methods array\n");
        return NULL;
    }

    return members;
}

PrivateMembersTypes *createPrivateMembersType(void)
{
    __STACK_FRAME__
    PrivateMembersTypes *members = (PrivateMembersTypes *)malloc(sizeof(PrivateMembersTypes));
    if (!members)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate PrivateMembersTypes\n");
        return NULL;
    }

    members->propertyCount = 0;
    members->propertyCapacity = PROPERTY_CAPACITY;
    members->properties = (DataType **)malloc(members->propertyCapacity * sizeof(DataType *));
    if (!members->properties)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate properties array\n");
        return NULL;
    }

    members->methodCount = 0;
    members->methodCapacity = METHOD_CAPACITY;
    members->methods = (DataType **)malloc(members->methodCapacity * sizeof(DataType *));
    if (!members->methods)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate methods array\n");
        return NULL;
    }

    return members;
}

ProtectedMembersTypes *createProtectedMembersType(void)
{
    __STACK_FRAME__
    ProtectedMembersTypes *members = (ProtectedMembersTypes *)malloc(sizeof(ProtectedMembersTypes));
    if (!members)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate ProtectedMembersTypes\n");
        return NULL;
    }

    members->propertyCount = 0;
    members->propertyCapacity = PROPERTY_CAPACITY;
    members->properties = (DataType **)malloc(members->propertyCapacity * sizeof(DataType *));
    if (!members->properties)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate properties array\n");
        return NULL;
    }

    members->methodCount = 0;
    members->methodCapacity = METHOD_CAPACITY;
    members->methods = (DataType **)malloc(members->methodCapacity * sizeof(DataType *));
    if (!members->methods)
    {
        fprintf(stderr, "[TypeTable] Error: Failed to allocate methods array\n");
        return NULL;
    }

    return members;
}

void linkPublicMemebers(PublicMembersTypes *membersType, PublicMembers *membersNode)
{
    __STACK_FRAME__
    if (!membersType || !membersNode)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid members type or members node\n");
        return;
    }

    for (int i = 0; i < membersNode->propertyCount; i++)
    {
        DataType *property = getDataTypeFromASTNode(membersNode->properties[i]);
        if (!property)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->properties[membersType->propertyCount++] = property;
    }

    for (int i = 0; i < membersNode->methodCount; i++)
    {
        DataType *method = getDataTypeFromASTNode(membersNode->methods[i]);
        if (!method)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->methods[membersType->methodCount++] = method;
    }
}

void linkPrivateMemebers(PrivateMembersTypes *membersType, PrivateMembers *membersNode)
{
    __STACK_FRAME__
    if (!membersType || !membersNode)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid members type or members node\n");
        return;
    }

    for (int i = 0; i < membersNode->propertyCount; i++)
    {
        DataType *property = getDataTypeFromASTNode(membersNode->properties[i]);
        if (!property)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->properties[membersType->propertyCount++] = property;
    }

    for (int i = 0; i < membersNode->methodCount; i++)
    {
        DataType *method = getDataTypeFromASTNode(membersNode->methods[i]);
        if (!method)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->methods[membersType->methodCount++] = method;
    }
}

void linkProtectedMemebers(ProtectedMembersTypes *membersType, ProtectedMembers *membersNode)
{
    __STACK_FRAME__
    if (!membersType || !membersNode)
    {
        fprintf(stderr, "[TypeTable] Error: Invalid members type or members node\n");
        return;
    }

    for (int i = 0; i < membersNode->propertyCount; i++)
    {
        DataType *property = getDataTypeFromASTNode(membersNode->properties[i]);
        if (!property)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->properties[membersType->propertyCount++] = property;
    }

    for (int i = 0; i < membersNode->methodCount; i++)
    {
        DataType *method = getDataTypeFromASTNode(membersNode->methods[i]);
        if (!method)
        {
            fprintf(stderr, "[TypeTable] Error: Failed to get data type from AST node\n");
            return;
        }

        membersType->methods[membersType->methodCount++] = method;
    }
}