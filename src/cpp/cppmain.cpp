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
#include "cpp/cppmain.h"
#include <iostream>

using namespace Cryo;

int generateCodeWrapper(ASTNode *node)
{
    std::cout << ">===------------- CPP Code Generation -------------===<\n"
              << std::endl;
    std::cout << "[CPP] Starting Code Generation..." << std::endl;

    // Recast the ASTNode to ASTNode
    ASTNode *root;
    memcpy(&root, &node, sizeof(ASTNode *) * node->data.program->statementCount);
    std::cout << "[CPP] Root Node Initialized" << std::endl;

    std::cout << "\n\n [ DEBUG ]"
              // << CryoDataTypeToString(node->data.program->statements[2]->data.varDecl->type)
              << strdup(root->data.program->statements[2]->data.varDecl->name)
              << " \n\n"
              << std::endl;

    Cryo::CryoCompiler compiler;
    compiler.compile(node);

    return 1;
}
