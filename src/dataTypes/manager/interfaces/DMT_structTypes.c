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
#include "frontend/frontendSymbolTable.h"
#include "dataTypes/dataTypeManager.h"

// --------------------------------------------------------------------------------------------------- //
// ------------------------------------- Struct Data Types ------------------------------------------- //
// --------------------------------------------------------------------------------------------------- //

DataType *DTMStructTypes_createStructTemplate(void)
{
    DTStructTy *structType = createDTStructTy();
    return DTM->dataTypes->wrapStructType(structType);
}

DataType *DTMStructTypes_createCompleteStructType(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount, bool hasConstructor, DataType **ctorArgs, int *ctorArgCount)
{
    DTStructTy *structType = createDTStructTy();
    structType->name = structName;
    structType->properties = properties;
    structType->propertyCount = propertyCount;
    structType->methods = methods;
    structType->methodCount = methodCount;
    structType->hasConstructor = hasConstructor;
    structType->ctorParams = ctorArgs;
    structType->ctorParamCount = *ctorArgCount;
    DataType *structDataType = DTM->dataTypes->wrapStructType(structType);
    structDataType->setTypeName(structDataType, structName);
    return structDataType;
}

DataType *DTMStructTypes_createStructType(const char *structName, DTPropertyTy **properties, int propertyCount, DataType **methods, int methodCount)
{
    DTStructTy *structType = createDTStructTy();
    structType->name = structName;
    structType->properties = properties;
    structType->propertyCount = propertyCount;
    structType->methods = methods;
    structType->methodCount = methodCount;
    return DTM->dataTypes->wrapStructType(structType);
}

DataType *DTMStructTypes_createProtoType(const char *structName)
{
    DTStructTy *structType = createDTStructTy();
    structType->name = structName;
    structType->isProtoType = true;
    DataType *type = DTM->dataTypes->wrapStructType(structType);
    type->setTypeName(type, structName);
    type->isPrototype = true;
    return type;
}

DTMStructTypes *createDTMStructTypes(void)
{
    DTMStructTypes *structTypes = (DTMStructTypes *)malloc(sizeof(DTMStructTypes));
    if (!structTypes)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate DTM Struct Types\n");
        CONDITION_FAILED;
    }

    // ==================== [ Function Assignments ] ==================== //

    structTypes->createStructTemplate = DTMStructTypes_createStructTemplate;
    structTypes->createCompleteStructType = DTMStructTypes_createCompleteStructType;
    structTypes->createStructType = DTMStructTypes_createStructType;
    structTypes->createStructProtoType = DTMStructTypes_createProtoType;

    return structTypes;
}
