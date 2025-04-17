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
    // Global symbol table instance
    GlobalSymbolTableInstance globalSymbolTableInstance;

    int GlobalSymbolTableInstance::setModule(const std::string &name, llvm::Module *module)
    {
        logMessage(LMI, "DEBUG", "GlobalSymbolTableInstance", "Setting module: %s", name.c_str());
        if (name.empty() || !module)
        {
            logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Invalid module name or module is null");
            return -1;
        }

        if (modules.find(name) != modules.end())
        {
            logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Module %s already exists", name.c_str());
            return -1;
        }

        modules[name] = module;
        logMessage(LMI, "DEBUG", "GlobalSymbolTableInstance", "Module %s added to symbol table", name.c_str());
        return 0;
    }

    llvm::Module *GlobalSymbolTableInstance::getModule(const std::string &name)
    {
        auto it = modules.find(name);
        if (it != modules.end())
        {
            return it->second;
        }
        return nullptr;
    }

    int GlobalSymbolTableInstance::getModuleCount()
    {
        return modules.size();
    }

    std::vector<llvm::Module *> GlobalSymbolTableInstance::getModules()
    {
        std::vector<llvm::Module *> moduleList;
        for (const auto &modulePair : modules)
        {
            moduleList.push_back(modulePair.second);
        }
        return moduleList;
    }

    llvm::Function *GlobalSymbolTableInstance::getFunction(const std::string &name)
    {
        for (const auto &modulePair : modules)
        {
            llvm::Module *module = modulePair.second;
            llvm::Function *function = module->getFunction(name);
            if (function)
            {
                logMessage(LMI, "DEBUG", "GlobalSymbolTableInstance", "Found function: %s in module: %s",
                           name.c_str(), module->getName().str().c_str());
                return function;
            }
        }

        logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Function %s not found in any module", name.c_str());
        return nullptr;
    }

    IRFunctionSymbol *GlobalSymbolTableInstance::wrapLLVMFunction(llvm::Function *func)
    {
        if (!func)
        {
            logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Function is null");
            return nullptr;
        }

        std::string funcName = func->getName().str();
        llvm::FunctionType *funcType = func->getFunctionType();
        llvm::BasicBlock *entryBlock = &func->getEntryBlock();

        IRFunctionSymbol *functionSymbol = new IRFunctionSymbol(func, funcName, func->getReturnType(),
                                                                funcType, entryBlock, func->isVarArg(),
                                                                func->isDeclaration());
        return functionSymbol;
    }

    llvm::StructType *GlobalSymbolTableInstance::getStructType(const std::string &name)
    {
        for (const auto &modulePair : modules)
        {
            llvm::Module *module = modulePair.second;
            std::vector<llvm::StructType *> structTypes = module->getIdentifiedStructTypes();
            for (llvm::StructType *structType : structTypes)
            {
                std::string structName = structType->getName().str();
                logMessage(LMI, "DEBUG", "GlobalSymbolTableInstance", "Struct Name: %s", structName.c_str());
                if (structName == name)
                {
                    logMessage(LMI, "DEBUG", "GlobalSymbolTableInstance", "Found struct type: %s", name.c_str());
                    return structType;
                }
            }
        }

        logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Type %s not found in any module", name.c_str());
        return nullptr;
    }

    IRTypeSymbol *GlobalSymbolTableInstance::wrapLLVMStructType(llvm::StructType *type)
    {
        if (!type)
        {
            logMessage(LMI, "ERROR", "GlobalSymbolTableInstance", "Type is null");
            return nullptr;
        }

        std::string typeName = type->getStructName().str();
        IRTypeSymbol *typeSymbol = new IRTypeSymbol(type, typeName);
        return typeSymbol;
    }

} // namespace Cryo
