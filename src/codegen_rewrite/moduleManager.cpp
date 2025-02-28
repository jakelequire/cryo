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
    void CodegenContext::DONOTUSEYET_mergeModule(llvm::Module *srcModule)
    {
        if (!module)
        {
            logMessage(LMI, "ERROR", "CryoContext", "Main module is null");
            CONDITION_FAILED;
            return;
        }

        if (!srcModule)
        {
            logMessage(LMI, "ERROR", "CryoContext", "Source module is null");
            CONDITION_FAILED;
            return;
        }

        logMessage(LMI, "INFO", "CryoContext", "Merging modules");
        logMessage(LMI, "INFO", "CryoContext", "Main Module: %s", module->getName().str().c_str());

        llvm::Linker::Flags linkerFlags = llvm::Linker::Flags::None;
        bool result = llvm::Linker::linkModules(
            *module,
            llvm::CloneModule(*srcModule),
            linkerFlags);
        if (result)
        {
            logMessage(LMI, "ERROR", "CryoContext", "Failed to merge modules");
            CONDITION_FAILED;
            return;
        }

        std::cout << "@mergeModule Module merged successfully" << std::endl;
    }

} // namespace Cryo
