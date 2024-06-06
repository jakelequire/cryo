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
/*------ <includes> ------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*-------- <LLVM> --------*/
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Analysis.h>
/*---<custom_includes>---*/
#include "ast.h"
#include "ir.h"
#include "token.h"
/*---------<end>---------*/



/*-----<function_prototypes>-----*/
void initializeLLVM();
void finalizeLLVM();

LLVMValueRef generateBinaryExprLLVM(LLVMBuilderRef builder, ASTNode* node);
LLVMValueRef generateLLVM(LLVMBuilderRef builder, ASTNode* node);
void generateProgramLLVM(ASTNode* root, const char* filename);
/*-----<end_prototypes>-----*/


#endif // CODEGEN_H
