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
#ifndef OLD_CODEGEN_H
#define OLD_CODEGEN_H
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

#include "codegen/devDebugger/devDebugger.hpp"
#include "codegen/IRSymTable/IRSymTable.hpp"
#include "frontend/AST.h"
#include "common/common.h"
#include "tools/macros/printMacros.h"
#include "linker/linker.hpp"
#include "symbolTable/globalSymtable.hpp"
#include "symbolTable/IRSymbolTable.hpp"

namespace Cryo
{
    struct SymTable;

    class IRSymTable;
    class CodeGen;
    class CryoCompiler;
    class Generator;
    class CryoDebugger;
    class OldTypes;
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
    class ErrorHandler;
    class Classes;
    class Objects;

#define DUMP_COMPILER_STATE                            \
    CompilerState state = compiler.getCompilerState(); \
    dumpCompilerStateCXX(state)

#define DUMP_COMPILER_SYMBOL_TABLE                     \
    CompilerState state = compiler.getCompilerState(); \
    dumpSymbolTableCXX(state)

#define IR_SYMBOL_TABLE compiler.getContext().symbolTable
#define SYMBOL_MANAGER compiler.getContext().symbolTable->getSymbolManager()
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
        std::unique_ptr<IRSymbolTable> symbolTable; // This will be the new interface for the IR symbol table
        void initializeSymbolTable(void);

        std::unordered_map<std::string, llvm::Value *> namedValues;
        std::unordered_map<std::string, llvm::StructType *> structTypes = {};
        void printStructTypesMap(void);
        std::unordered_map<std::string, llvm::StructType *> classTypes = {};

        std::unordered_map<std::string, DataType *> structDataTypes;
        std::unordered_map<std::string, DataType *> classDataTypes;

        std::string currentNamespace;
        llvm::Function *currentFunction;

        std::vector<CompiledFile> compiledFiles;

        std::vector<llvm::Module *> *getModules() { return modules.get(); }

        bool inGlobalScope = true;

        std::unordered_map<std::string, llvm::GlobalVariable *> stringTable;
        size_t stringCounter = 0;

        void mergeModule(llvm::Module *srcModule);

        llvm::GlobalVariable *getOrCreateGlobalString(const std::string &content);
        void initializeContext();
        void setModuleIdentifier(std::string name);
        void addCompiledFileInfo(CompiledFile file);
        void addStructToInstance(std::string name, llvm::StructType *structType);
        llvm::StructType *getStruct(std::string name);
        void addClassToInstance(std::string name, llvm::StructType *classType);
        llvm::StructType *getClass(std::string name);
        void addStructDataType(std::string name, DataType *dataType);
        void addClassDataType(std::string name, DataType *dataType);

        void setCurrentFunction(llvm::Function *function);
        void clearCurrentFunction();

