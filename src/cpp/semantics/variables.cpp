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
#include "cpp/codegen.hpp"

namespace Cryo
{

    void Variables::handleConstVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Const Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        processConstVariable(varDecl);

        return;
    }

    // TODO: Implement
    void Variables::handleRefVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Ref Variable");

        DEBUG_BREAKPOINT;
    }

    void Variables::handleMutableVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Mutable Variable");

        createMutableVariable(node);
    }

    // -----------------------------------------------------------------------------------------------

    /// ### ============================================================================= ###
    /// ###
    /// ### Type of Variables to Handle
    /// ### These functions handle the different types of variables that can be declared.
    /// ###
    /// ### ============================================================================= ###

    void Variables::handleVariableReassignment(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Handling Variable Reassignment");
        std::string currentModuleName = compiler.getContext().currentNamespace;

        // Find the variable in the symbol table
        std::string existingVarName = std::string(node->data.varReassignment->existingVarName);
        std::cout << "Variable Name: " << existingVarName << std::endl;
        ASTNode *varNode = compiler.getSymTable().getASTNode(currentModuleName, NODE_VAR_DECLARATION, existingVarName);
        if (!varNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            CONDITION_FAILED;
        }

        // Get the new value
        ASTNode *newValue = node->data.varReassignment->newVarNode;
        llvm::Value *newVal = compiler.getGenerator().getInitilizerValue(newValue);
        if (!newVal)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "New value not found");
            CONDITION_FAILED;
        }

        llvm::Value *varValue = compiler.getContext().namedValues[existingVarName];
        if (!varValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
            CONDITION_FAILED;
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Value Found");

        llvm::Instruction *inst = compiler.getContext().builder.CreateStore(newVal, varValue);
        llvm::GlobalVariable *key = inst->getModule()->getNamedGlobal(existingVarName);
        if (key)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");
            key->setInitializer(llvm::dyn_cast<llvm::Constant>(newVal));
        }
        else
        {
            DEBUG_BREAKPOINT;
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");
        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::createMutableVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Processing Const Variable");
        CryoVariableNode *varNode = node->data.varDecl;
        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        CryoDataType type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                debugger.logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                debugger.logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                if (type == DATA_TYPE_STRING)
                    _len += 1; // Add one for the null terminator
                debugger.logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = CryoDataTypeToString(type);
                debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(typeNode));

                switch (type)
                {
                case DATA_TYPE_INT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case DATA_TYPE_FLOAT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case DATA_TYPE_BOOLEAN:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, initializer->data.literal->value.stringValue);
                    break;
                }
                default:
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    CONDITION_FAILED;
                    break;
                }

                debugger.logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
                    CONDITION_FAILED;
                }

                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *cryoContext.module,
                    llvmType,
                    false,
                    llvm::GlobalValue::WeakAnyLinkage,
                    llvmConstant,
                    llvm::Twine(varName));

                compiler.getContext().namedValues[varName] = var;
            }
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::processConstVariable(CryoVariableNode *varNode)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Processing Const Variable");

        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        CryoDataType type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        bool isGlobal = varNode->isGlobal;

        if (!isGlobal)
        {
            // Get the current block
            llvm::BasicBlock *currentBlock = cryoContext.builder.GetInsertBlock();
            if (!currentBlock)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Current block not found");
                CONDITION_FAILED;
            }
            debugger.logMessage("INFO", __LINE__, "Variables", "Current block found");
            // Set the insert point
            cryoContext.builder.SetInsertPoint(currentBlock);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Global Variable");
        }

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                debugger.logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                debugger.logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                if (type == DATA_TYPE_STRING)
                    _len += 1; // Add one for the null terminator
                debugger.logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = CryoDataTypeToString(type);
                debugger.logMessage("INFO", __LINE__, "Variables", "Varname: " + std::string(varName));
                debugger.logMessage("INFO", __LINE__, "Variables", "Data Type: " + std::string(typeNode));

                switch (type)
                {
                case DATA_TYPE_INT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case DATA_TYPE_FLOAT:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case DATA_TYPE_BOOLEAN:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, initializer->data.literal->value.stringValue);
                    break;
                }
                default:
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    CONDITION_FAILED;
                    break;
                }

                debugger.logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
                    CONDITION_FAILED;
                }

                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *cryoContext.module,
                    llvmType,
                    false,
                    llvm::GlobalValue::ExternalLinkage,
                    llvmConstant,
                    llvm::Twine(varName));
                compiler.getContext().namedValues[varName] = var;
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");
            }
        }
        debugger.logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

    /**
     * @brief Main entry point to creating a localized variable.
     * This is triggered when a variable is declared within a function block specifically.
     *
     * The functions inside this are handling the creation of the variable with the given initializer (literal, function call, etc.)
     *
     * @param node The AST node
     * @return `llvm::Value *` The LLVM value of the variable
     */
    llvm::Value *Variables::createLocalVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        Arrays &arrays = compiler.getArrays();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        CryoDataType varType = varDecl->type;
        ASTNode *initializer = varDecl->initializer;
        assert(initializer != nullptr);

        CryoNodeType initializerNodeType = initializer->metaData->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;

        // Check if it already exists
        std::cout << "Variable Name: " << varName << std::endl;
        llvmValue = compiler.getContext().namedValues[varName];
        if (llvmValue)
        {
            debugger.logMessage("WARN", __LINE__, "Variables", "Variable already exists");
            return llvmValue;
        }

        switch (initializerNodeType)
        {
        case NODE_FUNCTION_CALL:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a function call");
            llvmValue = createVarWithFuncCallInitilizer(node);
            compiler.getContext().namedValues[varName] = llvmValue;
            return llvmValue;
        }
        case NODE_BINARY_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a binary expression");
            llvmValue = compiler.getBinaryExpressions().handleComplexBinOp(initializer);
            compiler.getContext().namedValues[varName] = llvmValue;
            llvmValue->setName(varName);
            return llvmValue;
        }
        case NODE_PARAM:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a parameter");
            // TODO: Implement
            DEBUG_BREAKPOINT;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable initializer is a VariableNameNode.");
            std::string varDeclName = std::string(varDecl->name);
            std::string refVarName = std::string(initializer->data.varName->varName);
            VariableNameNode *varNameNode = initializer->data.varName;
            return createVarNameInitializer(varNameNode, varDeclName, refVarName);
        }
        case NODE_LITERAL_EXPR:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable initializer is a LiteralNode.");
            LiteralNode *literal = initializer->data.literal;
            return createLiteralExprVariable(literal, varName);
        }
        case NODE_ARRAY_LITERAL:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable initializer is an ArrayLiteralNode.");
            CryoArrayNode *arrayNode = initializer->data.array;
            return createArrayLiteralInitializer(arrayNode, varType, varName);
        }
        case NODE_INDEX_EXPR:
        {
            IndexExprNode *indexNode = initializer->data.indexExpr;
            CryoNodeType indexNodeType = initializer->data.indexExpr->index->metaData->type;
            std::string varName = std::string(initializer->data.indexExpr->name);
            return createIndexExprInitializer(indexNode, indexNodeType, varName);
        }
        default:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is of unknown type");
            std::cout << "Unknown node type: " << CryoNodeTypeToString(initializerNodeType) << std::endl;
            CONDITION_FAILED;
        }
        }
    }

    /// ### ============================================================================= ###
    /// ###
    /// ### Specialized Functions
    /// ### These functions are used to create variables with specific initializers
    /// ###
    /// ### ============================================================================= ###

    ///
    /// @brief Create a variable with a literal expression initializer
    /// @param literalNode The literal node
    /// @param varName The name of the variable
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = 10;
    ///
    /// const bar: string = "Hello, World!";
    ///
    /// ```
    llvm::Value *Variables::createLiteralExprVariable(LiteralNode *literalNode, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Literal Expression Variable");

        std::string namespaceName = compiler.getContext().currentNamespace;
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        CryoDataType dataType = literalNode->dataType;
        switch (dataType)
        {
        case DATA_TYPE_INT:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is an int literal");
            int intValue = literalNode->value.intValue;
            // llvm::Type *ty = compiler.getTypes().getType(dataType, 0);
            llvm::Type *ty = compiler.getTypes().getLiteralType(literalNode);
            llvm::Value *varValue = compiler.getGenerator().getLiteralValue(literalNode);
            if (!varValue)
            {
                CONDITION_FAILED;
            }
            llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(ty, nullptr, varName);
            llvm::Value *storeValue = compiler.getContext().builder.CreateStore(varValue, ptrValue);
            compiler.getContext().namedValues[varName] = ptrValue;

            symTable.updateVariableNode(namespaceName, varName, ptrValue, ty);

            return ptrValue;
        }
        case DATA_TYPE_FLOAT:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Float Variable");
            llvmType = types.getType(dataType, 0);
            llvmConstant = llvm::ConstantFP::get(llvmType, literalNode->value.floatValue);
            llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);
            break;
        }
        case DATA_TYPE_BOOLEAN:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Boolean Variable");
            llvmType = types.getType(dataType, 0);
            llvmConstant = llvm::ConstantInt::get(llvmType, literalNode->value.booleanValue);
            llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);
            break;
        }
        case DATA_TYPE_STRING:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a string literal");
            int _len = compiler.getTypes().getLiteralValLength(literalNode);
            llvmType = compiler.getTypes().getType(dataType, _len + 1);
            debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(dataType)));

            llvm::Value *varValue = compiler.getGenerator().getLiteralValue(literalNode);
            if (!varValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                CONDITION_FAILED;
            }
            llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
            llvm::Value *ptrValue = compiler.getContext().builder.CreateStore(varValue, llvmValue);
            compiler.getContext().namedValues[varName] = llvmValue;

            llvm::Type *strType = types.getType(DATA_TYPE_STRING, _len + 1);
            symTable.updateVariableNode(namespaceName, varName, ptrValue, strType);

            return llvmValue;
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
            CONDITION_FAILED;
        }
        }

        return llvmValue;
    }

    ///
    /// @brief Create a variable with a variable name / reference initializer
    /// @param node The AST node
    /// @param varName The name of the variable
    /// @param refVarName The name of the variable being referenced
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = 34;
    ///
    /// const bar: int = foo;
    ///
    /// ```
    ///
    llvm::Value *Variables::createVarNameInitializer(VariableNameNode *varNameNode, std::string varName, std::string refVarName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        Types &types = compiler.getTypes();
        std::string namespaceName = compiler.getContext().currentNamespace;
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable Name Initializer");
        std::cout << "Variable Name: " << varName << std::endl;
        std::cout << "Referenced Variable Name: " << refVarName << std::endl;

        CryoDataType nodeDataType = varNameNode->refType;
        std::cout << "Node Type: " << CryoDataTypeToString(nodeDataType) << std::endl;

        // Create the variable alloca
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

        switch (nodeDataType)
        {
        case DATA_TYPE_INT:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Variable");
            llvmType = types.getType(nodeDataType, 0);
            llvmValue = compiler.getVariables().getVariable(varName);
            break;
        }
        case DATA_TYPE_STRING:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Variable");
            llvmType = types.getType(nodeDataType, 0);
            // llvmValue = compiler.getVariables().getVariable(varName);
            llvmValue = compiler.getContext().namedValues[refVarName];
            if (!llvmValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
                CONDITION_FAILED;
            }

            llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
            llvm::Value *storeValue = compiler.getContext().builder.CreateStore(llvmValue, ptrValue);
            compiler.getContext().namedValues[varName] = ptrValue;

            break;
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
            CONDITION_FAILED;
        }
        }

        return llvmValue;
    }

    ///
    /// @brief Create a variable with an array literal initializer. (e.g. [1, 2, 3])
    /// @param node The AST node
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int[] = [1, 2, 3];
    ///
    /// ```
    ///
    llvm::Value *Variables::createArrayLiteralInitializer(CryoArrayNode *arrayNode, CryoDataType dataType, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Array Literal Initializer");

        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        switch (dataType)
        {
        case DATA_TYPE_INT_ARRAY:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Array Literal");
            llvmType = compiler.getTypes().getType(dataType, 0);
            llvmValue = compiler.getArrays().createArrayLiteral(arrayNode, varName);
            break;
        }
        case DATA_TYPE_STRING_ARRAY:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Creating String Array Literal");
            llvmType = compiler.getTypes().getType(dataType, 0);
            llvmValue = compiler.getArrays().createArrayLiteral(arrayNode, varName);
            break;
        }
        default:
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Unknown data type");
            CONDITION_FAILED;
        }
        }

        return llvmValue;
    }

    ///
    /// @brief Creates a variable that has an index expression in its initializer.
    /// @param indexExprNode The index expression node
    /// @param nodeType The node type
    /// @param varName The name of the variable
    /// @return `llvm::Value *` A pointer to the LLVM Value representing the newly created variable
    ///
    /// @note UNTESTED
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = bar[0];
    ///
    /// ```
    ///
    llvm::Value *Variables::createIndexExprInitializer(IndexExprNode *indexExprNode, CryoNodeType nodeType, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Index Expression Initializer");

        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;
        llvm::Constant *llvmConstant = nullptr;

        std::string arrayName = std::string(indexExprNode->name);
        std::cout << "Array Name: " << arrayName << std::endl;
        ASTNode *arrayNode = compiler.getSymTable().getASTNode(compiler.getContext().currentNamespace, NODE_VAR_DECLARATION, arrayName);
        if (!arrayNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Array not found");
            CONDITION_FAILED;
        }

        std::string indexVarName = std::string(indexExprNode->index->data.varName->varName);
        llvm::Value *indexValue = compiler.getVariables().getVariable(indexVarName);
        if (!indexValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Index value not found");
            CONDITION_FAILED;
        }
        int indexNumValue = indexExprNode->index->data.literal->value.intValue;
        llvmValue = compiler.getArrays().indexArrayForValue(arrayNode, indexNumValue);
        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Indexed value not found");
            CONDITION_FAILED;
        }

        return llvmValue;
    }

    ///
    /// @brief Creates a variable with a function call initializer.
    /// @param node The AST node
    /// @return `llvm::Value *` The LLVM value of the variable
    ///
    /// ---
    ///
    /// ```cryo
    ///
    /// \// Example
    ///
    /// const foo: int = bar();
    ///
    /// ```
    ///
    llvm::Value *Variables::createVarWithFuncCallInitilizer(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Variable with Function Call Initializer");

        // Should be the function call node
        ASTNode *initializer = node->data.varDecl->initializer;
        // The variable node
        ASTNode *variable = node;
        assert(initializer != nullptr);

        std::string moduleName = compiler.getContext().currentNamespace;

        // Create the variable
        std::string varName = std::string(variable->data.varDecl->name);
        llvm::Type *varType = compiler.getTypes().getType(variable->data.varDecl->type, 0);
        llvm::Value *varValue = compiler.getContext().builder.CreateAlloca(varType, nullptr, varName);

        // Get the function call
        if (initializer->metaData->type != NODE_FUNCTION_CALL)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Initializer is not a function call");
            DEBUG_BREAKPOINT;
        }

        std::string functionName = std::string(initializer->data.functionCall->name);
        std::cout << "Function Name: " << functionName << std::endl;

        llvm::Value *functionCall = functions.createFunctionCall(initializer);
        if (!functionCall)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Function call not created");
            CONDITION_FAILED;
        }

        debugger.logMessage("INFO", __LINE__, "Variables", "Function Call Created, Storing in Variable");

        compiler.dumpModule();

        // Store the call into the variable
        compiler.getContext().builder.CreateStore(functionCall, varValue);

        debugger.logMessage("INFO", __LINE__, "Variables", "Function Call Created");

        return functionCall;
    }

    // -----------------------------------------------------------------------------------------------

    /// ### ============================================================================= ###
    /// ###
    /// ### Utility Functions
    /// ### These functions are used to access and manipulate variables
    /// ###
    /// ### ============================================================================= ###

    llvm::Value *Variables::getVariable(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Getting Variable");
        std::cout << "Name being passed: " << name << std::endl;

        llvm::Value *llvmValue = nullptr;

        llvmValue = compiler.getContext().module->getNamedGlobal(name);

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        std::cout << "Name being passed: " << name << std::endl;
        llvmValue = getLocalScopedVariable(name);
        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Found");
        }

        return llvmValue;
    }

    llvm::Value *Variables::getLocalScopedVariable(std::string name)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Variables", "Getting Local Scoped Variable");

        llvm::Value *llvmValue = nullptr;
        std::cout << "Name being passed (start): " << name << std::endl;

        llvmValue = compiler.getContext().namedValues[name];

        if (!llvmValue)
        {
            debugger.logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Found");
            // Print the variable
            llvm::Instruction *instruction = llvm::dyn_cast<llvm::Instruction>(llvmValue);
            if (instruction)
            {
                std::string instName = instruction->getName().str();
                std::cout << "Inst Name: " << instName << std::endl;

                llvm::Value *instValue = instruction->getOperand(0);
                std::string instValueName = instValue->getName().str();
                std::cout << "Inst Value Name: " << instValueName << std::endl;
            }
        }

        return llvmValue;
    }

    // -----------------------------------------------------------------------------------------------

} // namespace Cryo
