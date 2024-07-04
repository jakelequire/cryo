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



bool declareFunctions(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module) {
    bool mainFunctionExists = false;

    if (!node) {
        std::cerr << "[CPP] Error in declareFunctions: AST node is null\n";
        return mainFunctionExists;
    }

    if (node->type == CryoNodeType::NODE_FUNCTION_DECLARATION) {
        if (std::string(node->data.functionDecl.name) == "main") {
            mainFunctionExists = true;
        }
        generateFunctionPrototype(node, builder, module);
    }

    // Recursively declare functions for all child nodes
    switch (node->type) {
        case CryoNodeType::NODE_PROGRAM:
            for (int i = 0; i < node->data.program.stmtCount; ++i) {
                if (declareFunctions(node->data.program.statements[i], builder, module)) {
                    mainFunctionExists = true;
                }
            }
            break;

        case CryoNodeType::NODE_BLOCK:
            for (int i = 0; i < node->data.block.stmtCount; ++i) {
                if (declareFunctions(node->data.block.statements[i], builder, module)) {
                    mainFunctionExists = true;
                }
            }
            break;

        default:
            break;
    }
    return mainFunctionExists;
}




