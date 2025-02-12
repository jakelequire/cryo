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
#include "symbolTable/IRdefs.hpp"

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
        {
            llvm::AllocaInst *alloca = builder.CreateAlloca(symbol.type, nullptr, symbol.name);
            return Allocation(AllocaType::AllocaOnly, alloca);
        }
        case AllocaType::AllocaAndLoad:
        {
            llvm::AllocaInst *alloca = builder.CreateAlloca(symbol.type, nullptr, symbol.name);
            llvm::LoadInst *load = builder.CreateLoad(symbol.type, alloca, symbol.name + ".load");
            return Allocation(AllocaType::AllocaAndLoad, alloca, nullptr, load);
        }
        case AllocaType::AllocaAndStore:
        {
            llvm::AllocaInst *alloca = builder.CreateAlloca(symbol.type, nullptr, symbol.name);
            llvm::StoreInst *store = builder.CreateStore(symbol.value, alloca);
            return Allocation(AllocaType::AllocaAndStore, alloca, store);
        }
        case AllocaType::AllocaLoadStore:
        {
            llvm::AllocaInst *alloca = builder.CreateAlloca(symbol.type, nullptr, symbol.name);
            llvm::LoadInst *load = builder.CreateLoad(symbol.type, alloca, symbol.name + ".load");
            llvm::StoreInst *store = builder.CreateStore(symbol.value, alloca);
            return Allocation(AllocaType::AllocaLoadStore, alloca, store, load);
        }
        case AllocaType::Global:
        {
            return Allocation(llvm::dyn_cast<llvm::GlobalVariable>(symbol.value));
        }
        case AllocaType::Parameter:
        {
            return Allocation(symbol.value);
        }
        default:
        {
            // Handle the error case properly
            llvm::errs() << "Unknown AllocaType encountered.\n";
            std::abort();
        }
        }
    }
} // namespace Cryo
