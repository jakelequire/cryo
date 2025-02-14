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
#include "codegen/rewrite/codegen.hpp"

namespace Cryo
{
    void IRGeneration::processDeclarations(ASTNode *node)
    {
        if (!node)
            return;

        // Handle function declarations
        if (node->metaData->type == NODE_FUNCTION_DECLARATION)
        {
            processFunctionDeclaration(node);
        }

        // Handle type declarations (structs, classes)
        if (node->metaData->type == NODE_STRUCT_DECLARATION ||
            node->metaData->type == NODE_CLASS)
        {
            processTypeDeclaration(node);
        }

        // Recursively process children
        // for (auto child : node->children)
        // {
        //     processDeclarations(child);
        // }
    }

    void IRGeneration::processFunctionDeclaration(ASTNode *node)
    {
    }
    void IRGeneration::processTypeDeclaration(ASTNode *node)
    {
    }

} // namespace Cryo