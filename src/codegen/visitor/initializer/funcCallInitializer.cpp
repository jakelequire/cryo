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
        std::string funcName = node->data.functionCall->name;
        IRFunctionSymbol *funcSymbol = getSymbolTable()->findFunction(funcName);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", funcName.c_str());
            // Check the DTM to see if the function exists
            DataType *functionDataType = DTM->symbolTable->lookup(DTM->symbolTable, funcName.c_str());
            if (functionDataType)
            {
                logMessage(LMI, "INFO", "Initializer", "Function %s found in DTM", funcName.c_str());
            }
            else
            {
                return nullptr;
            }
        }

        // Get expected parameter types if available
        DataType **expectedParamTypes = nullptr;
        if (funcSymbol && funcSymbol->functionDataType &&
            funcSymbol->functionDataType->container->typeOf == FUNCTION_TYPE)
        {
            expectedParamTypes = funcSymbol->functionDataType->container->type.functionType->paramTypes;
        }

        // Process arguments using the new utility function
        std::vector<llvm::Value *> args = processArguments(
            node->data.functionCall->args,
            node->data.functionCall->argCount,
            expectedParamTypes,
            funcName);

        // Create the call
        llvm::Value *result = context.getInstance().builder.CreateCall(
            funcSymbol->function,
            args,
            funcName + ".call");

        // Get return type for processing
        DataType *returnType = nullptr;
        if (funcSymbol && funcSymbol->functionDataType &&
            funcSymbol->functionDataType->container->typeOf == FUNCTION_TYPE)
        {
            returnType = funcSymbol->functionDataType->container->type.functionType->returnType;
        }

        return processReturnValue(
            result,
            returnType,
            funcName);
    }

} // namespace Cryo
