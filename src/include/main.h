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
#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantics.h"
#include "codegen.h"
#include "cpp/cppmain.h"
// #include "ir.h"
// #include "codegen.h"
#include "utils/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

// #include <llvm-c/Core.h>
// #include <llvm-c/Analysis.h>
// #include <llvm-c/ExecutionEngine.h>
// #include <llvm-c/Target.h>
// #include <llvm-c/TargetMachine.h>

#ifdef __cplusplus
}
#endif

//char* readFile(const char* path);
int main(int argc, char* argv[]);


#endif // MAIN_H