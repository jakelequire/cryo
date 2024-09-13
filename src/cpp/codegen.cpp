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
        assert(root != nullptr);

        compiler.getGenerator().generateCode(root);
    }

    /// @private
    void Generator::preprocess(ASTNode *root)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Declarations &declarations = compiler.getDeclarations();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Code Generation");

        // Get / Set the symbol table for the module and its state
        std::string namespaceName = getNamespace(root);
        compiler.getSymTable().initModule(root, namespaceName);
        compiler.getContext().currentNamespace = namespaceName;

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Linting Tree");
        bool validateTree = debugger.lintTree(root);
        if (validateTree == false)
        {
            std::cerr << "[CPP] Tree is invalid!" << std::endl;
            return;
        }

        // Declare all functions in the AST tree
        // declarations.preprocessDeclare(root); <- TODO: Implement this function

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Preprocessing Complete");
        return;
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
        assert(cryoContext.module != nullptr);

        // Preprocess the AST tree
        preprocess(root);

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
        case NODE_EXTERN_FUNCTION:
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Extern Function");
            generator.handleExternFunction(root);
            break;
        case NODE_NAMESPACE:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Namespace");
            // set the module name
            std::string moduleName = std::string(root->data.cryoNamespace->name);
            std::cout << "Module Name: " << moduleName << std::endl;
            compiler.getContext().module->setModuleIdentifier(moduleName);
            // Set the namespace name
            break;
        }
        case NODE_BLOCK:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Block");
            ASTNode **statements = root->data.block->statements;
            int statementCount = root->data.block->statementCount;
            for (int i = 0; i < statementCount; i++)
            {
                generator.parseTree(statements[i]);
            }
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown Node Type");
            std::cout << "Received: " << CryoNodeTypeToString(root->metaData->type) << std::endl;
            exit(EXIT_FAILURE);
            break;
        }

        debugger.logMessage("INFO", __LINE__, "CodeGen", "Tree Parsed");

        return;
    }

    llvm::Value *Generator::getInitilizerValue(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Generator &generator = compiler.getGenerator();
        Types &types = compiler.getTypes();

        CryoNodeType nodeType = node->metaData->type;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            llvmValue = generator.handleLiteralExpression(node);
            break;
        }
        case NODE_RETURN_STATEMENT:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            // debugger.logNode(node);
            CryoDataType dataType = node->data.returnStatement->returnType;
            std::cout << "Data Type: " << CryoDataTypeToString(dataType) << std::endl;

            switch (dataType)
            {
            case DATA_TYPE_INT:
            case DATA_TYPE_FLOAT:
            case DATA_TYPE_BOOLEAN:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (int/float/boolean)");
                llvmValue = generator.handleLiteralExpression(node->data.returnStatement->expression);
                break;
            }
            case DATA_TYPE_STRING:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (string)");
                llvmValue = generator.handleLiteralExpression(node->data.returnStatement->expression);
                break;
            }
            case DATA_TYPE_VOID:
            {
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression (void)");
                llvmValue = llvm::UndefValue::get(llvm::Type::getVoidTy(cryoContext.context));
                debugger.logMessage("INFO", __LINE__, "CodeGen", "Returning Constant");
                break;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown data type");
                std::cout << "Received: " << CryoDataTypeToString(dataType) << std::endl;
                exit(1);
            }
            }
            break;
        }
        case NODE_VAR_DECLARATION:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            llvmValue = variables.getVariable(node->data.varDecl->name);
            break;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            llvmValue = variables.getVariable(node->data.varName->varName);
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            exit(1);
        }

        return llvmValue;
    }

    std::string Generator::getNamespace(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "CodeGen", "Getting Namespace");

        std::string namespaceName = "";

        int count = node->data.program->statementCount;
        for (int i = 0; i < count; i++)
        {
            CryoNodeType nodeType = node->data.program->statements[i]->metaData->type;
            if (nodeType == NODE_NAMESPACE)
            {
                namespaceName = node->data.program->statements[i]->data.cryoNamespace->name;
                break;
            }
        }
        std::cout << "Namespace: " << namespaceName << std::endl;

        return namespaceName;
    }

} // namespace Cryo
