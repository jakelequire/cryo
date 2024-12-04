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
#include "codegen/oldCodeGen.hpp"

namespace Cryo
{
    void Variables::handleConstVariable(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Const Variable");

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        processConstVariable(varDecl);

        return;
    }

    // TODO: Implement
    void Variables::handleRefVariable(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Ref Variable");

        DEBUG_BREAKPOINT;
    }

    void Variables::handleMutableVariable(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Mutable Variable");

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
        OldTypes &types = compiler.getTypes();
        IRSymTable &symTable = compiler.getSymTable();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Variable Reassignment");
        std::string currentModuleName = compiler.getContext().currentNamespace;

        // Find the variable in the symbol table
        std::string existingVarName = std::string(node->data.varReassignment->existingVarName);
        std::cout << "Variable Name: " << existingVarName << std::endl;

        STVariable *var = compiler.getSymTable().getVariable(currentModuleName, existingVarName);
        if (!var)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            CONDITION_FAILED;
        }

        llvm::Value *varValue = var->LLVMValue;
        if (!varValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
            CONDITION_FAILED;
        }

        // Get the new value
        ASTNode *newValue = node->data.varReassignment->newVarNode;
        CryoNodeType newVarType = newValue->metaData->type;
        switch (newVarType)
        {
        case NODE_LITERAL_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Literal Expression");
            llvm::Value *newVarValue = compiler.getGenerator().getInitilizerValue(newValue);
            if (!newVarValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "New Variable value not found");
                CONDITION_FAILED;
            }

