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

int generateCodeWrapper(ASTNode *node, CompilerState *state)
{
    std::cout << ">===------------- CPP Code Generation -------------===<\n"
              << std::endl;

    Cryo::CryoCompiler compiler;
    compiler.setCompilerState(state);

    std::string moduleName = state->fileName;
    std::cout << "Module Name: " << moduleName << std::endl;
    compiler.setModuleIdentifier(moduleName);

    compiler.compile(node);

    return 0;
}
