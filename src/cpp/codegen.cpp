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
#include "cpp/codegen.h"

namespace Cryo
{
    /**
     * The Entry Point to the generation process.
     */
    void CodeGen::executeCodeGeneration(ASTNode *root)
    {
        std::cout << "\nStarting Code Generation... @executeCodeGeneration\n";
        if (root == nullptr)
        {
            std::cerr << "Error: root is null." << std::endl;
            return;
        }

        CryoContext &cryoContext = compiler.getContext();
        CryoModules &cryoModulesInstance = compiler.getModules();
        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        std::cout << "\n\n ------ Debug AST View ------\n";
        cryoDebugger.logNode(root);
        std::cout << "------ Debug AST View ------\n\n";

        // First Pass: Declare all functions
        std::cout << "\nFirst Pass: Declaring all functions\n";
        bool mainFunctionExists = cryoModulesInstance.declareFunctions(root);

        if (!mainFunctionExists)
        {
            std::cout << "[CPP] Main function not found, creating default main function\n";
            cryoSyntaxInstance.createDefaultMainFunction();
        }

        // Second Pass: Generate code for the entire program
        std::cout << "\nSecond Pass: Generate code for the entire program\n";
        cryoSyntaxInstance.identifyNodeExpression(root);

        llvm::Function *defaultMain = cryoContext.module->getFunction("_defaulted");
        if (defaultMain)
        {
            llvm::BasicBlock &entryBlock = defaultMain->getEntryBlock();
            if (!entryBlock.getTerminator())
            {
                cryoContext.builder.SetInsertPoint(&entryBlock);
                cryoContext.builder.CreateRetVoid();
                std::cout << "[CPP] Added return statement to default main function in entry block\n";
            }
        }

        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            std::cout << "\n>===------- Error: LLVM module verification failed -------===<\n";
            cryoContext.module->print(llvm::errs(), nullptr);
            std::cout << "\n>===----------------- End Error -----------------===<\n";
            std::cerr << "Error: LLVM module verification failed\n";
            exit(1);
        }
        else
        {
            std::error_code EC;
            llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

            if (EC)
            {
                std::cerr << "Could not open file: " << EC.message() << std::endl;
            }
            else
            {
                std::cout << "\n>===------- LLVM IR Code -------===<\n"
                          << std::endl;
                cryoContext.module->print(dest, nullptr);
                cryoContext.module->print(llvm::outs(), nullptr);
                std::cout << "\n>===------- End IR Code ------===<\n"
                          << std::endl;
                std::cout << "LLVM IR written to output.ll" << std::endl;
            }
        }

