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
    llvm::Value *Initializer::generateScopedFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating scoped function call...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_SCOPED_FUNCTION_CALL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a scoped function call");
            return nullptr;
        }

        std::string scopeName = node->data.scopedFunctionCall->scopeName;
        std::string functionName = node->data.scopedFunctionCall->functionName;
        std::string fullFunctionName = scopeName + "." + functionName;

        logMessage(LMI, "INFO", "Initializer", "Scoped function call: %s", fullFunctionName.c_str());
        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = context.getInstance().symbolTable->findFunction(fullFunctionName);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", fullFunctionName.c_str());
            CONDITION_FAILED;
            return nullptr;
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.scopedFunctionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            ASTNode *argNode = node->data.scopedFunctionCall->args[i];
            DataType *argDataType = DTM->astInterface->getTypeofASTNode(argNode);
            if (!argDataType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Argument %d has no data type", i);
                return nullptr;
            }
            llvm::Value *argValue = getInitializerValue(argNode);
            if (!argValue)
            {
                logMessage(LMI, "ERROR", "Initializer", "Failed to generate argument %d", i);
                return nullptr;
            }
            if (argValue->getType()->isPointerTy())
            {
                logMessage(LMI, "INFO", "Initializer", "Argument %d is a pointer type", i);
                // Load the value if it's a pointer
                llvm::Type *argType = context.getInstance().symbolTable->getLLVMType(argDataType);
                argValue = context.getInstance().builder.CreateLoad(argType, argValue);
                logMessage(LMI, "INFO", "Initializer", "Loaded argument %d value: %s", i, argValue->getName().str().c_str());
            }
            else
            {
                logMessage(LMI, "INFO", "Initializer", "Argument %d is not a pointer type", i);
            }
            args.push_back(argValue);
        }

        logMessage(LMI, "INFO", "Initializer", "Function call: %s", fullFunctionName.c_str());
        llvm::Function *function = funcSymbol->function;
        if (!function)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", fullFunctionName.c_str());
            CONDITION_FAILED;
            return nullptr;
        }

        // Create the function call
        llvm::Value *call = context.getInstance().builder.CreateCall(function, args, fullFunctionName);
        if (!call)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function call failed");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Initializer", "Function call created: %s", call->getName().str().c_str());
        return call;
    }
} // namespace Cryo
