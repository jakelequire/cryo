/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "linker/linker.hpp"
#include "diagnostics/diagnostics.h"

namespace Cryo
{

    /// @brief This function will initialize the creation of the Cryo Runtime Module.
    /// This module is compiled along with a C runtime module to create the final
    /// runtime module that will be used in the Cryo Compiler before the main Cryo module.
    ///
    /// @return The Cryo Runtime Module
    ///
    /// @note this function is called from the C++ CodeGen API before code generation.
    llvm::Module *Linker::initMainModule(void)
    {
        __STACK_FRAME__
        std::cout << "Initializing Main Module before CodeGen..." << std::endl;

        // Get the path to the core.ll file
        std::string coreLLPath = dirInfo->runtimeDir + "/core.ll";
        logMessage(LMI, "INFO", "Linker", "Core library path: %s", coreLLPath.c_str());

        // Check if the file exists
        if (!std::filesystem::exists(coreLLPath))
        {
            logMessage(LMI, "ERROR", "Linker", "Core library file not found: %s", coreLLPath.c_str());
            CONDITION_FAILED;
            return nullptr;
        }
        // Parse the core.ll file
        llvm::SMDiagnostic err;
        std::unique_ptr<llvm::Module> coreModule = llvm::parseIRFile(coreLLPath, err, this->context);

        if (!coreModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to parse core library file");
            err.print("Linker", llvm::errs());
            CONDITION_FAILED;
            return nullptr;
        }

        // Set the module identifier
        coreModule->setModuleIdentifier("core.ll");

        logMessage(LMI, "INFO", "Linker", "Core library loaded successfully");

        // coreModule->print(llvm::errs(), nullptr);

        // Return the module (it will be cloned in mergeModule)
        return coreModule.release();
    }

} // namespace Cryo
