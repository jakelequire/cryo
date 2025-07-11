/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "tools/macros/printMacros.h"
#include "linker/linker.hpp"
#include "codegen/visitor.hpp"
#include "codegen/symTable/IRSymbolTable.hpp"
#include "tools/logger/logger_config.h"

namespace Cryo
{
#define IR_SYMBOL_TABLE compiler.getContext().symbolTable
#define SYMBOL_MANAGER compiler.getContext().symbolTable->getSymbolManager()
#define GetCXXLinker() reinterpret_cast<Cryo::Linker *>(globalLinker)

#define ASSERT_NODE_VOID_RET(node)                           \
    if (!node)                                               \
    {                                                        \
        logMessage(LMI, "ERROR", "Codegen", "Node is null"); \
        return;                                              \
    }
#define ASSERT_NODE_NULLPTR_RET(node)                        \
    if (!node)                                               \
    {                                                        \
        logMessage(LMI, "ERROR", "Codegen", "Node is null"); \
        return nullptr;                                      \
    }
    class CodegenContext;
    class IRSymbolTable;
    class CodeGenVisitor;
    class IRGeneration;
    class Initializer;

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

        friend class CodeGenVisitor;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<CodeGenVisitor> visitor;
        std::unique_ptr<IRSymbolTable> symbolTable;
        std::unique_ptr<Initializer> initializer;

        Linker *getLinker() { return GetCXXLinker(); }

        void printModule(void);
        void preInitMain(void);

        // Context Interface
        llvm::Function *currentFunction;

        void mergeModule(llvm::Module *srcModule);

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
        friend class Initializer;

    public:
        IRGeneration(CodegenContext &context) : context(context) {}
        ~IRGeneration() {}

        std::string buildDir = "";
        void setBuildDir(std::string dir) { buildDir = dir; }

        void generateIR(ASTNode *root);
        void completeGeneration(void);
        int finalize(void);

    private:
        CodegenContext &context;

        void processDeclarations(ASTNode *root);
        void processFunctionDeclaration(ASTNode *node);
        void processTypeDeclaration(ASTNode *node);

        void generateIRForNode(ASTNode *node);
    };

    // ======================================================================== //
    //                            Initializers                                  //
    // ======================================================================== //

    class Initializer
    {
    public:
        Initializer(CodegenContext &context) : context(context) {}
        ~Initializer() {}

        llvm::Value *getInitializerValue(ASTNode *node);
        llvm::Value *generateLiteralObject(ASTNode *node);
        llvm::Value *generateObjectInst(llvm::Value *varVal, ASTNode *node);
        void generateConstructorBody(ASTNode *node, llvm::Value *selfArg, llvm::Function *ctor);

        llvm::Value *generateStringInitializer(ASTNode *node);
        llvm::Value *generateArrayLiteral(ASTNode *node);

        llvm::Value *generateLiteralExpr(ASTNode *node);
        llvm::Value *generateVarName(ASTNode *node);
        llvm::Value *generateBinaryExpr(ASTNode *node);
        llvm::Value *generateFunctionCall(ASTNode *node);
        llvm::Value *generateReturnStatement(ASTNode *node);
        llvm::Value *generateUnaryExpr(ASTNode *node);
        llvm::Value *generateParam(ASTNode *node);
        llvm::Value *generatePropertyAccess(ASTNode *node);
        llvm::Value *generateScopedFunctionCall(ASTNode *node);
        llvm::Value *generateVarDeclaration(ASTNode *node);
        llvm::Value *generateObjectInst(ASTNode *node);
        llvm::Value *generateIndexExpr(ASTNode *node);

        llvm::Value *generateStringLiteral(ASTNode *node);
        llvm::Value *createIntegerLiteral(unsigned int numBits, uint64_t val, bool isSigned = false);
        llvm::Value *generateNullLiteral();

        std::vector<llvm::Value *> processArguments(
            ASTNode **args,
            int argCount,
            DataType **expectedParamTypes,
            const std::string &functionName);
        void processDeclarationParameters(
            llvm::Function *function,
            ASTNode **params,
            int paramCount,
            const std::string &functionName);
        llvm::Value *processReturnValue(
            llvm::Value *result,
            DataType *returnType,
            const std::string &functionName);

    private:
        CodegenContext &context;
        bool isStructOrObjectDataType(DataType *dataType);

    protected:
        IRSymbolTable *getSymbolTable(void) { return context.getInstance().symbolTable.get(); }

    public:
        void generateStructConstructor(ASTNode *node, llvm::StructType *structType);
        llvm::Type *derefValueForType(llvm::Value *value);

        IRMethodSymbol *createClassMethod(const std::string &className, ASTNode *method, DataType *methodDataType);

    private: // Unary expression handler methods
        llvm::Value *handleAddressOf(llvm::Value *operand);
        llvm::Value *handleDereference(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *handleUnaryMinus(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *handleLogicalNot(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *handleIncrement(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *handleDecrement(llvm::Value *operand, ASTNode *operandNode);

        llvm::Type *determinePointeeType(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *loadPointerOperandIfNeeded(llvm::Value *operand, ASTNode *operandNode);
        llvm::Value *convertToBool(llvm::Value *value);
        llvm::Value *handleIncrementOrDecrement(llvm::Value *operand, ASTNode *operandNode, bool isIncrement);

    private: // Property access helper methods
        bool validatePropertyAccessNode(ASTNode *node);
        llvm::StructType *getObjectStructType(const std::string &objectTypeName);
        llvm::Value *getObjectInstance(ASTNode *objectNode);
        llvm::Type *getPropertyType(llvm::StructType *structType, int propertyIndex);
        llvm::Value *ensurePointerType(llvm::Value *objectInstance, llvm::Type *objectType);
        llvm::Value *accessProperty(
            llvm::Value *objectInstance,
            llvm::Type *objectType,
            int propertyIndex,
            llvm::Type *propertyType,
            const std::string &objectTypeName,
            const std::string &propertyName);

    private: // Literal expression helpers
        bool validateLiteralNode(ASTNode *node);
        llvm::Value *generatePrimitiveValue(ASTNode *node, PrimitiveDataType primitiveType);
        llvm::Value *createIntObjectFromLiteral(ASTNode *node, DataType *literalDataType);
        bool callConstructor(std::vector<llvm::Value *> ctorArg, const std::string &typeName);

    private: // Helper methods for binary expression generation
        llvm::Value *loadIfPointerToValue(llvm::Value *val, const char *name);
        std::pair<llvm::Value *, llvm::Value *> handlePointerArithmetic(
            llvm::Value *left, llvm::Value *right, CryoOperatorType opType, const char *opName);
        llvm::Value *handleNullComparison(llvm::Value *lhs, llvm::Value *rhs,
                                          CryoOperatorType opType, ASTNode *node);
        llvm::Value *handleArithmeticOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType);
        llvm::Value *handleBitwiseOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType);
        llvm::Value *handleComparisonOp(llvm::Value *lhs, llvm::Value *rhs, CryoOperatorType opType);
        llvm::Value *handlePointerArithmeticOp(llvm::Value *left, llvm::Value *right,
                                               CryoOperatorType opType);
    };

} // namespace Cryo

#endif // CODEGEN_H_
