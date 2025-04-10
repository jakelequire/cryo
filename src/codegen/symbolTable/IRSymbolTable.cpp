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
#include "codegen/symTable/IRSymbolTable.hpp"

namespace Cryo
{
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

    void IRSymbolTable::enterFunctionScope(const std::string &funcName)
    {
        currentFunction = findFunction(funcName);
        pushScope(); // Create new scope for function body
    }

    void IRSymbolTable::exitFunctionScope()
    {
        popScope();
        currentFunction = nullptr;
    }

    // ======================================================================== //
    //                       Symbol Management Functions                        //
    // ======================================================================== //

    bool IRSymbolTable::addVariable(const IRVariableSymbol &symbol)
    {
        if (scopeStack.empty())
            return false;

        auto &currentScope = scopeStack.back();

        // If we're in a function scope, associate the variable with the function
        if (currentFunction)
        {
            auto modifiedSymbol = symbol;
            modifiedSymbol.parentFunction = currentFunction->function;
            currentScope.insert({symbol.name, modifiedSymbol});
        }
        else
        {
            currentScope.insert({symbol.name, symbol});
        }

        return true;
    }

    // Modify addFunction to automatically enter function scope
    bool IRSymbolTable::addFunction(const IRFunctionSymbol &symbol)
    {
        functions.insert({symbol.name, symbol});
        enterFunctionScope(symbol.name);
        return true;
    }

    bool IRSymbolTable::addExternFunction(const IRFunctionSymbol &symbol)
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
        // Search from current scope up to global, respecting function boundaries
        for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
        {
            auto found = it->find(name);
            if (found != it->end())
            {
                // If we're in a function scope, only return variables visible in this function
                if (currentFunction)
                {
                    if (found->second.parentFunction == currentFunction->function ||
                        found->second.parentFunction == nullptr)
                    { // nullptr indicates global
                        return &found->second;
                    }
                }
                else
                {
                    return &found->second;
                }
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

    IRFunctionSymbol *IRSymbolTable::findOrCreateFunction(const std::string &name)
    {
        auto it = functions.find(name);
        if (it != functions.end())
        {
            return &it->second;
        }
        logMessage(LMI, "INFO", "CodeGen", "Creating function: %s", name.c_str());
        DTM->symbolTable->lookup(DTM->symbolTable, name.c_str());
        auto functionType = DTM->symbolTable->lookup(DTM->symbolTable, name.c_str());
        if (!functionType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Function %s not found in DTM", name.c_str());
            return nullptr;
        }
        if (functionType->container->typeOf != FUNCTION_TYPE)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Function %s is not a function type", name.c_str());
            return nullptr;
        }
        // If no function found, create a new one. But it will only be a declaration / prototype.
        std::vector<llvm::Type *> paramTypes;
        for (int i = 0; i < functionType->container->type.functionType->paramCount; ++i)
        {
            paramTypes.push_back(getLLVMType(functionType->container->type.functionType->paramTypes[i]));
        }
        llvm::FunctionType *llvmFuncType = llvm::FunctionType::get(
            getLLVMType(functionType->container->type.functionType->returnType),
            paramTypes,
            false);
        llvm::Function *llvmFunc = llvm::Function::Create(
            llvmFuncType, llvm::Function::ExternalLinkage, name, currentModule);

        IRFunctionSymbol funcSymbol = IRFunctionSymbol(
            llvmFunc, name, getLLVMType(functionType->container->type.functionType->returnType),
            llvmFuncType, nullptr, false, true);

        // Add the function to the symbol table
        addFunction(funcSymbol);
        return findFunction(name);
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
        if (!type)
            return "Unknown";
        return CodeGenDebug::LLVMTypeIDToString(type);
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
            for (const auto &varPair : scopeStack[i])
            {
                const auto &var = varPair.second;
                std::cout << "    Name: " << BLUE << var.name << COLOR_RESET << std::endl;
                std::cout << "    Type: " << typeIDToString(var.type) << std::endl;
                std::cout << "    AllocaType: " << Allocation::allocaTypeToString(var.allocaType) << std::endl;
                std::cout << "    Parent Function: " << (var.parentFunction ? var.parentFunction->getName().str() : "None") << std::endl;
                std::cout << "    Scope " << "[" << i << "]" << ":" << scopeName << std::endl;
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
            IRTypeKind typeKind = type.kind;
            std::cout << "  Type Kind: " << IRTypeKindToString(typeKind) << std::endl;
            std::cout << "  ----------------------------------------" << std::endl;
        }

        std::cout << "===========================================================================" << std::endl;
        std::cout << "\n\n";
    }

    std::string IRSymbolTable::IRTypeKindToString(IRTypeKind kind) const
    {
        switch (kind)
        {
        case IRTypeKind::Aggregate:
            return "AGGREGATE";
        case IRTypeKind::Class:
            return "CLASS";
        case IRTypeKind::Enum:
            return "ENUM";
        case IRTypeKind::Function:
            return "FUNCTION";
        case IRTypeKind::Pointer:
            return "POINTER";
        case IRTypeKind::Primitive:
            return "PRIMITIVE";
        case IRTypeKind::Struct:
            return "STRUCT";
        default:
            return "UNKNOWN";
        }
    }

} // namespace Cryo
