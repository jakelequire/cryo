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
#include "codegen_rewrite/codegen.hpp"

namespace Cryo
{
    // [2]: Step 2. Begin IR Generation
    void Cryo::IRGeneration::generateIR(ASTNode *root)
    {
        if (!root)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Root node is null");
            return;
        }

        // Process declarations first (hoisting) [3]
        processDeclarations(root);

        // Start visiting nodes
        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            for (size_t i = 0; i < root->data.program->statementCount; i++)
            {
                generateIRForNode(root->data.program->statements[i]);
            }
            break;
        default:
            generateIRForNode(root);
            break;
        }

        logMessage(LMI, "INFO", "IRGeneration", "IR Generation Complete!");
    }

    // [4]: Step 4. Generate IR for each node
    void Cryo::IRGeneration::generateIRForNode(ASTNode *node)
    {
        if (!node)
            return;

        // Use the visitor through the context
        if (!context.visitor)
        {
            context.visitor = std::make_unique<CodeGenVisitor>(context);
        }

        context.visitor->visit(node);

        return;
    }

} // namespace Cryo
