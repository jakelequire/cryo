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
#include "cpp/backend_symtable.h"

namespace Cryo
{

    void BackendSymTable::initSymTable()
    {
        std::cout << "[BackendSymTable] SymTable Initialized" << std::endl;
    }

    void BackendSymTable::initModule(ASTNode *root, std::string namespaceName)
    {
        std::cout << "[BackendSymTable] Creating Module" << std::endl;
        // Create the main namespace
        SymTableNode program;

        assert(root != nullptr);
        assert(namespaceName != "");

        // Check to make sure the namespace doesn't already exist
        if (symTable.namespaces.find(namespaceName) != symTable.namespaces.end())
        {
            std::cerr << "[BackendSymTable] Namespace already exists" << std::endl;
            return;
        }

        // Add the namespace to the SymTable
        symTable.namespaces[namespaceName] = program;

        // Traverse the AST and populate the SymTable
        SymTableNode module = traverseModule(root, namespaceName);
        symTable.namespaces[namespaceName] = module;

        module.namespaceName = namespaceName;

        std::cout << "\n\n";
        std::cout << "[BackendSymTable] Module Created" << std::endl;
        std::cout << "\n\n";
    }

    ASTNode *BackendSymTable::getASTNode(std::string namespaceName, CryoNodeType nodeType, std::string nodeName)
    {
        CryoDebugger &debugger = getDebugger();
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        ASTNode *node = nullptr;

        switch (nodeType)
        {
        case NODE_VAR_DECLARATION:
        {
            // Find the variable in the SymTable
            CryoVariableNode varNode = symNode.variables[nodeName];
            // std::cout << "===-------------------===" << std::endl;
            // debugger.logNode(varNode.initializer);
            // std::cout << "===-------------------===" << std::endl;

            return varNode.initializer;
        }
        case NODE_FUNCTION_DECLARATION:
        {
            FunctionDeclNode funcNode = symNode.functions[nodeName];
            return funcNode.body;
        }
        case NODE_EXTERN_FUNCTION:
        {
            ExternFunctionNode externFuncNode = symNode.externFunctions[nodeName];
            node = nullptr;
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "BackendSymTable", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            break;
        }
        }

        return node;
    }

    SymTable BackendSymTable::getSymTable()
    {
        return symTable;
    }

    SymTableNode BackendSymTable::getSymTableNode(std::string namespaceName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = symTable.namespaces[namespaceName];
        return symNode;
    }

    void BackendSymTable::printTable(std::string namespaceName)
    {
        CryoDebugger &debugger = getDebugger();
        // Find the namespace in the SymTable and print it
        SymTableNode symNode = getSymTableNode(namespaceName);

        std::cout << "[BackendSymTable] Printing SymTable" << std::endl;

        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Namespace: " << namespaceName << std::endl;

        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Variables: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;

        for (auto const &var : symNode.variables)
        {
            std::cout << "Name: " << var.first << std::endl;
            std::cout << "Type: " << var.second.type << std::endl;
            std::cout << "Is Global: " << var.second.isGlobal << std::endl;
            std::cout << "Is Reference: " << var.second.isReference << std::endl;
            std::cout << "Initializer: " << std::endl;
            std::cout << ">>------------------<<" << std::endl;
            debugger.logNode(var.second.initializer);
            std::cout << ">>------------------<<" << std::endl;
        }

        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Functions: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;

        for (auto const &func : symNode.functions)
        {
            std::cout << "Name: " << func.first << std::endl;
            std::cout << "Return Type: " << func.second.returnType << std::endl;
            std::cout << "Param Count: " << func.second.paramCount << std::endl;
            for (int i = 0; i < func.second.paramCount; i++)
            {
                std::cout << ">>------------------<<" << std::endl;
                std::cout << "Param " << i << ": " << func.second.params[i] << std::endl;
                debugger.logNode(func.second.params[i]);
                std::cout << ">>------------------<<" << std::endl;
            }
            std::cout << "Body: " << std::endl;
            std::cout << ">>------------------<<" << std::endl;
            debugger.logNode(func.second.body);
            std::cout << ">>------------------<<" << std::endl;
        }

        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Extern Functions: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;

        for (auto const &externFunc : symNode.externFunctions)
        {
            std::cout << "Name: " << externFunc.first << std::endl;
            std::cout << "Return Type: " << externFunc.second.returnType << std::endl;
            std::cout << "Param Count: " << externFunc.second.paramCount << std::endl;
            for (int i = 0; i < externFunc.second.paramCount; i++)
            {
                std::cout << "Param " << i << ": " << std::endl;
                std::cout << ">>------------------<<" << std::endl;
                debugger.logNode(externFunc.second.params[i]);
                std::cout << ">>------------------<<" << std::endl;
            }
        }
    }

    // -----------------------------------------------------------------------------------------------

    SymTableNode BackendSymTable::traverseModule(ASTNode *root, std::string namespaceName)
    {
        std::cout << "[BackendSymTable] Traversing Module" << std::endl;
        // Traverse the AST and populate the SymTable

        SymTableNode program;

        // Traverse the AST and populate the SymTable
        int count = root->data.program->statementCount;
        for (int i = 0; i < count; i++)
        {
            ASTNode *node = root->data.program->statements[i];
            switch (node->metaData->type)
            {
            case NODE_VAR_DECLARATION:
            {
                if (node->data.varDecl->initializer->metaData->type == NODE_INDEX_EXPR)
                {
                    std::cout << "Index Expression" << std::endl;

                    DEBUG_BREAKPOINT;
                }
                // Add the variable to the SymTable
                char *varName = node->data.varDecl->name;
                CryoVariableNode varNode;
                varNode.name = varName;
                varNode.type = node->data.varDecl->type;
                varNode.isGlobal = node->data.varDecl->isGlobal;
                varNode.isReference = node->data.varDecl->isReference;
                varNode.initializer = node->data.varDecl->initializer;
                std::string varNameStr = std::string(varName);
                program.variables.insert({varNameStr, varNode});
                debugger->logNode(node);
                break;
            }
            case NODE_FUNCTION_DECLARATION:
            {
                // Add the function to the SymTable
                char *funcName = node->data.functionDecl->name;
                FunctionDeclNode funcNode;
                funcNode.name = funcName;
                funcNode.returnType = node->data.functionDecl->returnType;
                funcNode.paramCount = node->data.functionDecl->paramCount;
                funcNode.params = node->data.functionDecl->params;
                funcNode.body = node->data.functionDecl->body;
                std::string funcNameStr = std::string(funcName);
                program.functions.insert({funcNameStr, funcNode});
                break;
            }
            case NODE_EXTERN_FUNCTION:
            {
                // Add the extern function to the SymTable
                char *externFuncName = node->data.externFunction->name;
                ExternFunctionNode externFuncNode;
                externFuncNode.name = externFuncName;
                externFuncNode.returnType = node->data.externFunction->returnType;
                externFuncNode.paramCount = node->data.externFunction->paramCount;
                externFuncNode.params = node->data.externFunction->params;
                std::string externFuncNameStr = std::string(externFuncName);
                program.externFunctions.insert({externFuncNameStr, externFuncNode});
                break;
            }
            default:
                std::cerr << "[BackendSymTable] Unknown Node Type" << std::endl;
                break;
            }
        }
        std::cout << "[BackendSymTable] Module Traversal Complete" << std::endl;

        return program;
    }

} // namespace Cryo