    private:
        CryoContext() : builder(context) {}
    };

    class CryoCompiler
    {
    public:
        CryoCompiler();
        ~CryoCompiler();

        void setCompilerState(CompilerState *state) { CryoContext::getInstance().state = state; }
        CompilerState *getCompilerState() { return CryoContext::getInstance().state; }

        void setCompilerSettings(CompilerSettings *settings) { CryoContext::getInstance().state->settings = settings; }
        CompilerSettings *getCompilerSettings() { return CryoContext::getInstance().state->settings; }

        CryoContext &getContext() { return CryoContext::getInstance(); }
        CodeGen &getCodeGen() { return *codeGen; }
        Generator &getGenerator() { return *generator; }
        OldTypes &getTypes() { return *types; }
        Variables &getVariables() { return *variables; }
        Arrays &getArrays() { return *arrays; }
        Functions &getFunctions() { return *functions; }
        IfStatements &getIfStatements() { return *ifStatements; }
        Declarations &getDeclarations() { return *declarations; }
        IRSymTable &getSymTable() { return *symTable; }
        BinaryExpressions &getBinaryExpressions() { return *binaryExpressions; }
        Loops &getLoops() { return *loops; }
        Structs &getStructs() { return *structs; }
        Imports &getImports() { return *imports; }
        WhileStatements &getWhileStatements() { return *whileStatements; }
        ErrorHandler &getErrorHandler() { return *errorHandler; }
        Classes &getClasses() { return *classes; }
        Objects &getObjects() { return *objects; }

        llvm::Module &getModule() { return *CryoContext::getInstance().module; }

        Linker *getLinker() { return GetCXXLinker(); }

        std::string outputFile = "";
        std::string buildDir = "";
        std::string customOutputPath = "";
        bool isPreprocessing = false;
        bool isImporting = false;

        void preInitMain(void);

        void compile(ASTNode *root);
        void compileIRFile(ASTNode *root, std::string outputPath);
        void dumpModule(void);

        void setBuildDir(std::string dir);
        void setOutputFile(std::string file);
        void setModuleIdentifier(std::string name);
        void setCustomOutputPath(std::string path);
        void setCustomOutputPath(std::string path, bool isImport);
        void setPreprocessOutputPath(std::string path);
        void initDependencies();
        void linkDependencies(void);

    private:
        CryoContext &context;
        std::unique_ptr<CodeGen> codeGen;
        std::unique_ptr<Generator> generator;
        std::unique_ptr<OldTypes> types;
        std::unique_ptr<Variables> variables;
        std::unique_ptr<Arrays> arrays;
        std::unique_ptr<Functions> functions;
        std::unique_ptr<IfStatements> ifStatements;
        std::unique_ptr<Declarations> declarations;
        std::unique_ptr<IRSymTable> symTable;
        std::unique_ptr<BinaryExpressions> binaryExpressions;
        std::unique_ptr<Loops> loops;
        std::unique_ptr<Structs> structs;
        std::unique_ptr<Imports> imports;
        std::unique_ptr<WhileStatements> whileStatements;
        std::unique_ptr<ErrorHandler> errorHandler;
        std::unique_ptr<Classes> classes;
        std::unique_ptr<Objects> objects;
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
        CodeGen(CryoCompiler &compiler) : compiler(compiler) {}

        /**
         * @brief Destructs the CodeGen object and cleans up the code generation process.
         */
        //~CodeGen() = default;

        void compileIRFile(ASTNode *root, std::string outputPath);

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
        void generateBlock(ASTNode *node);
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

    class OldTypes
    {
    public:
        OldTypes(CryoCompiler &compiler) : compiler(compiler) {}

        /**
         * @brief Returns the LLVM type for the given DataType *.
         */
        llvm::Type *getType(DataType *type, int length);

        /**
         * @brief Returns the LLVM type for the given LiteralNode.
         * This should replace the `getType` function for all literal nodes.
         */
        llvm::Type *getLiteralType(LiteralNode *literal);

        /**
         * @brief Returns the LLVM return type for the given DataType *.
         * Used for non-initalized variables.
         */
        llvm::Type *getReturnType(DataType *type);

        llvm::Type *getFunctionReturnType(DataType *returnType);

        llvm::Type *getIntegerTypeFromPrimitive(PrimitiveDataType type);

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
         * Note: This is a dangerous operation and should be used with caution.
         */
        llvm::Value *explicitTypeToPtr(llvm::Value *value);

        /**
         * @brief Mutate a values type to the given type.
         * Note: This is a dangerous operation and should be used with caution.
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

        llvm::Type *getClassType(DataType *type);

        /**
         * @brief Converts a DataType * that is a struct to an LLVM struct type.
         */
        llvm::Type *getStructType(DataType *type);

        /**
         * @brief Checks if a struct exists from the given name, returns the llvm::Type if it does.
         */
        llvm::Type *doesStructExist(std::string structName);

        /**
         * @brief Creates an alloca from a struct type.
         */
        llvm::Value *createAllocaFromStructProps(llvm::StructType *structType, llvm::Value *thisPtr);

        /**
         * @brief Checks if a type is a custom type.
         */
        bool isCustomType(DataType *type);

        /**
         * @brief Checks if a value is an LLVM string type.
         */
        bool isLLVMStringType(llvm::Value *val);

    private:
        CryoCompiler &compiler;

    protected:
        /**
         * @brief This only converts simple types (int, float, boolean, void) to LLVM types.
         * Does not cover strings, arrays, or other complex types.
         */
        llvm::Type *convertSimpleType(DataType *type);

        /**
         * @brief Converts complex types (strings, arrays) to LLVM types.
         */
        llvm::Type *convertComplexType(DataType *types, int length);
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
        llvm::Value *createStructVariable(ASTNode *varDecl);
        llvm::Value *getStructFieldValue(const std::string &structVarName,
                                         const std::string &fieldName);
        llvm::Value *createStringIndexExpr(IndexExprNode *indexExprNode, std::string varName);

        void processConstVariable(ASTNode *node);
        void createMutableVariable(ASTNode *node);

        llvm::Value *createStringVariable(ASTNode *node);

        // DEBUGGING PURPOSES
        llvm::Value *createTestGlobalVariable(void);

    private:
        // Specialized variable creation functions
        llvm::Value *createLiteralExprVariable(LiteralNode *literalNode, std::string varName, DataType *type);
        llvm::Value *createVarNameInitializer(VariableNameNode *varNameNode, std::string varName, std::string refVarName);
        llvm::Value *createArrayLiteralInitializer(CryoArrayNode *arrayNode, DataType *dataType, std::string varName);
        llvm::Value *createIndexExprInitializer(IndexExprNode *indexExprNode, CryoNodeType nodeType, std::string varName);

        llvm::Value *createVarWithFuncCallInitilizer(ASTNode *node);
        llvm::Value *createVarWithBinOpInitilizer(ASTNode *node, std::string varName);
        llvm::Value *createMethodCallVariable(MethodCallNode *node, std::string varName, DataType *varType);
        llvm::Value *createPropertyAccessVariable(PropertyAccessNode *propAccessNode, std::string varName, DataType *varType);
        llvm::Value *createObjectInstanceVariable(ObjectNode *objectNode, std::string varName, DataType *varType);

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
        llvm::Value *getArrayLength(std::string arrayName);

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

        llvm::Value *createParamFromParamNode(ASTNode *paramNode);
        llvm::Value *createArgumentVar(ASTNode *node);

        void handleFunction(ASTNode *node);
        void handleMethodCall(ASTNode *node);
        void handleStaticMethodCall(ASTNode *node);

        llvm::Value *createPropertyAccessCall(PropertyAccessNode *propAccess);

        llvm::Function *getFunction(std::string functionName);
        llvm::Function *findClassMethod(std::string className, std::string methodName);
        bool doesExternFunctionExist(std::string functionName);

    private:
        CryoCompiler &compiler;

        void createFunctionDeclaration(ASTNode *node);
        void createFunctionBlock(ASTNode *node);
        void createReturnStatement(ASTNode *node);
        void createExternFunction(ASTNode *node);

        llvm::Type *traverseBlockReturnType(CryoFunctionBlock *blockNode);

        llvm::Value *anyTypeParam(std::string functionName, llvm::Value *argValue);
        llvm::Value *createArgCast(llvm::Value *argValue, llvm::Type *expectedType);
        std::vector<llvm::Value *> verifyCalleeArguments(llvm::Function *callee, const std::vector<llvm::Value *> &argValues);

        // -----------------------------------
        // Function Calls

        void createScopedFunctionCall(ASTNode *node);

        llvm::Value *createMethodCall(MethodCallNode *node);
        llvm::Value *createPropertyCall(PropertyNode *property);
        llvm::Value *createVarNameCall(VariableNameNode *varNameNode);
        llvm::Value *createLiteralCall(LiteralNode *literalNode);
        llvm::Value *createVarDeclCall(CryoVariableNode *varDeclNode);
        llvm::Value *createFunctionCallCall(FunctionCallNode *functionCallNode);
        llvm::Value *createIndexExprCall(IndexExprNode *indexNode);
        llvm::Value *createArrayCall(CryoArrayNode *arrayNode);
        llvm::Value *createTypeofCall(TypeofNode *node);
        llvm::Value *createUnaryExprCall(CryoUnaryOpNode *node);

        // -----------------------------------
        // Cryo entry point functions

        void createCryoMainFunction(ASTNode *node);
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
        llvm::Value *createComparisonExpression(ASTNode *left, ASTNode *right, CryoOperatorType op, llvm::BasicBlock *ifBlock);
        llvm::Value *handleComplexBinOp(ASTNode *node);
        llvm::Value *dereferenceElPointer(llvm::Value *value, std::string varName = "unknown");
        llvm::Value *createStringBinOpInitializer(ASTNode *lhs, ASTNode *rhs, CryoOperatorType op, std::string varName);
        llvm::Value *createStringConcatenation(llvm::Value *leftValue, llvm::Value *rightValue, std::string varName);
        bool isStringBinOp(ASTNode *binOpNode);

    private:
        CryoCompiler &compiler;

        bool isStringOperation(ASTNode *lhs, ASTNode *rhs);
        bool isStringLiteral(ASTNode *node);
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

        void handleMethod(ASTNode *methodNode, const std::string &structName, llvm::StructType *structType);
        void handleStructConstructor(StructNode *node, llvm::StructType *structType);
        llvm::Value *createStructInstance(ASTNode *node);
        llvm::Type *findExistingStruct(const std::string &structName);

    private:
        CryoCompiler &compiler;

        void callConstructor(const std::string &structName, llvm::Value *structPtr, llvm::Value *initValue);
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

    class ErrorHandler
    {
    public:
        ErrorHandler(CryoCompiler &compiler) : compiler(compiler),
                                               sharedOutOfBoundsBlock(nullptr),
                                               sharedErrorFunc(nullptr),
                                               isInitialized(false) {}

        void IsOutOfBoundsException(llvm::Value *index, std::string arrayName, llvm::Type *arrayType);
        llvm::BasicBlock *getOrCreateErrorBlock(llvm::Function *function);

    private:
        CryoCompiler &compiler;
        llvm::BasicBlock *sharedOutOfBoundsBlock;
        llvm::Function *sharedErrorFunc;
        bool isInitialized;
        std::unordered_map<llvm::Function *, llvm::BasicBlock *> functionErrorBlocks;

        void initializeErrorBlocks();
    };

    // -----------------------------------------------------------------------------------------------

    class Classes
    {
    public:
        Classes(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Handles class declarations in the AST.
         */
        void handleClassDeclaration(ASTNode *node);

        std::vector<llvm::Type *> handleFieldDeclarations(ASTNode *classNode, PrivateMembers *privateMembers, PublicMembers *publicMembers, ProtectedMembers *protectedMembers);
        llvm::Type *getClassFieldType(ASTNode *property);
        void handleClassConstructor(ClassConstructorNode *ctorNode, llvm::StructType *structType);
        void handleClassMethods(ASTNode *node, std::string className, llvm::StructType *classType);
        void createClassMethod(ASTNode *methodNode, llvm::StructType *classType);
        void createParameterVar(ASTNode **paramNode, llvm::Function *function);
        void addParametersToSymTable(ASTNode *paramNode, std::string paramName,
                                     llvm::Value *llvmValue, llvm::Type *llvmType, llvm::StoreInst *storeInst);

    private:
        CryoCompiler &compiler;
    };

    class Objects
    {
    public:
        Objects(CryoCompiler &compiler) : compiler(compiler) {}

        // Prototypes

        /**
         * @brief Handles object declarations in the AST.
         */
        void handleObjectDeclaration(ASTNode *node);

        llvm::Value *createObjectInstance(ASTNode *node);
        llvm::Value *createObjectInstance(ObjectNode *objectNode, std::string varName);

        llvm::Value *createObjectMethodCall(ASTNode *node);
        llvm::Value *createObjectPropertyCall(ASTNode *node);

    private:
        CryoCompiler &compiler;
    };

    // -----------------------------------------------------------------------------------------------
    inline CryoCompiler::CryoCompiler()
        : context(CryoContext::getInstance()),
          codeGen(std::make_unique<CodeGen>(*this)),
          generator(std::make_unique<Generator>(*this)),
          types(std::make_unique<OldTypes>(*this)),
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
          errorHandler(std::make_unique<ErrorHandler>(*this)),
          classes(std::make_unique<Classes>(*this)),
          objects(std::make_unique<Objects>(*this)),
          symTable(std::make_unique<IRSymTable>())
    {
        context.initializeContext();
    }

    inline CryoCompiler::~CryoCompiler()
    {
        // Clean up the compiler
    }

    inline void CryoCompiler::compile(ASTNode *root)
    {
        codeGen->executeCodeGeneration(root);
    }

    inline void CryoCompiler::compileIRFile(ASTNode *root, std::string outputPath)
    {
        codeGen->compileIRFile(root, outputPath);
    }

    inline void CryoCompiler::dumpModule(void)
    {
        std::cout << "\n";
        std::cout << BOLD;
        std::cout << "═══════════════════════════ Module IR Dump ═══════════════════════════" << std::endl;
        context.module->print(llvm::outs(), nullptr);
        std::cout << "══════════════════════════════════════════════════════════════════════" << std::endl;
        std::cout << COLOR_RESET;
        std::cout << "\n";

        // Call the Compilation class to output the current module to a file
        // Compilation compileCode = Compilation(*this);
        // compileCode.DumpModuleToDebugFile();
    }

} // namespace Cryo

#endif // OLD_CODEGEN_H
