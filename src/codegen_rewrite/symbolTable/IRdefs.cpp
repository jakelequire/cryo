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
#include "codegen_rewrite/symTable/IRdefs.hpp"

namespace Cryo
{
    // Create a new function symbol
    IRFunctionSymbol IRSymbolManager::createFunctionSymbol(llvm::Function *function, const std::string &name,
                                                           llvm::Type *returnType, llvm::FunctionType *functionType,
                                                           llvm::BasicBlock *entryBlock, bool isVariadic,
                                                           bool isExternal)
    {
        return IRFunctionSymbol(function, name, returnType, functionType, entryBlock, isVariadic, isExternal);
    }

    // Create a new variable symbol
    IRVariableSymbol IRSymbolManager::createVariableSymbol(llvm::Value *value, llvm::Type *type, const std::string &name,
                                                           AllocaType allocaType, const Allocation &allocation)
    {
        return IRVariableSymbol(value, type, name, allocaType, allocation);
    }

    IRVariableSymbol IRSymbolManager::createVariableSymbol(llvm::Function *func, llvm::Value *value, llvm::Type *type,
                                                           const std::string &name, AllocaType allocaType)
    {
        return IRVariableSymbol(func, value, type, name, allocaType);
    }

    // Create a new type symbol
    IRTypeSymbol IRSymbolManager::createTypeSymbol(llvm::Type *type, const std::string &name)
    {
        return IRTypeSymbol(type, name);
    }

    IRTypeSymbol IRSymbolManager::createTypeSymbol(llvm::StructType *type, const std::string &name,
                                                   const std::vector<IRPropertySymbol> &members,
                                                   const std::vector<IRMethodSymbol> &methods)
    {
        return IRTypeSymbol(type, name, members, methods);
    }

    // Create a new property symbol
    IRPropertySymbol IRSymbolManager::createPropertySymbol(llvm::Type *type, const std::string &name,
                                                           size_t offset, bool isPublic)
    {
        IRPropertySymbol symbol;
        symbol.type = type;
        symbol.name = name;
        symbol.offset = offset;
        symbol.isPublic = isPublic;
        return symbol;
    }

    // Create a new method symbol
    IRMethodSymbol IRSymbolManager::createMethodSymbol(IRFunctionSymbol function, bool isVirtual,
                                                       bool isStatic, bool isConstructor,
                                                       bool isDestructor, size_t vtableIndex,
                                                       IRTypeSymbol *parentType)
    {
        return IRMethodSymbol(function, isVirtual, isStatic, isConstructor, isDestructor, vtableIndex, parentType);
    }

    // ==============================================================================

    void IRSymbolManager::addMemberToTypeSymbol(IRTypeSymbol &type, const IRPropertySymbol &member)
    {
        type.members.push_back(member);
    }
    void IRSymbolManager::addMethodToTypeSymbol(IRTypeSymbol &type, const IRMethodSymbol &method)
    {
        type.methods.push_back(method);
    }
    void IRSymbolManager::addParameterToFunctionSymbol(IRFunctionSymbol &function, const IRVariableSymbol &parameter)
    {
        function.parameters.push_back(parameter);
    }
    void IRSymbolManager::addSymbolToSymbolTable(IRFunctionSymbol &function, const IRVariableSymbol &symbol)
    {
        const std::string name = symbol.name;
        function.symbolTable.insert({name, symbol});
    }

    // ==============================================================================

    Allocation IRSymbolManager::createAlloca(IRVariableSymbol &symbol, llvm::IRBuilder<> &builder)
    {
        switch (symbol.allocaType)
        {
        case AllocaType::AllocaOnly:
        case AllocaType::AllocaAndStore:
        {
            return Allocation::createLocal(builder, symbol.type, symbol.name, symbol.value);
        }
        case AllocaType::AllocaAndLoad:
        case AllocaType::AllocaLoadStore:
        {
            auto alloc = Allocation::createLocal(builder, symbol.type, symbol.name, symbol.value);
            alloc.load(builder, symbol.name + ".load");
            return alloc;
        }
        case AllocaType::Global:
        {
            auto *module = builder.GetInsertBlock()->getModule();
            auto *constValue = llvm::dyn_cast<llvm::Constant>(symbol.value);
            return Allocation::createGlobal(module, symbol.type, symbol.name, constValue);
        }
        case AllocaType::Parameter:
        {
            return Allocation::createParameter(symbol.value);
        }
        case AllocaType::DynamicArray:
        {
            // Assuming symbol.value contains the size for dynamic arrays
            return Allocation::createDynamicArray(builder, symbol.type, symbol.value, symbol.name);
        }
        default:
        {
            llvm::errs() << "Unknown AllocaType encountered.\n";
            std::abort();
        }
        }
    }

} // namespace Cryo
