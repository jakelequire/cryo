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
#ifndef CODEGEN_HPP
#define CODEGEN_HPP
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

#include "frontend/AST.h"
#include "codegen/moduleContext.hpp"
#include "codegen/IRGenerator.hpp"
#include "codegen/generation/genUtilities.hpp"
#include "codegen/devDebugger/devDebugger.hpp"
#include "codegen/IRSymTable/IRSymTable.hpp"

namespace Cryo
{
    class CodeGen : public IRGenerator
    {
    public:
        CodeGen(ModuleContext &context) : context(context) {}
        ~CodeGen() = default;

        // Unimplemented
        void initCodeGen();
        // Unimplemented
        void initCodeGen(const std::string &moduleName);
        // Unimplemented
        void initCodeGen(const std::string &moduleName, const std::string &outputDir);
        // -----------------------------------------------------------------------------------------------

        GenUtilities &getGenUtils() { return *genUtils; }
        IRSymTable &getIRSymTable() { return *irSymTable; }

        int preprocess(ASTNode *root);
        void generateModuleFromAST(ASTNode *root);

    private:
        ModuleContext &context;
        std::unique_ptr<GenUtilities> genUtils;
        std::unique_ptr<IRSymTable> irSymTable;

        void recursiveASTTraversal(ASTNode *root);
    };
}

#endif // CODEGEN_HPP
