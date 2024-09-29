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
#include <stdint.h>

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

#include "cpp/debugger.h"
#include "compiler/ast.h"
#include "cpp/backend_symtable.h"
#include "common/common.h"

namespace Cryo
{
    struct SymTable;       // Forward declaration
    class BackendSymTable; // Forward declaration if needed

    class CodeGen;
    class CryoCompiler;
    class Generator;
    class CryoDebugger;
    class Types;
    class Variables;
    class Literals;
    class Arrays;
    class Functions;
    class IfStatements;
    class Declarations;
    class Loops;
    class BinaryExpressions;
    class Structs;

#define DUMP_COMPILER_STATE                            \
    CompilerState state = compiler.getCompilerState(); \
    dumpCompilerStateCXX(state)

#define DUMP_COMPILER_SYMBOL_TABLE                     \
    CompilerState state = compiler.getCompilerState(); \
    dumpSymbolTableCXX(state)

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
        CompilerState state;
        std::unordered_map<std::string, llvm::Value *> namedValues;
        std::unordered_map<std::string, llvm::StructType *> structTypes;
        std::string currentNamespace;
        // Current Function
        llvm::Function *currentFunction;
        bool inGlobalScope = true;

        void initializeContext()
        {
            module = std::make_unique<llvm::Module>("main", context);
            module->setDataLayout("e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128");
            std::cout << "[CPP.h] Module Initialized" << std::endl;
        }

