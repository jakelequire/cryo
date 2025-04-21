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
        if (!currentFunction)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Function %s not found in symbol table", funcName.c_str());
            return;
        }
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

    IRVariableSymbol *IRSymbolTable::findLocalVariable(const std::string &name)
    {
        if (scopeStack.empty())
            return nullptr;

        auto &currentScope = scopeStack.back();
        auto found = currentScope.find(name);
        return (found != currentScope.end()) ? &found->second : nullptr;
    }

    IRVariableSymbol *IRSymbolTable::createGlobalVariable(const std::string &name, llvm::Type *type,
                                                          llvm::Value *initialValue)
    {
        auto *initVal = initialValue ? llvm::dyn_cast<llvm::Constant>(initialValue)
                                     : llvm::Constant::getNullValue(type);

        auto varSymbol = IRSymbolManager::createVariableSymbol(
            nullptr, initVal, type, name, AllocaType::Global);
        varSymbol.allocation = Allocation::createGlobal(context.getInstance().module.get(), type, name, initVal);

        addVariable(varSymbol);
        return findVariable(name);
    }

    IRFunctionSymbol *IRSymbolTable::findFunction(const std::string &name)
    {
        logMessage(LMI, "INFO", "CodeGen", "Finding function: %s", name.c_str());

        // First try local functions
        auto it = functions.find(name);
        if (it != functions.end())
        {
            logMessage(LMI, "INFO", "CodeGen", "Found function %s in local symbol table", name.c_str());
            return &it->second;
        }

        logMessage(LMI, "INFO", "CodeGen", "Function %s not found in local symbol table", name.c_str());
        // Check the global symbol table for the function
        llvm::Function *globalFunc = globalSymbolTableInstance.getFunction(name);
        if (globalFunc)
        {
            logMessage(LMI, "INFO", "CodeGen", "Found function %s in global symbol table", name.c_str());

            // Check if the function is already defined in the current module
            llvm::Function *existingFunc = context.getInstance().module->getFunction(name);
            if (existingFunc)
            {
                logMessage(LMI, "INFO", "CodeGen", "Function %s already defined in current module", name.c_str());
                return globalSymbolTableInstance.wrapLLVMFunction(existingFunc);
            }
            // If not, create a new function in the current module

            // Create an external declaration for the function in the current module
            llvm::FunctionType *funcType = globalFunc->getFunctionType();
            llvm::Function *llvmFunc = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, name, context.getInstance().module.get());
            llvmFunc->setCallingConv(globalFunc->getCallingConv());
            llvmFunc->setAttributes(globalFunc->getAttributes());
            llvmFunc->setName(name);
            llvmFunc->setLinkage(globalFunc->getLinkage());
            llvmFunc->setVisibility(globalFunc->getVisibility());

            IRFunctionSymbol *wrappedFunc = globalSymbolTableInstance.wrapLLVMFunction(globalFunc);
            if (wrappedFunc)
            {
                logMessage(LMI, "INFO", "CodeGen", "Found function %s in global symbol table", name.c_str());
                return wrappedFunc;
            }
            else
            {
                logMessage(LMI, "ERROR", "CodeGen", "Failed to wrap function %s from global symbol table", name.c_str());
                return nullptr;
            }
        }

        logMessage(LMI, "ERROR", "CodeGen", "Function %s not found in symbol table", name.c_str());
        return nullptr;
    }

    IRTypeSymbol *IRSymbolTable::findType(const std::string &name)
    {
        logMessage(LMI, "INFO", "CodeGen", "Finding type: %s", name.c_str());
        // First try exact match in local types
        auto it = types.find(name);
        if (it != types.end())
        {
            return &it->second;
        }

        // If not found and it's a struct/class name pattern, try prefix matching in local types
        if (name.find("struct.") == 0 || name.find("class.") == 0)
        {
            // Get the base name (e.g., "struct.Int" from "struct.Int")
            std::string baseName = name;

            // Find the best matching type (with the shortest suffix)
            IRTypeSymbol *bestMatch = nullptr;
            size_t shortestSuffix = std::string::npos;

            for (auto &typePair : types)
            {
                const std::string &typeName = typePair.first;

                // Check if the type name starts with our base name
                if (typeName.find(baseName) == 0)
                {
                    // It's a match! Now check if it's just the base name or has a suffix
                    if (typeName.length() == baseName.length())
                    {
                        // Exact match
                        return &typePair.second;
                    }

                    // It has a suffix (e.g., ".3" in "struct.Int.3")
                    // Check if the character after the base name is a dot (typical for LLVM versioning)
                    if (typeName[baseName.length()] == '.')
                    {
                        // Check if this suffix is shorter than any we've seen so far
                        size_t suffixLength = typeName.length() - baseName.length();
                        if (shortestSuffix == std::string::npos || suffixLength < shortestSuffix)
                        {
                            shortestSuffix = suffixLength;
                            bestMatch = &typePair.second;
                        }
                    }
                }
            }

            // Return the best match if found
            if (bestMatch)
            {
                return bestMatch;
            }
        }

        // If not found, check the global symbol table for the type
        llvm::StructType *globalType = globalSymbolTableInstance.getStructType(name);
        if (globalType)
        {
            IRTypeSymbol *wrappedType = globalSymbolTableInstance.wrapLLVMStructType(globalType);
            if (wrappedType)
            {
                logMessage(LMI, "INFO", "CodeGen", "Found type %s in global symbol table", name.c_str());
                return wrappedType;
            }
            else
            {
                logMessage(LMI, "ERROR", "CodeGen", "Failed to wrap type %s from global symbol table", name.c_str());
                return nullptr;
            }
        }

        // No match found
        logMessage(LMI, "ERROR", "CodeGen", "Type %s not found in symbol table", name.c_str());
        return nullptr;
    }

    llvm::Type *IRSymbolTable::derefencePointer(llvm::Type *type)
    {
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
        std::cout << "Module: " << BOLD LIGHT_BLUE << context.getInstance().module->getName().str() << COLOR_RESET << std::endl;
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
        std::cout << "Global Symbol Table Instance:" << std::endl;
        std::cout << "  Module Count: " << globalSymbolTableInstance.getModuleCount() << std::endl;
        // Print all the function in each module
        for (auto modulePair : globalSymbolTableInstance.getModules())
        {
            llvm::Module *module = modulePair;
            std::cout << "  ----------------------" << std::endl;
            std::cout << "  Module: " << module->getName().str() << std::endl;
            std::cout << "  ----------------------" << std::endl;
            for (auto &function : module->functions())
            {
                std::cout << "    Function: " << function.getName().str() << std::endl;
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        std::cout << "---------------------------------------------------------" << std::endl;
        std::cout << " Global Types:" << std::endl;
        // Print all the types in each module
        for (auto modulePair : globalSymbolTableInstance.getModules())
        {
            llvm::Module *module = modulePair;
            std::cout << "  ----------------------" << std::endl;
            std::cout << "  Module: " << module->getName().str() << std::endl;
            std::cout << "  ----------------------" << std::endl;
            for (auto &type : module->getIdentifiedStructTypes())
            {
                std::cout << "    Type: " << type->getName().str() << std::endl;
            }
        }

        std::cout << "\n";
        std::cout << "===========================================================================" << std::endl;
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

    bool IRSymbolTable::importModuleDefinitions(llvm::Module *sourceModule)
    {
        if (!sourceModule)
        {
            logMessage(LMI, "ERROR", "IRSymbolTable", "Source module is null");
            return false;
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "Importing definitions from module: %s",
                   sourceModule->getName().str().c_str());

        // Import functions
        for (auto &func : sourceModule->functions())
        {
            std::string funcName = func.getName().str();

            // Skip if function already exists
            if (findFunction(funcName))
            {
                logMessage(LMI, "INFO", "IRSymbolTable", "Function %s already exists, skipping", funcName.c_str());
                continue;
            }

            // Create function symbol
            IRFunctionSymbol funcSymbol(
                &func,
                funcName,
                func.getReturnType(),
                func.getFunctionType(),
                func.empty() ? nullptr : &func.getEntryBlock(),
                func.isVarArg(),
                func.isDeclaration());

            // Add to symbol table
            addExternFunction(funcSymbol);
            logMessage(LMI, "INFO", "IRSymbolTable", "Imported function: %s", funcName.c_str());
        }

        // Import global variables
        for (auto &global : sourceModule->globals())
        {
            std::string globalName = global.getName().str();

            // Skip if global variable already exists
            if (findVariable(globalName))
            {
                logMessage(LMI, "INFO", "IRSymbolTable", "Global variable %s already exists, skipping",
                           globalName.c_str());
                continue;
            }

            // Create global variable symbol with appropriate allocation
            IRVariableSymbol varSymbol(
                &global,
                global.getValueType(),
                globalName,
                AllocaType::Global,
                Allocation());
            varSymbol.allocation.global = &global;
            varSymbol.allocation.type = AllocaType::Global;

            // Add to symbol table
            addVariable(varSymbol);
            logMessage(LMI, "INFO", "IRSymbolTable", "Imported global variable: %s", globalName.c_str());
        }

        // Import struct types
        for (auto &namedType : sourceModule->getIdentifiedStructTypes())
        {
            std::string typeName = namedType->getName().str();

            // Skip if type already exists
            if (findType(typeName))
            {
                logMessage(LMI, "INFO", "IRSymbolTable", "Type %s already exists, skipping", typeName.c_str());
                continue;
            }

            // Create property and method vectors (these would be empty for imported types)
            std::vector<IRPropertySymbol> properties;
            std::vector<IRMethodSymbol> methods;

            // Create type symbol
            IRTypeSymbol typeSymbol(namedType, typeName, properties, methods);
            typeSymbol.kind = IRTypeKind::Struct;

            // Add to symbol table
            addType(typeSymbol);
            logMessage(LMI, "INFO", "IRSymbolTable", "Imported struct type: %s", typeName.c_str());
        }

        logMessage(LMI, "INFO", "IRSymbolTable", "Successfully imported module definitions");
        return true;
    }

    llvm::Value *IRSymbolTable::getOrCreateGlobalString(const std::string &str)
    {
        // Check if the string already exists
        auto it = globalStringMap.find(str);
        if (it != globalStringMap.end())
        {
            logMessage(LMI, "INFO", "IRSymbolTable", "Found existing global string: %s", str.c_str());
            return it->second;
        }

        // Create a new global string variable
        llvm::Constant *strConstant = llvm::ConstantDataArray::getString(
            context.getInstance().context, str, true); // true adds null terminator
        logMessage(LMI, "INFO", "IRSymbolTable", "Creating global string: %s", str.c_str());
        // Use a more consistent naming convention for string literals
        std::string globalName = ".str."; // You might want to use a more generic naming scheme
        void *address = static_cast<void *>(strConstant);
        globalName += std::to_string(reinterpret_cast<std::uintptr_t>(address));

        logMessage(LMI, "INFO", "IRSymbolTable", "Global string name: %s", globalName.c_str());
        llvm::GlobalVariable *globalStr = new llvm::GlobalVariable(
            *context.getInstance().module,      // Module
            strConstant->getType(),             // Type
            true,                               // isConstant
            llvm::GlobalValue::ExternalLinkage, // Linkage
            strConstant,                        // Initializer
            globalName,                         // Name
            nullptr,                            // InsertBefore
            llvm::GlobalValue::NotThreadLocal,  // ThreadLocalMode
            0                                   // AddressSpace
        );

        // Set to unnamed_addr as in your example
        globalStr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        globalStr->setAlignment(llvm::MaybeAlign(1));

        // Store the string in the map
        globalStringMap.insert({str, globalStr});

        logMessage(LMI, "INFO", "IRSymbolTable", "Global string created: %s", str.c_str());
        return globalStr;
    }

} // namespace Cryo
