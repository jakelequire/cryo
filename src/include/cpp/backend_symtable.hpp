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

#include "cpp/codegen.hpp"
#include "cpp/debugger.hpp"
#include "compiler/ast.h"

namespace Cryo
{
    class CryoDebugger;

    struct StructValue
    {
        StructNode ASTStruct;
        llvm::StructType *LLVMStruct;
    };

    struct STVariable
    {
        llvm::Value *LLVMValue;
        llvm::Type *LLVMType;
        CryoVariableNode *ASTNode;
        CryoDataType dataType;
    };

    struct STFunction
    {
        llvm::Function *LLVMFunction;
        llvm::Type *LLVMReturnType;
        std::vector<llvm::Type *> LLVMParamTypes;
        FunctionDeclNode *ASTNode;
        CryoDataType returnType;
    };

    struct STExternFunction
    {
        llvm::Function *LLVMFunction;
        llvm::Type *LLVMReturnType;
        std::vector<llvm::Type *> LLVMParamTypes;
        ExternFunctionNode *ASTNode;
        CryoDataType returnType;
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
    } SymTableNode;

    // This will contain all the namespaces for the entire program
    typedef struct SymTable
    {
        std::unordered_map<std::string, SymTableNode> namespaces;
    } SymTable;

    // -----------------------------------------------------------------------------------------------

    class BackendSymTable
    {
    public:
        BackendSymTable() = default; // Add this line if it doesn't exist
        ~BackendSymTable() = default;

        // Properties
        SymTable symTable;
        CryoDebugger &getDebugger() { return *debugger; }

        // Prototypes
        void initSymTable(void);
        void initModule(ASTNode *root, std::string namespaceName);
        void traverseASTNode(ASTNode *node, SymTableNode &program);

        // Containers
        STVariable createVarContainer(CryoVariableNode *varNode);
        STFunction createFuncContainer(FunctionDeclNode *funcNode);
        STExternFunction createExternFuncContainer(ExternFunctionNode *externNode);

        // Getters
        ASTNode *getASTNode(std::string namespaceName, CryoNodeType nodeType, std::string nodeName);
        CryoVariableNode *getVariableNode(std::string namespaceName, std::string varName);
        SymTableNode getSymTableNode(std::string namespaceName);
        SymTable getSymTable();

        STVariable *getVariable(std::string namespaceName, std::string varName);

        // Setters
        void addStruct(std::string namespaceName, llvm::StructType *structTy, StructNode *structNode);
        void addVariable(std::string namespaceName, std::string varName, CryoVariableNode varNode);
        void addFunction(std::string namespaceName, std::string funcName, FunctionDeclNode funcNode);
        void addExternFunciton(std::string namespaceName, std::string funcName, ExternFunctionNode externNode);

        // Updates
        void updateVariableNode(std::string namespaceName, std::string varName, llvm::Value *llvmValue, llvm::Type *llvmType);
        void updateFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes);
        void updateExternFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes);

        // Debugging
        void printTable(std::string namespaceName);

    private:
        CryoDebugger *debugger;
        SymTableNode traverseModule(ASTNode *root, std::string namespaceName);
    };
}
#endif // BACKEND_SYMTABLE_H
