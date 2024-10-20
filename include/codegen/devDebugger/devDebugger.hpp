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
#ifndef CRYO_DEBUGGER
#define CRYO_DEBUGGER
#include <iomanip>
#include <iostream>
#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"

#include "frontend/AST.h"

namespace Cryo
{
#define VALIDATE_ASTNODE(node) checkNode(node)

    class DevDebugger
    {
    public:
        static void logNode(ASTNode *node);
        static void logMessage(const char *type, int line, const std::string &category, const std::string &message);

        static bool lintTree(ASTNode *node);
        static bool assertNode(ASTNode *node);
        static bool isValidNodeType(CryoNodeType type);
        static void logLLVMValue(llvm::Value *valueNode);
        static void logLLVMStruct(llvm::StructType *structTy);
        static void logLLVMType(llvm::Type *type);
        static void logLLVMInst(llvm::Instruction *inst);
        static std::string LLVMTypeIDToString(llvm::Type *type);

        // Macro Implementations
        static void checkNode(ASTNode *node);

    private:
        static bool isNodeTypeValid(ASTNode *node);

    protected:
    };

}
#endif // CRYO_DEBUGGER