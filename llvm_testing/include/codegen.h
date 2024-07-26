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



extern "C" {
    #include "compiler/ast.h"
}

namespace Cryo {
    
    class CodeGen {
    public:
        /**
         * The Constructor for the CodeGen class (public)
         */
        CodeGen(ASTNode* root);
    
    private: 
        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        std::unique_ptr<llvm::Module> module;
        /**
         * This is (I think) where global variables are being loaded 
         */
        std::unordered_map<std::string, llvm::Value*> namedValues;

        /**
         * The main entry point for code generation. (private)
         */    
        void codegen(ASTNode* root);
    };

} // namespace Cryo







#endif