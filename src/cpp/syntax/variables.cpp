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
#include "cpp/codegen.h"

namespace Cryo {

void CryoSyntax::generateVarDeclaration(ASTNode* node) {}
llvm::Value* CryoSyntax::lookupVariable(char* name) {}
llvm::Value* CryoSyntax::createVariableDeclaration(ASTNode* node) {}
llvm::Value* CryoSyntax::getVariableValue(char* name) {}
llvm::GlobalVariable* CryoSyntax::createGlobalVariable(llvm::Type* varType, llvm::Constant* initialValue, char* varName) {}
llvm::Value* CryoSyntax::loadGlobalVariable(llvm::GlobalVariable* globalVar, char* name) {}

void CryoSyntax::generateArrayLiteral(ASTNode* node) {}
std::vector<llvm::Constant*> CryoSyntax::generateArrayElements(ASTNode* arrayLiteral) {}

}

