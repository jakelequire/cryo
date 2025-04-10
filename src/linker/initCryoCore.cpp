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

/**
 * @file initCryoCore.cpp
 * @brief Implementation of Cryo Core and Runtime initialization
 *
 * This file initializes the Cryo Core and Runtime environments.
 * The Cryo Standard Library is built in multiple phases:
 * 1. C/C++ Implementation creates Core Runtime and Basic Standard Library
 * 2. These are used to build a Cryo Implementation
 * 3. The Cryo Implementation enables a Full Standard Library in Cryo
 */

#include "linker/linker.hpp"
#include "linker/init.h"
#include "diagnostics/diagnostics.h"

namespace Cryo
{

    /**
     * @brief Initializes the core Cryo environment
     *
     * Creates and initializes the runtime module, compiles core.cryo, and
     * builds the standard library components.
     *
     * @param compilerRootPath Path to the compiler root directory
     * @param buildDir Directory for build outputs
     * @param state Current compiler state
     * @param globalTable Global symbol table
     */
    void Linker::initCryoCore(const char *compilerRootPath, const char *buildDir,
                              CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        __STACK_FRAME__

        // Initialize C runtime
        initializeRuntimeModule();
        if (!this->runtimeModuleInitialized)
        {
            fprintf(stderr, "[Linker] Error: Failed to initialize the runtime module\n");
            CONDITION_FAILED;
            return;
        }

        // Process core.cryo file
        if (!processCoreFile(compilerRootPath, buildDir, state, globalTable))
        {
            return;
        }

        // Complete core library creation and build standard library
        if (linkCryoCoreLibrary(compilerRootPath) != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to complete Cryo CryoLib\n");
            CONDITION_FAILED;
            return;
        }

        if (buildStandardLib(state, globalTable) != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to build standard library\n");
            CONDITION_FAILED;
            return;
        }

        return;
    }

    /**
     * @brief Processes the core.cryo file and generates IR
     *
     * @param compilerRootPath Path to the compiler root directory
     * @param buildDir Directory for build outputs
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return true if successful, false otherwise
     */
    bool Linker::processCoreFile(const char *compilerRootPath, const char *buildDir,
                                 CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // Create path to core.cryo
        const char *corePath = fs->appendStrings(compilerRootPath, "/cryo/Std/Core/core.cryo");
        if (!corePath)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
            CONDITION_FAILED;
            return false;
        }

