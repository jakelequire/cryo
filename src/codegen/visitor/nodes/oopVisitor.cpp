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
    // Object-oriented features
    void CodeGenVisitor::visitProperty(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

    void CodeGenVisitor::visitMethod(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

    void CodeGenVisitor::visitConstructor(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

    void CodeGenVisitor::visitPropertyAccess(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

    void CodeGenVisitor::visitPropertyReassignment(ASTNode *node)
    {
        // This function is called when a property is reassigned
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        if (node->metaData->type != NODE_PROPERTY_REASSIGN)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not a property reassignment");
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting property reassignment...");
        // Get the property name
        std::string propertyName = node->data.propertyReassignment->name;
        logMessage(LMI, "INFO", "Visitor", "Property Name: %s", propertyName.c_str());

        // Get the new value
        llvm::Value *newValue = getLLVMValue(node->data.propertyReassignment->value);
        if (!newValue)
        {
            logMessage(LMI, "ERROR", "Visitor", "New value is null");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Visitor", "New Value: %s", newValue->getName().str().c_str());

        std::string objTypeName = node->data.propertyReassignment->objectTypeName;
        logMessage(LMI, "INFO", "Visitor", "Object Type Name: %s", objTypeName.c_str());

        // Handle struct property reassignment
        if (this->symbolTable->inConstructorInstance)
        {
            // If we are in a constructor instance. There will always be a `self.alloc` variable in the current scope
            // that will be the pointer to the object instance.
            logMessage(LMI, "INFO", "Visitor", "In constructor instance");
            IRVariableSymbol *selfAllocSymbol = this->symbolTable->findLocalVariable("self.alloc");
            if (selfAllocSymbol)
            {
                logMessage(LMI, "INFO", "Visitor", "Found self allocation symbol");
                llvm::Value *selfAlloc = selfAllocSymbol->value;
                if (!selfAlloc)
                {
                    logMessage(LMI, "ERROR", "Visitor", "Self allocation symbol is null");
                    CONDITION_FAILED;
                }

                llvm::Type *llvmType = selfAllocSymbol->type;
                if (!llvmType)
                {
                    logMessage(LMI, "ERROR", "Visitor", "Property type is null");
                    CONDITION_FAILED;
                }

                // Create a store instruction to assign the new value to the property
                llvm::Value *propertyPtr = context.builder.CreateStructGEP(llvmType, selfAlloc, 0, propertyName);
                context.builder.CreateStore(newValue, propertyPtr);
            }
            else
            {
                logMessage(LMI, "ERROR", "Visitor", "Self allocation symbol not found");
                context.getInstance().symbolTable->debugPrint();
                CONDITION_FAILED;
            }
        }
    }

    void CodeGenVisitor::visitThis(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

} // namespace Cryo
