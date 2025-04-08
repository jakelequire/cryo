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
#include "linker/init.h"
#include "diagnostics/diagnostics.h"

/// This files purpose is to initialize the Cryo Core and the Cryo Runtime.
/// In the root directory of the Cryo Compiler, there is a `cryolib` directory.
/// Inside this directory is `{COMPILER_DIR}/cryo/Std/Core/core.cryo` and
/// `{COMPILER_DIR}/cryo/runtime/runtime.c`.
///
/// The `core.cryo` file is the baseline for the Cryo Standard Library, equivalent
/// to C's CRT0. The `runtime.c` file is to be compiled along with the Cryo Core.
/// ```
///     A[Phase 1: C/C++ Implementation] --> B[Core Runtime]
///     A --> C[Basic Standard Library]
///     B --> D[Phase 2: Cryo Implementation]
///     C --> D
///     D --> E[Full Standard Library in Cryo]
/// ```

namespace Cryo
{
    void Linker::initCryoCore(const char *compilerRootPath, const char *buildDir,
                              CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        __STACK_FRAME__
        // Create the llvm::Module for the `runtime.c` file.
        _initCRuntime_();

        if (!this->runtimeModuleInitialized)
        {
            fprintf(stderr, "[Linker] Error: Failed to initialize the runtime module\n");
            CONDITION_FAILED;
        }

        // Process the core.cryo
        const char *corePath = fs->appendStrings(compilerRootPath, "/cryo/Std/Core/core.cryo");
        if (!corePath)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
            CONDITION_FAILED;
        }

