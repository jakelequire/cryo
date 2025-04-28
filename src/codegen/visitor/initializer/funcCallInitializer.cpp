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

    llvm::Value *Initializer::generateFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating function call...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_FUNCTION_CALL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a function call");
            return nullptr;
        }

        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = getSymbolTable()->findFunction(node->data.functionCall->name);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", node->data.functionCall->name);
            // Check the DTM to see if the function exists
            DataType *functionDataType = DTM->symbolTable->lookup(DTM->symbolTable, node->data.functionCall->name);
            if (functionDataType)
            {
                logMessage(LMI, "INFO", "Initializer", "Function %s found in DTM", node->data.functionCall->name);
            }
            else
            {
                return nullptr;
            }
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.functionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            ASTNode *argNode = node->data.functionCall->args[i];
            DataType *argDataType = DTM->astInterface->getTypeofASTNode(argNode);
            if (!argDataType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Argument %d has no data type", i);
                return nullptr;
            }
            llvm::Type *argType = context.getInstance().symbolTable->getLLVMType(argDataType);
            llvm::Value *argVal = getInitializerValue(argNode);
            if (argDataType->container->primitive == PRIM_STR ||
                argVal->getType()->isPointerTy())
            {
                // Load the value if it's a pointer
                argVal = context.getInstance().builder.CreateLoad(argType, argVal);
            }

            args.push_back(argVal);
            logMessage(LMI, "INFO", "Initializer", "Argument %d: %s", i, argVal->getName().str().c_str());
        }
        std::string funcName = node->data.functionCall->name;
        logMessage(LMI, "INFO", "Initializer", "Function call: %s", funcName.c_str());

        return context.getInstance().builder.CreateCall(funcSymbol->function, args, funcName);
    }

} // namespace Cryo
