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

    void CodeGenVisitor::visitReturnStatement(ASTNode *node)
    {
        if (!node || !node->data.returnStatement)
            return;
        logMessage(LMI, "INFO", "CodeGenVisitor", "Visiting return statement node");

        // Get the generated value
        llvm::Value *retVal = getLLVMValue(node->data.returnStatement->expression);
        if (retVal)
        {
            logMessage(LMI, "INFO", "CodeGenVisitor", "Return value: %s", retVal->getName().str().c_str());
            llvm::Type *retType = context.getInstance().builder.GetInsertBlock()->getParent()->getReturnType();
            if (retType->isVoidTy())
            {
                logMessage(LMI, "ERROR", "CodeGenVisitor", "Function return type is void");
                return;
            }
            if (!retType->isPointerTy() && retVal->getType()->isPointerTy())
            {
                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(retVal))
                {
                    retVal = context.getInstance().builder.CreateLoad(allocaInst->getAllocatedType(), retVal, "load");
                }
            }
            logMessage(LMI, "INFO", "CodeGenVisitor", "Return value name: %s", retVal->getName().str().c_str());
        }
        else
        {
            logMessage(LMI, "INFO", "CodeGenVisitor", "No return value, returning void");
            context.getInstance().builder.CreateRetVoid();
        }

        logMessage(LMI, "INFO", "CodeGenVisitor", "Creating return statement");

        // Create the return statement
        if (retVal)
        {
            context.getInstance().builder.CreateRet(retVal);
            logMessage(LMI, "INFO", "CodeGenVisitor", "Return statement created");
        }
        else
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Return value is null");
        }

        // Clear the continue and break blocks
        context.getInstance().symbolTable->setContinueBlock(nullptr);
        context.getInstance().symbolTable->setBreakBlock(nullptr);

        return;
    }
} // namespace Cryo
