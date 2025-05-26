/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#ifndef CODEGEN_INSTANCES_HPP
#define CODEGEN_INSTANCES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "codegen/symTable/IRSymbolTable.hpp"
#include "codegen/codegen.hpp"
#include "codegen/codegenDebug.hpp"

typedef struct ASTNode ASTNode;

namespace Cryo
{

    class IRSymbolTable;
    class Visitor;
    class DefaultVisitor;
    class CodeGenVisitor;
    class CodegenContext;
    class Initializer;

    // This class is to create / manage instances of objects
    // It will be used to create instances of classes, structs, and enums
    // It will also be used to manage the lifetime of these instances
    class Instance
    {
    public:
        Instance(CodegenContext &context) : context(context) {}
        ~Instance() {}

        // Create an instance of a class or struct
        llvm::Value *createInstance(ASTNode *node);
        llvm::Value *callConstructor(ASTNode *structNode, llvm::Value *instance, ASTNode *constructorArgs);
        llvm::Value *convertToType(llvm::Value *value, llvm::Type *targetType);

    private:
        // Create an instance of a struct
        llvm::Value *createStructInstance(ASTNode *node);

    private:
        CodegenContext &context;
    };

} // namespace Cryo

#endif // CODEGEN_INSTANCES_HPP
