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

#include "cpp/debugger.hpp"
#include "compiler/ast.h"
#include "cpp/backend_symtable.hpp"
#include "common/common.h"
#include "compiler.h"

extern "C" CompiledFile compileFile(const char *filePath, const char *compilerFlags);

namespace Cryo
{
    struct SymTable;

    class BackendSymTable;
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
    class Imports;
    class WhileStatements;

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

        CompilerState *state;

        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<std::vector<llvm::Module *>> modules;

        std::unordered_map<std::string, llvm::Value *> namedValues;
        std::unordered_map<std::string, llvm::StructType *> structTypes;

        std::string currentNamespace;
        llvm::Function *currentFunction;

        std::vector<CompiledFile> compiledFiles;

        std::vector<llvm::Module *> *getModules() { return modules.get(); }

        bool inGlobalScope = true;

        void initializeContext()
        {
            // Get the filename from the CompilerState
            std::string moduleName = "CryoModuleDefaulted";
            module = std::make_unique<llvm::Module>(moduleName, context);
            std::cout << "[CPP.h] Module Initialized" << std::endl;
        }

        void setModuleIdentifier(std::string name)
        {
            module->setModuleIdentifier(name);
            module->setSourceFileName(name);
        }

        void addCompiledFileInfo(CompiledFile file)
        {
            compiledFiles.push_back(file);
        }

