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

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque pointer type for C
    typedef struct CryoLinker_t *CryoLinker;

    // C API
    CryoLinker CryoLinker_Create(void);
    void CryoLinker_Destroy(CryoLinker linker);
    void CryoLinker_SetBuildSrcDirectory(CryoLinker linker, const char *srcDir);
    void CryoLinker_AddRootModule(CryoLinker linker, void *module);
    void CryoLinker_AddModuleDependency(CryoLinker linker, void *module);
    void *CryoLinker_LinkModules(CryoLinker linker);

    void CryoLinker_InitDependencies(CryoLinker linker);

    void CryoLinker_LogState(CryoLinker linker);

#ifdef __cplusplus
}

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
#include <filesystem>
#include <functional>

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
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Linker/IRMover.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include "codegen/devDebugger/devDebugger.hpp"
#include "tools/macros/debugMacros.h"

namespace Cryo
{
    class Linker
    {
    public:
        Linker();
        ~Linker();

        std::vector<llvm::Module *> dependencies;
        llvm::Module *rootModule;

        void setRootModule(llvm::Module *mod) { rootModule = mod; }

        int dependencyCount = 0;
        void setDependencyCount(int count) { dependencyCount = count; }

        std::string buildSrcDir;
        std::string dependencyDir = "out/deps/";

        // This should be putting the `buildSrcDir` + `dependencyDir` together once the `buildSrcDir` is set
        void setDependencyDir(std::string dir);

        llvm::Module *linkModules();
        void addRootModule(llvm::Module *module);
        void addModuleDependency(llvm::Module *module);
        void setBuildSrcDirectory(std::string srcDir) { buildSrcDir = srcDir; }
        void setBuildSrcDirectory(const char *srcDir) { buildSrcDir = srcDir; }

        void logState() const;

        // The C++ Linker Implementation
        void initDependencies(void);
        void newInitDependencies(llvm::Module *srcModule);

        const std::vector<std::string> scanDependenciesDir(void);
        void appendDependenciesToRoot(llvm::Module *root);
        void hoistDeclarations(llvm::Module *module);
        void contextMismatchMerge(llvm::Module *dest, llvm::Module *src);

    private:
        void createModulesFromDependencies(const std::vector<std::string> &deps);
        llvm::Module *compileAndMergeModule(std::string inputFile);
        void setModuleAndContextFromRoot(llvm::Module *root);

        bool contextMatch(llvm::Module *mod1, llvm::Module *mod2);
    };

    // C API Implementation
    inline CryoLinker CryoLinker_Create()
    {
        return reinterpret_cast<CryoLinker>(new Linker());
    }

    inline void CryoLinker_Destroy(CryoLinker linker)
    {
        delete reinterpret_cast<Linker *>(linker);
    }

    inline void CryoLinker_SetBuildSrcDirectory(CryoLinker linker, const char *srcDir)
    {
        reinterpret_cast<Linker *>(linker)->setBuildSrcDirectory(srcDir);
        // Set the dependency directory
        reinterpret_cast<Linker *>(linker)->setDependencyDir(srcDir);
    }

    inline void CryoLinker_AddRootModule(CryoLinker linker, void *module)
    {
        reinterpret_cast<Linker *>(linker)->addRootModule(static_cast<llvm::Module *>(module));
    }

    inline void CryoLinker_AddModuleDependency(CryoLinker linker, void *module)
    {
        reinterpret_cast<Linker *>(linker)->addModuleDependency(static_cast<llvm::Module *>(module));
    }

    inline void *CryoLinker_LinkModules(CryoLinker linker)
    {
        return reinterpret_cast<void *>(reinterpret_cast<Linker *>(linker)->linkModules());
    }
} // namespace Cryo

#endif // __cplusplus
#endif // CRYO_LINKER_H
