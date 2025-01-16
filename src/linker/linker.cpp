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
#include "tools/logger/logger_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <cstdlib>
#include <iostream>
#include <bits/stdc++.h>

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
            logMessage(LMI, "ERROR", "CryoLinker", "Failed to create linker");
            return nullptr;
        }
    }

    void CryoLinker_Destroy(CryoLinker linker)
    {
        if (linker)
        {
            logMessage(LMI, "INFO", "CryoLinker", "Destroying linker");
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

    void CryoLinker_CompleteCompilationAndLink(CryoLinker linker, const char *buildDir)
    {
        if (linker)
        {
            reinterpret_cast<Cryo::Linker *>(linker)->completeCompilationAndLink(buildDir);
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
        DEBUG_PRINT_FILTER({
            std::cout << "\n";
            std::cout << "==================================================================================" << "\n";
            std::cout << "Linker State:\n"
                      << "  Build Source Directory: " << buildSrcDir << "\n"
                      << "  Dependency Directory: " << dependencyDir << "\n"
                      << "  Dependency Count: " << dependencyCount << "\n";
            std::cout << "==================================================================================" << "\n";
            std::cout << "\n";
        });
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
            logMessage(LMI, "ERROR", "Linker", "Source module is null");
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
            logMessage(LMI, "ERROR", "Linker", "Root module is null");
            CONDITION_FAILED;
        }

        setRootModule(root);
    }

    const std::vector<std::string> Linker::scanDependenciesDir(void)
    {
        std::filesystem::path _dir = dependencyDir;
        if (!std::filesystem::exists(_dir))
        {
            std::cerr << "Dependency directory does not exist: " << dependencyDir << std::endl;
            CONDITION_FAILED;
        }

        std::vector<std::string> dependenciePaths;
        DEBUG_PRINT_FILTER({
            std::cout << "\n";
            std::cout << "\033[1m" << "\033[92m";
            std::cout << "+======================== Dependency Paths ========================+" << "\n";
            std::cout << "\033[0m";
        });

        for (const auto &entry : std::filesystem::directory_iterator(_dir))
        {
            DEBUG_PRINT_FILTER({
                std::cout << "\n";
                std::cout << entry.path() << "\n";
            });
            dependenciePaths.push_back(entry.path());
        }
        DEBUG_PRINT_FILTER({
            std::cout << "\n";
            std::cout << "\033[1m" << "\033[92m";
            std::cout << "+==================================================================+" << "\n";
            std::cout << "\033[0m";
            std::cout << "\n";
        });

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
            logMessage(LMI, "ERROR", "Compilation", "Failed to parse IR file");
            CONDITION_FAILED;
        }

        // Verify the module before returning it
        std::string verifyStr;
        llvm::raw_string_ostream verifyStream(verifyStr);
        if (llvm::verifyModule(*module, &verifyStream))
        {
            logMessage(LMI, "ERROR", "Compilation",
                       "Module verification failed: %s", verifyStream.str().c_str());
            return nullptr;
        }

        return module.release();
    }

    void Linker::appendDependenciesToRoot(llvm::Module *root)
    {
        if (!root)
        {
            logMessage(LMI, "ERROR", "Linker", "Root module is null");
            return;
        }

        if (dependencies.empty())
        {
            logMessage(LMI, "WARN", "Linker", "No dependencies to link");
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
                logMessage(LMI, "WARN", "Linker", "Skipping null dependency module");
                continue;
            }

            // Check if contexts match
            if (!contextMatch(root, depModule))
            {
                logMessage(LMI, "WARN", "Linker",
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
                    logMessage(LMI, "ERROR", "Linker",
                               "Failed to link module: %s", depModule->getName().str().c_str());
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
        logMessage(LMI, "INFO", "Linker", "Merging modules with context mismatch");

        // Create a value mapping for cloning
        llvm::ValueToValueMapTy VMap;

        // Clone the source module
        std::unique_ptr<llvm::Module> newModule = llvm::CloneModule(*src, VMap);
        if (!newModule)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to clone module");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "Module cloned, linking to destination");
        // Link the cloned module
        std::string errorMsg;
        llvm::Linker::Flags linkerFlags = llvm::Linker::Flags::OverrideFromSrc;
        if (llvm::Linker::linkModules(*dest, std::move(newModule), linkerFlags))
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to link cloned module");
            CONDITION_FAILED;
        }

        logMessage(LMI, "INFO", "Linker", "Module linked, hoisting declarations");

        // Hoist declarations
        hoistDeclarations(dest);

        // Store the linked result
        logMessage(LMI, "INFO", "Linker", "Module context mismatch merge complete");

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

    // ========================================================================== #
    // Finished Compilation

    void Linker::completeCompilationAndLink(const char *buildDir)
    {
        std::string buildDirPath = buildDir;
        std::cout << "LINKER: Build Directory: " << buildDirPath << std::endl;

        std::string outDirPath = buildDirPath + "/out";
        std::string depsDirPath = outDirPath + "/deps";

        // Check if directories exist
        std::cout << "Checking directories..." << std::endl;
        checkDirectories(outDirPath, depsDirPath);
        std::cout << "Directories exist, linking modules..." << std::endl;

        // Create the runtime module
        std::cout << "Creating runtime module..." << std::endl;
        createRuntimeModule(buildDirPath);

        // Create a persistent LLVM context
        llvm::LLVMContext context;

        // Combine all valid .ll files
        std::unique_ptr<llvm::Module> combinedModule = combineModules(depsDirPath, context);

        // Optimize the combined IR
        std::cout << "Optimizing combined module..." << std::endl;
        optimizeModule(combinedModule);

        // Generate final object file
        std::cout << "Generating object file..." << std::endl;
        std::string objectFilePath = generateObjectFile(combinedModule, outDirPath);

        // Link the object files and place the output in the build directory
        std::cout << "Linking object files..." << std::endl;
        linkObjectFiles(objectFilePath, buildDirPath);
    }

    void Linker::checkDirectories(const std::string &outDirPath, const std::string &depsDirPath)
    {
        if (!std::filesystem::exists(outDirPath))
        {
            std::cerr << "Error: Output directory does not exist: " << outDirPath << std::endl;
            CONDITION_FAILED;
        }
        if (!std::filesystem::exists(depsDirPath))
        {
            std::cerr << "Error: Dependency directory does not exist: " << depsDirPath << std::endl;
            CONDITION_FAILED;
        }
    }

    std::unique_ptr<llvm::Module> Linker::combineModules(const std::string &depsDirPath, llvm::LLVMContext &context)
    {
        llvm::SMDiagnostic err;
        std::unique_ptr<llvm::Module> combinedModule = std::make_unique<llvm::Module>("combined", context);

        // Move up one directory, the main `.ll` file should be above the `/deps` directory
        std::string parentDir = depsDirPath.substr(0, depsDirPath.find_last_of("/"));
        std::cout << "Parent directory: " << parentDir << std::endl;

        std::string runtimeFile = parentDir + "/combined_runtime.ll";

        for (const auto &entry : std::filesystem::directory_iterator(depsDirPath))
        {
            if (entry.path().extension() == ".ll")
            {
                // Check if it's the `runtime.ll` file and skip it
                if (entry.path().filename() == "runtime.ll")
                {
                    std::cout << "Skipping runtime module." << std::endl;
                    continue;
                }

                std::cout << "Parsing IR file: " << entry.path() << std::endl;
                std::unique_ptr<llvm::Module> module = llvm::parseIRFile(entry.path().string(), err, context);
                if (!module)
                {
                    std::cerr << "Failed to parse IR file: " << entry.path() << std::endl;
                    CONDITION_FAILED;
                }

                if (llvm::Linker::linkModules(*combinedModule, std::move(module)))
                {
                    std::cerr << "Failed to link module: " << entry.path() << std::endl;
                    CONDITION_FAILED;
                }

                std::cout << "Module linked successfully." << std::endl;
            }
        }

        // Link the runtime module
        std::cout << "Parsing runtime module: " << runtimeFile << std::endl;
        std::unique_ptr<llvm::Module> runtimeModule = llvm::parseIRFile(runtimeFile, err, context);
        if (!runtimeModule)
        {
            std::cerr << "Failed to parse runtime module: " << runtimeFile << std::endl;
            CONDITION_FAILED;
        }

        if (llvm::Linker::linkModules(*combinedModule, std::move(runtimeModule)))
        {
            std::cerr << "Failed to link runtime module: " << runtimeFile << std::endl;
            CONDITION_FAILED;
        }

        // Get the main `.ll` file
        std::string mainFilePath = parentDir + "/main.ll";
        std::cout << "Parsing main IR file: " << mainFilePath << std::endl;

        std::unique_ptr<llvm::Module> mainModule = llvm::parseIRFile(mainFilePath, err, context);
        if (!mainModule)
        {
            std::cerr << "Failed to parse main IR file: " << mainFilePath << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Main module parsed successfully." << std::endl;

        if (llvm::Linker::linkModules(*combinedModule, std::move(mainModule)))
        {
            std::cerr << "Failed to link main module: " << mainFilePath << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Modules combined successfully." << std::endl;

        return combinedModule;
    }

    void Linker::optimizeModule(std::unique_ptr<llvm::Module> &module)
    {
        if (!module)
        {
            logMessage(LMI, "ERROR", "Linker", "Module is null");
            CONDITION_FAILED;
        }

        llvm::LLVMContext &context = module->getContext();
        std::cout << "LLVMContext address: " << &context << std::endl;

        if (!context.getDiagHandlerPtr())
        {
            logMessage(LMI, "ERROR", "Linker", "LLVM context diagnostic handler is null");
            CONDITION_FAILED;
        }

        std::cout << "LLVM context diagnostic handler is valid." << std::endl;

        llvm::legacy::PassManager passManager;
        passManager.add(llvm::createPromoteMemoryToRegisterPass());
        std::cout << "Promoting memory to register..." << std::endl;
        passManager.add(llvm::createInstructionCombiningPass());
        std::cout << "Combining instructions..." << std::endl;
        passManager.add(llvm::createReassociatePass());
        std::cout << "Reassociating instructions..." << std::endl;
        passManager.add(llvm::createGVNPass());
        std::cout << "Performing GVN..." << std::endl;
        passManager.add(llvm::createCFGSimplificationPass());
        std::cout << "Simplifying CFG..." << std::endl;

        std::cout << "Running pass manager..." << std::endl;
        passManager.run(*module);
        std::cout << "Module optimized successfully." << std::endl;
    }

    std::string Linker::generateObjectFile(std::unique_ptr<llvm::Module> &module, const std::string &outDirPath)
    {
        std::string objectFilePath = outDirPath + "/combined.o";
        std::error_code EC;
        llvm::raw_fd_ostream dest(objectFilePath, EC, llvm::sys::fs::OF_None);
        if (EC)
        {
            std::cerr << "Could not open file: " << EC.message() << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Destination file: " << objectFilePath << std::endl;

        std::cout << "Initializing targets..." << std::endl;
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllAsmParsers();
        std::cout << "Targets initialized." << std::endl;

        std::string targetTriple = std::string(LLVMGetDefaultTargetTriple());
        std::cout << "Target triple: " << targetTriple << std::endl;
        module->setTargetTriple(targetTriple);
        std::cout << "Module target triple set." << std::endl;

        std::string error;
        auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
        if (!target)
        {
            std::cerr << "Could not lookup target: " << error << std::endl;
            CONDITION_FAILED;
        }

        auto CPU = "generic";
        auto features = "";

        std::cout << "Creating target machine..." << std::endl;
        llvm::TargetOptions opt;
        auto targetMachine = target->createTargetMachine(
            targetTriple,
            CPU,
            features,
            opt,
            llvm::Reloc::PIC_);
        std::cout << "Target machine created." << std::endl;
        module->setDataLayout(targetMachine->createDataLayout());
        std::cout << "Module data layout set." << std::endl;

        llvm::legacy::PassManager pass;
        if (targetMachine->addPassesToEmitFile(
                pass,
                dest,
                nullptr,
                llvm::CodeGenFileType::ObjectFile))
        {
            std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Running pass manager..." << std::endl;
        pass.run(*module);
        std::cout << "Pass manager run." << std::endl;
        dest.flush();
        std::cout << "Object file flushed." << std::endl;

        if (!std::filesystem::exists(objectFilePath))
        {
            std::cerr << "Failed to create combined object file. Please check the LLVM IR files and the compilation process." << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Combined object file created successfully." << std::endl;

        return objectFilePath;
    }

    void Linker::linkObjectFiles(const std::string &objectFilePath, const std::string &buildDirPath)
    {
        std::string finalExecutablePath = buildDirPath + "/" + "finalExecutable";
        std::cout << "Linking object files..." << "\n"
                  << "Object file: " << objectFilePath << "\n"
                  << "Final executable: " << finalExecutablePath << std::endl;
        std::string command = "clang++-18 -fno-pie -no-pie " + objectFilePath + " -o " + finalExecutablePath;
        std::cout << "Running command: " << command << std::endl;
        if (system(command.c_str()) != 0)
        {
            std::cerr << "Failed to link the object files." << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Final executable created successfully at: " << finalExecutablePath << std::endl;
    }

    void Linker::generateObjectFileFromLLVMIRFile(const std::string pathToIR, const std::string outPath)
    {
    }

    void Linker::createRuntimeModule(const std::string buildDir)
    {
        std::string cryoRootDir = std::getenv("CRYO_ROOT");
        if (cryoRootDir.empty())
        {
            std::cerr << "Error: CRYO_ROOT environment variable not set." << std::endl;
            CONDITION_FAILED;
        }

        std::string cxxRuntimePath = cryoRootDir + "/Std/Runtime/cxx_support.cpp";
        std::string runtimePath = buildDir + "/out/deps/runtime.ll";
        std::string runtimeOutPath = buildDir + "/out";

        // Build the cxx file and output it to the build directory
        std::cout << "Compiling cxx support file..." << std::endl;
        std::string cxxCommand = "clang++-18 -S -emit-llvm " + cxxRuntimePath + " -o " + runtimeOutPath + "/cxx_support.ll";
        std::cout << "Running Command: \n"
                  << cxxCommand << std::endl;
        if (system(cxxCommand.c_str()) != 0)
        {
            std::cerr << "Failed to compile cxx support file." << std::endl;
            CONDITION_FAILED;
        }

        // Combine the two LL files using llvm-link-18
        std::cout << "Combining runtime files..." << std::endl;
        std::string linkCommand = "llvm-link-18 " + runtimePath + " " + runtimeOutPath + "/cxx_support.ll -S -o " + runtimeOutPath + "/combined_runtime.ll";
        std::cout << "Running Command:\n"
                  << linkCommand << std::endl;
        if (system(linkCommand.c_str()) != 0)
        {
            std::cerr << "Failed to combine runtime files." << std::endl;
            CONDITION_FAILED;
        }

        std::cout << "Runtime module created successfully." << std::endl;
    }

} // namespace Cryo
