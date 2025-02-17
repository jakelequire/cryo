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

    void IRGeneration::generateIR(ASTNode *root)
    {
        if (!root)
            return;

        // Process declarations first (hoisting)
        processDeclarations(root);

        // Generate code for the AST
        generateIRForNode(root);
    }

    void IRGeneration::generateIRForNode(ASTNode *node)
    {
        if (!node)
            return;

        switch (node->metaData->type)
        {
        case NODE_LITERAL_EXPR:
            // generateIRForLiteralExpr(node);
            break;

        case NODE_VAR_NAME:
            // generateIRForVarName(node);
            break;

        case NODE_BINARY_EXPR:
            // generateIRForBinaryExpr(node);
            break;

        case NODE_FUNCTION_CALL:
            // generateIRForFunctionCall(node);
            break;

            // ... handle other node types
        }

        // Process children after current node
        // for (auto child : node->children)
        // {
        //     generateIRForNode(child);
        // }
    }

} // namespace Cryo
