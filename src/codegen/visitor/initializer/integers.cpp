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
#include "codegen/codegen.hpp"

namespace Cryo
{

    llvm::Value *Initializer::createIntegerLiteral(unsigned int numBits, uint64_t val, bool isSigned)
    {
        logMessage(LMI, "INFO", "Initializer", "Creating integer literal: %llu", val);
        llvm::Type *intType = llvm::IntegerType::get(context.getInstance().context, numBits);
        llvm::ConstantInt *intLiteral = llvm::ConstantInt::get(
            llvm::IntegerType::get(context.getInstance().context, numBits), val, isSigned);

        return intLiteral;
    }

} // namespace Cryo