    private:
        CryoContext() : builder(context) {}
    };

    class CryoCompiler
    {
    public:
        CryoCompiler();
        ~CryoCompiler() = default;

        void setCompilerState(CompilerState *state) { CryoContext::getInstance().state = state; }
        CompilerState *getCompilerState() { return CryoContext::getInstance().state; }

        void setCompilerSettings(CompilerSettings *settings) { CryoContext::getInstance().state->settings = settings; }
        CompilerSettings *getCompilerSettings() { return CryoContext::getInstance().state->settings; }

        CryoContext &getContext()                   { return CryoContext::getInstance(); }
        CodeGen &getCodeGen()                       { return *codeGen; }
        Generator &getGenerator()                   { return *generator; }
        CryoDebugger &getDebugger()                 { return *debugger; }
        Types &getTypes()                           { return *types; }
        Variables &getVariables()                   { return *variables; }
        Arrays &getArrays()                         { return *arrays; }
        Functions &getFunctions()                   { return *functions; }
        IfStatements &getIfStatements()             { return *ifStatements; }
        Declarations &getDeclarations()             { return *declarations; }
        BackendSymTable &getSymTable()              { return *symTable; }
        BinaryExpressions &getBinaryExpressions()   { return *binaryExpressions; }
        Loops &getLoops()                           { return *loops; }
        Structs &getStructs()                       { return *structs; }
        Imports &getImports()                       { return *imports; }
        WhileStatements &getWhileStatements()       { return *whileStatements; }

        llvm::Module &getModule() { return *CryoContext::getInstance().module; }

        void compile(ASTNode *root);
        void dumpModule(void);

        void setModuleIdentifier(std::string name)
        {
            CryoContext::getInstance().setModuleIdentifier(name);
        }

    private:
        CryoContext &context;
        std::unique_ptr<CryoDebugger>       debugger;
        std::unique_ptr<CodeGen>            codeGen;
        std::unique_ptr<Generator>          generator;
        std::unique_ptr<Types>              types;
        std::unique_ptr<Variables>          variables;
        std::unique_ptr<Arrays>             arrays;
        std::unique_ptr<Functions>          functions;
        std::unique_ptr<IfStatements>       ifStatements;
        std::unique_ptr<Declarations>       declarations;
        std::unique_ptr<BackendSymTable>    symTable;
        std::unique_ptr<BinaryExpressions>  binaryExpressions;
        std::unique_ptr<Loops>              loops;
        std::unique_ptr<Structs>            structs;
        std::unique_ptr<Imports>            imports;
        std::unique_ptr<WhileStatements>    whileStatements;
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
        llvm::Value *getLiteralValue(LiteralNode *literalNode);
        std::string formatString(std::string str);

        void handleImportStatement(ASTNode *node);
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
        void handleScopedFunctionCall(ASTNode *node);

        void addCommentToIR(const std::string &comment);

        // Function to add a no-op instruction for whitespace
        void addWhitespaceAfter(llvm::Instruction *Inst, llvm::IRBuilder<> &Builder)
        {
            llvm::MDNode *Node = llvm::MDNode::get(Inst->getContext(), llvm::MDString::get(Inst->getContext(), "whitespace"));
            Inst->setMetadata("whitespace", Node);
        }

        // Function to add whitespace after load and store operations
        void addWhitespaceAfterLoadStore(llvm::Module &M)
        {
            llvm::IRBuilder<> Builder(M.getContext());

            for (auto &F : M)
            {
                for (auto &BB : F)
                {
                    for (auto &I : BB)
                    {
                        if (llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::StoreInst>(I))
                        {
                            // addWhitespaceAfter(&I, Builder);
                        }
                    }
                }
            }
        }

    private:
        CryoCompiler &compiler;
        void preprocess(ASTNode *root);
    };

    class LoadStoreWhitespaceAnnotator : public llvm::AssemblyAnnotationWriter
    {
    public:
        void emitInstructionAnnot(const llvm::Instruction *I, llvm::formatted_raw_ostream &OS) override
        {
            if (I->getMetadata("whitespace"))
            {
                OS << "\n";
            }
        }
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
         * @brief Returns the LLVM type for the given LiteralNode.
         * This should replace the `getType` function for all literal nodes.
         */
        llvm::Type *getLiteralType(LiteralNode *literal);

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
        int getLiteralValLength(LiteralNode *node);

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
         * @brief Get the type of an instruction.
         */
        llvm::Type *getInstType(llvm::Value *val);

        /**
         * @brief Parse an instruction for its type.
         */
        llvm::Type *parseInstForType(llvm::Instruction *inst);

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

        void processConstVariable(CryoVariableNode *varNode);
        void createMutableVariable(ASTNode *node);

    private:
        // Specialized variable creation functions
        llvm::Value *createLiteralExprVariable(LiteralNode *literalNode, std::string varName);
        llvm::Value *createVarNameInitializer(VariableNameNode *varNameNode, std::string varName, std::string refVarName);
        llvm::Value *createArrayLiteralInitializer(CryoArrayNode *arrayNode, CryoDataType dataType, std::string varName);
        llvm::Value *createIndexExprInitializer(IndexExprNode *indexExprNode, CryoNodeType nodeType, std::string varName);
        llvm::Value *createVarWithFuncCallInitilizer(ASTNode *node);
        llvm::Value *createVarWithBinOpInitilizer(ASTNode *node, std::string varName);

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
        llvm::Value *createArrayLiteral(CryoArrayNode *array, std::string varName = "array");
        void handleArrayLiteral(ASTNode *node);
        llvm::ArrayType *getArrayType(ASTNode *node);
        int getArrayLength(ASTNode *node);
        void handleIndexExpression(ASTNode *node, std::string varName);
        llvm::Value *indexArrayForValue(ASTNode *array, int index);
        llvm::Value *getIndexExpressionValue(ASTNode *node);
        void isOutOfBoundsException(llvm::Value *array, llvm::Value *index);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    class Functions
    {
    public:
        Functions(CryoCompiler &compiler) : compiler(compiler) {}
        llvm::Value *createParameter(llvm::Argument *param, llvm::Type *argTypes, ASTNode *paramNode);
        llvm::Value *createFunctionCall(ASTNode *node);

        llvm::Value *createReturnNode(ASTNode *node);

        void handleFunction(ASTNode *node);

    private:
        CryoCompiler &compiler;

        void createFunctionDeclaration(ASTNode *node);
        void createFunctionBlock(ASTNode *node);
        void createReturnStatement(ASTNode *node);
        void createExternFunction(ASTNode *node);
        void createScopedFunctionCall(ASTNode *node);
        llvm::Type *traverseBlockReturnType(CryoFunctionBlock *blockNode);

        llvm::Value *createVarNameCall(VariableNameNode *varNameNode);
        llvm::Value *createLiteralCall(LiteralNode *literalNode);
        llvm::Value *createVarDeclCall(CryoVariableNode *varDeclNode);
        llvm::Value *createFunctionCallCall(FunctionCallNode *functionCallNode);
        llvm::Value *createIndexExprCall(IndexExprNode *indexNode);
        llvm::Value *createArrayCall(CryoArrayNode *arrayNode);

        llvm::Value *anyTypeParam(std::string functionName, llvm::Value *argValue);

        std::vector<llvm::Value *> verifyCalleeArguments(llvm::Function *callee, const std::vector<llvm::Value *> &argValues);
        llvm::Value *createArgCast(llvm::Value *argValue, llvm::Type *expectedType);
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

        llvm::Value *createTempValueForPointer(llvm::Value *value, std::string varName);
        llvm::Value *createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op);
        llvm::Value *handleComplexBinOp(ASTNode *node);
        llvm::Value *dereferenceElPointer(llvm::Value *value, std::string varName = "unknown");

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

    class Imports
    {
    public:
        Imports(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Handles import statements in the AST.
         */
        void handleImportStatement(ASTNode *node);

        /**
         * @brief Imports the Cryo Standard Library.
         */
        void importCryoSTD(std::string subModuleName);

        /**
         * @brief Finds the IR build file for the given file name.
         */
        std::string findIRBuildFile(std::string filePath);

    private:
        CryoCompiler &compiler;
    };
    // -----------------------------------------------------------------------------------------------

    class WhileStatements
    {
    public:
        WhileStatements(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief The main entry point to handle while loops.
         */
        void handleWhileLoop(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------

    class Compilation
    {
    public:
        Compilation(CryoCompiler &compiler) : compiler(compiler) {}

        void compileIRFile(void);

    private:
        CryoCompiler &compiler;

        std::string getErrorMessage(void);
        void isValidDir(std::string dirPath);
        void isValidFile(std::string filePath);
        void makeOutputDir(std::string dirPath);
        void compile(std::string inputFile, std::string outputPath);
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
          imports(std::make_unique<Imports>(*this)),
          whileStatements(std::make_unique<WhileStatements>(*this)),
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

    ///
    /// Just some macros for logging. Felt more cluttered to have them in the main code.
    ///

#define LLVM_MODULE_FAILED_MESSAGE_START                                                                           \
    std::cerr << "\n\n";                                                                                           \
    std::cerr << "<!> ========================================================================= <!>" << std::endl; \
    std::cerr << "<!> =======------------! LLVM Module Verification Failed !------------======= <!>" << std::endl; \
    std::cerr << "<!> ========================================================================= <!>" << std::endl; \
    std::cerr << "\n";

#define LLVM_MODULE_FAILED_MESSAGE_END                                                                             \
    std::cerr << "\n";                                                                                             \
    std::cerr << "<!> ========================================================================= <!>" << std::endl; \
    std::cerr << "<!> =======---------------! Module Verification FAILED !--------------======= <!>" << std::endl; \
    std::cerr << "<!> ========================================================================= <!>" << std::endl; \
    std::cerr << "\n";

#define LLVM_MODULE_ERROR_START                                                               \
    std::cerr << "\n";                                                                        \
    std::cerr << "<!> ********************************************************" << std::endl; \
    std::cerr << "<!> *** Error: Module Verification Failed, errors: " << std::endl;          \
    std::cerr << "<!> ********************************************************" << std::endl; \
    std::cerr << "\n";

#define LLVM_MODULE_ERROR_END                                                                 \
    std::cerr << "\n";                                                                        \
    std::cerr << "<!> ********************************************************" << std::endl; \
    std::cerr << "\n";

#define LLVM_MODULE_COMPLETE_START                                                                                \
    std::cout << "\n\n";                                                                                          \
    std::cout << "<*> ======================================================================== <*>" << std::endl; \
    std::cout << "<*> ==========--------------- LLVM Module Complete ---------------========== <*>" << std::endl; \
    std::cout << "<*> ======================================================================== <*>" << std::endl; \
    std::cout << "\n";

#define LLVM_MODULE_COMPLETE_END                                                                                  \
    std::cout << "\n";                                                                                            \
    std::cout << "<*> ======================================================================== <*>" << std::endl; \
    std::cout << "<*> ==========----------- Module Successfully Verified -----------========== <*>" << std::endl; \
    std::cout << "<*> ======================================================================== <*>" << std::endl; \
    std::cout << "\n";

}
#endif // SANDBOX_H
