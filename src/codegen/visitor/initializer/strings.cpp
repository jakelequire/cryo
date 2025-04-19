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
        logMessage(LMI, "INFO", "Initializer", "Generating string literal...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a literal expression");
            CONDITION_FAILED;
            return nullptr;
        }

        DataType *literalDataType = node->data.literal->type;
        if (literalDataType->container->primitive != PRIM_STRING)
        {
            logMessage(LMI, "ERROR", "Initializer", "Data type is not a string");
            CONDITION_FAILED;
            return nullptr;
        }

        std::string strValue = node->data.literal->value.stringValue;
        logMessage(LMI, "INFO", "Initializer", "String value: %s", strValue.c_str());

        llvm::Value *globalStr = context.getInstance().symbolTable->getOrCreateGlobalString(strValue);
        if (!globalStr)
            return nullptr;

        llvm::GlobalVariable *globalStrVar = llvm::dyn_cast<llvm::GlobalVariable>(globalStr);
        if (!globalStrVar)
            return nullptr;

        // Get the array type
        llvm::ConstantDataArray *init = llvm::dyn_cast<llvm::ConstantDataArray>(globalStrVar->getInitializer());
        if (!init)
            return nullptr;

        int length = init->getNumElements();
        llvm::ArrayType *arrayType = llvm::ArrayType::get(context.getInstance().symbolTable->llvmTypes.i8Ty, length);

        // Create the GEP instruction to get a pointer to the first character
        llvm::Value *stringGEP = context.getInstance().builder.CreateInBoundsGEP(
            arrayType,
            globalStr,
            {llvm::ConstantInt::get(context.getInstance().symbolTable->llvmTypes.i32Ty, 0),
             llvm::ConstantInt::get(context.getInstance().symbolTable->llvmTypes.i32Ty, 0)},
            "");

        // Return the loaded string value
        return stringGEP;
    }

} // namespace Cryo
