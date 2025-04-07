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
#include "codegen_rewrite/codegen.hpp"

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
        // Verify the module
        // if (llvm::verifyModule(*context.module))
        // {
        //     logMessage(LMI, "ERROR", "IRGeneration", "Module verification failed");
        //     return;
        // }

        // Optimize the module
        llvm::legacy::PassManager passManager;
        passManager.add(llvm::createInstructionCombiningPass());

        // Run the optimization passes
        // passManager.run(*context.module);

        // Print Symbol Table
        context.symbolTable->debugPrint();

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

} // namespace Cryo
