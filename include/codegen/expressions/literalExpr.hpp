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

#include "codegen/moduleContext.hpp"
#include "codegen/expressions/expressions.hpp"

namespace Cryo
{
    class LiteralExpr : public Expression
    {
    public:
        LiteralExpr(ModuleContext &context);
        ~LiteralExpr() = default;

    protected:
        ModuleContext &context;
    };

} // namespace Cryos

#endif // LITERAL_EXPR_HPP
