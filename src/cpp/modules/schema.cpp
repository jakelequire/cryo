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

namespace Cryo {

bool CryoModules::declareFunctions(ASTNode* node) {
    if (node == nullptr) {
        std::cerr << "Error: node is null." << std::endl;
        return false;
    }

    CryoSyntax& cryoSyntaxInstance = compiler.getSyntax();

    if (node->type == NODE_PROGRAM) {
        ASTNode* child = node->firstChild;
        while (child != nullptr) {
            if (child == nullptr) {
                std::cerr << "Error: child node is null." << std::endl;
                continue;
            }

            std::cerr << "Processing node of type: " << child->type << std::endl;

            if (child->type == NODE_FUNCTION_DECLARATION) {
                // Call to generateFunctionPrototype
                cryoSyntaxInstance.generateFunctionPrototype(child);
            }
        }
    }
    return true;
}

} // namespace Cryo