        std::cout << "\nCode generation complete\n";
    } // <executeCodeGeneration>

    /**
     * Identify the type of node and call the appropriate function to generate code.
     */
    void CryoSyntax::identifyNodeExpression(ASTNode *node)
    {
        if (!node)
        {
            std::cerr << "[CodeGen] Error: Code Generation Failed, ASTNode is null. @<identifyExpression>";
            exit(0);
            return;
        }
        std::cout << "[CodeGen] @identifyNodeExpression Starting Code Generation...\n";

        CryoModules cryoModulesInstance = compiler.getModules();
        CryoSyntax cryoSyntaxInstance = compiler.getSyntax();

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
        {
            std::cout << "[CodeGen] Identified NODE_PROGRAM.\n";
            cryoModulesInstance.generateProgram(node);
            break;
        }

        case NODE_FUNCTION_DECLARATION:
        {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_DECLARATION.\n";
            cryoSyntaxInstance.generateFunction(node);
            break;
        }

        case NODE_VAR_DECLARATION:
        {
            std::cout << "[CodeGen] Identified NODE_VAR_DECLARATION.\n";
            cryoSyntaxInstance.generateVarDeclaration(node);
            break;
        }

        case NODE_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_STATEMENT.\n";
            cryoSyntaxInstance.generateStatement(node);
            break;
        }

        case NODE_EXPRESSION:
        {
            std::cout << "[CodeGen] Identified NODE_EXPRESSION.\n";
            cryoSyntaxInstance.generateExpression(node);
            break;
        }

        case NODE_BINARY_EXPR:
        {
            std::cout << "[CodeGen] Identified NODE_BINARY_EXPR.\n";
            cryoSyntaxInstance.generateBinaryOperation(node);
            break;
        }

        case NODE_UNARY_EXPR:
        {
            std::cout << "[CodeGen] Identified NODE_UNARY_EXPR.\n";
            // Todo
            break;
        }

        case NODE_LITERAL_EXPR:
        {
            std::cout << "[CodeGen] Identified NODE_LITERAL_EXPR.\n";
            // Todo
            break;
        }

        case NODE_VAR_NAME:
        {
            std::cout << "[CodeGen] Identified NODE_VAR_NAME.\n";
            // Todo
            break;
        }

        case NODE_FUNCTION_CALL:
        {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_CALL.\n";
            cryoSyntaxInstance.generateFunctionCall(node);
            break;
        }

        case NODE_IF_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_IF_STATEMENT.\n";
            // Todo
            break;
        }

        case NODE_WHILE_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_WHILE_STATEMENT.\n";
            // Todo
            break;
        }

        case NODE_FOR_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_FOR_STATEMENT.\n";
            // Todo
            break;
        }

        case NODE_RETURN_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_RETURN_STATEMENT.\n";
            cryoSyntaxInstance.generateReturnStatement(node);
            break;
        }

        case NODE_BLOCK:
        {
            std::cout << "[CodeGen] Identified NODE_BLOCK.\n";
            cryoModulesInstance.generateBlock(node);
            break;
        }

        case NODE_FUNCTION_BLOCK:
        {
            std::cout << "[CodeGen] Identified NODE_FUNCTION_BLOCK.\n";
            cryoSyntaxInstance.generateFunctionBlock(node);
            break;
        }

        case NODE_EXPRESSION_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_EXPRESSION_STATEMENT.\n";
            // cryoSyntaxInstance.generateExpression(node->data.stmt.stmt);
            break;
        }

        case NODE_ASSIGN:
        {
            std::cout << "[CodeGen] Identified NODE_ASSIGN.\n";
            // Todo
            break;
        }

        case NODE_PARAM_LIST:
        {
            std::cout << "[CodeGen] Identified NODE_PARAM_LIST.\n";
            // Todo
            break;
        }

        case NODE_TYPE:
        {
            std::cout << "[CodeGen] Identified NODE_TYPE.\n";
            // Todo
            break;
        }

        case NODE_STRING_LITERAL:
        {
            std::cout << "[CodeGen] Identified NODE_STRING_LITERAL.\n";
            // Todo
            break;
        }

        case NODE_STRING_EXPRESSION:
        {
            std::cout << "[CodeGen] Identified NODE_STRING_EXPRESSION.\n";
            // Todo
            break;
        }

        case NODE_BOOLEAN_LITERAL:
        {
            std::cout << "[CodeGen] Identified NODE_BOOLEAN_LITERAL.\n";
            // Todo
            break;
        }

        case NODE_ARRAY_LITERAL:
        {
            std::cout << "[CodeGen] Identified NODE_ARRAY_LITERAL.\n";
            cryoSyntaxInstance.generateArrayLiteral(node);
            break;
        }

        case NODE_IMPORT_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_IMPORT_STATEMENT.\n";
            // Todo
            break;
        }

        case NODE_EXTERN_STATEMENT:
        {
            std::cout << "[CodeGen] Identified NODE_EXTERN_STATEMENT.\n";
            // Todo
            break;
        }

        case NODE_EXTERN_FUNCTION:
        {
            std::cout << "[CodeGen] Identified NODE_EXTERN_FUNCTION.\n";
            cryoSyntaxInstance.generateExternalDeclaration(node);
            break;
        }

        case NODE_ARG_LIST:
        {
            std::cout << "[CodeGen] Identified NODE_ARG_LIST.\n";
            // Todo
            break;
        }

        case NODE_UNKNOWN:
        {
            std::cerr << "[CodeGen] Error: `NODE_UNKNOWN` found. @<identifyNodeExpression>.\n";
            break;
        }

        default:
        {
            std::cerr << "[CodeGen] Error: Unknown Node Type. @<identifyNodeExpression>\n";
            break;
        }
        }
        return;
    } // <identifyNodeExpression>

} // namespace Cryo