        ASTNode *defsNode = compileForASTNode(strdup(corePath), state, globalTable);
        if (!defsNode)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
            CONDITION_FAILED;
        }

        // DTM->symbolTable->importASTnode(DTM->symbolTable, defsNode);
        DTM->symbolTable->printTable(DTM->symbolTable);

        logMessage(LMI, "INFO", "DTM", "Definitions Path: %s", corePath);
        CompilationUnitDir dir = createCompilationUnitDir(corePath, buildDir, CRYO_RUNTIME);
        dir.print(dir);

        CompilationUnit *unit = createNewCompilationUnit(defsNode, dir);
        if (!unit)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
            CONDITION_FAILED;
            return;
        }
        if (unit->verify(unit) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
            CONDITION_FAILED;
            return;
        }

        // Create the IR from the ASTNode
        CryoLinker *linker = reinterpret_cast<CryoLinker *>(this);
        if (UNFINISHED_generateIRFromAST(unit, state, linker, globalTable) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
            CONDITION_FAILED;
        }

        if (completeCryoCryoLib(compilerRootPath) != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to complete Cryo CryoLib\n");
            CONDITION_FAILED;
        }

        if (buildStandardLib(state, globalTable) != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to build standard library\n");
            CONDITION_FAILED;
        }

        DEBUG_BREAKPOINT;
    }

    void Linker::_initCRuntime_(void)
    {
        logMessage(LMI, "INFO", "Linker", "Initializing C runtime module...");
        std::string compilerRootPath = this->dirInfo->compilerDir;
        const char *runtimePath = fs->appendStrings(compilerRootPath.c_str(), "/cryo/runtime/runtime.c");
        if (!runtimePath)
        {
            fprintf(stderr, "[Linker] Error: Failed to allocate memory for runtime path\n");
            CONDITION_FAILED;
        }

        // Check to see if the file exists
        if (!fs->fileExists(runtimePath))
        {
            fprintf(stderr, "[Linker] Error: Runtime file does not exist: %s\n", runtimePath);
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", runtimePath);

        // We have verified that the file exists, now we need to convert it to LLVM IR

        // Create an LLVM context and module for the runtime
        llvm::LLVMContext *runtimeContext = new llvm::LLVMContext();
        this->runtimeModule = new llvm::Module("cryo_runtime", *runtimeContext);

        // Set up Clang to compile the C file to LLVM IR
        std::vector<const char *> args;

        // Basic compilation arguments
        args.push_back("clang-18");
        args.push_back("-c");
        args.push_back("-emit-llvm");
        args.push_back("-O2");       // Optimization level
        args.push_back("-fPIC");     // Position-Independent Code for shared library
        args.push_back("-x");        // Specify input language
        args.push_back("c");         // C language
        args.push_back(runtimePath); // Input file

        // Standard includes
        args.push_back("-I");
        args.push_back(fs->appendStrings(compilerRootPath.c_str(), "/include"));

        // Create temporary output file for IR
        std::string userBuildDir = std::string(this->dirInfo->runtimeDir) + "/c_runtime.ll";
        args.push_back("-o");
        args.push_back(userBuildDir.c_str());

        // Execute the clang command
        std::string command = "clang-" + CLANG_VERSION_MAJOR;
        for (const char *arg : args)
        {
            command += " " + std::string(arg);
        }
        int result = system(command.c_str());
        if (result != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile runtime file to LLVM IR\n");
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Successfully compiled runtime file to LLVM IR");

        // Mark runtime as initialized
        this->runtimeModuleInitialized = true;

        logMessage(LMI, "INFO", "Linker", "Successfully initialized C runtime module");

        return;
    }

    // This function is called when `c_runtime.ll` & `core.ll` file are generated in `dirInfo->runtimeDir`
    // It will merge the runtime module into the core module
    int Linker::completeCryoCryoLib(const char *compilerRootPath)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Merging runtime module into core module...");

        std::string c_runtimePath = std::string(this->dirInfo->runtimeDir) + "/c_runtime.ll";
        std::string corePath = std::string(this->dirInfo->runtimeDir) + "/core.ll";
        if (!fs->fileExists(c_runtimePath.c_str()))
        {
            fprintf(stderr, "[Linker] Error: Runtime file does not exist: %s\n", c_runtimePath.c_str());
            CONDITION_FAILED;
        }
        if (!fs->fileExists(corePath.c_str()))
        {
            fprintf(stderr, "[Linker] Error: Core file does not exist: %s\n", corePath.c_str());
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", c_runtimePath.c_str());
        logMessage(LMI, "INFO", "Linker", "Core Path: %s", corePath.c_str());

        // Use llc to compile the IR to object code
        std::string outputPath = std::string(this->dirInfo->runtimeDir) + "/core.o";
        // With this:
        std::string tempPath = std::string(this->dirInfo->runtimeDir) + "/merged.ll";
        std::string linkCommand = "llvm-link-18 -S -o " + tempPath + " " + corePath + " " + c_runtimePath;
        logMessage(LMI, "INFO", "Linker", "Linking LLVM IR files: %s", linkCommand.c_str());
        int linkResult = system(linkCommand.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to link LLVM IR files\n");
            CONDITION_FAILED;
        }

        std::string compileCommand = "llc-18 -filetype=obj -relocation-model=pic -o " + outputPath + " " + tempPath;
        int compileResult = system(compileCommand.c_str());
        if (compileResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile linked IR to object code\n");
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Successfully compiled core file to object code");

        // Now that we have the object code, we can create a shared library for the compiler standard library
        std::string sharedLibPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/libcryo_core.so";
        if (fs->fileExists(sharedLibPath.c_str()))
        {
            fs->removeFile(sharedLibPath.c_str());
        }
        else
        {
            fs->createNewEmptyFileWpath(sharedLibPath.c_str());
        }
        std::string sharedLibCommand = "clang++ -shared -fPIC -o " + sharedLibPath + " " + outputPath;
        int sharedLibResult = system(sharedLibCommand.c_str());
        if (sharedLibResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to create shared library\n");
            CONDITION_FAILED;
        }
        logMessage(LMI, "INFO", "Linker", "Successfully created shared library: %s", sharedLibPath.c_str());

        // Mark the shared library as initialized
        this->shared_lib_initialized = true;
        logMessage(LMI, "INFO", "Linker", "Successfully initialized shared library");
        return 0;
    }

    int Linker::buildStandardLib(CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // This function will compile the whole `/cryo/Std` directory and create individual shared libraries
        // for each module.
        // Each of these modules are independent and can be used in any Cryo program.
        // All modules will be compiled ontop of the `libcryo_core.so` shared library.

        std::string stdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std";
        std::string stdBinDir = stdDir + "/bin";

        // Create bin directory if it doesn't exist
        if (!std::filesystem::exists(stdBinDir))
        {
            std::filesystem::create_directory(stdBinDir);
        }

        // Vector to store all .cryo files
        std::vector<std::string> cryoFiles;

        // Recursively iterate through the standard library directory
        for (const auto &entry : std::filesystem::recursive_directory_iterator(stdDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".cryo")
            {
                // Skip files in the Core directory
                if (entry.path().string().find("/Std/Core/") == std::string::npos)
                {
                    cryoFiles.push_back(entry.path().string());
                }
            }
        }

        // Print out the files
        std::cout << "\n=============== {Files } ===============\n";
        for (const std::string &file : cryoFiles)
        {
            std::cout << "File: " << file << std::endl;
        }
        std::cout << "========================================\n\n";

        // Iterate through the files and compile them with the core library

        for (const std::string &file : cryoFiles)
        {
            // Get the file name without the extension
            std::string fileName = file.substr(file.find_last_of("/\\") + 1);
            std::string fileNameWithoutExt = fileName.substr(0, fileName.find_last_of('.'));

            // Create the output path for the shared library
            std::string outputPath = stdBinDir + "/lib" + fileNameWithoutExt + ".so";

            // Compile the file and create the shared library
            int result = compileLibItem(file, state, globalTable);
            if (result != 0)
            {
                fprintf(stderr, "[Linker] Error: Failed to compile %s\n", file.c_str());
                return result;
            }
        }

        // Create a `verified` file to indicate that the standard library has been built
        // This is just a temporary implementation below, in the future, this should be a more
        // robust solution that verifies the standard library and creates a checksum for it.
        // This will be used to verify that the standard library has been built and is up to date.
        std::string verifiedFilePath = stdBinDir + "/verified";
        if (fs->fileExists(verifiedFilePath.c_str()))
        {
            fs->removeFile(verifiedFilePath.c_str());
        }
        fs->createNewEmptyFileWpath(verifiedFilePath.c_str());
        logMessage(LMI, "INFO", "Linker", "Successfully built standard library: %s", verifiedFilePath.c_str());

        return 0;
    }

    int Linker::compileLibItem(std::string filePath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // This function will compile a single file and create a shared library for it.
        // It will also link the shared library with the core library.

        std::string compilerRootPath = this->dirInfo->compilerDir;
        std::cout << "Compiler Root Path @compileLibItem: " << compilerRootPath << std::endl;
        std::string buildDir = compilerRootPath + "/cryo/Std/bin/.ll/";

        ASTNode *programNode = compileForASTNode(filePath.c_str(), state, globalTable);
        if (programNode == NULL)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile file: %s\n", filePath.c_str());
            return 1;
        }

        programNode->print(programNode);

        CompilationUnitDir dir = createCompilerCompilationUnitDir(filePath.c_str(), buildDir.c_str(), compilerRootPath.c_str(), CRYO_STDLIB);
        dir.print(dir);

        CompilationUnit *unit = createNewCompilationUnit(programNode, dir);
        if (!unit)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
            CONDITION_FAILED;
            return 1;
        }

        if (unit->verify(unit) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
            CONDITION_FAILED;
            return 1;
        }

        // Create the IR from the ASTNode
        CryoLinker *linker = reinterpret_cast<CryoLinker *>(this);
        if (UNFINISHED_generateIRFromAST(unit, state, linker, globalTable) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
            CONDITION_FAILED;
            return 1;
        }

        // Link the newly compiled module with the core library
        std::string coreLibPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/libcryo_core.so";
        std::string outputPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/" + dir.out_fileName + ".so";
        std::string linkCommand = "clang++ -shared -fPIC -o " + outputPath + " " + coreLibPath + " " + dir.out_filePath + ".ll";
        int linkResult = system(linkCommand.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to link module with core library\n");
            CONDITION_FAILED;
            return 1;
        }
        logMessage(LMI, "INFO", "Linker", "Successfully linked module with core library");
        logMessage(LMI, "INFO", "Linker", "Successfully created shared library: %s", outputPath.c_str());

        return 0;
    }

} // namespace Cryo
