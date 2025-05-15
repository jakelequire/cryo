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
#include "codegen/codegen.hpp"

namespace Cryo
{
    void CodegenContext::preInitMain(void)
    {
        llvm::Module *depMod = this->getLinker()->initMainModule();
        if (!depMod)
        {
            logMessage(LMI, "ERROR", "CodegenContext", "Failed to initialize main module");
            return;
        }

        // Import the definitions into the symbol table
        if (!this->symbolTable->importModuleDefinitions(depMod))
        {
            logMessage(LMI, "ERROR", "CodegenContext", "Failed to import module definitions");
            return;
        }

        this->mergeModule(depMod);
        return;
    }

    void CodegenContext::initializeCodegenContext(void)
    {
        std::string moduleName = "CryoModuleDefaulted";
        module = std::make_unique<llvm::Module>(moduleName, context);
        symbolTable = std::make_unique<IRSymbolTable>(*this);
        visitor = std::make_unique<CodeGenVisitor>(*this);

        return;
    }

    void CodegenContext::setModuleIdentifier(std::string name)
    {
        module->setModuleIdentifier(name);
        module->setSourceFileName(name);

        return;
    }

    void CodegenContext::setCurrentFunction(llvm::Function *function)
    {
        this->currentFunction = function;
        return;
    }

    void CodegenContext::clearCurrentFunction(void)
    {
        this->currentFunction = nullptr;
        return;
    }

    void CodegenContext::printModule(void)
    {
        std::string moduleName = module->getName().str();
        std::cout << "\n\n";
        std::cout << "<!> [ NEW CODEGEN ] Printing Module: " << moduleName << "\n";
        std::cout << "\n\n";
        std::cout << "<!> ===----------------------- Module: " << moduleName << " -----------------------=== <!>\n";
        module->print(llvm::errs(), nullptr);
        std::cout << "<!> ===------------------------------------------------------------=== <!>\n";
        std::cout << "\n\n";
        return;
    }
} // namespace Cryo
