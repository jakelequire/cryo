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
#include "symbolTable/IRSymbolTable.hpp"

namespace Cryo
{
    template <typename T>
    llvm::Type *IRSymbolTable::getLLVMType()
    {
        if constexpr (std::is_integral_v<T>)
        {
            return llvm::Type::getInt32Ty(currentModule->getContext());
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return llvm::Type::getDoubleTy(currentModule->getContext());
        }
        // Add more type mappings as needed
        return nullptr;
    }

    template <typename T>
    llvm::Value *IRSymbolTable::createLLVMValue(const T &value)
    {
        llvm::IRBuilder<> builder(currentModule->getContext());
        if constexpr (std::is_integral_v<T>)
        {
            return llvm::ConstantInt::get(getLLVMType<T>(), value);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return llvm::ConstantFP::get(getLLVMType<T>(), value);
        }
        return nullptr;
    }

    // ======================================================================== //
    //                       Scope Management Functions                         //
    // ======================================================================== //

    void IRSymbolTable::pushScope()
    {
        scopeStack.push_back({});
    }

    void IRSymbolTable::popScope()
    {
        if (scopeStack.size() > 1)
        { // Keep global scope
            scopeStack.pop_back();
        }
    }

    // ======================================================================== //
    //                       Symbol Management Functions                        //
    // ======================================================================== //

    bool IRSymbolTable::addVariable(const IRVariableSymbol &symbol)
    {
        if (scopeStack.empty())
            return false;
        auto &currentScope = scopeStack.back();
        currentScope.insert({symbol.name, symbol});
        return true;
    }

    bool IRSymbolTable::addFunction(const IRFunctionSymbol &symbol)
    {
        functions.insert({symbol.name, symbol});
        return true;
    }

    bool IRSymbolTable::addType(const IRTypeSymbol &symbol)
    {
        types.insert({symbol.name, symbol});
        return true;
    }

    // ======================================================================== //
    //                         Symbol Lookup Functions                          //
    // ======================================================================== //

    IRVariableSymbol *IRSymbolTable::findVariable(const std::string &name)
    {
        // Search from current scope up to global
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
        {
            auto found = it->find(name);
            if (found != it->end())
            {
                return &found->second;
            }
        }
        return nullptr;
    }

    IRVariableSymbol *IRSymbolTable::createGlobalVariable(const std::string &name, llvm::Type *type,
                                                          llvm::Value *initialValue)
    {
        auto *initVal = initialValue ? llvm::dyn_cast<llvm::Constant>(initialValue)
                                     : llvm::Constant::getNullValue(type);

        auto varSymbol = IRSymbolManager::createVariableSymbol(
            nullptr, initVal, type, name, AllocaType::Global);
        varSymbol.allocation = Allocation::createGlobal(currentModule, type, name, initVal);

        addVariable(varSymbol);
        return findVariable(name);
    }

    IRFunctionSymbol *IRSymbolTable::findFunction(const std::string &name)
    {
        auto it = functions.find(name);
        return (it != functions.end()) ? &it->second : nullptr;
    }

    IRTypeSymbol *IRSymbolTable::findType(const std::string &name)
    {
        auto it = types.find(name);
        return (it != types.end()) ? &it->second : nullptr;
    }

    // ======================================================================== //
    //                         Debug Print Function                             //
    // ======================================================================== //

    // Helper function to convert TypeID to string
    std::string typeIDToString(llvm::Type *type)
    {
        std::string typeStr;
        llvm::raw_string_ostream rso(typeStr);
        type->print(rso, true);
        return rso.str();
    }

    void IRSymbolTable::debugPrint() const
    {
        std::cout << "\n\n";
        std::cout << "======================== Symbol Table Debug Print ========================" << std::endl;
        std::cout << "Module: " << BOLD LIGHT_BLUE << currentModule->getName().str() << COLOR_RESET << std::endl;
        std::cout << "===========================================================================" << std::endl;

        // Print variables in each scope
        std::cout << "\n";
        std::cout << BOLD UNDERLINE << "Variables:" << COLOR_RESET << std::endl;
        for (size_t i = 0; i < scopeStack.size(); ++i)
        {
            std::string scopeName = scopeStack.size() == 1 ? "Global" : "Local";
            std::cout << "  Scope " << "[" << i << "]" << ":" << scopeName << std::endl;
            for (const auto &varPair : scopeStack[i])
            {
                const auto &var = varPair.second;
                std::cout << "    Name: " << BLUE << var.name << COLOR_RESET << std::endl;
                std::cout << "    Type: " << typeIDToString(var.type) << std::endl;
                std::cout << "    AllocaType: " << Allocation::allocaTypeToString(var.allocaType) << std::endl;
                std::cout << "    ----------------------------------------" << std::endl;
            }
        }

        // Print functions
        std::cout << "\n";
        std::cout << BOLD UNDERLINE << "Functions:" << COLOR_RESET << std::endl;
        for (const auto &funcPair : functions)
        {
            const auto &func = funcPair.second;
            std::cout << "  Name: " << YELLOW << func.name << COLOR_RESET << std::endl;
            std::cout << "  Return Type: " << typeIDToString(func.returnType) << std::endl;
            std::cout << "  Is Variadic: " << (func.isVariadic ? "Yes" : "No") << std::endl;
            std::cout << "  Is External: " << (func.isExternal ? "Yes" : "No") << std::endl;
            std::cout << "  ----------------------------------------" << std::endl;
        }

        // Print types
        std::cout << "\n";
        std::cout << BOLD UNDERLINE << "Types:" << COLOR_RESET << std::endl;
        for (const auto &typePair : types)
        {
            const auto &type = typePair.second;
            std::cout << "  Name: " << CYAN << type.name << COLOR_RESET << std::endl;
            std::cout << "  Type ID: " << typeIDToString(type.type) << std::endl;
            std::cout << "  ----------------------------------------" << std::endl;
        }

        std::cout << "===========================================================================" << std::endl;
        std::cout << "\n\n";
    }

} // namespace Cryo
