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
            return nullptr;
        }

        DataType *literalDataType = node->data.literal->type;
        if (literalDataType->container->primitive != PRIM_STRING)
        {
            logMessage(LMI, "ERROR", "Initializer", "Data type is not a string");
            return nullptr;
        }

        std::string strValue = node->data.literal->value.stringValue;

        logMessage(LMI, "INFO", "Initializer", "String value: %s", strValue.c_str());

        // Create a constant array for the string
        llvm::ArrayType *strArrayType = llvm::ArrayType::get(
            llvm::Type::getInt8Ty(context.getInstance().context), strValue.size() + 1); // +1 for null terminator
        llvm::Constant *strConstant = llvm::ConstantDataArray::getString(
            context.getInstance().context, strValue, true);

        // Allocate memory for the string on the stack
        llvm::Value *strAlloc = context.getInstance().builder.CreateAlloca(strArrayType, nullptr, "strAlloc");

        // Store the constant string into the allocated memory
        context.getInstance().builder.CreateStore(strConstant, strAlloc);

        // Cast the pointer to i8* for compatibility
        llvm::Value *strPtr = context.getInstance().builder.CreatePointerCast(
            strAlloc, llvm::Type::getInt8Ty(context.getInstance().context));

        logMessage(LMI, "INFO", "Initializer", "String literal generated successfully.");

        return strPtr;
    }
    
} // namespace Cryo
