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
#include "linker/linker.hpp"

namespace Cryo
{

    void LinkerModule::generateIRFromModule(llvm::Module *module)
    {
    }

    void LinkerModule::addNewModule(llvm::Module *mod, std::string name)
    {
    }

    void LinkerModule::addMainModule(llvm::Module *mod)
    {
    }

    void LinkerModule::addPreprocessedModule(llvm::Module *mod)
    {
        std::cout << "Adding Preprocessed Module..." << std::endl;
        logMessage(LMI, "INFO", "Linker", "Adding Preprocessed Module...");

        std::string runtimeDir = getDirInfo()->runtimeDir;
        if (runtimeDir.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Runtime directory is empty");
            return;
        }

        std::error_code EC;
        llvm::raw_fd_ostream dest(runtimeDir, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Error opening file for writing");
            return;
        }

        mod->print(dest, nullptr);
        dest.flush();
        dest.close();

        return;
    }

    // ========================================================================== //

    bool LinkerModule::contextMatch(llvm::Module *mod1, llvm::Module *mod2)
    {
        return &mod1->getContext() == &mod2->getContext();
    }

} // namespace Cryo
