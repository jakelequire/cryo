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
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <assert.h>
#include <any>

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
#include <llvm/Support/raw_ostream.h>

#include "cpp/cppLogger.h"

extern "C" {
    #include "compiler/ast.h"
    #include "compiler/symtable.h"
}

namespace Cryo {
    
class CodeGen {
public:
    CodeGen(ASTNode* root);

private: 
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::unordered_map<std::string, llvm::Value*> namedValues;
    std::string LLVMTypeToString(llvm::Type* type);
    void codegen(ASTNode* root);
    void generateCode(ASTNode* node);
    void generateProgram(ASTNode* node);
    void generateStatement(ASTNode* node);
    void generateBlock(ASTNode* blockNode);
    llvm::Value* generateExpression(ASTNode* node);
    std::pair<llvm::Value*, bool> generateExpression(ASTNode* node, bool checkStringLiteral);
    llvm::Value* generateBinaryOperation(ASTNode* node);
    llvm::Type* getLLVMType(CryoDataType type, bool isReference);
    void generateFunctionPrototype(ASTNode* node);
    void createDefaultMainFunction();
    void generateFunctionCall(ASTNode* node);
    void generateFunction(ASTNode* node);
    void generateFunctionBlock(ASTNode* node);
    void generateReturnStatement(ASTNode* node);
    void generateExternalDeclaration(ASTNode* node);
    llvm::Function* getCryoFunction(char* name, llvm::ArrayRef<llvm::Type*> argTypes);
    bool declareFunctions(ASTNode* node);
    llvm::StructType *createStringStruct();
    llvm::StructType *createStringType();
    llvm::Value *createString(const std::string &str);
    llvm::Value* createNumber(int num);
    llvm::Constant* createConstantInt(int value);
    llvm::Value* createReferenceInt(int value);
    llvm::Value* getVariable(char* name);
    llvm::Value* createVariableDeclaration(ASTNode* node);
    llvm::Value* lookupVariable(char* name);
    llvm::GlobalVariable* createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName);
    llvm::Value* loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name);
    llvm::Value* loadPointerVariable(llvm::Value* var, char* name);
    llvm::Value* getVariableValue(char* name);
    void generateVarDeclaration(ASTNode* node);
    std::vector<llvm::Constant*> generateArrayElements(ASTNode* arrayLiteral);
    void generateCodeForArrayLiteral(ASTNode* node);
    void generateIfStatement(ASTNode* node);
    void generateForLoop(ASTNode* node);
};

} // namespace Cryo

#endif // CODEGEN_H
