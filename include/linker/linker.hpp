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
#ifndef CRYO_LINKER_H
#define CRYO_LINKER_H

typedef struct CompilerState CompilerState;

// ================================================================ //
// C Interface
#ifdef __cplusplus
extern "C"
{
#endif

    // C API
    // Opaque Pointer for C API
    typedef struct CryoLinker_t *CryoLinker;

    // Constructors & Destructors
    CryoLinker *CryoLinker_Create(const char *buildDir, const char *compilerRootPath);

    // Linker Functions
    void CryoLinker_InitCRuntime(CryoLinker *linker);
    void CryoLinker_LinkAll(CryoLinker *linker);
    void CryoLinker_InitCryoCore(CryoLinker *linker, const char *compilerRootPath, const char *buildDir,
                                 CompilerState *state);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Macros
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define INIT_LINKER_C_RUNTIME(linker) CryoLinker_InitCRuntime(linker)
#define LINK_ALL_MODULES(linker) CryoLinker_LinkAll(linker)

#define CreateCryoLinker(buildDir, compilerRootPath) CryoLinker_Create(buildDir, compilerRootPath)

#ifdef __cplusplus
}
// ================================================================ //

// -----------------------------------------------
// C++ includes

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <unistd.h>
#include <optional>
#include <wait.h>
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>

// -----------------------------------------------
// LLVM includes

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/CodeGen.h"

#include "llvm/Linker/IRMover.h"
#include "llvm/Linker/Linker.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/CodeGenCWrappers.h"
#include "llvm/Target/CGPassBuilderOption.h"
#include <llvm/Target/TargetOptions.h>
#include "llvm/MC/TargetRegistry.h"

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"

#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

// -----------------------------------------------
// C Includes

#include "tools/macros/debugMacros.h"
#include "tools/utils/env.h"
#include "linker/compilationUnit.h"

// Check clang version for the system command "clang-{version}"
#if defined(__clang_major__) && defined(__clang_minor__)
#define CLANG_VERSION_MAJOR __clang_major__
#define CLANG_VERSION_MINOR __clang_minor__
#else
#define CLANG_VERSION_MAJOR 0
#define CLANG_VERSION_MINOR 0
#endif

struct DirectoryInfo
{
    std::string rootDir;
    std::string buildDir;
    std::string outDir;
    std::string depDir;
    std::string runtimeDir;
    std::string compilerDir;
};

namespace Cryo
{
    class Linker;
    extern CryoLinker *globalLinker; // Global Linker Object
#define GetCXXLinker() reinterpret_cast<Cryo::Linker *>(globalLinker)
#define GetCLinker() globalLinker

    // ================================================================ //
    //                        Linker Manager                            //
    // ================================================================ //

    class Linker
    {
    public:
        Linker(const char *buildDir, const char *compilerRootPath)
        {
            std::cout << "Linker Constructor Called..." << std::endl;
            std::cout << "Linker Build Directory: " << buildDir << std::endl;
            std::cout << "Linker Compiler Root Path: " << compilerRootPath << std::endl;

            std::string rootDir = std::string(buildDir).substr(0, std::string(buildDir).find_last_of("/"));
            dirInfo = createDirectoryInfo(rootDir);
            dirInfo->compilerDir.assign(compilerRootPath);

            // Print the directory info
            logDirectoryInfo(dirInfo);

            // Set the global linker object
            globalLinker = reinterpret_cast<CryoLinker *>(this);
            std::cout << "Global Linker Object Set..." << std::endl;
        }

        Cryo::Linker *getCXXLinker() { return reinterpret_cast<Cryo::Linker *>(globalLinker); }
        CryoLinker *getCLinker() { return globalLinker; }

        // ================================================================ //
        // Linker Properties

        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> finalModule;
        llvm::Module *preprocessedModule;