            // Store the new value in the existing variable
            compiler.getContext().builder.CreateStore(newVarValue, varValue);
            break;
        }
        case NODE_VAR_NAME:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Variable Name");
            std::string newVarName = std::string(newValue->data.varName->varName);
            STVariable *newVar = compiler.getSymTable().getVariable(currentModuleName, newVarName);
            if (!newVar)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "New Variable not found");
                CONDITION_FAILED;
            }

            llvm::Value *newVarValue = newVar->LLVMValue;
            if (!newVarValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "New Variable value not found");
                CONDITION_FAILED;
            }

            // Load the value of the new variable
            std::string loadedVarName = newVarName + ".load.var";
            llvm::Type *newVarType = newVarValue->getType();
            llvm::LoadInst *loadedValue = compiler.getContext().builder.CreateLoad(newVarType, newVarValue, loadedVarName);
            if (!loadedValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Failed to load new variable value");
                CONDITION_FAILED;
            }

            // Store the new value in the existing variable
            compiler.getContext().builder.CreateStore(loadedValue, varValue);

            // Update the variable in the symbol table
            symTable.updateVariableNode(currentModuleName, existingVarName, varValue, newVarValue->getType());
            symTable.addLoadInstToVar(currentModuleName, existingVarName, loadedValue);

            break;
        }
        case NODE_BINARY_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Handling Binary Expression");
            llvm::Value *newVarValue = compiler.getBinaryExpressions().handleComplexBinOp(newValue);
            if (!newVarValue)
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "New Variable value not found");
                CONDITION_FAILED;
            }

            // Store the new value in the existing variable
            compiler.getContext().builder.CreateStore(newVarValue, varValue);

            // Update the variable in the symbol table
            symTable.updateVariableNode(currentModuleName, existingVarName, varValue, newVarValue->getType());

            break;
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown node type");
            std::cout << "Unknown node type: " << CryoNodeTypeToString(newVarType) << std::endl;
            CONDITION_FAILED;
            break;
        }
        }

        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Reassignment Handled");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::createMutableVariable(ASTNode *node)
    {
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Mutable Variable");
        CryoVariableNode *varNode = node->data.varDecl;
        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        DataType *type = varNode->type;
        ASTNode *initializer = varNode->initializer;

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = DataTypeToString(type);
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(typeNode));

                switch (type->container->primitive)
                {
                case PRIM_INT:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case PRIM_FLOAT:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case PRIM_BOOLEAN:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    const char *str = strdup(initializer->data.literal->value.stringValue);
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "String: " + std::string(str));
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, str);
                    break;
                }
                default:
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    CONDITION_FAILED;
                    break;
                }

                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
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
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Created");

        return;
    }

    // -----------------------------------------------------------------------------------------------

    void Variables::processConstVariable(CryoVariableNode *varNode)
    {
        CryoContext &cryoContext = compiler.getContext();
        Arrays &arrays = compiler.getArrays();

        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Const Variable");

        char *varName = varNode->name;
        std::cout << "Variable Name: " << varName << std::endl;
        DataType *type = varNode->type;
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
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Current block not found");
                CONDITION_FAILED;
            }
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Current block found");
            // Set the insert point
            cryoContext.builder.SetInsertPoint(currentBlock);
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Global Variable");
        }

        if (initializer)
        {
            if (initializer->metaData->type == NODE_INDEX_EXPR)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Array Index");
                char *arrayName = initializer->data.indexExpr->name;
                std::cout << "Array Name: " << arrayName << std::endl;
                arrays.handleIndexExpression(initializer, varName);
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Array Index Processed");
                return;
            }
            // Check if the initializer is an array
            if (initializer->metaData->type == NODE_ARRAY_LITERAL)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Processing Array Literal");

                arrays.handleArrayLiteral(initializer);

                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Array Literal Processed");
            }
            else
            {
                int _len = types.getLiteralValLength(initializer);
                if (type->container->baseType == PRIM_STRING)
                    _len += 1; // Add one for the null terminator
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Length: " + std::to_string(_len));
                llvmType = types.getType(type, _len);
                char *typeNode = DataTypeToString(type);
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Varname: " + std::string(varName));
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Data Type: " + std::string(typeNode));

                switch (type->container->primitive)
                {
                case PRIM_INT:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Int Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.intValue);
                    break;
                }
                case PRIM_FLOAT:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Float Constant");
                    llvmConstant = llvm::ConstantFP::get(llvmType, initializer->data.literal->value.floatValue);
                    break;
                }
                case PRIM_BOOLEAN:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Boolean Constant");
                    llvmConstant = llvm::ConstantInt::get(llvmType, initializer->data.literal->value.booleanValue);
                    break;
                }
                case PRIM_STRING:
                {
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Constant");
                    const char *str = strdup(initializer->data.literal->value.stringValue);
                    DevDebugger::logMessage("INFO", __LINE__, "Variables", "String: " + std::string(str));
                    llvmConstant = llvm::ConstantDataArray::getString(cryoContext.context, str);
                    break;
                }
                default:
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown type");
                    CONDITION_FAILED;
                    break;
                }

                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Constant Created");
                llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

                if (!llvmValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Failed to cast constant to value");
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
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Created");
            }
        }
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Created");
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
        OldTypes &types = compiler.getTypes();
        Arrays &arrays = compiler.getArrays();
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");
        std::string namespaceName = compiler.getContext().currentNamespace;

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        DataType *varType = varDecl->type;
        ASTNode *initializer = varDecl->initializer;
        assert(initializer != nullptr);

        // Handle custom types
        if (varType->container->baseType == STRUCT_TYPE)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is a struct");
            return createStructVariable(varDecl);
        }

        CryoNodeType initializerNodeType = initializer->metaData->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;

        // Check if it already exists
        std::cout << "Variable Name: " << varName << std::endl;
        llvmValue = compiler.getContext().namedValues[varName];
        if (llvmValue)
        {
            DevDebugger::logMessage("WARN", __LINE__, "Variables", "Variable already exists");
            return llvmValue;
        }

        switch (initializerNodeType)
        {
        case NODE_FUNCTION_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is a function call");
            llvmValue = createVarWithFuncCallInitilizer(node);
            compiler.getContext().namedValues[varName] = llvmValue;
            return llvmValue;
        }
        case NODE_BINARY_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is a binary expression");
            std::string varDeclName = std::string(varDecl->name);
            return createVarWithBinOpInitilizer(initializer, varDeclName);
        }
        case NODE_PARAM:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is a parameter");
            // TODO: Implement
            DEBUG_BREAKPOINT;
        }
        case NODE_VAR_NAME:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable initializer is a VariableNameNode.");
            std::string varDeclName = std::string(varDecl->name);
            std::string refVarName = std::string(initializer->data.varName->varName);
            VariableNameNode *varNameNode = initializer->data.varName;
            return createVarNameInitializer(varNameNode, varDeclName, refVarName);
        }
        case NODE_LITERAL_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable initializer is a LiteralNode.");
            LiteralNode *literal = initializer->data.literal;
            return createLiteralExprVariable(literal, varName, varType);
        }
        case NODE_ARRAY_LITERAL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable initializer is an ArrayLiteralNode.");
            CryoArrayNode *arrayNode = initializer->data.array;
            return createArrayLiteralInitializer(arrayNode, varType, varName);
        }
        case NODE_INDEX_EXPR:
        {
            IndexExprNode *indexNode = initializer->data.indexExpr;
            CryoNodeType indexNodeType = initializer->data.indexExpr->index->metaData->type;
            llvm::Value *indexExprInitializer = createIndexExprInitializer(indexNode, indexNodeType, varName);
            return indexExprInitializer;
        }
        case NODE_METHOD_CALL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable initializer is a MethodCallNode.");
            MethodCallNode *methodCall = initializer->data.methodCall;
            std::string methodName = std::string(methodCall->name);
            std::string varName = std::string(varDecl->name);
            DataType *varType = varDecl->type;
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Method Name: " + methodName);
            return createMethodCallVariable(methodCall, varName, varType);
        }
        case NODE_PROPERTY_ACCESS:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable initializer is a PropertyAccessNode.");
            PropertyAccessNode *propertyAccess = initializer->data.propertyAccess;
            std::string varName = std::string(varDecl->name);
            DataType *varType = varDecl->type;
            return createPropertyAccessVariable(propertyAccess, varName, varType);
        }
        default:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable is of unknown type");
            std::cout << "Unknown node type: " << CryoNodeTypeToString(initializerNodeType) << std::endl;
            CONDITION_FAILED;
        }
        }
    }

    /// ### ============================================================================= ###
    /// ###
    /// ### Utility Functions
    /// ### These functions are used to access and manipulate variables
    /// ###
    /// ### ============================================================================= ###

    llvm::Value *Variables::getVariable(std::string name)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Getting Variable");
        std::cout << "Name being passed: " << name << std::endl;

        llvm::Value *llvmValue = nullptr;

        llvmValue = compiler.getContext().module->getNamedGlobal(name);

        if (!llvmValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        std::cout << "Name being passed: " << name << std::endl;
        llvmValue = getLocalScopedVariable(name);
        if (!llvmValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Found");
        }

        return llvmValue;
    }

    llvm::Value *Variables::getLocalScopedVariable(std::string name)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Getting Local Scoped Variable");

        llvm::Value *llvmValue = nullptr;
        std::cout << "Name being passed (start): " << name << std::endl;

        llvmValue = compiler.getContext().namedValues[name];

        if (!llvmValue)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Variable not found");
            llvmValue = nullptr;
        }

        if (llvmValue != nullptr)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Variable Found");
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

    llvm::Value *Variables::createStringVariable(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Variable");

        CryoNodeType nodeType = node->metaData->type;
        std::string nodeTypeStr = CryoNodeTypeToString(nodeType);
        std::cout << "Node Type: " << nodeTypeStr << std::endl;
        std::string varName = "literal.";

        switch (nodeType)
        {
        case NODE_LITERAL_EXPR:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating Literal Variable");
            DataType *literalType = node->data.literal->type;
            switch (literalType->container->primitive)
            {
            case PRIM_STRING:
            {
                DevDebugger::logMessage("INFO", __LINE__, "Variables", "Creating String Literal Variable");
                std::string literalValue = node->data.literal->value.stringValue;
                std::cout << "Literal Value: " << literalValue << std::endl;

                varName += "string";

                llvm::Type *stringType = llvm::Type::getInt8Ty(compiler.getContext().context)->getPointerTo();
                llvm::Constant *stringConstVal = llvm::ConstantDataArray::getString(compiler.getContext().context, literalValue);
                llvm::Type *stringDataType = stringConstVal->getType();
                llvm::Value *stringVal = stringConstVal;

                llvm::AllocaInst *allocaInst = compiler.getContext().builder.CreateAlloca(stringDataType, nullptr, varName);

                // Store the string value
                llvm::StoreInst *storeInst = compiler.getContext().builder.CreateStore(stringVal, allocaInst);
                if (!storeInst)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Failed to store string value");
                    CONDITION_FAILED;
                }

                // Load the string value
                llvm::Value *loadedValue = compiler.getContext().builder.CreateLoad(stringDataType->getPointerTo(), allocaInst, varName + ".load.var");
                if (!loadedValue)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Failed to load string value");
                    CONDITION_FAILED;
                }

                return allocaInst;
            }
            default:
            {
                DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown primitive type");
                CONDITION_FAILED;
            }
            }
        }
        default:
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Variables", "Unknown node type");
            std::string nodeTypeStr = CryoNodeTypeToString(nodeType);
            std::cout << "Node Type Received: " << nodeTypeStr << std::endl;
            CONDITION_FAILED;
        }
        }
    }

    // -----------------------------------------------------------------------------------------------

    llvm::Value *Variables::createTestGlobalVariable(void)
    {
        // Create a global
        llvm::Type *intType = llvm::Type::getInt32Ty(compiler.getContext().context);
        llvm::Constant *intConstant = llvm::ConstantInt::get(intType, 80085);
        llvm::GlobalVariable *intGlobal = new llvm::GlobalVariable(
            *compiler.getContext().module,
            intType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            intConstant,
            "testInt");

        return intGlobal;
    }

} // namespace Cryo
