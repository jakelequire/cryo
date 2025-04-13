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
    // Statements
    void CodeGenVisitor::visitIfStatement(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitForStatement(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }
    void CodeGenVisitor::visitWhileStatement(ASTNode *node)
    {
        DEBUG_BREAKPOINT;
    }

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
            context.getInstance().builder.CreateRet(retVal);
        }
        else
        {
            logMessage(LMI, "INFO", "CodeGenVisitor", "No return value, returning void");
            context.getInstance().builder.CreateRetVoid();
        }
    }
} // namespace Cryo
