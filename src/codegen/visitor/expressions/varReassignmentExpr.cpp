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
            return;

        // Store the value in the variable
        builder.CreateStore(rhsValue, symbol->allocation.getValue(), true);

        // Load the variable if needed
        if (symbol->allocaType == AllocaType::AllocaAndLoad ||
            symbol->allocaType == AllocaType::AllocaLoadStore)
        {
            symbol->allocation.load(builder, std::string(varName) + "_load");
        }

        // Update the last value
        lastValue = rhsValue;
        logMessage(LMI, "INFO", "CodeGenVisitor", "Variable %s reassigned", varName.c_str());
        // Update the symbol table with the new value
        symbol->value = rhsValue;
        symbol->allocation.setValue(rhsValue);
        symbol->allocaType = AllocaType::AllocaAndLoad;
        logMessage(LMI, "INFO", "CodeGenVisitor", "Variable %s updated in symbol table", varName.c_str());

        return;
    }

} // namespace Cryo