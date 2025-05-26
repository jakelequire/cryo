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
#include "frontend/frontendSymbolTable.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// -------------------------------------- Property Data Types ---------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DTPropertyTy *DTMPropertyTypes_createPropertyTemplate(void)
{
    DTPropertyTy *property = createDTProperty();
    if (!property)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Template\n");
        CONDITION_FAILED;
    }
    return property;
}

DTPropertyTy *DTMPropertyTypes_createPropertyType(const char *name, DataType *type, ASTNode *node, bool isStatic, bool isConst, bool isPublic, bool isPrivate, bool isProtected)
{
    DTPropertyTy *property = createDTProperty();
    if (!property)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Type\n");
        CONDITION_FAILED;
    }
    property->node = node;
    property->setName(property, name);
    property->setType(property, type);
    property->setStatic(property, isStatic);
    property->setConst(property, isConst);
    property->setPublic(property, isPublic);
    property->setPrivate(property, isPrivate);
    property->setProtected(property, isProtected);
    return property;
}

ASTNode *DTMPropertyTypes_findStructPropertyNode(DTStructTy *structNode, const char *propertyName)
{
    for (int i = 0; i < structNode->propertyCount; i++)
    {
        logMessage(LMI, "INFO", "DTM", "Checking Property: %s == %s", structNode->properties[i]->name, propertyName);
        if (strcmp(structNode->properties[i]->name, propertyName) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found Property: %s == %s", structNode->properties[i]->name, propertyName);
            return structNode->properties[i]->node;
        }
    }
    logMessage(LMI, "INFO", "DTM", "Property not found: %s", propertyName);
    return NULL;
}

int DTMPropertyTypes_getStructPropertyIndex(DataType *structType, const char *propertyName)
{
    if (!structType)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
        structType->debug->printType(structType);
        DEBUG_BREAKPOINT;
        return -99;
    }
    if (structType->container->objectType != STRUCT_OBJ)
    {
        fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
        structType->debug->printType(structType);
        DEBUG_BREAKPOINT;
        return -99;
    }

    DataType *structDataType = structType;

    if (structType->container->typeOf == POINTER_TYPE)
    {
        DataType *baseType = structType->container->type.pointerType->baseType;
        if (baseType->container->objectType != STRUCT_OBJ)
        {
            fprintf(stderr, "[Data Type Manager] Error: Invalid Struct Type\n");
            baseType->debug->printType(baseType);
            DEBUG_BREAKPOINT;
            return -99;
        }

        structDataType = baseType;
    }

    logMessage(LMI, "INFO", "DTM", "Getting Struct Property Index: %s", propertyName);
    logMessage(LMI, "INFO", "DTM", "Struct Type: %s", structDataType->typeName);
    structDataType->debug->printType(structDataType);

    DTStructTy *structNode = structDataType->container->type.structType;
    for (int i = 0; i < structNode->propertyCount; i++)
    {
        if (strcmp(structNode->properties[i]->name, propertyName) == 0)
        {
            logMessage(LMI, "INFO", "DTM", "Found Property: %s == %s", structNode->properties[i]->name, propertyName);
            return i;
        }
    }

    logMessage(LMI, "INFO", "DTM", "Property not found: %s in %s", propertyName, structDataType->typeName);
    return -99;
}

DTMPropertyTypes *createDTMPropertyTypes(void)
{
    DTMPropertyTypes *propertyTypes = (DTMPropertyTypes *)malloc(sizeof(DTMPropertyTypes));
    if (!propertyTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Property Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    propertyTypes->createPropertyTemplate = DTMPropertyTypes_createPropertyTemplate;
    propertyTypes->createPropertyType = DTMPropertyTypes_createPropertyType;
    propertyTypes->findStructPropertyNode = DTMPropertyTypes_findStructPropertyNode;
    propertyTypes->getStructPropertyIndex = DTMPropertyTypes_getStructPropertyIndex;

    return propertyTypes;
}
