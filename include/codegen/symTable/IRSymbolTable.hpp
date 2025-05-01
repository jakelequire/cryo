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
#ifndef IR_SYMBOL_TABLE_H
#define IR_SYMBOL_TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdbool.h>

#include "IRdefs.hpp"
#include "tools/macros/consoleColors.h"
#include "codegen/codegen.hpp"
#include "codegen/codegenDebug.hpp"

namespace Cryo
{
    class CodegenContext;

    struct GlobalSymbolTableInstance
    {

        std::unordered_map<std::string, llvm::Module *> modules;

        // Methods
        int setModule(const std::string &name, llvm::Module *module);
        llvm::Module *getModule(const std::string &name);
        std::vector<llvm::Module *> getModules();
        int getModuleCount();

        llvm::Function *getFunction(const std::string &name);
        IRFunctionSymbol *wrapLLVMFunction(llvm::Function *func);

        llvm::StructType *getStructType(const std::string &name);
        IRTypeSymbol *wrapLLVMStructType(llvm::StructType *type);
    };

    // Global symbol table instance
    extern GlobalSymbolTableInstance globalSymbolTableInstance;

    // This struct is to preload all the LLVM types that are used in the IRSymbolTable
    struct LLVMTypes
    {
        // Integer types
        llvm::Type *i1Ty;
        llvm::Type *i8Ty;
        llvm::Type *i16Ty;
        llvm::Type *i32Ty;
        llvm::Type *i64Ty;
        llvm::Type *i128Ty;

        // Floating point types
        llvm::Type *halfTy;
        llvm::Type *floatTy;
        llvm::Type *doubleTy;
        llvm::Type *fp128Ty;
        llvm::Type *x86_fp80;

        // Void type
        llvm::Type *voidTy;
        // Pointer type
        llvm::Type *ptrTy;
    };

    class IRSymbolTable
    {
    private:
        std::vector<std::unordered_map<std::string, IRVariableSymbol>> scopeStack;
        std::unordered_map<std::string, IRFunctionSymbol> functions;
        std::unordered_map<std::string, IRTypeSymbol> types;
        IRFunctionSymbol *currentFunction = nullptr; // Track current function scope
        llvm::BasicBlock *breakBlock = nullptr;      // Track break block
        llvm::BasicBlock *continueBlock = nullptr;   // Track continue block

    public:
        std::unordered_map<std::string, IRFunctionSymbol> getFunctions() const
        {
            return functions;
        }
        std::unordered_map<std::string, IRTypeSymbol> getTypes() const
        {
            return types;
        }

        llvm::BasicBlock *getBreakBlock() const
        {
            return breakBlock;
        }
        llvm::BasicBlock *getContinueBlock() const
        {
            return continueBlock;
        }
        void setBreakBlock(llvm::BasicBlock *block)
        {
            breakBlock = block;
        }
        void setContinueBlock(llvm::BasicBlock *block)
        {
            continueBlock = block;
        }

    public:
        explicit IRSymbolTable(CodegenContext &context) : context(context)
        {
            // Initialize with global scope
            scopeStack.push_back({});
            // Initialize the LLVM types
            initLLVMTypes();
        }

    private:
        CodegenContext &context;

    public:
        friend class IRSymbolManager;
        friend class CodeGenDebug;

        IRSymbolManager *getSymbolManager() { return new IRSymbolManager(); }
        void setCurrentFunction(IRFunctionSymbol *function) { currentFunction = function; }
        void clearCurrentFunction() { currentFunction = nullptr; }
        bool inConstructorInstance = false;
        // Global String Map
        std::unordered_map<std::string, llvm::Value *> globalStringMap;

        // Global symbol table instance
        GlobalSymbolTableInstance *getGlobalSymbolTableInstance()
        {
            return &globalSymbolTableInstance;
        }

        bool importModuleDefinitions(llvm::Module *sourceModule);

        // Core scope operations
        void pushScope();
        void popScope();

        void enterConstructorInstance() { inConstructorInstance = true; }
        void exitConstructorInstance() { inConstructorInstance = false; }

        void enterFunctionScope(const std::string &funcName);
        void exitFunctionScope();

        llvm::Value *getOrCreateGlobalString(const std::string &str);

        // Symbol management
        bool addVariable(const IRVariableSymbol &symbol);
        bool addFunction(const IRFunctionSymbol &symbol);
        bool addExternFunction(const IRFunctionSymbol &symbol);
        bool addType(const IRTypeSymbol &symbol);
        void removeType(const std::string &name);

        // Symbol lookup
        IRVariableSymbol *findVariable(const std::string &name);
        IRVariableSymbol *findLocalVariable(const std::string &name);

        // Helper method for creating global variables
        IRVariableSymbol *createGlobalVariable(const std::string &name, llvm::Type *type,
                                               llvm::Value *initialValue = nullptr);

        IRFunctionSymbol *findFunction(const std::string &name);
        IRTypeSymbol *findType(const std::string &name);
        llvm::StructType *getStructType(const std::string &name);

        void debugPrint() const;
        std::string IRTypeKindToString(IRTypeKind kind) const;

    private:
        llvm::StructType *getLLVMObjectType(DataType *dataType);
        llvm::StructType *getLLVMStructType(DataType *dataType);
        llvm::StructType *getLLVMClassType(DataType *dataType);
        llvm::FunctionType *getLLVMFunctionType(DataType *dataType);

    protected:
        bool typesInitialized = false;
        void initLLVMTypes();

    public:
        LLVMTypes llvmTypes;
        llvm::Type *getLLVMType(DataType *dataType);
        std::vector<llvm::Type *> getLLVMTypes(DataType **dataTypes);
        llvm::Type *derefencePointer(llvm::Type *type);

        llvm::StructType *getVA_ARGSType(DataType *dataType);

        llvm::Type *getLLVMPrimitiveType(DataType *dataType);
        llvm::Type *getLLVMArrayType(DataType *dataType);
    };

} // namespace Cryo

#endif // IR_SYMBOL_TABLE_H
