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
            logMessage("ERROR", __LINE__, "CryoLinker", "Failed to create linker");
            return nullptr;
        }
    }

    void CryoLinker_Destroy(CryoLinker linker)
    {
        if (linker)
        {
            logMessage("INFO", __LINE__, "CryoLinker", "Destroying linker");
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

    void Linker::newInitDependencies(llvm::Module *srcModule)
    {
        if (!srcModule)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Linker", "Source module is null");
            CONDITION_FAILED;
        }

        // Set the linkers module and context from the source module
        setModuleAndContextFromRoot(srcModule);

        // Scan the dependency directory
        const std::vector<std::string> deps = scanDependenciesDir();
        int count = deps.size();

        setDependencyCount(count);

        // Create the modules from the dependencies
        createModulesFromDependencies(deps);

        // Append the dependencies to the source module
        appendDependenciesToRoot(srcModule);

        return;
    }

    void Linker::setModuleAndContextFromRoot(llvm::Module *root)
    {
        if (!root)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Linker", "Root module is null");
            CONDITION_FAILED;
        }

        setRootModule(root);
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

        std::unique_ptr<llvm::Module> module = llvm::parseIRFile(inputFile, err, rootModule->getContext());
        if (!module)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation", "Failed to parse IR file");
            CONDITION_FAILED;
        }

        // Verify the module before returning it
        std::string verifyStr;
        llvm::raw_string_ostream verifyStream(verifyStr);
        if (llvm::verifyModule(*module, &verifyStream))
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Compilation",
                                    "Module verification failed: " + verifyStream.str());
            return nullptr;
        }

        return module.release();
    }

    void Linker::appendDependenciesToRoot(llvm::Module *root)
    {
        if (!root)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Linker", "Root module is null");
            return;
        }

        if (dependencies.empty())
        {
            DevDebugger::logMessage("WARNING", __LINE__, "Linker", "No dependencies to link");
            rootModule = root;
            return;
        }

        // Get the context from the root module
        llvm::LLVMContext &rootContext = root->getContext();

        // Linker flags
        llvm::Linker::Flags linkerFlags = llvm::Linker::Flags::OverrideFromSrc;

        for (auto *depModule : dependencies)
        {
            if (!depModule)
            {
                DevDebugger::logMessage("WARNING", __LINE__, "Linker", "Skipping null dependency module");
                continue;
            }

            // Check if contexts match
            if (!contextMatch(root, depModule))
            {
                DevDebugger::logMessage("WARNING", __LINE__, "Linker",
                                        "Module context mismatch - attempting to clone module");
                contextMismatchMerge(root, depModule);

                // Continue to next module
                continue;
            }
            else
            {
                // Contexts match, link directly
                std::unique_ptr<llvm::Module> depModulePtr(depModule);
                if (llvm::Linker::linkModules(*root, std::move(depModulePtr), linkerFlags))
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Linker",
                                            "Failed to link module: " + depModule->getName().str());
                }
            }
        }

        // After all linking is done, hoist declarations
        hoistDeclarations(root);

        // Store the linked result
        rootModule = root;
    }

    // This function is executed when the context of the two modules do not match.
    // We will attempt to make the destination module the base context,
    // and merge the source modules into it and make the context match.
    void Linker::contextMismatchMerge(llvm::Module *dest, llvm::Module *src)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Linker", "Merging modules with context mismatch");

        // Create a value mapping for cloning
        llvm::ValueToValueMapTy VMap;

        // Clone the source module
        std::unique_ptr<llvm::Module> newModule = llvm::CloneModule(*src, VMap);
        if (!newModule)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Linker", "Failed to clone module");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Linker", "Module cloned, linking to destination");
        // Link the cloned module
        std::string errorMsg;
        llvm::Linker::Flags linkerFlags = llvm::Linker::Flags::OverrideFromSrc;
        if (llvm::Linker::linkModules(*dest, std::move(newModule), linkerFlags))
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Linker", "Failed to link cloned module");
            CONDITION_FAILED;
        }

        DevDebugger::logMessage("INFO", __LINE__, "Linker", "Module linked, hoisting declarations");

        // Hoist declarations
        hoistDeclarations(dest);

        // Store the linked result
        DevDebugger::logMessage("INFO", __LINE__, "Linker", "Module context mismatch merge complete");

        return;
    }

    void Linker::hoistDeclarations(llvm::Module *module)
    {
        if (!module)
            return;

        // Handle functions only
        std::vector<llvm::Function *> declarations;
        std::vector<llvm::Function *> definitions;

        // Separate function declarations and definitions
        for (auto &F : *module)
        {
            if (F.isDeclaration())
            {
                declarations.push_back(&F);
            }
            else
            {
                definitions.push_back(&F);
            }
        }

        // Sort declarations by name for consistency
        std::sort(declarations.begin(), declarations.end(),
                  [](const llvm::Function *a, const llvm::Function *b)
                  {
                      return a->getName() < b->getName();
                  });

        // Remove all functions from module
        for (auto *F : declarations)
            F->removeFromParent();
        for (auto *F : definitions)
            F->removeFromParent();

        // Add them back in proper order:
        // 1. Function declarations
        // 2. Function definitions
        for (auto *F : declarations)
        {
            module->getFunctionList().push_back(F);
        }
        for (auto *F : definitions)
        {
            module->getFunctionList().push_back(F);
        }
    }

    bool Linker::contextMatch(llvm::Module *mod1, llvm::Module *mod2)
    {
        return &mod1->getContext() == &mod2->getContext();
    }

} // namespace Cryo
