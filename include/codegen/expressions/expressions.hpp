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
#ifndef LITERAL_EXPR_HPP
#define LITERAL_EXPR_HPP
#include <iostream>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"

#include "codegen/generation/codegen.hpp"

namespace llvm
{
    class Value;
}

namespace Cryo
{

    class Expression : public CodeGen
    {
    public:
        Expression();
        virtual ~Expression() = default;

        virtual llvm::Value *codegen() = 0;

    protected:
        // If you need to store a reference to a parent CodeGen object,
        // consider using a raw pointer or std::weak_ptr to avoid circular references
        CodeGen *parentGen;
    };

} // namespace Cryo

#endif // LITERAL_EXPR_HPP
