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

        CryoContext &cryoContext = compiler.getContext();
        CryoModules &cryoModulesInstance = compiler.getModules();
        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Starting Code Generation");

        if (root == nullptr)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Root node is null");
            return;
        }

        std::cout << "\n\n ------ Debug AST View ------\n";
        cryoDebugger.logNode(root);
        std::cout << "------^ Debug AST View ^------\n\n";

        // First Pass: Declare all functions
        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "First Pass: Declare all functions");
        bool mainFunctionExists = cryoModulesInstance.declareFunctions(root);

        if (!mainFunctionExists)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Main function does not exist. Creating default main function");
            cryoSyntaxInstance.createDefaultMainFunction();
        }

        // Second Pass: Generate code for the entire program
        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Second Pass: Generate code for the entire program");
        cryoSyntaxInstance.identifyNodeExpression(root);

        llvm::Function *defaultMain = cryoContext.module->getFunction("_defaulted");
        if (defaultMain)
        {
            llvm::BasicBlock &entryBlock = defaultMain->getEntryBlock();
            if (!entryBlock.getTerminator())
            {
                cryoContext.builder.SetInsertPoint(&entryBlock);
                cryoContext.builder.CreateRetVoid();
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Default main function has no return statement. Adding return void");
            }
        }

        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            std::cout << "\n>===------- Error: LLVM module verification failed -------===<\n";
            cryoContext.module->print(llvm::errs(), nullptr);
            std::cout << "\n>===----------------- End Error -----------------===<\n";
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "LLVM module verification failed");
            exit(1);
        }
        else
        {
            std::error_code EC;
            llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

            if (EC)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Error opening file for writing");
            }
            else
            {
                std::cout << "\n>===------- LLVM IR Code -------===<\n"
                          << std::endl;
                cryoContext.module->print(dest, nullptr);
                cryoContext.module->print(llvm::outs(), nullptr);
                std::cout << "\n>===------- End IR Code ------===<\n"
                          << std::endl;
                dest.flush();
                dest.close();

                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Code Generation Complete");
            }
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Code Generation Complete");
        return;
    } // <executeCodeGeneration>

    /**
     * Identify the type of node and call the appropriate function to generate code.
     */
    void CryoSyntax::identifyNodeExpression(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Node is null in identifyNodeExpression");
            exit(0);
            return;
        }
        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identifying Node Expression");

        CryoModules cryoModulesInstance = compiler.getModules();
        CryoSyntax cryoSyntaxInstance = compiler.getSyntax();

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_PROGRAM");
            cryoModulesInstance.generateProgram(node);
            break;
        }

        case NODE_FUNCTION_DECLARATION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_FUNCTION_DECLARATION");
            cryoSyntaxInstance.generateFunction(node);
            break;
        }

        case NODE_VAR_DECLARATION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_VAR_DECLARATION");
            cryoSyntaxInstance.generateVarDeclaration(node);
            break;
        }

        case NODE_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_STATEMENT");
            cryoSyntaxInstance.generateStatement(node);
            break;
        }

        case NODE_EXPRESSION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_EXPRESSION");
            cryoSyntaxInstance.generateExpression(node);
            break;
        }

        case NODE_BINARY_EXPR:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_BINARY_EXPR");
            cryoSyntaxInstance.generateBinaryOperation(node);
            break;
        }

        case NODE_UNARY_EXPR:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_UNARY_EXPR");
            // Todo
            break;
        }

        case NODE_LITERAL_EXPR:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_LITERAL_EXPR");
            // Todo
            break;
        }

        case NODE_VAR_NAME:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_VAR_NAME");
            // Todo
            break;
        }

        case NODE_FUNCTION_CALL:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_FUNCTION_CALL");
            cryoSyntaxInstance.generateFunctionCall(node);
            break;
        }

        case NODE_IF_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_IF_STATEMENT");
            // Todo
            break;
        }

        case NODE_WHILE_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_WHILE_STATEMENT");
            // Todo
            break;
        }

        case NODE_FOR_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_FOR_STATEMENT");
            // Todo
            break;
        }

        case NODE_RETURN_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_RETURN_STATEMENT");
            cryoSyntaxInstance.generateReturnStatement(node);
            break;
        }

        case NODE_BLOCK:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_BLOCK");
            cryoModulesInstance.generateBlock(node);
            break;
        }

        case NODE_FUNCTION_BLOCK:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_FUNCTION_BLOCK");
            cryoSyntaxInstance.generateFunctionBlock(node);
            break;
        }

        case NODE_EXPRESSION_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_EXPRESSION_STATEMENT");
            // cryoSyntaxInstance.generateExpression(node->data.stmt.stmt);
            break;
        }

        case NODE_ASSIGN:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_ASSIGN");
            // Todo
            break;
        }

        case NODE_PARAM_LIST:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_PARAM_LIST");
            // Todo
            break;
        }

        case NODE_TYPE:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_TYPE");
            // Todo
            break;
        }

        case NODE_STRING_LITERAL:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_STRING_LITERAL");
            // Todo
            break;
        }

        case NODE_STRING_EXPRESSION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_STRING_EXPRESSION");
            // Todo
            break;
        }

        case NODE_BOOLEAN_LITERAL:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_BOOLEAN_LITERAL");
            // Todo
            break;
        }

        case NODE_ARRAY_LITERAL:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_ARRAY_LITERAL");
            cryoSyntaxInstance.generateArrayLiteral(node);
            break;
        }

        case NODE_IMPORT_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_IMPORT_STATEMENT");
            // Todo
            break;
        }

        case NODE_EXTERN_STATEMENT:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_EXTERN_STATEMENT");
            // Todo
            break;
        }

        case NODE_EXTERN_FUNCTION:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_EXTERN_FUNCTION");
            cryoSyntaxInstance.generateExternalDeclaration(node);
            break;
        }

        case NODE_ARG_LIST:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_ARG_LIST");
            // Todo
            break;
        }

        case NODE_UNKNOWN:
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Identified NODE_UNKNOWN");
            break;
        }

        default:
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Unsupported node type");
            break;
        }
        }
        return;
    } // <identifyNodeExpression>

} // namespace Cryo