        // Compile the core file to AST
        ASTNode *defsNode = compileForASTNode(strdup(corePath), state, globalTable);
        if (!defsNode)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
            CONDITION_FAILED;
            return false;
        }

        // Process symbol table
        DTM->symbolTable->printTable(DTM->symbolTable);
        logMessage(LMI, "INFO", "DTM", "Definitions Path: %s", corePath);

        // Create compilation unit and generate IR
        return generateCoreIR(defsNode, corePath, buildDir, state, globalTable);
    }

    /**
     * @brief Creates compilation unit and generates IR for core file
     *
     * @param defsNode AST node for core definitions
     * @param corePath Path to core.cryo file
     * @param buildDir Build directory
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return true if successful, false otherwise
     */
    bool Linker::generateCoreIR(ASTNode *defsNode, const char *corePath, const char *buildDir,
                                CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // Create compilation unit directory
        CompilationUnitDir dir = createCompilationUnitDir(corePath, buildDir, CRYO_RUNTIME);
        dir.print(dir);

        // Create compilation unit
        CompilationUnit *unit = createNewCompilationUnit(defsNode, dir);
        if (!unit)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
            CONDITION_FAILED;
            return false;
        }

        // Verify compilation unit
        if (unit->verify(unit) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
            CONDITION_FAILED;
            return false;
        }

        // Generate IR from AST
        CryoLinker *linker = reinterpret_cast<CryoLinker *>(this);
        if (generateIRFromAST(unit, state, linker, globalTable) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
            CONDITION_FAILED;
            return false;
        }

        return true;
    }

    /**
     * @brief Initializes the C runtime module
     *
     * Compiles the runtime.c file to LLVM IR and initializes the runtime module.
     */
    void Linker::initializeRuntimeModule()
    {
        logMessage(LMI, "INFO", "Linker", "Initializing C runtime module...");

        // Create path to runtime.c
        std::string compilerRootPath = this->dirInfo->compilerDir;
        const char *runtimePath = fs->appendStrings(compilerRootPath.c_str(), "/cryo/runtime/runtime.c");

        // Validate runtime path
        if (!runtimePath)
        {
            fprintf(stderr, "[Linker] Error: Failed to allocate memory for runtime path\n");
            CONDITION_FAILED;
            return;
        }

        // Check if file exists
        if (!fs->fileExists(runtimePath))
        {
            fprintf(stderr, "[Linker] Error: Runtime file does not exist: %s\n", runtimePath);
            CONDITION_FAILED;
            return;
        }
        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", runtimePath);

        // Create LLVM context and module
        llvm::LLVMContext *runtimeContext = new llvm::LLVMContext();
        this->runtimeModule = new llvm::Module("cryo_runtime", *runtimeContext);

        // Compile runtime.c to LLVM IR
        if (compileRuntimeToIR(compilerRootPath, runtimePath))
        {
            this->runtimeModuleInitialized = true;
            logMessage(LMI, "INFO", "Linker", "Successfully initialized C runtime module");
        }
    }

    /**
     * @brief Compiles runtime.c to LLVM IR
     *
     * @param compilerRootPath Root path of the compiler
     * @param runtimePath Path to runtime.c
     * @return true if successful, false otherwise
     */
    bool Linker::compileRuntimeToIR(const std::string &compilerRootPath, const char *runtimePath)
    {
        // Set up Clang compilation arguments
        std::vector<const char *> args;
        args.push_back("clang-18");
        args.push_back("-c");
        args.push_back("-emit-llvm");
        args.push_back("-O2");       // Optimization level
        args.push_back("-fPIC");     // Position-Independent Code
        args.push_back("-x");        // Specify input language
        args.push_back("c");         // C language
        args.push_back(runtimePath); // Input file

        // Add standard includes
        args.push_back("-I");
        args.push_back(fs->appendStrings(compilerRootPath.c_str(), "/include"));

        // Set output file
        std::string userBuildDir = std::string(this->dirInfo->runtimeDir) + "/c_runtime.ll";
        args.push_back("-o");
        args.push_back(userBuildDir.c_str());

        // Build and execute command
        std::string command = "clang-" + CLANG_VERSION_MAJOR;
        for (const char *arg : args)
        {
            command += " " + std::string(arg);
        }

        int result = system(command.c_str());
        if (result != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile runtime file to LLVM IR\n");
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully compiled runtime file to LLVM IR");
        return true;
    }

    /**
     * @brief Links the C runtime and core modules
     *
     * Merges the runtime module into the core module and creates a shared library.
     *
     * @param compilerRootPath Path to the compiler root directory
     * @return 0 if successful, non-zero otherwise
     */
    int Linker::linkCryoCoreLibrary(const char *compilerRootPath)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Merging runtime module into core module...");

        // Get paths to input files
        std::string c_runtimePath = std::string(this->dirInfo->runtimeDir) + "/c_runtime.ll";
        std::string corePath = std::string(this->dirInfo->runtimeDir) + "/core.ll";

        // Verify files exist
        if (!verifyInputFiles(c_runtimePath, corePath))
        {
            return 1;
        }

        // Link LLVM IR files
        std::string tempPath = std::string(this->dirInfo->runtimeDir) + "/merged.ll";
        std::string outputPath = std::string(this->dirInfo->runtimeDir) + "/core.o";

        if (!linkLLVMFiles(c_runtimePath, corePath, tempPath))
        {
            return 1;
        }

        // Compile linked IR to object code
        if (!compileIRToObject(tempPath, outputPath))
        {
            return 1;
        }

        // Create shared library
        if (!createCoreSharedLibrary(compilerRootPath, outputPath))
        {
            return 1;
        }

        return 0;
    }

    /**
     * @brief Verifies that the input files exist
     *
     * @param c_runtimePath Path to C runtime LLVM IR
     * @param corePath Path to core LLVM IR
     * @return true if files exist, false otherwise
     */
    bool Linker::verifyInputFiles(const std::string &c_runtimePath, const std::string &corePath)
    {
        if (!fs->fileExists(c_runtimePath.c_str()))
        {
            fprintf(stderr, "[Linker] Error: Runtime file does not exist: %s\n", c_runtimePath.c_str());
            CONDITION_FAILED;
            return false;
        }

        if (!fs->fileExists(corePath.c_str()))
        {
            fprintf(stderr, "[Linker] Error: Core file does not exist: %s\n", corePath.c_str());
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", c_runtimePath.c_str());
        logMessage(LMI, "INFO", "Linker", "Core Path: %s", corePath.c_str());

        return true;
    }

    /**
     * @brief Links LLVM IR files
     *
     * @param c_runtimePath Path to C runtime LLVM IR
     * @param corePath Path to core LLVM IR
     * @param tempPath Output path for linked IR
     * @return true if successful, false otherwise
     */
    bool Linker::linkLLVMFiles(const std::string &c_runtimePath, const std::string &corePath,
                               const std::string &tempPath)
    {
        std::string linkCommand = "llvm-link-18 -S -o " + tempPath + " " + corePath + " " + c_runtimePath;
        logMessage(LMI, "INFO", "Linker", "Linking LLVM IR files: %s", linkCommand.c_str());

        int linkResult = system(linkCommand.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to link LLVM IR files\n");
            CONDITION_FAILED;
            return false;
        }

        return true;
    }

    /**
     * @brief Compiles LLVM IR to object code
     *
     * @param tempPath Path to LLVM IR
     * @param outputPath Output path for object code
     * @return true if successful, false otherwise
     */
    bool Linker::compileIRToObject(const std::string &tempPath, const std::string &outputPath)
    {
        std::string compileCommand = "llc-18 -filetype=obj -relocation-model=pic -o " + outputPath + " " + tempPath;
        int compileResult = system(compileCommand.c_str());

        if (compileResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile linked IR to object code\n");
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully compiled core file to object code");
        return true;
    }

    /**
     * @brief Creates the core shared library
     *
     * @param compilerRootPath Path to compiler root
     * @param objectPath Path to object file
     * @return true if successful, false otherwise
     */
    bool Linker::createCoreSharedLibrary(const char *compilerRootPath, const std::string &objectPath)
    {
        std::string sharedLibPath = std::string(compilerRootPath) + "/cryo/Std/bin/libcryo_core.so";

        // Create or clear the output file
        if (fs->fileExists(sharedLibPath.c_str()))
        {
            fs->removeFile(sharedLibPath.c_str());
        }
        else
        {
            fs->createNewEmptyFileWpath(sharedLibPath.c_str());
        }

        // Create the shared library
        std::string sharedLibCommand = "clang++ -shared -fPIC -o " + sharedLibPath + " " + objectPath;
        int sharedLibResult = system(sharedLibCommand.c_str());

        if (sharedLibResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to create shared library\n");
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully created shared library: %s", sharedLibPath.c_str());
        this->shared_lib_initialized = true;
        return true;
    }

    /**
     * @brief Builds the standard library
     *
     * Compiles all .cryo files in the Std directory (except Core) and creates
     * individual shared libraries for each module.
     *
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return 0 if successful, non-zero otherwise
     */
    int Linker::buildStandardLib(CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // Get paths to standard library directories
        std::string stdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std";
        std::string stdBinDir = stdDir + "/bin";

        // Create bin directory if it doesn't exist
        if (!std::filesystem::exists(stdBinDir))
        {
            std::filesystem::create_directory(stdBinDir);
        }

        // Find all .cryo files in the standard library
        std::vector<std::string> cryoFiles = findStandardLibraryFiles(stdDir);

        // Print out the files
        printFoundFiles(cryoFiles);

        // Compile each file
        if (!compileAllStandardLibraryFiles(cryoFiles, state, globalTable))
        {
            return 1;
        }

        // Create verification file
        if (!createVerificationFile(stdBinDir))
        {
            return 1;
        }

        return 0;
    }

    /**
     * @brief Finds all .cryo files in the standard library directory
     *
     * @param stdDir Path to standard library directory
     * @return Vector of file paths
     */
    std::vector<std::string> Linker::findStandardLibraryFiles(const std::string &stdDir)
    {
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

        return cryoFiles;
    }

    /**
     * @brief Prints the list of found files
     *
     * @param cryoFiles Vector of file paths
     */
    void Linker::printFoundFiles(const std::vector<std::string> &cryoFiles)
    {
        std::cout << "\n=============== {Files } ===============\n";
        for (const std::string &file : cryoFiles)
        {
            std::cout << "File: " << file << std::endl;
        }
        std::cout << "========================================\n\n";
    }

    /**
     * @brief Compiles all standard library files
     *
     * @param cryoFiles Vector of file paths
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return true if successful, false otherwise
     */
    bool Linker::compileAllStandardLibraryFiles(const std::vector<std::string> &cryoFiles,
                                                CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        for (const std::string &file : cryoFiles)
        {
            // Compile the file and create the shared library
            int result = compileLibItem(file, state, globalTable);
            if (result != 0)
            {
                fprintf(stderr, "[Linker] Error: Failed to compile %s\n", file.c_str());
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Creates a verification file
     *
     * This is a temporary implementation. In the future, this should be a more
     * robust solution that verifies the standard library and creates a checksum.
     *
     * @param stdBinDir Path to standard library binary directory
     * @return true if successful, false otherwise
     */
    bool Linker::createVerificationFile(const std::string &stdBinDir)
    {
        std::string verifiedFilePath = stdBinDir + "/verified";

        if (fs->fileExists(verifiedFilePath.c_str()))
        {
            fs->removeFile(verifiedFilePath.c_str());
        }

        fs->createNewEmptyFileWpath(verifiedFilePath.c_str());
        logMessage(LMI, "INFO", "Linker", "Successfully built standard library: %s", verifiedFilePath.c_str());

        return true;
    }

    /**
     * @brief Compiles a single standard library item
     *
     * Compiles a single .cryo file and creates a shared library linked with the core library.
     *
     * @param filePath Path to .cryo file
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return 0 if successful, non-zero otherwise
     */
    int Linker::compileLibItem(std::string filePath, CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        std::string compilerRootPath = this->dirInfo->compilerDir;
        std::cout << "Compiler Root Path @compileLibItem: " << compilerRootPath << std::endl;
        std::string buildDir = compilerRootPath + "/cryo/Std/bin/.ll/";

        // Compile to AST
        ASTNode *programNode = compileForASTNode(filePath.c_str(), state, globalTable);
        if (programNode == NULL)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile file: %s\n", filePath.c_str());
            return 1;
        }

        programNode->print(programNode);

        // Create compilation unit
        return createAndLinkLibraryItem(programNode, filePath, buildDir, compilerRootPath, state, globalTable);
    }

    /**
     * @brief Creates and links a library item
     *
     * Creates a compilation unit and links it with the core library.
     *
     * @param programNode AST node for the library item
     * @param filePath Path to .cryo file
     * @param buildDir Build directory
     * @param compilerRootPath Compiler root path
     * @param state Current compiler state
     * @param globalTable Global symbol table
     * @return 0 if successful, non-zero otherwise
     */
    int Linker::createAndLinkLibraryItem(ASTNode *programNode, const std::string &filePath,
                                         const std::string &buildDir, const std::string &compilerRootPath,
                                         CompilerState *state, CryoGlobalSymbolTable *globalTable)
    {
        // Create compilation unit directory
        CompilationUnitDir dir = createCompilerCompilationUnitDir(
            filePath.c_str(), buildDir.c_str(), compilerRootPath.c_str(), CRYO_STDLIB);
        dir.print(dir);

        // Create compilation unit
        CompilationUnit *unit = createNewCompilationUnit(programNode, dir);
        if (!unit)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to create CompilationUnit");
            CONDITION_FAILED;
            return 1;
        }

        // Verify compilation unit
        if (unit->verify(unit) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to verify CompilationUnit");
            CONDITION_FAILED;
            return 1;
        }

        // Generate IR from AST
        CryoLinker *linker = reinterpret_cast<CryoLinker *>(this);
        if (generateIRFromAST(unit, state, linker, globalTable) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
            CONDITION_FAILED;
            return 1;
        }

        // Link with core library
        if (!linkWithCoreLibrary(dir, compilerRootPath))
        {
            return 1;
        }

        return 0;
    }

    /**
     * @brief Links a library item with the core library
     *
     * @param dir Compilation unit directory
     * @param compilerRootPath Compiler root path
     * @return true if successful, false otherwise
     */
    bool Linker::linkWithCoreLibrary(const CompilationUnitDir &dir, const std::string &compilerRootPath)
    {
        std::string coreLibPath = compilerRootPath + "/cryo/Std/bin/libcryo_core.so";
        std::string outputPath = compilerRootPath + "/cryo/Std/bin/" + dir.out_fileName + ".so";

        std::string linkCommand = "clang++ -shared -fPIC -o " + outputPath + " " +
                                  coreLibPath + " " + dir.out_filePath + ".ll";

        int linkResult = system(linkCommand.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to link module with core library\n");
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully linked module with core library");
        logMessage(LMI, "INFO", "Linker", "Successfully created shared library: %s", outputPath.c_str());

        return true;
    }

} // namespace Cryo
