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
#include "codegen/codegen.hpp"

namespace Cryo
{
    llvm::Value *Initializer::generateStringLiteral(ASTNode *node)
    {
        std::string strValue = node->data.literal->value.stringValue;
        DataType *strDataType = node->data.literal->type;
        if (strDataType->container->typeOf != OBJECT_TYPE)
        {
            // 1. Create global constant string
            llvm::Value *globalStr = context.getInstance().symbolTable->getOrCreateGlobalString(strValue);
            llvm::GlobalVariable *globalStrVar = llvm::dyn_cast<llvm::GlobalVariable>(globalStr);

            // 2. Get array type info from global constant
            llvm::ConstantDataArray *init = llvm::dyn_cast<llvm::ConstantDataArray>(globalStrVar->getInitializer());
            int length = init->getNumElements();
            llvm::ArrayType *arrayType = llvm::ArrayType::get(
                llvm::Type::getInt8Ty(context.getInstance().context), length);

            // 3. Return pointer to first character using GEP
            return context.getInstance().builder.CreateInBoundsGEP(
                arrayType,
                globalStrVar,
                llvm::ArrayRef<llvm::Value *>{
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.getInstance().context), 0),
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.getInstance().context), 0)},
                "str.ptr");
        }

        // Handled as struct String { char* ptr; }
        // 1. Create global string constant (same as above)
        llvm::Value *globalStr = context.getInstance().symbolTable->getOrCreateGlobalString(strValue);
        llvm::StructType *stringType = context.getInstance().symbolTable->getStructType("struct.String");

        // 2. Allocate String struct on stack
        llvm::Value *stringStruct = context.getInstance().builder.CreateAlloca(
            stringType, nullptr, "string.temp");

        // 3. Store string pointer in struct
        llvm::Value *ptrField = context.getInstance().builder.CreateStructGEP(
            stringType, stringStruct, 0, "str.ptr");
        context.getInstance().builder.CreateStore(globalStr, ptrField);

        // 4. Return String struct by value
        return context.getInstance().builder.CreateLoad(
            stringType, stringStruct, "string.load");
    }

    // This function is called when a variable is declared with a string literal initializer.
    // This will create a raw string literal and return a pointer to it.
    // It is equivalent to `const char *` in C/C++.
    llvm::Value *Initializer::generateStringInitializer(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is null");
            CONDITION_FAILED;
            return nullptr;
        }

        return generateStringLiteral(node);
    }
} // namespace Cryo
