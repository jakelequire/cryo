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
#include "cpp/backend_symtable.hpp"

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

    // -----------------------------------------------------------------------------------------------

    ///
    /// Getters
    ///

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

    STVariable *BackendSymTable::getVariable(std::string namespaceName, std::string varName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable *varNode = &symNode.variableNode[varName];
        return varNode;
    }

    // -----------------------------------------------------------------------------------------------

    /// Struct Containers
    STVariable BackendSymTable::createVarContainer(ASTNode *varNode)
    {
        STVariable varContainer;
        varContainer.ASTNode = varNode;
        varContainer.nodeType = varNode->metaData->type;
        varContainer.dataType = DATA_TYPE_UNKNOWN;
        varContainer.LLVMValue = nullptr;
        varContainer.LLVMType = nullptr;
        varContainer.LLVMStoreInst = nullptr;

        return varContainer;
    }

    STFunction BackendSymTable::createFuncContainer(FunctionDeclNode *funcNode)
    {
        STFunction funcContainer;
        funcContainer.ASTNode = funcNode;
        funcContainer.LLVMFunction = nullptr;
        funcContainer.LLVMReturnType = nullptr;
        funcContainer.LLVMParamTypes.clear();
        funcContainer.returnType = funcNode->returnType;

        return funcContainer;
    }

    STExternFunction BackendSymTable::createExternFuncContainer(ExternFunctionNode *externNode)
    {
        STExternFunction externFuncContainer;
        externFuncContainer.ASTNode = externNode;
        externFuncContainer.LLVMFunction = nullptr;
        externFuncContainer.LLVMReturnType = nullptr;
        externFuncContainer.LLVMParamTypes.clear();
        externFuncContainer.returnType = externNode->returnType;

        return externFuncContainer;
    }

    // -----------------------------------------------------------------------------------------------

    ///
    /// Setters
    ///

    void BackendSymTable::addStruct(std::string namespaceName, llvm::StructType *structTy, StructNode *structNode)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Struct to SymTable");

        // Add the struct to the SymTable
        std::string structName = structTy->getName().str();
        std::cout << "Struct Name: " << structName << std::endl;

        StructValue structValue;
        structValue.ASTStruct = *structNode;
        structValue.LLVMStruct = structTy;

        SymTableNode symNode = getSymTableNode(namespaceName);
        symNode.structs[structName] = structValue;

        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Struct Added" << std::endl;

        return;
    }

    void BackendSymTable::addVariable(std::string namespaceName, std::string varName, ASTNode *varNode)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Variable to SymTable");

        // Add the variable to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Create the variable container
        STVariable varContainer = createVarContainer(varNode);

        // Add the variable to the SymTable
        symNode.variableNode[varName] = varContainer;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Variable Added" << std::endl;

        return;
    }

    void BackendSymTable::addFunction(std::string namespaceName, std::string funcName, FunctionDeclNode funcNode)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Function to SymTable");

        // Add the function to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Create the function container
        STFunction funcContainer = createFuncContainer(&funcNode);

        // Add the function to the SymTable
        symNode.functionNode[funcName] = funcContainer;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Function Added" << std::endl;

        return;
    }

    void BackendSymTable::addExternFunciton(std::string namespaceName, std::string funcName, ExternFunctionNode externNode)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Extern Function to SymTable");

        // Add the extern function to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Create the extern function container
        STExternFunction externFuncContainer = createExternFuncContainer(&externNode);

        // Add the extern function to the SymTable
        symNode.externFunctionNode[funcName] = externFuncContainer;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Extern Function Added" << std::endl;

        return;
    }

    // -----------------------------------------------------------------------------------------------

    /// ### ============================================================================= ###
    /// ###
    /// ### Update Functions
    /// ### These functions are used to update existing nodes in the SymTable
    /// ###
    /// ### ============================================================================= ###

    void BackendSymTable::updateVariableNode(std::string namespaceName, std::string varName, llvm::Value *llvmValue, llvm::Type *llvmType)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Updating Variable Node");

        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];

        // Update the variable node
        varNode.LLVMValue = llvmValue;
        varNode.LLVMType = llvmType;

        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Variable Node Updated" << std::endl;

        return;
    }

    void BackendSymTable::addStoreInstToVar(std::string namespaceName, std::string varName, llvm::StoreInst *storeInst)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Store Instruction to Variable");

        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];

        // Add the store instruction to the variable node
        varNode.LLVMStoreInst = storeInst;

        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Store Instruction Added to Variable" << std::endl;

        return;
    }

    void BackendSymTable::addParamAsVariable(std::string namespaceName, std::string paramName, llvm::Value *llvmValue, llvm::Type *llvmType, llvm::StoreInst *storeInst)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Adding Parameter as Variable");

        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Create the variable container
        STVariable varContainer;
        varContainer.LLVMValue = llvmValue;
        varContainer.LLVMType = llvmType;
        varContainer.LLVMStoreInst = storeInst;

        // Add the variable to the SymTable
        symNode.variableNode[paramName] = varContainer;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Parameter Added as Variable" << std::endl;

        return;
    }

    void BackendSymTable::updateFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Updating Function Node");

        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Find the function in the SymTable
        STFunction funcNode = symNode.functionNode[funcName];

        // Update the function node
        funcNode.LLVMFunction = llvmFunction;
        funcNode.LLVMReturnType = llvmReturnType;
        funcNode.LLVMParamTypes = llvmParamTypes;

        // Update the function in the SymTable
        symNode.functionNode[funcName] = funcNode;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Function Node Updated" << std::endl;

        return;
    }

    void BackendSymTable::updateExternFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes)
    {
        CryoDebugger &debugger = getDebugger();
        debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Updating Extern Function Node");

        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);

        // Find the extern function in the SymTable
        STExternFunction externFuncNode = symNode.externFunctionNode[funcName];

        // Update the extern function node
        externFuncNode.LLVMFunction = llvmFunction;
        externFuncNode.LLVMReturnType = llvmReturnType;
        externFuncNode.LLVMParamTypes = llvmParamTypes;

        // Update the extern function in the SymTable
        symNode.externFunctionNode[funcName] = externFuncNode;
        symTable.namespaces[namespaceName] = symNode;

        std::cout << "[BackendSymTable] Extern Function Node Updated" << std::endl;

        return;
    }

    // -----------------------------------------------------------------------------------------------

    ///
    /// Debugging
    ///

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

    void BackendSymTable::traverseASTNode(ASTNode *node, SymTableNode &symTable)
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
                symTable.externFunctions.insert({externFuncNameStr, externFuncNode});

                // New Implementation
                STExternFunction externFuncContainer = createExternFuncContainer(&externFuncNode);
                symTable.externFunctionNode[externFuncNameStr] = externFuncContainer;
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
                symTable.functions.insert({funcNameStr, funcNode});

                // New Implementation
                STFunction funcContainer = createFuncContainer(&funcNode);
                symTable.functionNode[funcNameStr] = funcContainer;
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
                symTable.variables.insert({varNameStr, varNode});

                // New Implementation
                STVariable varContainer = createVarContainer(node);
                symTable.variableNode[varNameStr] = varContainer;
            }
            break;

        case NODE_VAR_NAME:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing VAR_NAME node");
            if (node->data.varName)
            {
                STVariable varContainer = createVarContainer(node);
                std::string varNameStr = std::string(node->data.varName->varName);
                symTable.variableNode[varNameStr] = varContainer;
            }
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

        case NODE_STRUCT_DECLARATION:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing STRUCT_DECLARATION node");
            break;
        case NODE_SCOPED_FUNCTION_CALL:
            debugger.logMessage("INFO", __LINE__, "BackendSymTable", "Processing SCOPED_FUNCTION_CALL node");
            break;
        default:
        {
            CryoNodeType nodeType = node->metaData->type;
            debugger.logMessage("ERROR", __LINE__, "BackendSymTable", "Unknown node type: " + std::to_string(nodeType));
            break;
        }
        }

        // Recursively traverse child nodes
        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            if (node->data.program)
            {
                for (size_t i = 0; i < node->data.program->statementCount; i++)
                {
                    traverseASTNode(node->data.program->statements[i], symTable);
                }
            }
            break;

        case NODE_BLOCK:
            if (node->data.block)
            {
                for (int i = 0; i < node->data.block->statementCount; i++)
                {
                    traverseASTNode(node->data.block->statements[i], symTable);
                }
            }
            break;

        case NODE_FUNCTION_BLOCK:
            if (node->data.functionBlock)
            {
                for (int i = 0; i < node->data.functionBlock->statementCount; i++)
                {
                    traverseASTNode(node->data.functionBlock->statements[i], symTable);
                }
            }
            break;

        case NODE_FUNCTION_DECLARATION:
            if (node->data.functionDecl)
            {
                for (int i = 0; i < node->data.functionDecl->paramCount; i++)
                {
                    traverseASTNode(node->data.functionDecl->params[i], symTable);
                }
                traverseASTNode(node->data.functionDecl->body, symTable);
            }
            break;

        case NODE_FUNCTION_CALL:
            if (node->data.functionCall)
            {
                for (int i = 0; i < node->data.functionCall->argCount; i++)
                {
                    traverseASTNode(node->data.functionCall->args[i], symTable);
                }
            }
            break;

        case NODE_VAR_DECLARATION:
            if (node->data.varDecl)
            {
                traverseASTNode(node->data.varDecl->initializer, symTable);
                if (node->data.varDecl->hasIndexExpr)
                {
                    traverseASTNode(reinterpret_cast<ASTNode *>(node->data.varDecl->indexExpr), symTable);
                }
            }
            break;

        case NODE_IF_STATEMENT:
            if (node->data.ifStatement)
            {
                traverseASTNode(node->data.ifStatement->condition, symTable);
                traverseASTNode(node->data.ifStatement->thenBranch, symTable);
                traverseASTNode(node->data.ifStatement->elseBranch, symTable);
            }
            break;

        case NODE_FOR_STATEMENT:
            if (node->data.forStatement)
            {
                traverseASTNode(node->data.forStatement->initializer, symTable);
                traverseASTNode(node->data.forStatement->condition, symTable);
                traverseASTNode(node->data.forStatement->increment, symTable);
                traverseASTNode(node->data.forStatement->body, symTable);
            }
            break;

        case NODE_WHILE_STATEMENT:
            if (node->data.whileStatement)
            {
                traverseASTNode(node->data.whileStatement->condition, symTable);
                traverseASTNode(node->data.whileStatement->body, symTable);
            }
            break;

        case NODE_BINARY_EXPR:
            if (node->data.bin_op)
            {
                traverseASTNode(node->data.bin_op->left, symTable);
                traverseASTNode(node->data.bin_op->right, symTable);
            }
            break;

        case NODE_UNARY_EXPR:
            if (node->data.unary_op)
            {
                traverseASTNode(node->data.unary_op->operand, symTable);
                traverseASTNode(node->data.unary_op->expression, symTable);
            }
            break;

        case NODE_RETURN_STATEMENT:
            if (node->data.returnStatement)
            {
                traverseASTNode(node->data.returnStatement->returnValue, symTable);
                traverseASTNode(node->data.returnStatement->expression, symTable);
            }
            break;

        case NODE_ARRAY_LITERAL:
            if (node->data.array)
            {
                for (int i = 0; i < node->data.array->elementCount; i++)
                {
                    traverseASTNode(node->data.array->elements[i], symTable);
                }
            }
            break;

        case NODE_INDEX_EXPR:
            if (node->data.indexExpr)
            {
                traverseASTNode(node->data.indexExpr->array, symTable);
                traverseASTNode(node->data.indexExpr->index, symTable);
            }
            break;

        case NODE_VAR_REASSIGN:
            if (node->data.varReassignment)
            {
                traverseASTNode(node->data.varReassignment->existingVarNode, symTable);
                traverseASTNode(node->data.varReassignment->newVarNode, symTable);
            }
            break;

        case NODE_STRUCT_DECLARATION:
        {
            if (node->data.structNode)
            {
                for (int i = 0; i < node->data.structNode->propertyCount; i++)
                {
                    traverseASTNode(node->data.structNode->properties[i], symTable);
                }
            }
            break;
        }

        case NODE_SCOPED_FUNCTION_CALL:
        {
        }

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
