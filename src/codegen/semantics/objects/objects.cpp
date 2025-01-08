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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    llvm::Value *Objects::createObjectInstance(ObjectNode *objectNode, std::string varName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Objects", "Creating Object Instance");

        llvm::Value *objectPtr = nullptr;
        llvm::Type *objectType = nullptr;
        llvm::Value *objectValue = nullptr;

        DataType *objectDataType = objectNode->objType;
        std::string objectName = objectNode->name;
        std::string objectTypeName = getDataTypeName(objectDataType);

        DevDebugger::logMessage("INFO", __LINE__, "Objects", "Object Name: " + objectName);
        DevDebugger::logMessage("INFO", __LINE__, "Objects", "Object Type: " + objectTypeName);

        llvm::StructType *structType = nullptr;
        if (objectDataType->container->baseType == STRUCT_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Objects", "Struct type");
            structType = compiler.getContext().getStruct(objectName);
        }
        else if (objectDataType->container->baseType == CLASS_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Objects", "Class type");
            structType = compiler.getContext().getClass(objectName);
        }
        else
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Objects", "Invalid object type");
            CONDITION_FAILED;
        }

        if (structType == nullptr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Objects", "Struct type not found");
            CONDITION_FAILED;
        }

        objectType = structType->getPointerTo();
        objectPtr = compiler.getContext().builder.CreateAlloca(objectType, nullptr, varName);
        objectValue = compiler.getContext().builder.CreateLoad(objectType, objectPtr);

        if (!objectPtr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Objects", "Object pointer not created");
            CONDITION_FAILED;
        }

        if (!objectValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Objects", "Object value not created");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Objects", "Object Instance Created");

        return objectPtr;
    }
} // namespace Cryo
