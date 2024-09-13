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
#include "cpp/codegen.h"

namespace Cryo
{
    /// @public
    void IfStatements::handleIfStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "IfStatements", "Handling If Statement");
        assert(node != nullptr);

        createIfStatement(node);
    }

    void IfStatements::createIfStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "IfStatements", "Creating If Statement");

        IfStatementNode *ifNode = node->data.ifStatement;

        // Get the condition
        ASTNode *condition = ifNode->condition;
        assert(condition != nullptr);

        // Get the then branch
        ASTNode *thenBranch = ifNode->thenBranch;
        assert(thenBranch != nullptr);

        // Get the else branch
        ASTNode *elseBranch = ifNode->elseBranch;

        // Create the condition
        llvm::BasicBlock *conditionBlock = createIfCondition(condition);

        exit(0);
    }

    llvm::BasicBlock *IfStatements::createIfCondition(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "IfStatements", "Creating If Condition");

        CryoNodeType nodeType = node->metaData->type;
        std::cout << "Node Type: " << CryoNodeTypeToString(nodeType) << std::endl;

        exit(0);

        return nullptr;
    }
} // namespace Cryo
