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

// The global Data Type Manager instance. This is a static instance that is used to manage data types in the compiler.
// This is initialized in the `main` function of the compiler.
DataTypeManager *globalDataTypeManager = NULL;

// --------------------------------------------------------------------------------------------------- //
// ---------------------------------- Data Type Manager Implementation ------------------------------- //
// --------------------------------------------------------------------------------------------------- //

void initGlobalDataTypeManagerInstance(void)
{
    globalDataTypeManager = createDataTypeManager();
    globalDataTypeManager->initialized = true;
}

void DataTypeManager_initDefinitions(void)
{
    register_VA_ARGS_type();
    return;
}

DataTypeManager *createDataTypeManager(void)
{
    DataTypeManager *manager = (DataTypeManager *)malloc(sizeof(DataTypeManager));
    if (!manager)
    {
        fprintf(stderr, "[Data Type Manager] Error: Failed to allocate Data Type Manager\n");
        CONDITION_FAILED;
    }

    // ===================== [ Property Assignments ] ===================== //

    manager->initialized = false;
    manager->defsInitialized = false;

    manager->symbolTable = createDTMSymbolTable();
    manager->dataTypes = createDTMDataTypes();
    manager->validation = createDTMTypeValidation();

    manager->primitives = createDTMPrimitives();
    manager->propertyTypes = createDTMPropertyTypes();
    manager->astInterface = createDTMAstInterface();
    manager->structTypes = createDTMStructTypes();
    manager->classTypes = createDTMClassTypes();
    manager->functionTypes = createDTMFunctionTypes();
    manager->generics = createDTMGenerics();
    manager->enums = createDTMEnums();
    manager->arrayTypes = createDTMArrayTypes();

    manager->compilerDefs = createDTMCompilerDefs();

    manager->helpers = createDTMHelpers();
    manager->debug = createDTMDebug();

    // ===================== [ Function Assignments ] ===================== //

    manager->initDefinitions = DataTypeManager_initDefinitions;
    manager->parseType = DTMParseType;
    manager->resolveType = DTMResolveType;
    manager->getTypeofASTNode = DTMastInterface_getTypeofASTNode;

    return manager;
}
