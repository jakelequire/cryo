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
    // [2]: Step 2. Begin IR Generation
    void Cryo::IRGeneration::generateIR(ASTNode *root)
    {
        ASSERT_NODE_VOID_RET(root);

        // Process declarations first (hoisting) [3]
        logMessage(LMI, "INFO", "IRGeneration", "Processing declarations...");
        processDeclarations(root);

        // Start visiting nodes
        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            for (size_t i = 0; i < root->data.program->statementCount; i++)
            {
                generateIRForNode(root->data.program->statements[i]);
            }
            break;
        default:
            generateIRForNode(root);
            break;
        }

        logMessage(LMI, "INFO", "IRGeneration", "IR Generation Complete!");

        // Complete the generation
        completeGeneration();
    }

    // [4]: Step 4. Generate IR for each node
    void Cryo::IRGeneration::generateIRForNode(ASTNode *node)
    {
        ASSERT_NODE_VOID_RET(node);

        logMessage(LMI, "INFO", "IRGeneration", "Generating IR for node...");
        // Use the visitor through the context
        if (!context.visitor)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Visitor is null, creating new visitor...");
            context.visitor = std::make_unique<CodeGenVisitor>(context);
        }

        logMessage(LMI, "INFO", "IRGeneration", "Visiting node...");
        context.visitor->visit(node);

        return;
    }

    void Cryo::IRGeneration::completeGeneration(void)
    {
        logMessage(LMI, "INFO", "IRGeneration", "Completing IR Generation...");
        if (!context.module)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Module is null, cannot finalize.");
            CONDITION_FAILED;
        }

        // Finalize the module
        if (this->finalize() != 0)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Failed to finalize module.");
            CONDITION_FAILED;
        }

        // Write the module to a file
        std::string fileName = ".ll";
        std::string outputPath = "";
        if (this->buildDir.empty())
        {
            outputPath = fileName;
        }
        else
        {
            outputPath = this->buildDir + fileName;
        }

        GetCXXLinker()->generateIRFromCodegen(context.module.get(), outputPath.c_str());
    }

    int Cryo::IRGeneration::finalize(void)
    {
        std::string moduleName = context.module->getName().str();
        logMessage(LMI, "INFO", "IRGeneration", "Finalizing IR Generation...");

        // Verify the module
        // logMessage(LMI, "INFO", "IRGeneration", "Verifying module...");
        // if (llvm::verifyModule(*context.module, &llvm::errs()))
        // {
        //     logMessage(LMI, "ERROR", "IRGeneration", "Module verification failed");
        //     return -1;
        // }

        std::cout << "\n\n";
        std::cout << "--------------------------------------------------------------------------------------------\n";
        std::cout << "Module: " << moduleName << "\n";
        std::cout << "--------------------------------------------------------------------------------------------\n";
        context.getInstance().module->print(llvm::errs(), nullptr);
        std::cout << "--------------------------------------------------------------------------------------------\n";
        std::cout << "\n\n";

        logMessage(LMI, "INFO", "IRGeneration", "Module verified successfully.");

        // Clone the module to avoid modifying the original
        logMessage(LMI, "INFO", "IRGeneration", "Cloning module...");
        std::unique_ptr<llvm::Module> clonedModule = llvm::CloneModule(*context.getInstance().module);
        if (!clonedModule)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Failed to clone module");
            return -1;
        }

        // optimize the module
        logMessage(LMI, "INFO", "IRGeneration", "Optimizing module...");
        llvm::legacy::PassManager passManager;
        passManager.add(llvm::createInstructionCombiningPass());
        passManager.add(llvm::createReassociatePass());
        passManager.add(llvm::createCFGSimplificationPass());
        passManager.add(llvm::createConstantHoistingPass());
        passManager.add(llvm::createDeadCodeEliminationPass());

        logMessage(LMI, "INFO", "IRGeneration", "Running optimization passes...");
        // Run the optimization passes
        passManager.run(*clonedModule);
        logMessage(LMI, "INFO", "IRGeneration", "Module optimized successfully.");

        logMessage(LMI, "INFO", "IRGeneration", "Cloned module successfully.");
        // Add the module to the global symbol table instance
        logMessage(LMI, "INFO", "IRGeneration", "Setting module in global symbol table instance...");
        if (globalSymbolTableInstance.setModule(moduleName, clonedModule.release()) != 0)
        {
            logMessage(LMI, "ERROR", "IRGeneration", "Failed to set module in global symbol table instance");
            return -1;
        }

        // Print the module to the console
        llvm::errs() << "Finalized Module:\n";
        std::cout << "--------------------------------------------------------------------------------------------\n";
        context.module->print(llvm::errs(), nullptr);
        std::cout << "--------------------------------------------------------------------------------------------\n";
        std::cout << "\n\n";
        std::cout << "Symbol Table:\n";
        context.symbolTable->debugPrint();
        std::cout << "--------------------------------------------------------------------------------------------\n";
        std::cout << "\n\n";
        return 0;
    }

} // namespace Cryo
