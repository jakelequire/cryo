// /********************************************************************************
//  *  Copyright 2024 Jacob LeQuire                                                *
//  *  SPDX-License-Identifier: Apache-2.0                                         *
//  *    Licensed under the Apache License, Version 2.0 (the "License");           *
//  *    you may not use this file except in compliance with the License.          *
//  *    You may obtain a copy of the License at                                   *
//  *                                                                              *
//  *    http://www.apache.org/licenses/LICENSE-2.0                                *
//  *                                                                              *
//  *    Unless required by applicable law or agreed to in writing, software       *
//  *    distributed under the License is distributed on an "AS IS" BASIS,         *
//  *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
//  *    See the License for the specific language governing permissions and       *
//  *    limitations under the License.                                            *
//  *                                                                              *
//  ********************************************************************************/
#include "codegen/IRSymTable/IRSymTable.hpp"

namespace Cryo
{
    void IRSymTable::initSymTable()
    {
        std::cout << "[IRSymTable] SymTable Initialized" << std::endl;
    }

    void IRSymTable::initModule(ASTNode *root, std::string namespaceName)
    {
        std::cout << "[IRSymTable] Creating Module" << std::endl;
        // Create the main namespace
        SymTableNode program;
        assert(root != nullptr);
        assert(namespaceName != "");
        // Check to make sure the namespace doesn't already exist
        if (symTable.namespaces.find(namespaceName) != symTable.namespaces.end())
        {
            std::cerr << "[IRSymTable] Namespace already exists" << std::endl;
            return;
        }
        // Add the namespace to the SymTable
        symTable.namespaces[namespaceName] = program;
        // Traverse the AST and populate the SymTable
        SymTableNode module = traverseModule(root, namespaceName);
        symTable.namespaces[namespaceName] = module;
        module.namespaceName = namespaceName;
        std::cout << "\n\n";
        std::cout << "[IRSymTable] Module Created" << std::endl;
        std::cout << "\n\n";
    }

    // -----------------------------------------------------------------------------------------------
    ///
    /// Getters
    ///

