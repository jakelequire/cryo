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
#include "cpp/codegen.hpp"

namespace Cryo
{
    void Structs::handleStructDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Structs", "Handling Struct Declaration");

        debugger.logNode(node);

        StructNode *structNode = node->data.structNode;
        if (!structNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Structs", "StructNode is null");
            CONDITION_FAILED;
        }

        std::string structName = structNode->name;
        if (structName.empty())
        {
            debugger.logMessage("ERROR", __LINE__, "Structs", "Struct name is empty");
            CONDITION_FAILED;
        }
        debugger.logMessage("INFO", __LINE__, "Structs", "Struct name: " + structName);

        std::vector<llvm::Type *> structFields;
        for (int i = 0; i < structNode->propertyCount; ++i)
        {
            PropertyNode *property = structNode->properties[i]->data.property;
            llvm::Type *fieldType = getStructFieldType(property);
            structFields.push_back(fieldType);
        }
        debugger.logMessage("INFO", __LINE__, "Structs", "Struct fields created");

        llvm::StructType *structType = llvm::StructType::create(compiler.getContext().context, structFields, structName);
        if (!structType)
        {
            debugger.logMessage("ERROR", __LINE__, "Structs", "Failed to create struct type");
            CONDITION_FAILED;
        }
        debugger.logMessage("INFO", __LINE__, "Structs", "Struct type created");
        debugger.logLLVMStruct(structType);

        // Add to the module
        llvm::GlobalVariable *structVar = new llvm::GlobalVariable(
            *compiler.getContext().module,
            structType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            nullptr,
            llvm::Twine(structName));

        // Add to the symbol table
        compiler.getSymTable().addStruct(structName, structType, structNode);

        return;
    }

    llvm::Type *Structs::getStructFieldType(PropertyNode *property)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Structs", "Getting Struct Field Type");

        if (!property)
        {
            debugger.logMessage("ERROR", __LINE__, "Structs", "Property is null");
            CONDITION_FAILED;
        }

        CryoDataType dataType = property->type;
        std::string dataTypeStr = CryoDataTypeToString(dataType);
        debugger.logMessage("INFO", __LINE__, "Structs", "Data Type: " + dataTypeStr);

        std::string propertyName = property->name;
        if (propertyName.empty())
        {
            debugger.logMessage("ERROR", __LINE__, "Structs", "Property name is empty");
            CONDITION_FAILED;
        }
        debugger.logMessage("INFO", __LINE__, "Structs", "Property name: " + propertyName);

        llvm::Type *llvmType = compiler.getTypes().getType(dataType, 0);

        return llvmType;
    }

}