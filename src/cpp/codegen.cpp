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
     * @brief The entry point to the generation process (passed from the top-level compiler).
     * @param node
     */
    void CodeGen::executeCodeGeneration(ASTNode *root)
    {
        std::cout << "[CPP] Executing Code Generation" << std::endl;

        CryoCompiler &compiler = this->compiler;

        compiler.getGenerator().generateCode(root);
    }

    /**
     * Generates code from the given AST tree.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::generateCode(ASTNode *root)
    {
        std::cout << "[CPP] Generating Code" << std::endl;
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();

        if (!root)
        {
            std::cerr << "[CPP] Root is null!" << std::endl;
            return;
        }
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Linting Tree");
        bool validateTree = debugger.lintTree(root);
        if (validateTree == false)
        {
            std::cerr << "[CPP] Tree is invalid!" << std::endl;
            return;
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");
        parseTree(root);

        if (llvm::verifyModule(*cryoContext.module, &llvm::errs()))
        {
            std::cout << "\n>===------- Error: LLVM module verification failed -------===<\n";
            cryoContext.module->print(llvm::errs(), nullptr);
            std::cout << "\n>===----------------- End Error -----------------===<\n";
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "LLVM module verification failed");
            exit(1);
        }
        else
        {
            std::error_code EC;
            llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);

            if (EC)
            {
                debugger.logMessage("ERROR", __LINE__, "CodeGen", "Error opening file for writing");
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

                debugger.logMessage("INFO", __LINE__, "CodeGen", "Code CodeGen Complete");
            }
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Code CodeGen Complete");
        return;
    }

    /**
     * Parses the given AST tree starting from the root node.
     *
     * @param root The root node of the AST tree.
     */
    void Generator::parseTree(ASTNode *root)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");

        if (!root)
        {
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Root is null!");
            return;
        }

        Generator &generator = this->compiler.getGenerator();

        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Program");
            generator.handleProgram(root);
            break;
        case NODE_FUNCTION_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Declaration");
            generator.handleFunctionDeclaration(root);
            break;
        case NODE_FUNCTION_BLOCK:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Block");
            generator.handleFunctionBlock(root);
            break;
        case NODE_RETURN_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            generator.handleReturnStatement(root);
            break;
        case NODE_FUNCTION_CALL:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            generator.handleFunctionCall(root);
            break;
        case NODE_VAR_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            generator.handleVariableDeclaration(root);
            break;
        case NODE_BINARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            generator.handleBinaryExpression(root);
            break;
        case NODE_UNARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Unary Expression");
            generator.handleUnaryExpression(root);
            break;
        case NODE_LITERAL_EXPR:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            generator.handleLiteralExpression(root);
            break;
        case NODE_IF_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling If Statement");
            generator.handleIfStatement(root);
            break;
        case NODE_WHILE_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling While Statement");
            generator.handleWhileStatement(root);
            break;
        case NODE_FOR_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling For Statement");
            generator.handleForStatement(root);
            break;
        default:
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown Node Type");
            exit(EXIT_FAILURE);
            break;
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Tree Parsed");

        return;
    }
} // namespace Cryo