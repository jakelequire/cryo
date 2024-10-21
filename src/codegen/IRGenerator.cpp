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
#include "codegen/IRGenerator.hpp"

using namespace Cryo;

/// @brief This is a C Interface function to generate IR from an ASTNode passed from the C API.
/// This will initialize the IR Generation process and generate the IR for the ASTNode.
/// @param node
/// @param state
/// @return
extern "C"
{
    int ASTModuleToIR(ASTNode *node, CompilerState *state);
}

int ASTModuleToIR(ASTNode *node, CompilerState *state)
{
    Cryo::IRGenerator irGen;
    int result = irGen.generateIR(node, state);

    if (result != 0)
    {
        return 1;
    }

    DevDebugger::logMessage("INFO", __LINE__, "IRGenerator", "IR Generation Complete");

    return 0;
}

namespace Cryo
{
    int IRGenerator::generateIR(ASTNode *node, CompilerState *state)
    {
        CodeGen codeGen(context);
        codeGen.generateModuleFromAST(node);

        return 0;
    }
}