    private:
        CryoContext() : builder(context) {}
    };

    class CryoCompiler
    {
    public:
        CryoCompiler();
        ~CryoCompiler() = default;

        void setCompilerState(CompilerState state) { CryoContext::getInstance().state = state; }
        CompilerState getCompilerState() { return CryoContext::getInstance().state; }

        CryoContext &getContext() { return CryoContext::getInstance(); }
        CodeGen &getCodeGen() { return *codeGen; }
        Generator &getGenerator() { return *generator; }
        CryoDebugger &getDebugger() { return *debugger; }
        Types &getTypes() { return *types; }
        Variables &getVariables() { return *variables; }
        Arrays &getArrays() { return *arrays; }
        Functions &getFunctions() { return *functions; }
        IfStatements &getIfStatements() { return *ifStatements; }
        Declarations &getDeclarations() { return *declarations; }
        BackendSymTable &getSymTable() { return *symTable; }
        BinaryExpressions &getBinaryExpressions() { return *binaryExpressions; }
        Loops &getLoops() { return *loops; }
        Structs &getStructs() { return *structs; }

        void compile(ASTNode *root);
        void dumpModule(void);

    private:
        CryoContext &context;
        std::unique_ptr<CryoDebugger> debugger;
        std::unique_ptr<CodeGen> codeGen;
        std::unique_ptr<Generator> generator;
        std::unique_ptr<Types> types;
        std::unique_ptr<Variables> variables;
        std::unique_ptr<Arrays> arrays;
        std::unique_ptr<Functions> functions;
        std::unique_ptr<IfStatements> ifStatements;
        std::unique_ptr<Declarations> declarations;
        std::unique_ptr<BackendSymTable> symTable;
        std::unique_ptr<BinaryExpressions> binaryExpressions;
        std::unique_ptr<Loops> loops;
        std::unique_ptr<Structs> structs;
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
        llvm::Value *getNamedValue(std::string name);
        std::string getNamespace(ASTNode *node);
        void printCurrentNamedValues(void);

        // Actual logic for handling different types of nodes
        // This is the main entry point for the generator

        void handleProgram(ASTNode *node);
        llvm::Value *handleLiteralExpression(ASTNode *node);

        void handleExternFunction(ASTNode *node);
        void handleFunctionDeclaration(ASTNode *node);
        void handleFunctionBlock(ASTNode *node);
        void handleReturnStatement(ASTNode *node);
        void handleFunctionCall(ASTNode *node);
        void handleVariableDeclaration(ASTNode *node);
        void handleBinaryExpression(ASTNode *node);
        void handleUnaryExpression(ASTNode *node);
        void handleIfStatement(ASTNode *node);
        void handleWhileStatement(ASTNode *node);
        void handleForStatement(ASTNode *node);
        void handleReassignment(ASTNode *node);
        void handleParam(ASTNode *node);
        void handleStruct(ASTNode *node);

    private:
        CryoCompiler &compiler;
        void preprocess(ASTNode *root);
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

        /**
         * @brief Returns the integer value of a literal node.
         */
        int getLiteralIntValue(LiteralNode *node);

        /**
         * @brief Returns the LLVM integer constant value of an integer.
         */
        llvm::ConstantInt *getLiteralIntValue(int value);

        /**
         * @brief Returns the LLVM string constant value of a string.
         */
        llvm::Constant *getLiteralStringValue(std::string value);

        /**
         * @brief Mutates a value to a pointer to the explicit type.
         */
        llvm::Value *ptrToExplicitType(llvm::Value *value);

        /**
         * @brief Mutates an explicit type to a pointer.
         */
        llvm::Value *explicitTypeToPtr(llvm::Value *value);

        /**
         * @brief Mutate a values type to the given type.
         */
        llvm::Value *castTyToVal(llvm::Value *val, llvm::Type *ty);

        /**
         * @brief Trims the quotes from a string.
         */
        std::string trimStrQuotes(std::string str);

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
        void handleMutableVariable(ASTNode *node);
        void handleVariableReassignment(ASTNode *node);
        llvm::Value *createLocalVariable(ASTNode *node);
        llvm::Value *getVariable(std::string name);
        llvm::Value *getLocalScopedVariable(std::string name);
        llvm::Value *createVarWithFuncCallInitilizer(ASTNode *node);

        void processConstVariable(CryoVariableNode *varNode);
        void createMutableVariable(ASTNode *node);

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
        llvm::Value *createArrayLiteral(ASTNode *node, std::string varName = "array");
        void handleArrayLiteral(ASTNode *node);
        llvm::ArrayType *getArrayType(ASTNode *node);
        int getArrayLength(ASTNode *node);
        void handleIndexExpression(ASTNode *node, std::string varName);
        llvm::Value *indexArrayForValue(ASTNode *array, int index);
        llvm::Value *getIndexExpressionValue(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Functions
    {
    public:
        Functions(CryoCompiler &compiler) : compiler(compiler) {}
        llvm::Value *createParameter(llvm::Argument *param, llvm::Type *argTypes);
        llvm::Value *createFunctionCall(ASTNode *node);

        void handleFunction(ASTNode *node);

    private:
        CryoCompiler &compiler;

        void createFunctionDeclaration(ASTNode *node);
        void createFunctionBlock(ASTNode *node);
        void createReturnStatement(ASTNode *node);
        void createExternFunction(ASTNode *node);
        llvm::Type *traverseBlockReturnType(CryoFunctionBlock *blockNode);
    };

    // -----------------------------------------------------------------------------------------------
    class IfStatements
    {
    public:
        IfStatements(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief The main entry point to handle if statements.
         */
        void handleIfStatement(ASTNode *node);

    private:
        CryoCompiler &compiler;

        llvm::BasicBlock *createIfStatement(ASTNode *node);
        std::pair<llvm::BasicBlock *, llvm::Value *> createIfCondition(ASTNode *node);
    };

    // -----------------------------------------------------------------------------------------------
    class Declarations
    {
    public:
        Declarations(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Declares all functions in the AST tree and hoists them to the top of the IR.
         */
        void preprocessDeclare(ASTNode *root);

    private:
        CryoCompiler &compiler;

        /**
         * @brief Creates a function declaration in the IR.
         */
        void createFunctionDeclaration(ASTNode *node);
    };
    // -----------------------------------------------------------------------------------------------

    class Loops
    {
    public:
        Loops(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief The main entry point to handle while loops.
         */
        void handleForLoop(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class BinaryExpressions
    {
    public:
        BinaryExpressions(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Handles binary expressions in the AST.
         */
        void handleBinaryExpression(ASTNode *node);

        /**
         * @brief Creates a binary expression in the IR.
         */
        llvm::Value *createBinaryExpression(ASTNode *node, llvm::Value *leftValue, llvm::Value *rightValue);

        llvm::Value *createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op);

        llvm::Value *handleComplexBinOp(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------

    class Structs
    {
    public:
        Structs(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Handles struct declarations in the AST.
         */
        void handleStructDeclaration(ASTNode *node);

        /**
         * @brief Returns the LLVM type for the given property.
         */
        llvm::Type *getStructFieldType(PropertyNode *property);

    private:
        CryoCompiler &compiler;
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
          functions(std::make_unique<Functions>(*this)),
          ifStatements(std::make_unique<IfStatements>(*this)),
          declarations(std::make_unique<Declarations>(*this)),
          loops(std::make_unique<Loops>(*this)),
          binaryExpressions(std::make_unique<BinaryExpressions>(*this)),
          structs(std::make_unique<Structs>(*this)),
          symTable(std::make_unique<BackendSymTable>())

    {
        context.initializeContext();
    }

    inline void CryoCompiler::compile(ASTNode *root)
    {
        codeGen->executeCodeGeneration(root);
    }

    inline void CryoCompiler::dumpModule(void)
    {
        context.module->print(llvm::outs(), nullptr);
    }
}

#endif // SANDBOX_H
