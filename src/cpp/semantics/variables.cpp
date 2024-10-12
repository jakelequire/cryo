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

    llvm::Value *Variables::createLocalVariable(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        Arrays &arrays = compiler.getArrays();
        debugger.logMessage("INFO", __LINE__, "Variables", "Creating Local Variable");

        std::string _varName = std::string(node->data.varDecl->name);

        CryoVariableNode *varDecl = node->data.varDecl;
        assert(varDecl != nullptr);

        CryoDataType varType = varDecl->type;
        ASTNode *initializer = varDecl->initializer;
        assert(initializer != nullptr);
        CryoNodeType initializerNodeType = initializer->metaData->type;
        std::string varName = std::string(varDecl->name);
        llvm::Value *llvmValue = nullptr;
        llvm::Type *llvmType = nullptr;

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

            DEBUG_BREAKPOINT;
        }
        case NODE_VAR_NAME:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a variable");

            std::string varDeclName = std::string(varDecl->name);
            std::string varNameRef = std::string(initializer->data.varName->varName);
            llvm::Value *varValue = compiler.getVariables().getVariable(varNameRef);
            if (!varValue)
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                CONDITION_FAILED;
            }
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable Value Found");
            llvm::Value *llvmValue = compiler.getContext().builder.CreateAlloca(varValue->getType(), varValue, varDeclName);
            llvmValue->setName(varDeclName);
            llvm::Value *ptrValue = compiler.getContext().builder.CreateStore(varValue, llvmValue);
            compiler.getContext().namedValues[varDeclName] = llvmValue;

            return llvmValue;
        }
        case NODE_LITERAL_EXPR:
        {
            CryoDataType dataType = initializer->data.literal->dataType;
            switch (dataType)
            {
            case DATA_TYPE_INT:
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable is an int literal");
                int intValue = initializer->data.literal->value.intValue;
                llvm::Type *ty = compiler.getTypes().getType(varType, 0);
                llvm::Value *varValue = compiler.getGenerator().getInitilizerValue(initializer);
                if (!varValue)
                {
                    CONDITION_FAILED;
                }
                llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(ty, varValue, varName);
                compiler.getContext().namedValues[varName] = ptrValue;

                return ptrValue;
            }
            case DATA_TYPE_STRING:
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable is a string literal");
                int _len = compiler.getTypes().getLiteralValLength(initializer);
                llvmType = compiler.getTypes().getType(varType, _len + 1);
                debugger.logMessage("INFO", __LINE__, "Variables", "Type: " + std::string(CryoDataTypeToString(varType)));

                llvm::Value *varValue = compiler.getGenerator().getInitilizerValue(initializer);
                if (!varValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                    CONDITION_FAILED;
                }
                llvmValue = compiler.getContext().builder.CreateAlloca(llvmType, nullptr, varName);
                llvmValue->setName("str");
                llvm::Value *ptrValue = compiler.getContext().builder.CreateStore(varValue, llvmValue);
                compiler.getContext().namedValues[varName] = llvmValue;

                return llvmValue;
            }
            default:
            {
                debugger.logMessage("ERROR", __LINE__, "Variables", "Literal Variable has unknown data type");
                CONDITION_FAILED;
            }
            }
        }
        case NODE_ARRAY_LITERAL:
        {
            if (varType == DATA_TYPE_INT_ARRAY)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Creating Int Array Variable");
                llvm::Type *ty = compiler.getTypes().getType(varType, 0);
                llvm::Value *varValue = arrays.createArrayLiteral(varDecl->initializer, varName);
                if (!varValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Variable value not found");
                    CONDITION_FAILED;
                }
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable Value Found");
                // llvm::Value *ptrValue = compiler.getContext().builder.CreateAlloca(ty, varValue, varName);
                compiler.getContext().namedValues[varName] = varValue;

                return varValue;
            }
        }
        case NODE_INDEX_EXPR:
        {
            debugger.logNode(initializer);
            CryoNodeType indexNodeType = initializer->data.indexExpr->index->metaData->type;
            if (indexNodeType == NODE_VAR_NAME)
            {
                debugger.logMessage("INFO", __LINE__, "Variables", "Index is a variable");
                std::string indexVarName = std::string(initializer->data.indexExpr->index->data.varName->varName);
                std::cout << "Index Variable Name: " << indexVarName << std::endl;

                // Get the variable value
                llvm::Value *indexValue = compiler.getVariables().getVariable(indexVarName);
                if (!indexValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Index value not found");
                    CONDITION_FAILED;
                }
                debugger.logMessage("INFO", __LINE__, "Variables", "Index Value Found");
                debugger.logLLVMValue(indexValue);

                std::string llvmVarName = indexValue->getName().str();
                std::cout << "LLVM Variable Name: " << llvmVarName << std::endl;

                // Get the address of the index variable "exampleName.addr"
                llvm::Value *llvmIndexValue = compiler.getContext().namedValues[llvmVarName];
                if (!llvmIndexValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Index value not found");
                    CONDITION_FAILED;
                }
                debugger.logLLVMValue(llvmIndexValue);

                // Get the array name
                std::string arrayName = std::string(initializer->data.indexExpr->name);
                std::cout << "Array Name: " << arrayName << std::endl;

                // Get the array
                ASTNode *arrayNode = compiler.getSymTable().getASTNode(compiler.getContext().currentNamespace, NODE_VAR_DECLARATION, arrayName);
                if (!arrayNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Array not found");
                    CONDITION_FAILED;
                }

                // Get the array type
                llvm::Type *arrayType = arrays.getArrayType(arrayNode);
                llvm::Type *elementType = arrayType->getArrayElementType();

                // Get the Index Type
                llvm::Type *indexType = llvmIndexValue->getType();
                if (indexType->isPointerTy())
                {
                    indexType = indexType->getWithNewType(llvm::Type::getInt32Ty(compiler.getContext().context));
                }

                arrays.isOutOfBoundsException(compiler.getContext().namedValues[arrayName], llvmIndexValue);

                llvm::Value *arrayPtr = compiler.getContext().builder.CreateGEP(indexType, compiler.getContext().namedValues[arrayName], llvmIndexValue);
                llvm::Value *loadedValue = compiler.getContext().builder.CreateLoad(elementType, arrayPtr);

                // Store the value in the variable
                llvm::Value *var = compiler.getContext().builder.CreateAlloca(elementType, loadedValue, varName);
                compiler.getContext().builder.CreateStore(loadedValue, var); // This was the solution to the undefined behavior

                compiler.getContext().namedValues[varName] = var;

                return var;
            }
            else
            {
                std::cout << "Index Node Type: " << CryoNodeTypeToString(indexNodeType) << std::endl;
                debugger.logMessage("INFO", __LINE__, "Variables", "Variable is an index expression");
                int indexValue = initializer->data.indexExpr->index->data.literal->value.intValue;
                std::cout << "Index Value: " << indexValue << std::endl;
                std::string arrayName = std::string(initializer->data.indexExpr->name);
                std::cout << "Array Name: " << arrayName << std::endl;

                ASTNode *arrNode = compiler.getSymTable().getASTNode(compiler.getContext().currentNamespace, NODE_VAR_DECLARATION, arrayName);
                if (!arrNode)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Array not found");
                    CONDITION_FAILED;
                }
                llvm::Value *indexedValue = arrays.indexArrayForValue(arrNode, indexValue);
                if (!indexedValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Variables", "Indexed value not found");
                    CONDITION_FAILED;
                }
                // Allocate the variable
                llvmValue = compiler.getContext().builder.CreateAlloca(indexedValue->getType(), indexedValue, varName);
                compiler.getContext().namedValues[varName] = llvmValue;

                return indexedValue;
            }
        }
        default:
        {
            debugger.logMessage("INFO", __LINE__, "Variables", "Variable is of unknown type");
            CONDITION_FAILED;
        }
        }
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

    // -----------------------------------------------------------------------------------------------

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

        // Store the call into the variable
        llvm::Instruction *inst = compiler.getContext().builder.CreateStore(functionCall, varValue);

        debugger.logMessage("INFO", __LINE__, "Variables", "Function Call Created");

        return functionCall;
    }

} // namespace Cryo
