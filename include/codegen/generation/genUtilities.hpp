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
#ifndef GEN_UTILITIES_HPP
#define GEN_UTILITIES_HPP
#include <iostream>
#include <string>
#include <vector>

#include "codegen/devDebugger/devDebugger.hpp"
#include "codegen/generation/codegen.hpp"

namespace Cryo
{
    class CodeGen;

    class GenUtilities : public CodeGen
    {
    public:
        GenUtilities(ModuleContext &context) : CodeGen(context) {}
        ~GenUtilities() = default;

        std::string getNamespaceFromAST(ASTNode *node);
        std::string formatString(std::string str);

    private:
    };
}

#endif // GEN_UTILITIES_HPP