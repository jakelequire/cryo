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
#ifndef IRGENERATOR_HPP
#define IRGENERATOR_HPP
#include <string>
#include "common/common.h"

#include "codegen/devDebugger/devDebugger.hpp"
#include "codegen/generation/codegen.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

    int ASTModuleToIR(ASTNode *node, CompilerState *state);

#ifdef __cplusplus
}
#endif

namespace Cryo
{

    class IRGenerator : public ModuleContext
    {
    public:
        IRGenerator() = default;
        ~IRGenerator() = default;

        int generateIR(ASTNode *node, CompilerState *state);

        CodeGen &getCodeGen() { return *codeGen; }

    private:
        ModuleContext &context = ModuleContext::getInstance();
        std::unique_ptr<CodeGen> codeGen;
    };

} // namespace Cryo

#endif // IRGENERATOR_HPP
