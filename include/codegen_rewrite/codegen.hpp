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
#ifndef CODEGEN_H_
#define CODEGEN_H_
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

#include "visitor.hpp"
#include "codegen/devDebugger/devDebugger.hpp"
#include "frontend/AST.h"
#include "tools/macros/printMacros.h"
#include "linker/linker.hpp"
#include "codegen_rewrite/symTable/IRSymbolTable.hpp"
#include "tools/logger/logger_config.h"

namespace Cryo
{
#define IR_SYMBOL_TABLE compiler.getContext().symbolTable
#define SYMBOL_MANAGER compiler.getContext().symbolTable->getSymbolManager()
#define GetCXXLinker() reinterpret_cast<Cryo::Linker *>(globalLinker)

    class CodegenContext;
    class IRSymbolTable;

    // ======================================================================== //
    //                            Codegen Context                               //
    // ======================================================================== //

    class CodegenContext
    {
    public:
        static CodegenContext &getInstance()
        {
            static CodegenContext instance;
            return instance;
        }
        CodegenContext(CodegenContext const &) = delete;
        void operator=(CodegenContext const &) = delete;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<Cryo::CodeGenVisitor> visitor;
        std::unique_ptr<IRSymbolTable> symbolTable;

        Linker *getLinker() { return GetCXXLinker(); }

        void preInitMain(void);

        // Context Interface
        llvm::Function *currentFunction;

        void DONOTUSEYET_mergeModule(llvm::Module *srcModule);

        void initializeCodegenContext(void);
        void setModuleIdentifier(std::string name);

        void setCurrentFunction(llvm::Function *function);
        void clearCurrentFunction(void);

    private:
        CodegenContext() : builder(context) {}
    };

    // ======================================================================== //
    //                           IR Generation                                  //
    // ======================================================================== //

    class IRGeneration
    {
    public:
        IRGeneration(CodegenContext &context) : context(context) {}
        ~IRGeneration() {}

        void generateIR(ASTNode *root);

    private:
        CodegenContext &context;

        void processDeclarations(ASTNode *root);
        void processFunctionDeclaration(ASTNode *node);
        void processTypeDeclaration(ASTNode *node);

        void generateIRForNode(ASTNode *node);
        void generateIRForLiteralExpr(ASTNode *node);
        void generateIRForVarName(ASTNode *node);
        void generateIRForBinaryExpr(ASTNode *node);
        void generateIRForFunctionCall(ASTNode *node);
        void generateIRForReturnStatement(ASTNode *node);
        // Add other generation methods as needed...
    };

    // ======================================================================== //
    //                            Initializers                                  //
    // ======================================================================== //

    class Initilizer
    {
    public:
        Initilizer(CodegenContext &context) : context(context) {}
        ~Initilizer() {}

        llvm::Value *getInitilizerValue(ASTNode *node);

    private:
        CodegenContext &context;

        llvm::Value *generateLiteralExpr(ASTNode *node);
        llvm::Value *generateVarName(ASTNode *node);
        llvm::Value *generateBinaryExpr(ASTNode *node);
        llvm::Value *generateFunctionCall(ASTNode *node);
        llvm::Value *generateReturnStatement(ASTNode *node);
        // Add other generation methods as needed...
    };

} // namespace Cryo

#endif // CODEGEN_H_
