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
    void Linker::initCryoCore(const char *compilerRootPath, const char *buildDir, CompilerState *state)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Initializing Cryo Core...");
        // Initialize C runtime
        initializeRuntimeModule();
        if (!this->runtimeModuleInitialized)
        {
            fprintf(stderr, "[Linker] Error: Failed to initialize the runtime module @initCryoCore\n");
            CONDITION_FAILED;
            return;
        }

        // Process core.cryo file
        if (!processCoreFile(compilerRootPath, buildDir, state))
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

        if (buildStandardLib(state) != 0)
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
                                 CompilerState *state)
    {
        __STACK_FRAME__
        logMessage(LMI, "INFO", "Linker", "Processing core file...");
        // Create path to core.cryo
        const char *corePath = fs->appendStrings(compilerRootPath, "/cryo/Std/Core/core.cryo");
        if (!corePath)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to allocate memory for definitions path\n");
            CONDITION_FAILED;
            return false;
        }

        // Compile the core file to AST
        ASTNode *defsNode = compileForASTNode(strdup(corePath), state);
        if (!defsNode)
        {
            fprintf(stderr, "[Data Type Manager] Error: Failed to compile definitions\n");
            CONDITION_FAILED;
            return false;
        }
        logMessage(LMI, "INFO", "DTM", "Definitions Path: %s", corePath);

        // Create compilation unit and generate IR
        return generateCoreIR(defsNode, corePath, buildDir, state);
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
    bool Linker::generateCoreIR(ASTNode *defsNode, const char *corePath, const char *buildDir, CompilerState *state)
    {
        logMessage(LMI, "INFO", "Linker", "Generating IR for core file...");
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
        if (generateIRFromAST(unit, state, linker) != 0)
        {
            logMessage(LMI, "ERROR", "CryoCompiler", "Failed to generate IR from AST");
            CONDITION_FAILED;
            return false;
        }

        // Check for the existence of the core.ll file
        std::string coreFilePath = std::string(this->dirInfo->runtimeDir) + "/core.ll";
        logMessage(LMI, "INFO", "Linker", "Core file exists: %s", coreFilePath.c_str());

        // Check if all directories exist `std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/`
        std::string cryoStdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/";
        if (!std::filesystem::exists(cryoStdDir) || !std::filesystem::is_directory(cryoStdDir))
        {
            // Create the directory
            try
            {
                std::filesystem::create_directories(cryoStdDir);
                logMessage(LMI, "INFO", "Linker", "Created directory: %s", cryoStdDir.c_str());
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logMessage(LMI, "ERROR", "Linker", "Failed to create directory %s: %s",
                           cryoStdDir.c_str(), e.what());
                CONDITION_FAILED;
                return false;
            }
        }

        // Copy the core.ll file to {compilerDir}/cryo/Std/bin/.ll/core.ll
        std::string destPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/core.ll";
        std::string copyCommand = "cp " + coreFilePath + " " + destPath;
        int result = system(copyCommand.c_str());
        if (result != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to copy core.ll file\n");
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully copied core.ll to %s", destPath.c_str());

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
        if (!runtimePath)
        {
            fprintf(stderr, "[Linker] Error: Failed to allocate memory for runtime path\n");
            CONDITION_FAILED;
            return;
        }
        logMessage(LMI, "INFO", "Linker", "Runtime Path: %s", runtimePath);

        // Check if {compilerRootPath}/cryo/bin/.ll/ exists
        std::string cryoBinDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/";
        // If it doesn't, recursively create it
        if (!std::filesystem::exists(cryoBinDir) || !std::filesystem::is_directory(cryoBinDir))
        {
            // Create the directory
            try
            {
                std::filesystem::create_directories(cryoBinDir);
                logMessage(LMI, "INFO", "Linker", "Created directory: %s", cryoBinDir.c_str());
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                logMessage(LMI, "ERROR", "Linker", "Failed to create directory %s: %s",
                           cryoBinDir.c_str(), e.what());
                CONDITION_FAILED;
                return;
            }
        }

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
        logMessage(LMI, "INFO", "Linker", "Compiling runtime.c to LLVM IR...");
        // Set up Clang compilation arguments
        std::vector<const char *> args;
        args.push_back("clang++-18");
        args.push_back("-c");
        args.push_back("-emit-llvm");
        args.push_back("-O1");       // Optimization level
        args.push_back("-fPIC");     // Position-Independent Code
        args.push_back("-x");        // Specify input language
        args.push_back("c");         // C language
        args.push_back(runtimePath); // Input file

        // Add standard includes
        args.push_back("-I");
        args.push_back(fs->appendStrings(compilerRootPath.c_str(), "/cryo/runtime/include"));

        // Set output file
        std::string userBuildDir = std::string(this->dirInfo->runtimeDir) + "/c_runtime.ll";
        std::string outputPath = compilerRootPath + "/cryo/Std/bin/.ll/c_runtime.ll";
        args.push_back("-o");
        args.push_back(outputPath.c_str());

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
        // Check for an unexpected error
        if (result == -1)
        {
            fprintf(stderr, "[Linker] Error: Unexpected error occurred while compiling runtime file\n");
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
        std::string c_runtimePath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/c_runtime.ll";
        std::string corePath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/core.ll";

        // Verify files exist
        if (!verifyInputFiles(c_runtimePath, corePath))
        {
            return 1;
        }

        // Link LLVM IR files
        std::string tempPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/merged.ll";
        std::string outputPath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/bin/.ll/core.o";

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
        logMessage(LMI, "INFO", "Linker", "Verifying input files...");
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
        logMessage(LMI, "INFO", "Linker", "Linking LLVM IR files: %s and %s", c_runtimePath.c_str(), corePath.c_str());
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
        logMessage(LMI, "INFO", "Linker", "Compiling linked IR to object code: %s", tempPath.c_str());
        std::string compileCommand = "llc-18 -filetype=obj -relocation-model=pic -o " + outputPath + " " + tempPath;
        int compileResult = system(compileCommand.c_str());

        if (compileResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to compile linked IR to object code\n");
            CONDITION_FAILED;
            return false;
        }

        // Destroy the temp file
        std::string removeCommand = "rm -f " + tempPath;
        int removeResult = system(removeCommand.c_str());
        if (removeResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to remove temporary file\n");
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
        logMessage(LMI, "INFO", "Linker", "Creating core shared library...");
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
    int Linker::buildStandardLib(CompilerState *state)
    {
        logMessage(LMI, "INFO", "Linker", "Building standard library...");
        // Get paths to standard library directories
        std::string stdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std";
        std::string stdBinDir = stdDir + "/bin";

        // Create bin directory if it doesn't exist
        if (!std::filesystem::exists(stdBinDir))
        {
            std::filesystem::create_directory(stdBinDir);
        }

        // Commented out for now. Right now we always rebuild the standard library.
        // if (isStandardLibraryUpToDate())
        // {
        //     logMessage(LMI, "INFO", "Linker", "Standard library is up to date, skipping rebuild");
        //     return 0;
        // }

        logMessage(LMI, "INFO", "Linker", "Standard library needs rebuilding");

        // Find all .cryo files in the standard library
        std::vector<std::string> cryoFiles = findStandardLibraryFiles(stdDir);

        // Print out the files
        // printFoundFiles(cryoFiles);

        // Compile each file
        if (!compileAllStandardLibraryFiles(cryoFiles, state))
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
        logMessage(LMI, "INFO", "Linker", "Finding standard library files in %s", stdDir.c_str());
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
                                                CompilerState *state)
    {
        logMessage(LMI, "INFO", "Linker", "Compiling standard library files...");
        for (const std::string &file : cryoFiles)
        {
            // Compile the file and create the shared library
            int result = compileLibItem(file, state);
            if (result != 0)
            {
                fprintf(stderr, "[Linker] Error: Failed to compile %s\n", file.c_str());
                return false;
            }
        }

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
    int Linker::compileLibItem(std::string filePath, CompilerState *state)
    {
        logMessage(LMI, "INFO", "Linker", "Compiling library item: %s", filePath.c_str());

        std::string compilerRootPath = this->dirInfo->compilerDir;
        logMessage(LMI, "INFO", "Linker", "Compiler Root Path: %s", compilerRootPath.c_str());
        std::string buildDir = compilerRootPath + "/cryo/Std/bin/.ll/";

        // Compile to AST
        ASTNode *programNode = compileForASTNode(filePath.c_str(), state);
        if (programNode == NULL)
        {
            logMessage(LMI, "ERROR", "Linker", "Failed to compile %s", filePath.c_str());
            return 1;
        }

        programNode->print(programNode);

        // Create compilation unit
        return createAndLinkLibraryItem(programNode, filePath, buildDir, compilerRootPath, state);
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
                                         CompilerState *state)
    {
        logMessage(LMI, "INFO", "Linker", "Creating and linking library item...");

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
        if (generateIRFromAST(unit, state, linker) != 0)
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
        logMessage(LMI, "INFO", "Linker", "Linking with core library...");

        std::string coreLibPath = compilerRootPath + "/cryo/Std/bin/.ll/core.ll";
        std::string outputPath = compilerRootPath + "/cryo/Std/bin/" + dir.out_fileName + ".so";
        std::string tempLinkedIRPath = compilerRootPath + "/cryo/Std/bin/" + dir.out_fileName + "_linked.ll";

        std::string linkCommand = "llvm-link-18 -v --only-needed --internalize -o " + outputPath + " " +
                                  coreLibPath + " " + dir.out_filePath + ".ll";

        logMessage(LMI, "INFO", "Linker", "Linking with core library: %s", linkCommand.c_str());
        int linkResult = system(linkCommand.c_str());
        if (linkResult != 0)
        {
            fprintf(stderr, "[Linker] Error: Failed to create shared library using clang++\n");
            CONDITION_FAILED;
            return false;
        }

        logMessage(LMI, "INFO", "Linker", "Successfully linked module with core library");
        logMessage(LMI, "INFO", "Linker", "Successfully created shared library: %s", outputPath.c_str());

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
        std::string manifestPath = stdBinDir + "/manifest.txt";
        std::ofstream manifestFile(manifestPath);

        if (!manifestFile.is_open())
        {
            fprintf(stderr, "[Linker] Error: Failed to create manifest file\n");
            CONDITION_FAILED;
            return false;
        }

        // Write compiler version and timestamp
        manifestFile << "COMPILER_VERSION=" << COMPILER_VERSION << "\n";
        manifestFile << "BUILD_TIMESTAMP=" << getCurrentTimestamp() << "\n";
        manifestFile << "MODULE_COUNT=";

        // Count the modules first
        size_t moduleCount = 0;
        for (const auto &entry : std::filesystem::directory_iterator(stdBinDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".so")
            {
                moduleCount++;
            }
        }
        manifestFile << moduleCount << "\n\n";

        // Write each module's information
        for (const auto &entry : std::filesystem::directory_iterator(stdBinDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".so")
            {
                std::string modulePath = entry.path().string();
                std::string moduleName = std::filesystem::path(modulePath).stem().string();

                manifestFile << "[MODULE]\n";
                manifestFile << "NAME=" << moduleName << "\n";
                manifestFile << "SO_PATH=" << modulePath << "\n";

                // Special handling for libcryo_core
                if (moduleName == "libcryo_core")
                {
                    // Use the core.cryo file as the source
                    std::string corePath = std::string(this->dirInfo->compilerDir) + "/cryo/Std/Core/core.cryo";
                    manifestFile << "SOURCE_PATH=" << corePath << "\n";
                    manifestFile << "SOURCE_CHECKSUM=" << calculateFileChecksum(corePath) << "\n";
                }
                else
                {
                    // Regular module handling
                    std::string sourcePath = getSourcePathForModule(modulePath);
                    manifestFile << "SOURCE_PATH=" << sourcePath << "\n";
                    manifestFile << "SOURCE_CHECKSUM=" << calculateFileChecksum(sourcePath) << "\n";
                }

                manifestFile << "BINARY_CHECKSUM=" << calculateFileChecksum(modulePath) << "\n";
                manifestFile << "BUILD_TIME=" << getFileModificationTime(modulePath) << "\n";
                manifestFile << "\n";
            }
        }

        manifestFile.close();
        logMessage(LMI, "INFO", "Linker", "Successfully created build manifest: %s", manifestPath.c_str());
        return true;
    }

    bool Linker::isStandardLibraryUpToDate()
    {
        std::string stdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std";
        std::string stdBinDir = stdDir + "/bin";
        std::string manifestPath = stdBinDir + "/manifest.txt";

        // If no manifest exists, rebuild is needed
        if (!fs->fileExists(manifestPath.c_str()))
        {
            logMessage(LMI, "INFO", "Linker", "Manifest file does not exist, rebuilding standard library");
            return false;
        }

        // Parse the manifest file
        std::ifstream manifestFile(manifestPath);
        if (!manifestFile.is_open())
        {
            fprintf(stderr, "[Linker] Error: Failed to open manifest file\n");
            return false;
        }

        std::string line;
        std::string currentCompilerVersion;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> modules;
        std::unordered_map<std::string, std::string> *currentModule = nullptr;

        while (std::getline(manifestFile, line))
        {
            // Skip empty lines
            if (line.empty())
            {
                continue;
            }

            // Start of a new module section
            if (line == "[MODULE]")
            {
                logMessage(LMI, "INFO", "Linker", "Found new module section");
                currentModule = &modules[std::to_string(modules.size())];
                continue;
            }

            // Parse key-value pairs
            size_t equalPos = line.find('=');
            if (equalPos != std::string::npos)
            {
                std::string key = line.substr(0, equalPos);
                std::string value = line.substr(equalPos + 1);

                // Store compiler version separately
                if (key == "COMPILER_VERSION")
                {
                    logMessage(LMI, "INFO", "Linker", "Found compiler version: %s", value.c_str());
                    currentCompilerVersion = value;
                }
                // Store module information
                else if (currentModule != nullptr)
                {
                    logMessage(LMI, "INFO", "Linker", "Found module info: %s = %s", key.c_str(), value.c_str());
                    (*currentModule)[key] = value;
                }
            }
        }

        // Check compiler version
        if (currentCompilerVersion != COMPILER_VERSION)
        {
            logMessage(LMI, "INFO", "Linker", "Compiler version mismatch, rebuilding standard library");
            return false;
        }

        // Find all .cryo files
        std::vector<std::string> cryoFiles = findStandardLibraryFiles(stdDir);
        std::unordered_set<std::string> processedSourcePaths;

        // Check each source file
        for (const std::string &file : cryoFiles)
        {
            std::string currentChecksum = calculateFileChecksum(file);
            bool fileFound = false;
            logMessage(LMI, "INFO", "Linker", "Processing file: %s", file.c_str());
            logMessage(LMI, "INFO", "Linker", "Current checksum: %s", currentChecksum.c_str());

            // Search for this file in the modules
            for (const auto &modulePair : modules)
            {
                const auto &module = modulePair.second;
                auto sourcePathIt = module.find("SOURCE_PATH");

                if (sourcePathIt != module.end() && sourcePathIt->second == file)
                {
                    fileFound = true;
                    processedSourcePaths.insert(file);

                    // Check source checksum
                    auto sourceChecksumIt = module.find("SOURCE_CHECKSUM");
                    if (sourceChecksumIt == module.end() || sourceChecksumIt->second != currentChecksum)
                    {
                        logMessage(LMI, "INFO", "Linker", "Source checksum mismatch, rebuilding needed");
                        return false;
                    }

                    // Check if .so file exists
                    auto soPathIt = module.find("SO_PATH");
                    if (soPathIt == module.end() || !fs->fileExists(soPathIt->second.c_str()))
                    {
                        logMessage(LMI, "INFO", "Linker", "Shared object file not found, rebuilding needed");
                        return false;
                    }

                    // Check binary checksum
                    auto binaryChecksumIt = module.find("BINARY_CHECKSUM");
                    std::string currentBinaryChecksum = calculateFileChecksum(soPathIt->second);
                    if (binaryChecksumIt == module.end() || binaryChecksumIt->second != currentBinaryChecksum)
                    {
                        logMessage(LMI, "INFO", "Linker", "Binary checksum mismatch, rebuilding needed");
                        return false;
                    }

                    break;
                }
            }

            // If source file not found in manifest, rebuild needed
            if (!fileFound)
            {
                logMessage(LMI, "INFO", "Linker", "Source file not found in manifest, rebuilding needed");
                logMessage(LMI, "INFO", "Linker", "Attempted Manifest Path: %s", manifestPath.c_str());
                return false;
            }
        }

        // Check if all modules in the manifest still have corresponding source files
        for (const auto &modulePair : modules)
        {
            const auto &module = modulePair.second;
            auto sourcePathIt = module.find("SOURCE_PATH");
            auto nameIt = module.find("NAME");

            // Skip this check for libcryo_core since it's a special case
            if (nameIt != module.end() && nameIt->second == "libcryo_core")
            {
                continue;
            }

            if (sourcePathIt != module.end() && processedSourcePaths.find(sourcePathIt->second) == processedSourcePaths.end())
            {
                // Source file no longer exists, rebuild needed
                logMessage(LMI, "INFO", "Linker", "Source file no longer exists, rebuilding needed");
                logMessage(LMI, "INFO", "Linker", "Missing source path: %s", sourcePathIt->second.c_str());
                return false;
            }
        }

        // Everything matches, no rebuild needed
        logMessage(LMI, "INFO", "Linker", "Standard library is up to date");
        return true;
    }

    std::string Linker::getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string Linker::getFileModificationTime(const std::string &filePath)
    {
        auto fsTime = std::filesystem::last_write_time(filePath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            fsTime - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
        auto time = std::chrono::system_clock::to_time_t(sctp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string Linker::getSourcePathForModule(const std::string &modulePath)
    {
        // Extract module name from path
        std::filesystem::path path(modulePath);
        std::string moduleName = path.stem().string();

        // Search for matching .cryo file
        std::string stdDir = std::string(this->dirInfo->compilerDir) + "/cryo/Std";
        for (const auto &entry : std::filesystem::recursive_directory_iterator(stdDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".cryo")
            {
                // Skip files in the Core directory if needed
                if (entry.path().string().find("/Std/Core/") == std::string::npos)
                {
                    // Check if this is the source file for our module
                    std::filesystem::path sourcePath(entry.path());
                    if (sourcePath.stem().string() == moduleName)
                    {
                        return entry.path().string();
                    }
                }
            }
        }

        return "";
    }

    uint32_t Linker::crc32(const std::string &data)
    {
        static uint32_t crc32_table[256];
        static bool table_initialized = false;

        // Initialize the CRC table on first use
        if (!table_initialized)
        {
            for (uint32_t i = 0; i < 256; i++)
            {
                uint32_t c = i;
                for (int j = 0; j < 8; j++)
                {
                    c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
                }
                crc32_table[i] = c;
            }
            table_initialized = true;
        }

        // Calculate CRC
        uint32_t crc = 0xFFFFFFFF;
        for (unsigned char byte : data)
        {
            crc = crc32_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
        }

        return ~crc;
    }

    std::string Linker::calculateFileChecksum(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file)
        {
            return "FILE_NOT_FOUND";
        }

        // Initialize CRC table
        static uint32_t crc32_table[256];
        // Read file in chunks to handle large files
        constexpr size_t BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        uint32_t crc = 0xFFFFFFFF;

        while (file)
        {
            file.read(buffer, BUFFER_SIZE);
            std::streamsize count = file.gcount();
            if (count == 0)
                break;

            for (std::streamsize i = 0; i < count; i++)
            {
                crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ static_cast<unsigned char>(buffer[i])];
            }
        }

        crc = ~crc;

        // Convert CRC to hex string
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(8) << crc;
        return ss.str();
    }

} // namespace Cryo
