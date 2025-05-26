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
#include "codegen/codegen.hpp"

namespace Cryo
{

    void CodeGenVisitor::visitVarReassignment(ASTNode *node)
    {
        if (!node || !node->data.varReassignment)
            return;

        VariableReassignmentNode *varReassignment = node->data.varReassignment;
        std::string varName = varReassignment->existingVarName;

        // Find the variable in the symbol table
        IRVariableSymbol *symbol = symbolTable->findVariable(varName);
        if (!symbol)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Variable %s not found", varName.c_str());
            return;
        }

        // Generate the value for the right-hand side expression
        llvm::Value *rhsValue = getLLVMValue(varReassignment->newVarNode);
        if (!rhsValue)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Failed to generate RHS value for %s", varName.c_str());
            return;
        }

        // For variable names, we need to load the value first
        if (varReassignment->newVarNode->metaData->type == NODE_VAR_NAME)
        {
            // If the RHS is a variable name, we need to load its value
            std::string rhsName = varReassignment->newVarNode->data.varName->varName;
            IRVariableSymbol *rhsSymbol = symbolTable->findVariable(rhsName);
            if (rhsSymbol && rhsSymbol->value)
            {
                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(rhsSymbol->value))
                {
                    // Load the value from the RHS variable
                    llvm::Type *rhsType = allocaInst->getAllocatedType();
                    rhsValue = context.getInstance().builder.CreateLoad(
                        rhsType, rhsSymbol->value, rhsName + ".load");
                }
            }
        }

        // Store the value
        llvm::Value *targetPtr = symbol->value;

        // Handle type compatibility
        llvm::Type *targetType = nullptr;
        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(targetPtr))
        {
            targetType = allocaInst->getAllocatedType();
        }
        else if (targetPtr->getType()->isPointerTy())
        {
            // Get the type through pointer inspection for non-alloca pointers
            targetType = targetPtr->getType();

            // We want the element type, not the pointer type
            while (targetType->isPointerTy())
            {
                // Extract the element type from the pointer type
                if (llvm::PointerType *ptrTy = llvm::dyn_cast<llvm::PointerType>(targetType))
                {
                    targetType = ptrTy->getContainedType(0);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Cannot determine target type for %s", varName.c_str());
            return;
        }

        // Handle type casting if needed
        if (rhsValue->getType() != targetType)
        {
            logMessage(LMI, "INFO", "CodeGenVisitor", "Type mismatch in assignment to %s, attempting cast", varName.c_str());
            if (targetType->isIntegerTy() && rhsValue->getType()->isIntegerTy())
            {
                // Integer to integer cast
                rhsValue = context.getInstance().builder.CreateIntCast(
                    rhsValue, targetType, true, "cast_to_" + varName);
            }
            else if (targetType->isFloatingPointTy() && rhsValue->getType()->isFloatingPointTy())
            {
                // Float to float cast
                rhsValue = context.getInstance().builder.CreateFPCast(
                    rhsValue, targetType, "cast_to_" + varName);
            }
            else if (targetType->isPointerTy() && rhsValue->getType()->isPointerTy())
            {
                // Pointer to pointer cast
                rhsValue = context.getInstance().builder.CreatePointerCast(
                    rhsValue, targetType, "cast_to_" + varName);
            }
            else
            {
                logMessage(LMI, "WARNING", "CodeGenVisitor", "Incompatible types in assignment to %s", varName.c_str());
            }
        }

        // Store based on allocation type - ONLY ONCE
        llvm::Value *loadValue = nullptr;

        if (symbol->allocaType == AllocaType::AllocaOnly)
        {
            context.getInstance().builder.CreateStore(rhsValue, targetPtr);
        }
        else if (symbol->allocaType == AllocaType::AllocaAndLoad ||
                 symbol->allocaType == AllocaType::AllocaLoadStore)
        {
            // Store to the pointer and then load it back
            context.getInstance().builder.CreateStore(rhsValue, targetPtr);

            // Optionally load the value back (if your AllocaType requires this)
            loadValue = context.getInstance().builder.CreateLoad(
                targetType, targetPtr, "load_after_store." + varName);
        }
        else
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Unknown allocation type for %s", varName.c_str());
            return;
        }

        logMessage(LMI, "INFO", "CodeGenVisitor", "Variable %s reassigned successfully", varName.c_str());
        return;
    }

} // namespace Cryo
