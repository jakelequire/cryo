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

extern "C"
{

    CryoLinker CryoLinker_Create(void)
    {
        try
        {
            auto linker = new Cryo::Linker();
            return reinterpret_cast<CryoLinker>(linker);
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void CryoLinker_Destroy(CryoLinker linker)
    {
        if (linker)
        {
            delete reinterpret_cast<Cryo::Linker *>(linker);
        }
    }

    void CryoLinker_SetBuildSrcDirectory(CryoLinker linker, const char *srcDir)
    {
        if (linker && srcDir)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->setBuildSrcDirectory(srcDir);
            // Set the dependency directory
            reinterpret_cast<Cryo::Linker *>(linker)->setDependencyDir(srcDir);
        }
    }

    void CryoLinker_AddRootModule(CryoLinker linker, void *module)
    {
        if (linker && module)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->addRootModule(
                static_cast<llvm::Module *>(module));
        }
    }

    void CryoLinker_AddModuleDependency(CryoLinker linker, void *module)
    {
        if (linker && module)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->addModuleDependency(
                static_cast<llvm::Module *>(module));
        }
    }

    void *CryoLinker_LinkModules(CryoLinker linker)
    {
        if (!linker)
            return nullptr;
        return reinterpret_cast<void *>(
            reinterpret_cast<Cryo::Linker *>(linker)->linkModules());
    }

    void CryoLinker_LogState(CryoLinker linker)
    {
        if (linker)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->logState();
        }
    }

    void CryoLinker_InitDependencies(CryoLinker linker)
    {
        if (linker)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->initDependencies();
        }
    }
}

namespace Cryo
{

    Linker::Linker()
    {
        rootModule = nullptr;
        dependencyCount = 0;
        buildSrcDir = "";
        dependencyDir = "out/deps/";
    }

    Linker::~Linker()
    {
        // Cleanup if needed
    }

    void Linker::addRootModule(llvm::Module *mod)
    {
    }

    void Linker::addModuleDependency(llvm::Module *mod)
    {
        if (mod)
        {
            dependencies.push_back(mod);
        }
    }

    llvm::Module *Linker::linkModules()
    {
    }

    void Linker::setDependencyDir(std::string dir)
    {
        std::string fullPath = buildSrcDir + "/" + dependencyDir;
        dependencyDir = fullPath;
    }

    void Linker::logState() const
    {
        std::cout << "\n";
        std::cout << "==================================================================================" << "\n";
        std::cout << "Linker State:\n"
                  << "  Build Source Directory: " << buildSrcDir << "\n"
                  << "  Dependency Directory: " << dependencyDir << "\n"
                  << "  Dependency Count: " << dependencyCount << "\n";
        std::cout << "==================================================================================" << "\n";
        std::cout << "\n";
    }

    // # ========================================================================== #
    // # C++ Implementation

    void Linker::initDependencies(void)
    {
        // Scan the dependency directory
        const std::vector<std::string> deps = scanDependenciesDir();
        int count = deps.size();

        setDependencyCount(count);

        // Create the modules from the dependencies
        createModulesFromDependencies(deps);
    }

    const std::vector<std::string> Linker::scanDependenciesDir(void)
    {
        std::filesystem::path dir(dependencyDir);
        if (!std::filesystem::exists(dir))
        {
            std::cout << "Dependency directory does not exist" << std::endl;
            CONDITION_FAILED;
        }

        std::vector<std::string> dependenciePaths;

        std::cout << "\n";
        std::cout << "\033[1m" << "\033[92m";
        std::cout << "+======================== Dependency Paths ========================+" << "\n";
        std::cout << "\033[0m";
        for (const auto &entry : std::filesystem::directory_iterator(dir))
        {
            std::cout << "\n";
            std::cout << entry.path() << "\n";
            dependenciePaths.push_back(entry.path());
        }
        std::cout << "\n";
        std::cout << "\033[1m" << "\033[92m";
        std::cout << "+==================================================================+" << "\n";
        std::cout << "\033[0m";
        std::cout << "\n";

        return dependenciePaths;
    }

    void Linker::createModulesFromDependencies(const std::vector<std::string> &deps)
    {
        // We have to take the LLVM IR file paths within the vector, create a new module for each one, and add it to the dependencies vector
        for (const auto &dep : deps)
        {
            llvm::Module *module = compileAndMergeModule(dep);
            if (module)
            {
                addModuleDependency(module);
            }
        }
    }

    llvm::Module *Linker::compileAndMergeModule(std::string inputFile)
    {
        llvm::SMDiagnostic err;

        std::unique_ptr<llvm::Module> module = llvm::parseIRFile(inputFile, err, context);
        if (!module)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Failed to parse IR file");
            CONDITION_FAILED;
        }

        return module.release();
    }

    void Linker::appendDependenciesToRoot(llvm::Module *root)
    {
        if (root)
        {
        }
    }

} // namespace Cryo