        llvm::Module *coreModule;              // [NEW]
        bool coreModuleInitialized = false;    // [NEW]
        llvm::Module *runtimeModule;           // [NEW]
        bool runtimeModuleInitialized = false; // [NEW]

        void setPreprocessedModule(llvm::Module *mod) { preprocessedModule = mod; }
        std::vector<llvm::Module *> dependencies;

        DirectoryInfo *dirInfo;
        bool c_runtime_initialized = false;
        bool auto_run_binary = false;
        bool shared_lib_initialized = false;
        bool autoRunEnabled = false;

        llvm::LLVMContext &getLinkerContext() { return context; }

        // ================================================================ //
        // Linker Functions

        llvm::Module *initMainModule(void);

        llvm::Module *mergeRuntimeToModule(void);

        void mergeInModule(llvm::Module *destModule, llvm::Module *srcModule);

        // ================================================================ //
        // CodeGen Interface for Linker

        /// @brief After running the ASTNode through the code generation process, this function will
        /// compile the module and output the IR to a file given the `CompilationUnit` object.
        /// @param unit The compilation unit object that contains the module and the output path.
        void compileModule(CompilationUnit *unit, llvm::Module *mod);

        void generateIRFromCodegen(llvm::Module *mod, const char *outputPath);

        // ================================================================ //
        // C Runtime Initialization

        std::string getCRuntimePath();
        void initCRuntime(void);
        void createCRuntimeFile(void);

        // ================================================================ //
        // Supporting Functions

        std::vector<std::string> listDir(const char *dirPath);

    private:
        DirectoryInfo *createDirectoryInfo(std::string rootDir);
        DirectoryInfo *getDirInfo() { return dirInfo; }
        void logDirectoryInfo(DirectoryInfo *dirInfo);

        int covertCRuntimeToLLVMIR(std::string cRuntimePath, std::string outDir);

    public:
        /**
         * @brief Main entry point for the final linking process
         *
         * Called at the end of compilation to link all components together.
         */
        void linkAll(void);

        /**
         * @brief Links the main module with the core library
         *
         * @param mod The main module to link
         */

        void linkMain(llvm::Module *mod);

        /**
         * @brief Creates the main object file
         *
         * Converts main.ll to main.o for linking.
         */
        void createMainObject(void);

        /**
         * @brief Combines all object files to create the main executable
         *
         * Links all object files and standard libraries.
         */
        void combineAllObjectsToMainExe(void);

        /**
         * @brief Signals completion of code generation
         *
         * Called when IR generation is complete to start the linking process.
         */
        void completeCodeGeneration(void);

        /**
         * @brief Runs the completed binary
         *
         * Executes the program after successful linking.
         */
        void runCompletedBinary(void);

        /**
         * @brief Set automatic running of the binary after compilation
         *
         * @param enabled Whether to automatically run the binary
         */
        void setAutoRun(bool enabled) { autoRunEnabled = enabled; }

    private:
        // End of Compilation (EOC) functions
        bool convertIRToObjects();
        bool ensureDirectoryExists(const std::string &dirPath);
        bool convertMainIRToObject(const std::string &irDir, const std::string &objDir);
        bool convertModuleIRsToObjects(const std::string &irDir, const std::string &objDir);
        bool linkFinalExecutable();
        std::vector<std::string> collectObjectFiles(const std::string &objDir);
        std::vector<std::string> collectStandardLibraries(const std::string &compilerDir);
        std::string buildLinkCommand(const std::string &outputPath,
                                     const std::vector<std::string> &objectFiles,
                                     const std::vector<std::string> &stdLibFiles);
        bool executeLinkCommand(const std::string &linkCommand, const std::string &outputPath);

    public:
        /**
         * @brief Initializes the core Cryo environment
         *
         * @param compilerRootPath Path to the compiler root directory
         * @param buildDir Directory for build outputs
         * @param state Current compiler state
         * @param globalTable Global symbol table
         */
        void initCryoCore(const char *compilerRootPath, const char *buildDir,
                          CompilerState *state);

