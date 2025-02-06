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
#ifndef BACKEND_SYMTABLE_H
#define BACKEND_SYMTABLE_H
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/GlobalVariable.h"

#include "codegen/devDebugger/devDebugger.hpp"
#include "frontend/AST.h"
#include "frontend/dataTypes.h"

namespace Cryo
{
    class DevDebugger;
    class ModuleContext;

    struct StructValue
    {
        StructNode *ASTStruct;
        llvm::StructType *LLVMStruct;
    };

    struct STVariable
    {
        llvm::Value *LLVMValue;
        llvm::Type *LLVMType;
        llvm::StoreInst *LLVMStoreInst;
        llvm::LoadInst *LLVMLoadInst;
        llvm::Value *LLVMFunctionPtr;
        ASTNode *ASTNode;
        CryoNodeType nodeType;
        DataType *dataType;
    };

    struct STParameter
    {
        llvm::Value *LLVMValue;
        llvm::Type *LLVMType;
        llvm::StoreInst *LLVMStoreInst;
        llvm::LoadInst *LLVMLoadInst;
        std::string paramName;
        std::string functionName;
        ASTNode *ASTNode;
        CryoNodeType nodeType;
        DataType *dataType;
    };

    struct STFunction
    {
        llvm::Function *LLVMFunction;
        llvm::Type *LLVMReturnType;
        std::vector<llvm::Type *> LLVMParamTypes;
        FunctionDeclNode *ASTNode;
        DataType *returnType;
    };

    struct STExternFunction
    {
        llvm::Function *LLVMFunction;
        llvm::Type *LLVMReturnType;
        std::vector<llvm::Type *> LLVMParamTypes;
        ExternFunctionNode *ASTNode;
        DataType *returnType;
    };

    struct STStruct
    {
        llvm::StructType *LLVMStruct;
        llvm::Function *LLVMConstructor;
        std::vector<llvm::Function *> LLVMMethods;
        StructNode *ASTNode;
        DataType *structType;
    };

    struct STClass
    {
        llvm::StructType *LLVMStruct;
        llvm::Function *LLVMConstructor;
        std::vector<llvm::Function *> LLVMMethods;
        ClassNode *ASTNode;
        DataType *classType;
    };

    // -----------------------------------------------------------------------------------------------

    // For each file, we will have a SymTable that contains all the variables and functions
    typedef struct SymTableNode
    {
        std::string namespaceName;
        std::unordered_map<std::string, CryoVariableNode> variables;
        std::unordered_map<std::string, CryoParameterNode> parameters;
        std::unordered_map<std::string, FunctionDeclNode> functions;
        std::unordered_map<std::string, ExternFunctionNode> externFunctions;

        // Migrate to these when ready
        std::unordered_map<std::string, StructValue> structs;
        std::unordered_map<std::string, STVariable> variableNode;
        std::unordered_map<std::string, STFunction> functionNode;
        std::unordered_map<std::string, STExternFunction> externFunctionNode;
        std::unordered_map<std::string, STParameter> parameterNode;
        std::unordered_map<std::string, STStruct> structNode;
        std::unordered_map<std::string, STClass> classNode;

    } SymTableNode;

    // This will contain all the namespaces for the entire program
    typedef struct SymTable
    {
        std::unordered_map<std::string, SymTableNode> namespaces;
    } SymTable;

    // -----------------------------------------------------------------------------------------------

    class IRSymTable
    {
    public:
        IRSymTable() = default;
        ~IRSymTable() = default;

        // Properties
        SymTable symTable;

        // Prototypes
        void initSymTable(void);
        void initModule(ASTNode *root, std::string namespaceName);
        void traverseASTNode(ASTNode *node, SymTableNode &program);
        void importExternalASTRoot(ASTNode *root);

        // Containers
        STVariable createVarContainer(ASTNode *varNode);
        STFunction createFuncContainer(FunctionDeclNode *funcNode);
        STExternFunction createExternFuncContainer(ExternFunctionNode *externNode);
        STParameter createParamContainer(void);
        STClass createClassContainer(ClassNode *classNode);

        // Getters
        SymTable getSymTable();
        ASTNode *getASTNode(std::string namespaceName, CryoNodeType nodeType, std::string nodeName);
        CryoVariableNode *getVariableNode(std::string namespaceName, std::string varName);
        SymTableNode getSymTableNode(std::string namespaceName);
        STVariable *getVariable(std::string namespaceName, std::string varName);
        STParameter *getParameter(std::string namespaceName, std::string paramName);
        STStruct *getStruct(std::string namespaceName, std::string structName);
        PropertyNode *getPropertyNode(std::string namespaceName, std::string propName, std::string structName);

        // Setters
        void addClass(std::string namespaceName, llvm::StructType *classTy, ClassNode *classNode, DataType *classType);
        void addStruct(std::string namespaceName, llvm::StructType *structTy, StructNode *structNode, DataType *structType);
        void addVariable(std::string namespaceName, std::string varName, ASTNode *varNode);
        void addFunction(std::string namespaceName, std::string funcName, FunctionDeclNode funcNode, llvm::Function *llvmFunction, llvm::Type *llvmReturnType);
        void addExternFunciton(std::string namespaceName, std::string funcName, ExternFunctionNode externNode);
        void addParameter(std::string namespaceName, std::string paramName, ASTNode *paramNode);
        void addValueToParam(std::string namespaceName, std::string paramName, llvm::Value *llvmValue);
        void addStructMethod(std::string namespaceName, std::string structName, llvm::Function *llvmFunction);

        // Updates
        void updateVariableNode(std::string namespaceName, std::string varName, llvm::Value *llvmValue, llvm::Type *llvmType);
        void addStoreInstToVar(std::string namespaceName, std::string varName, llvm::StoreInst *storeInst);
        void addLoadInstToVar(std::string namespaceName, std::string varName, llvm::LoadInst *loadInst);
        void addFunctionToVar(std::string namespaceName, std::string varName, llvm::Value *functionPtr);
        void addDataTypeToVar(std::string namespaceName, std::string varName, DataType *dataType);

        void addParamAsVariable(std::string namespaceName, std::string paramName, DataType *dataType, llvm::Value *llvmValue, llvm::Type *llvmType, llvm::StoreInst *storeInst);
        void updateFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes);
        void updateExternFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes);
        void updateParam(std::string namespaceName, std::string paramName, llvm::Value *llvmValue, llvm::Type *llvmType);
        void updateStructConstructor(std::string namespaceName, std::string structName, llvm::Function *llvmConstructor);

        // Creation
        void createNewStructDecl(std::string namespaceName, StructNode *structNode);

        // Debugging
        void printTable(std::string namespaceName);

    private:
        SymTableNode traverseModule(ASTNode *root, std::string namespaceName);
    };
}
#endif // BACKEND_SYMTABLE_H
