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



using namespace llvm;

extern "C" {
    #include "compiler/ast.h"
}



// Code Generation
void generateCode(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateProgram(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateVarDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateFunction(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateBlock(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateFunctionBlock(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateReturnStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

// Variables
llvm::Value* getVariable(const std::string& name);
llvm::Value* getVariableValue(const std::string& name, llvm::IRBuilder<>& builder);
llvm::GlobalVariable* createGlobalVariable(llvm::Module& module, llvm::Type* varType, llvm::Constant* initialValue, const std::string& varName);
llvm::Value* loadGlobalVariable(llvm::GlobalVariable* globalVar, llvm::IRBuilder<>& builder, const std::string& name);
llvm::Value* loadPointerVariable(llvm::Value* var, llvm::IRBuilder<>& builder, const std::string& name);

// Expressions
llvm::Value* generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
std::pair<llvm::Value*, bool> generateExpression(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module, bool checkStringLiteral);
llvm::Value* generateBinaryOperation(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);


// Structs
llvm::Type* cryoTypeToLLVMType(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, const std::string& typeName);
llvm::StructType *createStringStruct(llvm::LLVMContext &context);
llvm::StructType *createStringType(llvm::LLVMContext &context, llvm::IRBuilder<> &builder);
llvm::Value *createString(llvm::IRBuilder<> &builder, llvm::Module &module, const std::string &str);
llvm::Value* createNumber(llvm::IRBuilder<>& builder, llvm::Module& module, int num);
llvm::Constant* createConstantInt(llvm::IRBuilder<>& builder, int value);
llvm::Value* createReferenceInt(llvm::IRBuilder<>& builder, llvm::Module& module, int value);
std::vector<llvm::Constant*> generateArrayElements(ASTNode* arrayLiteral, llvm::IRBuilder<>& builder, llvm::Module& module);

// Functions
void generateFunctionPrototype(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void createDefaultMainFunction(llvm::IRBuilder<>& builder, llvm::Module& module);
void generateFunctionCall(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateExternalDeclaration(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
llvm::Function* getCryoFunction(llvm::Module& module, const std::string& name, llvm::ArrayRef<llvm::Type*> argTypes);
void generateForLoop(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void generateIfStatement(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);

void generateCodeForArrayLiteral(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);



bool declareFunctions(ASTNode* node, llvm::IRBuilder<>& builder, llvm::Module& module);
void codegen(ASTNode* root);




#endif // CODEGEN_H