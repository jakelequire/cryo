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
#ifndef CRYO_LINKER_V2_H
#define CRYO_LINKER_V2_H

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
    void CryoLinker_SetRootDir(CryoLinker *linker, const char *rootDir);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Macros
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define CreateCryoLinker() CryoLinker_Create()
#define SetLinkerRootDir(linker, rootDir) CryoLinker_SetRootDir(linker, rootDir)

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

namespace Cryo
{
    class LinkerModule;
    class CryoLinker;

    class CryoLinker
    {
    public:
        CryoLinker() {}
        ~CryoLinker() {}

        std::string projectRoot;
        std::string buildDir;

        void setRootDir(const char *rootDir)
        {
            projectRoot = std::string(rootDir);
            buildDir = projectRoot + "/build";
        }

    private:
        std::unique_ptr<LinkerModule> linkerModule;
    };

    class LinkerModule
    {
    public:
        LinkerModule(LinkerModule const &) = delete;
        void operator=(LinkerModule const &) = delete;

        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> finalModule;

    private:
    };

    // ================================================================ //

} // namespace Cryo

#endif // __cplusplus
#endif // CRYO_LINKER_V2_H
