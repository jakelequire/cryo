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
    CryoLinker *CryoLinker_Create(void);

    // State Functions
    void CryoLinker_SetBuildSrcDirectory(CryoLinker *linker, const char *buildDir);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Macros
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define CreateCryoLinker() CryoLinker_Create()
#define SetLinkerBuildDir(linker, rootDir) CryoLinker_SetBuildSrcDirectory(linker, rootDir)

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
    class LinkerModule;
    class Linker;

    // ================================================================ //
    //                        Linker Manager                            //
    // ================================================================ //

    class Linker
    {
    public:
        Linker()
        {
            linkerModule = std::make_unique<LinkerModule>();
        }
        ~Linker() {}

        DirectoryInfo *dirInfo;
        DirectoryInfo *getDirInfo() { return dirInfo; }

        void setBuildDir(const char *buildDir);

        void newInitDependencies(llvm::Module *srcModule);
        void appendDependenciesToRoot(llvm::Module *root);

        void addPreprocessingModule(llvm::Module *mod);

    private:
        std::unique_ptr<LinkerModule> linkerModule;
    };

    // ================================================================ //
    //                     Linker Module Handler                        //
    // ================================================================ //

    class LinkerModule : public Linker
    {
    public:
        LinkerModule()
        {
            std::cout << "Linker Module Constructor Called..." << std::endl;
        }

        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> finalModule;

        std::vector<llvm::Module *> dependencies;
        llvm::Module *preprocessedModule;

        size_t moduleCount = 0;

        void addMainModule(llvm::Module *mod);
        void addNewModule(llvm::Module *mod, std::string namename);
        void addPreprocessedModule(llvm::Module *mod);

    private:
        void generateIRFromModule(llvm::Module *module);
        bool contextMatch(llvm::Module *mod1, llvm::Module *mod2);

        // Public Debug Methods
    public:
        void logDirectoryInfo(DirectoryInfo *dirInfo);
    };

    // ================================================================ //
    //                     C API Implementation                         //
    // ================================================================ //

    inline CryoLinker *CryoLinker_Create(void)
    {
        try
        {
            auto linker = new Linker();
            return reinterpret_cast<CryoLinker *>(linker);
        }
        catch (...)
        {
            return nullptr;
        }
    }

    inline void CryoLinker_SetBuildSrcDirectory(CryoLinker *linker, const char *buildDir)
    {
        if (linker)
        {
            reinterpret_cast<Linker *>(linker)->setBuildDir(buildDir);
        }
    }

} // namespace Cryo

#endif // __cplusplus
#endif // CRYO_LINKER_V2_H
