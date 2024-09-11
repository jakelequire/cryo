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
#ifndef CODEGEN_H
#define CODEGEN_H
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

#include "cpp/debugger.h"
#include "compiler/ast.h"

namespace Cryo
{
    class CodeGen;
    class CryoCompiler;
    class Generator;
    class CryoDebugger;
    class Types;
    class Variables;
    class Literals;
    class Arrays;
    class Functions;


    typedef struct VariableIR
    {
        llvm::Value *value;
        llvm::Type *type;
        std::string name;
        CryoDataType dataType;
    } VariableIR;

    typedef struct ExpressionIR
    {
        llvm::Value *value;
        llvm::Type *type;
        CryoDataType dataType;
    } ExpressionIR;

    typedef struct FunctionIR
    {
        llvm::Function *function;
        llvm::Type *returnType;
        std::vector<llvm::Type *> argTypes;
        std::vector<llvm::Value *> argValues;
        std::string name;
    } FunctionIR;

    /// -----------------------------------------------------------------------------------------------
    /**
     * @class CryoContext
     * @brief Manages the LLVM context, builder, and module for the Cryo compiler.
     */
    class CryoContext
    {
    public:
        static CryoContext &getInstance()
        {
            static CryoContext instance;
            return instance;
        }

        CryoContext(CryoContext const &) = delete;

        void operator=(CryoContext const &) = delete;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::unordered_map<std::string, llvm::Value *> namedValues;

        void initializeContext()
        {
            module = std::make_unique<llvm::Module>("main", context);
            std::cout << "[CPP.h] Module Initialized" << std::endl;
            module->setDataLayout("e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
            // namedValues = std::unordered_map<std::string, llvm::Value *>();

            std::cout << "[CPP.h] Context Initialized" << std::endl;
        }

    private:
        CryoContext() : builder(context) {}
    };

    class CryoCompiler
    {
    public:
        CryoCompiler();
        ~CryoCompiler() = default;

        CryoContext &getContext() { return CryoContext::getInstance(); }
        CodeGen &getCodeGen() { return *codeGen; }
        Generator &getGenerator() { return *generator; }
        CryoDebugger &getDebugger() { return *debugger; }
        Types &getTypes() { return *types; }
        Variables &getVariables() { return *variables; }
        Arrays &getArrays() { return *arrays; }
        Functions &getFunctions() { return *functions; }

        void compile(ASTNode *root);

    private:
        CryoContext &context;
        std::unique_ptr<CryoDebugger> debugger;
        std::unique_ptr<CodeGen> codeGen;
        std::unique_ptr<Generator> generator;
        std::unique_ptr<Types> types;
        std::unique_ptr<Variables> variables;
        std::unique_ptr<Arrays> arrays;
        std::unique_ptr<Functions> functions;
    };

    /**
     * @class CodeGen
     * @brief Responsible for generating LLVM Intermediate Representation (IR) from the abstract syntax tree (AST).
     */
    class CodeGen
    {
    public:
        /**
         * @brief Constructs a CodeGen object and initializes the code generation process.
         * @param context The context to be used during code generation.
         */
        CodeGen(CryoCompiler &compiler) : compiler(compiler)
        {
            std::cout << "[CPP.h] CodeGen constructor start" << std::endl;
            std::cout << "[CPP.h] CodeGen Initialized" << std::endl;
        }

        /**
         * @brief Destructs the CodeGen object and cleans up the code generation process.
         */
        //~CodeGen() = default;

        /**
         * @brief The Entry Point to the generation process.
         */
        void executeCodeGeneration(ASTNode *root);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Generator
    {
    public:
        Generator(CryoCompiler &compiler) : compiler(compiler) {}

        void generateCode(ASTNode *root);
        void parseTree(ASTNode *root);
        llvm::Value *getInitilizerValue(ASTNode *node);

        // Actual logic for handling different types of nodes
        // This is the main entry point for the generator

        void handleProgram(ASTNode *node);

        void handleExternFunction(ASTNode *node);
        void handleFunctionDeclaration(ASTNode *node);
        void handleFunctionBlock(ASTNode *node);
        void handleReturnStatement(ASTNode *node);
        void handleFunctionCall(ASTNode *node);
        void handleVariableDeclaration(ASTNode *node);
        void handleBinaryExpression(ASTNode *node);
        void handleUnaryExpression(ASTNode *node);
        llvm::Value* handleLiteralExpression(ASTNode *node);
        void handleIfStatement(ASTNode *node);
        void handleWhileStatement(ASTNode *node);
        void handleForStatement(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Types
    {
    public:
        Types(CryoCompiler &compiler) : compiler(compiler) {}

        /**
         * @brief Returns the LLVM type for the given CryoDataType.
         */
        llvm::Type *getType(CryoDataType type, int length);

        /**
         * @brief Returns the LLVM return type for the given CryoDataType.
         * Used for non-initalized variables.
         */
        llvm::Type *getReturnType(CryoDataType type);

        /**
         * @brief Returns the length of a string literal or Array type.
         * All other types return `0`.
         */
        int getLiteralValLength(ASTNode *node);

    private:
        CryoCompiler &compiler;

    protected:
        /**
         * @brief This only converts simple types (int, float, boolean, void) to LLVM types.
         * Does not cover strings, arrays, or other complex types.
         */
        llvm::Type *convertSimpleType(CryoDataType type);

        /**
         * @brief Converts complex types (strings, arrays) to LLVM types.
         */
        llvm::Type *convertComplexType(CryoDataType types, int length);
    };

    // -----------------------------------------------------------------------------------------------

    class Variables
    {
    public:
        Variables(CryoCompiler &compiler) : compiler(compiler) {}

        void handleConstVariable(ASTNode *node);
        void handleRefVariable(ASTNode *node);
        VariableIR *createNewLocalVariable(ASTNode* node);
        llvm::Value *getVariable(std::string name);

        void processConstVariable(CryoVariableNode *varNode);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Literals
    {
    public:
        Literals(CryoCompiler &compiler) : compiler(compiler) {}

        llvm::Value *createIntLiteral(int value);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Arrays
    {
    public:
        Arrays(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes
        void handleArrayLiteral(ASTNode *node);
        llvm::ArrayType *getArrayType(ASTNode *node);
        int getArrayLength(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Functions
    {
    public:
        Functions(CryoCompiler &compiler) : compiler(compiler) {}

        void handleFunction(ASTNode *node);

    private:
        CryoCompiler &compiler;

        void createFunctionDeclaration(ASTNode *node);
        void createFunctionBlock(ASTNode *node);
        void createReturnStatement(ASTNode *node);
        void createExternFunction(ASTNode* node);
        void createFunctionCall(ASTNode *node);
        llvm::Type* traverseBlockReturnType(CryoFunctionBlock* blockNode);
    };

    // -----------------------------------------------------------------------------------------------
    inline CryoCompiler::CryoCompiler()
        : context(CryoContext::getInstance()),
          debugger(std::make_unique<CryoDebugger>(context)),
          codeGen(std::make_unique<CodeGen>(*this)),
          generator(std::make_unique<Generator>(*this)),
          types(std::make_unique<Types>(*this)),
          variables(std::make_unique<Variables>(*this)),
          arrays(std::make_unique<Arrays>(*this)),
          functions(std::make_unique<Functions>(*this))

    {
        context.initializeContext();
    }

    inline void CryoCompiler::compile(ASTNode *root)
    {
        codeGen->executeCodeGeneration(root);
    }
}

#endif // SANDBOX_H