    ASTNode *IRSymTable::getASTNode(std::string namespaceName, CryoNodeType nodeType, std::string nodeName)
    {
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
            // DevDebugger::logNode(varNode.initializer);
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
            DevDebugger::logMessage("ERROR", __LINE__, "IRSymTable", "Unknown node type");
            std::cout << "Received: " << CryoNodeTypeToString(nodeType) << std::endl;
            break;
        }
        }
        return node;
    }

    CryoVariableNode *IRSymTable::getVariableNode(std::string namespaceName, std::string varName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        CryoVariableNode *varNode = &symNode.variables[varName];
        return varNode;
    }

    SymTable IRSymTable::getSymTable()
    {
        return symTable;
    }

    SymTableNode IRSymTable::getSymTableNode(std::string namespaceName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = symTable.namespaces[namespaceName];
        return symNode;
    }

    STVariable *IRSymTable::getVariable(std::string namespaceName, std::string varName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable *varNode = &symNode.variableNode[varName];
        return varNode;
    }

    STParameter *IRSymTable::getParameter(std::string namespaceName, std::string paramName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the parameter in the SymTable
        STParameter *paramNode = &symNode.parameterNode[paramName];
        return paramNode;
    }

    STStruct *IRSymTable::getStruct(std::string namespaceName, std::string structName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the struct in the SymTable
        STStruct *structNode = &symNode.structNode[structName];
        return structNode;
    }

    PropertyNode *IRSymTable::getPropertyNode(std::string namespaceName, std::string propName, std::string structName)
    {
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the struct in the SymTable
        STStruct structNode = symNode.structNode[structName];
        if (structNode.ASTNode)
        {
            // Find the property in the struct
            for (int i = 0; i < structNode.ASTNode->propertyCount; ++i)
            {
                PropertyNode *propNode = structNode.ASTNode->properties[i]->data.property;
                if (propNode->name == propName)
                {
                    return propNode;
                }
            }
        }
        else
        {
            DevDebugger::logMessage("ERROR", __LINE__, "IRSymTable", "Struct Node not found");
        }
    }

    // -----------------------------------------------------------------------------------------------
    /// Struct Containers

    STVariable IRSymTable::createVarContainer(ASTNode *varNode)
    {
        STVariable varContainer;
        varContainer.ASTNode = varNode;
        varContainer.nodeType = varNode->metaData->type;
        varContainer.LLVMValue = nullptr;
        varContainer.LLVMType = nullptr;
        varContainer.LLVMStoreInst = nullptr;
        varContainer.LLVMLoadInst = nullptr;
        varContainer.LLVMFunctionPtr = nullptr;
        varContainer.dataType = varNode->data.varDecl->type;
        return varContainer;
    }

    STFunction IRSymTable::createFuncContainer(FunctionDeclNode *funcNode)
    {
        STFunction funcContainer;
        funcContainer.ASTNode = funcNode;
        funcContainer.LLVMFunction = nullptr;
        funcContainer.LLVMReturnType = nullptr;
        funcContainer.LLVMParamTypes.clear();
        funcContainer.returnType = funcNode->type;
        return funcContainer;
    }

    STExternFunction IRSymTable::createExternFuncContainer(ExternFunctionNode *externNode)
    {
        STExternFunction externFuncContainer;
        externFuncContainer.ASTNode = externNode;
        externFuncContainer.LLVMFunction = nullptr;
        externFuncContainer.LLVMReturnType = nullptr;
        externFuncContainer.LLVMParamTypes.clear();
        externFuncContainer.returnType = externNode->type;
        return externFuncContainer;
    }

    STParameter IRSymTable::createParamContainer(void)
    {

        STParameter paramContainer;
        paramContainer.ASTNode = nullptr;
        paramContainer.LLVMValue = nullptr;
        paramContainer.LLVMType = nullptr;
        paramContainer.LLVMStoreInst = nullptr;
        paramContainer.paramName = "NULL";
        paramContainer.functionName = "NULL";
        paramContainer.nodeType = NODE_UNKNOWN;
        paramContainer.dataType = createUnknownType();
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Parameter Container Created");
        return paramContainer;
    }

    // -----------------------------------------------------------------------------------------------
    ///
    /// Setters
    ///

    void IRSymTable::addStruct(std::string namespaceName, llvm::StructType *structTy, StructNode *structNode, DataType *structType)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Struct to SymTable");
        // Add the struct to the SymTable
        std::string structName = structTy->getName().str();
        std::cout << "Struct Name: " << structName << std::endl;

        STStruct structContainer;
        structContainer.LLVMStruct = structTy;
        structContainer.ASTNode = structNode;
        structContainer.structType = structType;

        SymTableNode symNode = getSymTableNode(namespaceName);
        symNode.structNode[structName] = structContainer;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Struct Added to SymTable");

        return;
    }

    void IRSymTable::addStructMethod(std::string namespaceName, std::string structName, llvm::Function *llvmFunction)
    {
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Struct Method to SymTable");
        // Add the struct method to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        STStruct structNode = symNode.structNode[structName];
        structNode.LLVMMethods.push_back(llvmFunction);
        symNode.structNode[structName] = structNode;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Struct Method Added to SymTable");

        return;
    }

    void IRSymTable::addVariable(std::string namespaceName, std::string varName, ASTNode *varNode)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Variable to SymTable");
        // Add the variable to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Create the variable container
        STVariable varContainer = createVarContainer(varNode);
        // Add the variable to the SymTable
        symNode.variableNode[varName] = varContainer;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Variable Added to SymTable");

        return;
    }

    void IRSymTable::addFunction(std::string namespaceName, std::string funcName, FunctionDeclNode funcNode, llvm::Function *llvmFunction, llvm::Type *llvmReturnType)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Function to SymTable");
        std::cout << "Function Name: " << funcName << std::endl;
        std::cout << "Namespace Name: " << namespaceName << std::endl;
        // Add the function to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Create the function container
        STFunction funcContainer = createFuncContainer(&funcNode);
        funcContainer.LLVMFunction = llvmFunction;
        funcContainer.LLVMReturnType = llvmReturnType;
        funcContainer.ASTNode = &funcNode;

        // Add the function to the SymTable
        symNode.functionNode[funcName] = funcContainer;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Function Added to SymTable");

        return;
    }

    void IRSymTable::addExternFunciton(std::string namespaceName, std::string funcName, ExternFunctionNode externNode)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Extern Function to SymTable");
        // Add the extern function to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Create the extern function container
        STExternFunction externFuncContainer = createExternFuncContainer(&externNode);
        // Add the extern function to the SymTable
        symNode.externFunctionNode[funcName] = externFuncContainer;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Extern Function Added to SymTable");

        return;
    }

    void IRSymTable::addParameter(std::string namespaceName, std::string paramName, ASTNode *paramNode)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Parameter to SymTable");
        // Add the parameter to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        std::cout << "Parameter Name: " << paramName << std::endl;
        std::cout << "Namespace Name: " << namespaceName << std::endl;
        // Create the parameter container
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Creating Parameter Container");
        STParameter paramContainer = createParamContainer();
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Parameter Container Created");
        paramContainer.paramName = paramName;
        paramContainer.ASTNode = paramNode;
        paramContainer.dataType = paramNode->data.param->type;
        paramContainer.nodeType = paramNode->metaData->type;
        // Add the parameter to the SymTable
        symNode.parameterNode[paramName] = paramContainer;
        symTable.namespaces[namespaceName] = symNode;

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Parameter Added to SymTable");

        return;
    }

    // -----------------------------------------------------------------------------------------------
    /// ### ============================================================================= ###
    /// ###
    /// ### Update Functions
    /// ### These functions are used to update existing nodes in the SymTable
    /// ###
    /// ### ============================================================================= ###

    void IRSymTable::updateVariableNode(std::string namespaceName, std::string varName, llvm::Value *llvmValue, llvm::Type *llvmType)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Variable Node");
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
        std::cout << "[IRSymTable] Variable Node Updated" << std::endl;
        return;
    }

    void IRSymTable::addStoreInstToVar(std::string namespaceName, std::string varName, llvm::StoreInst *storeInst)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Store Instruction to Variable");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];
        // Add the store instruction to the variable node
        varNode.LLVMStoreInst = storeInst;
        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Store Instruction Added to Variable" << std::endl;
        return;
    }

    void IRSymTable::addDataTypeToVar(std::string namespaceName, std::string varName, DataType *dataType)
    {
        // This should only update the data type if the data type in the `STVariable` is a nullptr or a `UnknownType`
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Data Type to Variable");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];
        // Add the data type to the variable node
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Data Type");
        varNode.dataType = dataType;
        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Data Type Added to Variable" << std::endl;
        return;
    }

    void IRSymTable::addFunctionToVar(std::string namespaceName, std::string varName, llvm::Value *functionPtr)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Function to Variable");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];
        // Add the function to the variable node
        varNode.LLVMFunctionPtr = functionPtr;
        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Function Added to Variable" << std::endl;
        return;
    }

    void IRSymTable::addLoadInstToVar(std::string namespaceName, std::string varName, llvm::LoadInst *loadInst)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Load Instruction to Variable");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the variable in the SymTable
        STVariable varNode = symNode.variableNode[varName];
        // Add the load instruction to the variable node
        varNode.LLVMLoadInst = loadInst;
        // Update the variable in the SymTable
        symNode.variableNode[varName] = varNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Load Instruction Added to Variable" << std::endl;
        return;
    }

    void IRSymTable::addParamAsVariable(std::string namespaceName, std::string paramName, llvm::Value *llvmValue, llvm::Type *llvmType, llvm::StoreInst *storeInst)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Adding Parameter as Variable");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Create the variable container
        STVariable varContainer;
        varContainer.LLVMValue = llvmValue;
        varContainer.LLVMType = llvmType;
        varContainer.LLVMStoreInst = storeInst;
        varContainer.ASTNode = nullptr;
        // Add the variable to the SymTable
        symNode.variableNode[paramName] = varContainer;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Parameter Added as Variable" << std::endl;
        return;
    }

    void IRSymTable::updateFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Function Node");
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
        std::cout << "[IRSymTable] Function Node Updated" << std::endl;
        return;
    }

    void IRSymTable::updateExternFunctionNode(std::string namespaceName, std::string funcName, llvm::Function *llvmFunction, llvm::Type *llvmReturnType, std::vector<llvm::Type *> llvmParamTypes)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Extern Function Node");
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
        std::cout << "[IRSymTable] Extern Function Node Updated" << std::endl;
        return;
    }

    void IRSymTable::updateParam(std::string namespaceName, std::string paramName, llvm::Value *llvmValue, llvm::Type *llvmType)
    {

        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Parameter");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the parameter in the SymTable
        STParameter paramNode = symNode.parameterNode[paramName];
        // Update the parameter
        paramNode.LLVMValue = llvmValue;
        paramNode.LLVMType = llvmType;
        // Update the parameter in the SymTable
        symNode.parameterNode[paramName] = paramNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Parameter Updated" << std::endl;
        return;
    }

    void IRSymTable::updateStructConstructor(std::string namespaceName, std::string structName, llvm::Function *llvmConstructor)
    {
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Updating Struct Constructor");
        // Find the namespace in the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Find the struct in the SymTable
        STStruct structNode = symNode.structNode[structName];
        // Update the struct constructor
        structNode.LLVMConstructor = llvmConstructor;
        // Update the struct in the SymTable
        symNode.structNode[structName] = structNode;
        symTable.namespaces[namespaceName] = symNode;
        std::cout << "[IRSymTable] Struct Constructor Updated" << std::endl;
        return;
    }

    // -----------------------------------------------------------------------------------------------
    /// ### ============================================================================= ###
    /// ###
    /// ### Creation Functions
    /// ### These functions are used to create new nodes in the SymTable
    /// ###
    /// ### ============================================================================= ###

    void IRSymTable::createNewStructDecl(std::string namespaceName, StructNode *structNode)
    {
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Creating New Struct Declaration");
        // Add the struct to the SymTable
        SymTableNode symNode = getSymTableNode(namespaceName);
        // Create the struct container
        STStruct structContainer;
        structContainer.ASTNode = structNode;
        structContainer.structType = structNode->type;
        structContainer.LLVMStruct = nullptr;
        structContainer.LLVMConstructor = nullptr;
        structContainer.LLVMMethods.clear();
        // Add the struct to the SymTable
        symNode.structNode[structNode->name] = structContainer;
        symTable.namespaces[namespaceName] = symNode;
        DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "New Struct Declaration Created");
        return;
    }

    // -----------------------------------------------------------------------------------------------
    ///
    /// Debugging
    ///

    void IRSymTable::printTable(std::string namespaceName)
    {

        // Find the namespace in the SymTable and print it
        SymTableNode symNode = getSymTableNode(namespaceName);
        std::cout << "[IRSymTable] Printing SymTable" << std::endl;
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
            DevDebugger::logNode(var.second.initializer);
            std::cout << ">>------------------<<" << std::endl;
        }
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Functions: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        for (auto const &func : symNode.functions)
        {
            std::cout << "Name: " << func.first << std::endl;
            std::cout << "Return Type: " << func.second.type << std::endl;
            std::cout << "Param Count: " << func.second.paramCount << std::endl;
            for (int i = 0; i < func.second.paramCount; i++)
            {
                std::cout << ">>------------------<<" << std::endl;
                std::cout << "Param " << i << ": " << func.second.params[i] << std::endl;
                DevDebugger::logNode(func.second.params[i]);
                std::cout << ">>------------------<<" << std::endl;
            }
            std::cout << "Body: " << std::endl;
            std::cout << ">>------------------<<" << std::endl;
            DevDebugger::logNode(func.second.body);
            std::cout << ">>------------------<<" << std::endl;
        }
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Extern Functions: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        for (auto const &externFunc : symNode.externFunctions)
        {
            std::cout << "Name: " << externFunc.first << std::endl;
            std::cout << "Return Type: " << externFunc.second.type << std::endl;
            std::cout << "Param Count: " << externFunc.second.paramCount << std::endl;
            for (int i = 0; i < externFunc.second.paramCount; i++)
            {
                std::cout << "Param " << i << ": " << std::endl;
                std::cout << ">>------------------<<" << std::endl;
                DevDebugger::logNode(externFunc.second.params[i]);
                std::cout << ">>------------------<<" << std::endl;
            }
        }
    }

    // -----------------------------------------------------------------------------------------------
    SymTableNode IRSymTable::traverseModule(ASTNode *root, std::string namespaceName)
    {
        std::cout << "[IRSymTable] Traversing Module" << std::endl;
        SymTableNode program;
        // Start the recursive traversal
        traverseASTNode(root, program);
        std::cout << "[IRSymTable] Module Traversal Complete" << std::endl;
        return program;
    }

    void IRSymTable::traverseASTNode(ASTNode *node, SymTableNode &symTable)
    {
        if (!node)
            return;

        // Process the current node
        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing PROGRAM node");
            break;
        case NODE_NAMESPACE:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing NAMESPACE node");
            if (node->data.cryoNamespace)
            {
                // Store namespace information if needed
                // program.currentNamespace = node->data.cryoNamespace->name;
            }
            break;
        case NODE_BLOCK:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing BLOCK node");
            break;
        case NODE_FUNCTION_BLOCK:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing FUNCTION_BLOCK node");
            break;
        case NODE_EXTERN_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing EXTERN node");
            break;
        case NODE_EXTERN_FUNCTION:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing EXTERN_FUNCTION node");
            if (node->data.externFunction)
            {
                ExternFunctionNode externFuncNode;
                externFuncNode.name = node->data.externFunction->name;
                externFuncNode.type = node->data.externFunction->type;
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
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing FUNCTION_DECLARATION node");
            if (node->data.functionDecl)
            {
                FunctionDeclNode funcNode;
                funcNode.name = node->data.functionDecl->name;
                funcNode.type = node->data.functionDecl->type;
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
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing FUNCTION_CALL node");
            break;
        case NODE_LITERAL_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing LITERAL node");
            break;
        case NODE_VAR_DECLARATION:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing VAR_DECLARATION node");
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
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing VAR_NAME node");
            if (node->data.varName)
            {
                STVariable varContainer = createVarContainer(node);
                std::string varNameStr = std::string(node->data.varName->varName);
                symTable.variableNode[varNameStr] = varContainer;
            }
            break;
        case NODE_EXPRESSION:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing EXPRESSION node");
            break;
        case NODE_IF_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing IF_STATEMENT node");
            break;
        case NODE_FOR_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing FOR_STATEMENT node");
            break;
        case NODE_WHILE_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing WHILE_STATEMENT node");
            break;
        case NODE_BINARY_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing BINARY_EXPR node");
            break;
        case NODE_UNARY_EXPR:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing UNARY_EXPR node");
            break;
        case NODE_PARAM_LIST:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing PARAM_LIST node");
            break;
        case NODE_ARG_LIST:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing ARG_LIST node");
            break;
        case NODE_RETURN_STATEMENT:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing RETURN_STATEMENT node");
            break;
        case NODE_ARRAY_LITERAL:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing ARRAY_LITERAL node");
            break;
        case NODE_VAR_REASSIGN:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing VAR_REASSIGNMENT node");
            break;
        case NODE_STRUCT_DECLARATION:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing STRUCT_DECLARATION node");
            break;
        case NODE_SCOPED_FUNCTION_CALL:
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing SCOPED_FUNCTION_CALL node");
            break;
        case NODE_INDEX_EXPR:
        {
            if (node->data.indexExpr)
            {
                DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing INDEX_EXPR node");
                traverseASTNode(node->data.indexExpr->array, symTable);
                traverseASTNode(node->data.indexExpr->index, symTable);
            }
        }
        case NODE_PARAM:
        {
            DevDebugger::logMessage("INFO", __LINE__, "IRSymTable", "Processing PARAM node");
            break;
        }
        default:
        {
            CryoNodeType nodeType = node->metaData->type;
            DevDebugger::logMessage("ERROR", __LINE__, "IRSymTable", "Unknown node type: " + std::to_string(nodeType));
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
            DevDebugger::logMessage("WARNING", __LINE__, "IRSymTable", "No traversal defined for node type: " + std::to_string(node->metaData->type));
            break;
        }
    }

} // namespace Cryo