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
// ------------------------------------- Class Data Types -------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMClassTypes_createClassTemplate(void)
{
    DTClassTy *classType = createDTClassTy();
    return DTM->dataTypes->wrapClassType(classType);
}

DataType *DTMClassTypes_createClassType(const char *className, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount)
{
    DTClassTy *classType = createDTClassTy();
    classType->name = className;
    classType->properties = properties;
    classType->propertyCount = propertyCount;
    classType->methods = methods;
    classType->methodCount = methodCount;
    return DTM->dataTypes->wrapClassType(classType);
}

DataType *DTMClassTypes_createClassTypeWithMembers(
    const char *className,
    ASTNode **publicProperties, int publicPropertyCount,
    ASTNode **privateProperties, int privatePropertyCount,
    ASTNode **protectedProperties, int protectedPropertyCount,
    ASTNode **publicMethods, int publicMethodCount,
    ASTNode **privateMethods, int privateMethodCount,
    ASTNode **protectedMethods, int protectedMethodCount,
    bool hasConstructor,
    ASTNode **constructors, int ctorCount)
{
    DTClassTy *classType = createDTClassTy();
    classType->name = className;

    DTPropertyTy **publicProps = DTM->astInterface->createPropertyArrayFromAST(publicProperties, publicPropertyCount);
    DTPropertyTy **privateProps = DTM->astInterface->createPropertyArrayFromAST(privateProperties, privatePropertyCount);
    DTPropertyTy **protectedProps = DTM->astInterface->createPropertyArrayFromAST(protectedProperties, protectedPropertyCount);

    classType->publicMembers->properties = publicProps;
    classType->publicMembers->propertyCount = publicPropertyCount;
    classType->privateMembers->properties = privateProps;
    classType->privateMembers->propertyCount = privatePropertyCount;
    classType->protectedMembers->properties = protectedProps;
    classType->protectedMembers->propertyCount = protectedPropertyCount;

    classType->publicMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(publicMethods, publicMethodCount);
    classType->publicMembers->methodCount = publicMethodCount;
    classType->privateMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(privateMethods, privateMethodCount);
    classType->privateMembers->methodCount = privateMethodCount;
    classType->protectedMembers->methods = DTM->astInterface->createTypeArrayFromASTArray(protectedMethods, protectedMethodCount);
    classType->protectedMembers->methodCount = protectedMethodCount;

    classType->hasConstructor = hasConstructor;
    classType->constructors = DTM->astInterface->createTypeArrayFromASTArray(constructors, ctorCount);
    classType->ctorCount = ctorCount;

    DataType *classDataType = DTM->dataTypes->wrapClassType(classType);
    classDataType->setTypeName(classDataType, className);

    return classDataType;
}

DTMClassTypes *createDTMClassTypes(void)
{
    DTMClassTypes *classTypes = (DTMClassTypes *)malloc(sizeof(DTMClassTypes));
    if (!classTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Class Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //
    classTypes->createClassTemplate = DTMClassTypes_createClassTemplate;
    classTypes->createClassType = DTMClassTypes_createClassType;
    classTypes->createClassTypeWithMembers = DTMClassTypes_createClassTypeWithMembers;

    return classTypes;
}
