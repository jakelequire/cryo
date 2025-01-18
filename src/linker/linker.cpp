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
    void Linker::setBuildDir(const char *buildDir)
    {
        std::cout << "Setting build directory for Linker..." << std::endl;
        std::cout << "Path passed to Linker: " << buildDir << std::endl;

        std::string cxxbuildDir = std::string(buildDir);

        std::string rootDir = cxxbuildDir.substr(0, cxxbuildDir.find_last_of("/"));
        std::cout << "Setting build directory for Linker. Root Directory: " << rootDir << std::endl;

        DirectoryInfo *dirInfo = new DirectoryInfo();
        dirInfo->rootDir = rootDir;
        dirInfo->buildDir = cxxbuildDir;
        dirInfo->outDir = cxxbuildDir + "/out";
        dirInfo->depDir = cxxbuildDir + "/out/deps";
        dirInfo->runtimeDir = cxxbuildDir + "/out/runtime";

        this->dirInfo = dirInfo;
    }

    // ==================================================================

    void Linker::addPreprocessingModule(llvm::Module *mod)
    {
        logMessage(LMI, "INFO", "Linker", "Adding Preprocessing Module...");

        if (!mod)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            return;
        }

        std::cout << "Adding Preprocessing Module..." << std::endl;

        linkerModule->addPreprocessedModule(mod);

        return;
    }

    void Linker::newInitDependencies(llvm::Module *srcModule)
    {
    }

    void Linker::appendDependenciesToRoot(llvm::Module *root)
    {
        if (!root)
        {
            logMessage(LMI, "ERROR", "Linker", "Root module is null");
            return;
        }

        std::string rootName = root->getName().str();
        if (rootName.empty())
        {
            logMessage(LMI, "ERROR", "Linker", "Root module name is empty");
            return;
        }

        linkerModule->addNewModule(root, rootName);

        return;
    }

} // namespace Cryo
