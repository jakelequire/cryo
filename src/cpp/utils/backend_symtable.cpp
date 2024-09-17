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
            std::cout << "Variable Name: " << nodeName << std::endl;
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

    CryoVariableNode *BackendSymTable::getVariableNode(std::string namespaceName, std::string varName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        CryoVariableNode *varNode = &symNode.variables[varName];
        return varNode;
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
            std::cout << "Has Index Expression: " << var.second.hasIndexExpr << std::endl;
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

        SymTableNode program;

        // Start the recursive traversal
        traverseASTNode(root, program);

        std::cout << "[BackendSymTable] Module Traversal Complete" << std::endl;

        return program;
    }

    void BackendSymTable::traverseASTNode(ASTNode *node, SymTableNode &program)
    {
        if (!node)
            return;

        CryoDebugger &debugger = getDebugger();

        // Process the current node
        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing PROGRAM node");
            break;

        case NODE_NAMESPACE:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing NAMESPACE node");
            if (node->data.cryoNamespace)
            {
                // Store namespace information if needed
                // program.currentNamespace = node->data.cryoNamespace->name;
            }
            break;

        case NODE_BLOCK:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing BLOCK node");
            break;

        case NODE_FUNCTION_BLOCK:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing FUNCTION_BLOCK node");
            break;

        case NODE_EXTERN_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing EXTERN node");
            break;

        case NODE_EXTERN_FUNCTION:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing EXTERN_FUNCTION node");
            if (node->data.externFunction)
            {
                ExternFunctionNode externFuncNode;
                externFuncNode.name = node->data.externFunction->name;
                externFuncNode.returnType = node->data.externFunction->returnType;
                externFuncNode.paramCount = node->data.externFunction->paramCount;
                externFuncNode.params = node->data.externFunction->params;
                std::string externFuncNameStr = std::string(node->data.externFunction->name);
                program.externFunctions.insert({externFuncNameStr, externFuncNode});
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing FUNCTION_DECLARATION node");
            if (node->data.functionDecl)
            {
                FunctionDeclNode funcNode;
                funcNode.name = node->data.functionDecl->name;
                funcNode.returnType = node->data.functionDecl->returnType;
                funcNode.paramCount = node->data.functionDecl->paramCount;
                funcNode.params = node->data.functionDecl->params;
                funcNode.body = node->data.functionDecl->body;
                std::string funcNameStr = std::string(node->data.functionDecl->name);
                program.functions.insert({funcNameStr, funcNode});
            }
            break;

        case NODE_FUNCTION_CALL:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing FUNCTION_CALL node");
            break;

        case NODE_LITERAL_EXPR:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing LITERAL node");
            break;

        case NODE_VAR_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing VAR_DECLARATION node");
            if (node->data.varDecl)
            {
                CryoVariableNode varNode;
                varNode.name = node->data.varDecl->name;
                varNode.type = node->data.varDecl->type;
                varNode.isGlobal = node->data.varDecl->isGlobal;
                varNode.isReference = node->data.varDecl->isReference;
                varNode.initializer = node->data.varDecl->initializer;
                varNode.hasIndexExpr = node->data.varDecl->hasIndexExpr;
                varNode.indexExpr = node->data.varDecl->indexExpr;
                std::string varNameStr = std::string(node->data.varDecl->name);
                program.variables.insert({varNameStr, varNode});
            }
            break;

        case NODE_VAR_NAME:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing VAR_NAME node");
            break;

        case NODE_EXPRESSION:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing EXPRESSION node");
            break;

        case NODE_IF_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing IF_STATEMENT node");
            break;

        case NODE_FOR_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing FOR_STATEMENT node");
            break;

        case NODE_WHILE_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing WHILE_STATEMENT node");
            break;

        case NODE_BINARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing BINARY_EXPR node");
            break;

        case NODE_UNARY_EXPR:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing UNARY_EXPR node");
            break;

        case NODE_PARAM_LIST:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing PARAM_LIST node");
            break;

        case NODE_ARG_LIST:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing ARG_LIST node");
            break;

        case NODE_RETURN_STATEMENT:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing RETURN_STATEMENT node");
            break;

        case NODE_ARRAY_LITERAL:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing ARRAY_LITERAL node");
            break;

        case NODE_INDEX_EXPR:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing INDEX_EXPR node");
            break;

        case NODE_VAR_REASSIGN:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing VAR_REASSIGNMENT node");
            break;

        default:
            debugger.logMessage("WARNING", __LINE__, "BackendSymTable", "Unknown node type: " + std::to_string(node->metaData->type));
            break;
        }

        // Recursively traverse child nodes
        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            if (node->data.program)
            {
                for (size_t i = 0; i < node->data.program->statementCount; i++)
                {
                    traverseASTNode(node->data.program->statements[i], program);
                }
            }
            break;

        case NODE_BLOCK:
            if (node->data.block)
            {
                for (int i = 0; i < node->data.block->statementCount; i++)
                {
                    traverseASTNode(node->data.block->statements[i], program);
                }
            }
            break;

        case NODE_FUNCTION_BLOCK:
            if (node->data.functionBlock)
            {
                for (int i = 0; i < node->data.functionBlock->statementCount; i++)
                {
                    traverseASTNode(node->data.functionBlock->statements[i], program);
                }
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            if (node->data.functionDecl)
            {
                for (int i = 0; i < node->data.functionDecl->paramCount; i++)
                {
                    traverseASTNode(node->data.functionDecl->params[i], program);
                }
                traverseASTNode(node->data.functionDecl->body, program);
            }
            break;

        case NODE_FUNCTION_CALL:
            if (node->data.functionCall)
            {
                for (int i = 0; i < node->data.functionCall->argCount; i++)
                {
                    traverseASTNode(node->data.functionCall->args[i], program);
                }
            }
            break;

        case NODE_VAR_DECLARATION:
            if (node->data.varDecl)
            {
                traverseASTNode(node->data.varDecl->initializer, program);
                if (node->data.varDecl->hasIndexExpr)
                {
                    traverseASTNode(reinterpret_cast<ASTNode *>(node->data.varDecl->indexExpr), program);
                }
            }
            break;

        case NODE_IF_STATEMENT:
            if (node->data.ifStatement)
            {
                traverseASTNode(node->data.ifStatement->condition, program);
                traverseASTNode(node->data.ifStatement->thenBranch, program);
                traverseASTNode(node->data.ifStatement->elseBranch, program);
            }
            break;

        case NODE_FOR_STATEMENT:
            if (node->data.forStatement)
            {
                traverseASTNode(node->data.forStatement->initializer, program);
                traverseASTNode(node->data.forStatement->condition, program);
                traverseASTNode(node->data.forStatement->increment, program);
                traverseASTNode(node->data.forStatement->body, program);
            }
            break;

        case NODE_WHILE_STATEMENT:
            if (node->data.whileStatement)
            {
                traverseASTNode(node->data.whileStatement->condition, program);
                traverseASTNode(node->data.whileStatement->body, program);
            }
            break;

        case NODE_BINARY_EXPR:
            if (node->data.bin_op)
            {
                traverseASTNode(node->data.bin_op->left, program);
                traverseASTNode(node->data.bin_op->right, program);
            }
            break;

        case NODE_UNARY_EXPR:
            if (node->data.unary_op)
            {
                traverseASTNode(node->data.unary_op->operand, program);
                traverseASTNode(node->data.unary_op->expression, program);
            }
            break;

        case NODE_RETURN_STATEMENT:
            if (node->data.returnStatement)
            {
                traverseASTNode(node->data.returnStatement->returnValue, program);
                traverseASTNode(node->data.returnStatement->expression, program);
            }
            break;

        case NODE_ARRAY_LITERAL:
            if (node->data.array)
            {
                for (int i = 0; i < node->data.array->elementCount; i++)
                {
                    traverseASTNode(node->data.array->elements[i], program);
                }
            }
            break;

        case NODE_INDEX_EXPR:
            if (node->data.indexExpr)
            {
                traverseASTNode(node->data.indexExpr->array, program);
                traverseASTNode(node->data.indexExpr->index, program);
            }
            break;

        case NODE_VAR_REASSIGN:
            if (node->data.varReassignment)
            {
                traverseASTNode(node->data.varReassignment->existingVarNode, program);
                traverseASTNode(node->data.varReassignment->newVarNode, program);
            }
            break;

        // For node types that don't have child nodes or have been fully processed
        case NODE_NAMESPACE:
        case NODE_EXTERN_STATEMENT:
        case NODE_EXTERN_FUNCTION:
        case NODE_LITERAL_EXPR:
        case NODE_VAR_NAME:
        case NODE_EXPRESSION:
        case NODE_PARAM_LIST:
        case NODE_ARG_LIST:
            // No further traversal needed
            break;

        default:
            debugger.logMessage("WARNING", __LINE__, "BackendSymTable", "No traversal defined for node type: " + std::to_string(node->metaData->type));
            break;
        }
    }
} // namespace Cryo
