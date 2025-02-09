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
    CryoLinker *CryoLinker_Create(const char *buildDir);

    // Linker Functions
    void CryoLinker_InitCRuntime(CryoLinker *linker);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Macros
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define INIT_LINKER_C_RUNTIME(linker) CryoLinker_InitCRuntime(linker)

#define CreateCryoLinker(buildDir) CryoLinker_Create(buildDir)

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

#include <wait.h>

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

#include "llvm/Support/FileSystem.h"
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

#include "codegen/devDebugger/devDebugger.hpp"
#include "tools/macros/debugMacros.h"
#include "tools/utils/env.h"
#include "linker/compilationUnit.h"

struct DirectoryInfo
{
    std::string rootDir;
    std::string buildDir;
    std::string outDir;
    std::string depDir;
    std::string runtimeDir;
};

namespace Cryo
{
    class Linker;
    extern CryoLinker *globalLinker; // Global Linker Object
#define GetCXXLinker() reinterpret_cast<Cryo::Linker *>(globalLinker)

    // ================================================================ //
    //                        Linker Manager                            //
    // ================================================================ //

    class Linker
    {
    public:
        Linker(const char *buildDir)
        {
            std::cout << "Linker Constructor Called..." << std::endl;
            std::string rootDir = std::string(buildDir).substr(0, std::string(buildDir).find_last_of("/"));
            dirInfo = createDirectoryInfo(rootDir);

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
        void setPreprocessedModule(llvm::Module *mod) { preprocessedModule = mod; }
        std::vector<llvm::Module *> dependencies;
        DirectoryInfo *dirInfo;
        bool c_runtime_initialized = false;

        llvm::LLVMContext &getLinkerContext() { return context; }

        // ================================================================ //
        // Linker Functions

        llvm::Module *initMainModule(void);

        void addPreprocessingModule(llvm::Module *mod);

        std::string createIRFromModule(llvm::Module *module, std::string outDir);
        llvm::Module *getCryoRuntimeModule(void);

        void generateIRFromCodegen(llvm::Module *mod, const char *outputPath);
        void completeCodeGeneration(void);

        // ================================================================ //
        // CodeGen Interface for Linker

        /// @brief After running the ASTNode through the code generation process, this function will
        /// compile the module and output the IR to a file given the `CompilationUnit` object.
        /// @param unit The compilation unit object that contains the module and the output path.
        void compileModule(CompilationUnit *unit, llvm::Module *mod);

        // ================================================================ //
        // C Runtime Initialization

        std::string getCRuntimePath();
        void initCRuntime(void);
        void createCRuntimeFile(void);

    private:
        DirectoryInfo *createDirectoryInfo(std::string rootDir);
        DirectoryInfo *getDirInfo() { return dirInfo; }
        void logDirectoryInfo(DirectoryInfo *dirInfo);

        int covertCRuntimeToLLVMIR(std::string cRuntimePath, std::string outDir);
        bool mergeAllRuntimeFiles();
        std::string mergeTwoIRFiles(std::string file1, std::string file2, std::string fileName, std::string outDir);

        std::vector<std::string> listDir(const char *path);
        void runCompletedBinary();

    public:
        std::string getCryoRuntimeFilePath(void);
        void mergeTwoModules(llvm::Module *destMod, std::unique_ptr<llvm::Module> srcMod);
    };

    // ================================================================ //
    //                     C API Implementation                         //
    // ================================================================ //

    inline CryoLinker *CryoLinker_Create(const char *buildDir)
    {
        try
        {
            auto linker = new Linker(buildDir);
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
} // namespace Cryo

#endif // __cplusplus
#endif // CRYO_LINKER_V2_H