        /**
         * @brief Builds the standard library
         *
         * @param state Current compiler state
         * @param globalTable Global symbol table
         * @return 0 if successful, non-zero otherwise
         */
        int buildStandardLib(CompilerState *state);

        /**
         * @brief Compiles a single standard library item
         *
         * @param filePath Path to .cryo file
         * @param state Current compiler state
         * @param globalTable Global symbol table
         * @return 0 if successful, non-zero otherwise
         */
        int compileLibItem(std::string filePath, CompilerState *state);

    private:
        // Runtime initialization
        void initializeRuntimeModule();
        void _initCRuntime_() { initializeRuntimeModule(); } // Legacy method
        bool compileRuntimeToIR(const std::string &compilerRootPath, const char *runtimePath);

        // Core file processing
        bool processCoreFile(const char *compilerRootPath, const char *buildDir,
                             CompilerState *state);
        bool generateCoreIR(ASTNode *defsNode, const char *corePath, const char *buildDir,
                            CompilerState *state);

        // Core library creation
        int linkCryoCoreLibrary(const char *compilerRootPath);
        int completeCryoCryoLib(const char *compilerRootPath) { return linkCryoCoreLibrary(compilerRootPath); } // Legacy method
        bool verifyInputFiles(const std::string &c_runtimePath, const std::string &corePath);
        bool linkLLVMFiles(const std::string &c_runtimePath, const std::string &corePath, const std::string &tempPath);
        bool compileIRToObject(const std::string &tempPath, const std::string &outputPath);
        bool createCoreSharedLibrary(const char *compilerRootPath, const std::string &objectPath);

        // Standard library building
        std::vector<std::string> findStandardLibraryFiles(const std::string &stdDir);
        void printFoundFiles(const std::vector<std::string> &cryoFiles);
        bool compileAllStandardLibraryFiles(const std::vector<std::string> &cryoFiles,
                                            CompilerState *state);

        // Library item compilation
        int createAndLinkLibraryItem(ASTNode *programNode, const std::string &filePath,
                                     const std::string &buildDir, const std::string &compilerRootPath,
                                     CompilerState *state);
        bool linkWithCoreLibrary(const CompilationUnitDir &dir, const std::string &compilerRootPath);

        // Build verification
        bool createVerificationFile(const std::string &stdBinDir);
        bool isStandardLibraryUpToDate();
        std::string calculateFileChecksum(const std::string &filePath);
        std::string getCurrentTimestamp();
        std::string getFileModificationTime(const std::string &filePath);
        std::string getSourcePathForModule(const std::string &modulePath);
        uint32_t crc32(const std::string &data);
    };

    // ================================================================ //
    //                     C API Implementation                         //
    // ================================================================ //

    inline CryoLinker *CryoLinker_Create(const char *buildDir, const char *compilerRootPath)
    {
        try
        {
            auto linker = new Linker(buildDir, compilerRootPath);
            return reinterpret_cast<CryoLinker *>(linker);
        }
        catch (...)
        {
            return nullptr;
        }
    }

    inline void CryoLinker_InitCRuntime(CryoLinker *linker)
    {
        auto _linker = reinterpret_cast<Linker *>(linker);
        _linker->initCRuntime();
    }

    inline void CryoLinker_LinkAll(CryoLinker *linker)
    {
        auto _linker = reinterpret_cast<Linker *>(linker);
        _linker->linkAll();
    }

    inline void CryoLinker_InitCryoCore(CryoLinker *linker, const char *compilerRootPath, const char *buildDir,
                                        CompilerState *state)
    {
        auto _linker = reinterpret_cast<Linker *>(linker);
        _linker->initCryoCore(compilerRootPath, buildDir, state);
    }

} // namespace Cryo

#endif // __cplusplus
#endif // CRYO_LINKER_V2_H
