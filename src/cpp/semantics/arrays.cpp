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
    llvm::Value *Arrays::createArrayLiteral(ASTNode *node, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Creating Array Literal");

        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        // Get the type of the array
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (element->metaData->type == NODE_LITERAL_EXPR)
            {
                // Note to self, might need to get the length of each element
                CryoDataType dataType = element->data.literal->dataType;
                switch (dataType)
                {
                case DATA_TYPE_INT:
                {
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, 0);
                    int index = element->data.literal->value.intValue;
                    llvm::Constant *llvmElement = llvm::ConstantInt::get(llvmType, index);
                    elements.push_back(llvmElement);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    int _len = compiler.getTypes().getLiteralValLength(element);
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, _len + 1);
                    llvm::Constant *llvmElement = llvm::ConstantDataArray::getString(compiler.getContext().context, element->data.literal->value.stringValue);
                    elements.push_back(llvmElement);
                    break;
                }
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown element type");
                DEBUG_BREAKPOINT;
            }
        }
        debugger.logMessage("INFO", __LINE__, "Arrays", "Finished processing elements");

        // Create the array
        llvmArrayType = llvm::ArrayType::get(llvmType, arrayNode->elementCount);
        llvmValue = llvm::ConstantArray::get(llvmArrayType, elements);

        // Get the current block that we are in
        llvm::BasicBlock *block = compiler.getContext().builder.GetInsertBlock();
        if (!block)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Block not found");
            CONDITION_FAILED;
        }

        // Alloc the array without initializing it
        llvm::Value *llvmAlloc = compiler.getContext().builder.CreateAlloca(llvmArrayType, nullptr, varName);
        // Store the array in the variable
        llvm::Value *llvmStore = compiler.getContext().builder.CreateStore(llvmValue, llvmAlloc);
        // Add the variable to the named values
        compiler.getContext().namedValues[varName] = llvmAlloc;

        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Literal Created");

        return llvmAlloc;
    }

    llvm::Value *Arrays::createArrayLiteral(CryoArrayNode *array, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Creating Array Literal");

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        // Get the type of the array
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < array->elementCount; ++i)
        {
            ASTNode *element = array->elements[i];
            if (element->metaData->type == NODE_LITERAL_EXPR)
            {
                // Note to self, might need to get the length of each element
                CryoDataType dataType = element->data.literal->dataType;
                switch (dataType)
                {
                case DATA_TYPE_INT:
                {
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, 0);
                    int index = element->data.literal->value.intValue;
                    llvm::Constant *llvmElement = llvm::ConstantInt::get(llvmType, index);
                    elements.push_back(llvmElement);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    int _len = compiler.getTypes().getLiteralValLength(element);
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, _len + 1);
                    llvm::Constant *llvmElement = llvm::ConstantDataArray::getString(compiler.getContext().context, element->data.literal->value.stringValue);
                    elements.push_back(llvmElement);
                    break;
                }
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown element type");
                DEBUG_BREAKPOINT;
            }
        }
        debugger.logMessage("INFO", __LINE__, "Arrays", "Finished processing elements");

        // Create the array
        llvmArrayType = llvm::ArrayType::get(llvmType, array->elementCount);
        llvmValue = llvm::ConstantArray::get(llvmArrayType, elements);

        // Get the current block that we are in
        llvm::BasicBlock *block = compiler.getContext().builder.GetInsertBlock();
        if (!block)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Block not found");
            CONDITION_FAILED;
        }

        // Alloc the array without initializing it
        llvm::Value *llvmAlloc = compiler.getContext().builder.CreateAlloca(llvmArrayType, nullptr, varName);
        // Store the array in the variable
        llvm::Value *llvmStore = compiler.getContext().builder.CreateStore(llvmValue, llvmAlloc);
        // Add the variable to the named values
        compiler.getContext().namedValues[varName] = llvmAlloc;

        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Literal Created");
        return llvmAlloc;
    }

    /**
     * @brief Handles the array literal node. Processes the array elements and returns the LLVM value.
     * @param node The array literal node.
     * @return The LLVM value of the array literal.
     */
    void Arrays::handleArrayLiteral(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Handling Array Literal");

        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        llvm::Type *llvmType = nullptr;
        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        // Get the type of the array
        std::vector<llvm::Constant *> elements;
        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (element->metaData->type == NODE_LITERAL_EXPR)
            {
                // Note to self, might need to get the length of each element
                CryoDataType dataType = element->data.literal->dataType;
                switch (dataType)
                {
                case DATA_TYPE_INT:
                {
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, 0);
                    llvm::Constant *llvmElement = llvm::ConstantInt::get(llvmType, element->data.literal->value.intValue);
                    elements.push_back(llvmElement);
                    break;
                }
                case DATA_TYPE_STRING:
                {
                    int _len = compiler.getTypes().getLiteralValLength(element);
                    llvmType = compiler.getTypes().getType(element->data.literal->dataType, _len + 1);
                    llvm::Constant *llvmElement = llvm::ConstantDataArray::getString(compiler.getContext().context, element->data.literal->value.stringValue);
                    elements.push_back(llvmElement);
                    break;
                }
                }
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown element type");
                CONDITION_FAILED;
            }
        }

        // Create the array
        llvmArrayType = llvm::ArrayType::get(llvmType, arrayNode->elementCount);
        llvmValue = llvm::ConstantArray::get(llvmArrayType, elements);

        // Create the constant
        llvmConstant = llvm::dyn_cast<llvm::Constant>(llvmValue);

        // Create the global variable
        llvm::GlobalVariable *var = new llvm::GlobalVariable(
            *compiler.getContext().module,
            llvmArrayType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            llvmConstant,
            llvm::Twine("arr"));

        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Literal Processed");

        return;
    }

    llvm::ArrayType *Arrays::getArrayType(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Converting array to LLVM type");

        llvm::Type *llvmType = nullptr;
        llvm::ArrayType *llvmArrayType = nullptr;

        switch (node->metaData->type)
        {
        case NODE_ARRAY_LITERAL:
        {
            debugger.logMessage("INFO", __LINE__, "Arrays", "Converting array literal to LLVM type");

            llvmType = compiler.getTypes().getType(DATA_TYPE_INT, 0);
            llvmArrayType = llvm::ArrayType::get(llvmType, getArrayLength(node));
            break;
        }

        default:
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown node type");
            CONDITION_FAILED;
        }

        return llvmArrayType;
    }

    /**
     * @brief Get the length of an array.
     * Of type `node->data.array`.
     * (ASSERTS)
     * @param node The array node.
     * @return The length of the array.
     */
    int Arrays::getArrayLength(ASTNode *node)
    {
        CryoArrayNode *arrayNode = node->data.array;
        assert(arrayNode != nullptr);

        return arrayNode->elementCount;
    }

    // This will be creating the global variable.
    void Arrays::handleIndexExpression(ASTNode *node, std::string varName)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        CryoContext &context = compiler.getContext();
        Variables &variables = compiler.getVariables();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Handling Index Expression");

        IndexExprNode *indexNode = node->data.indexExpr;
        assert(indexNode != nullptr);

        std::string arrayName = std::string(indexNode->name);

        if (node->metaData->type == NODE_INDEX_EXPR)
        {
            debugger.logMessage("INFO", __LINE__, "Arrays", "Variable is an index expression");
            CryoNodeType indexNodeType = node->data.indexExpr->index->metaData->type;

            // Get the array type
            ASTNode *arrayNode = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrayName);
            if (!arrayNode)
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Array not found");
                CONDITION_FAILED;
            }
            llvm::Type *arrayType = getArrayType(arrayNode);
            llvm::Type *elementType = arrayType->getArrayElementType();

            // Check if it's a literal `foo[5]`
            if (indexNodeType == NODE_LITERAL_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Arrays", "Index is a literal");
                int indexValue = indexNode->index->data.literal->value.intValue;

                // Multiply the index by the element size
                llvm::Value *scaledIndex = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context.context), indexValue);

                llvm::Value *arrayPtr = context.builder.CreateGEP(arrayType, context.namedValues[arrayName], scaledIndex);
                llvm::Value *loadedValue = context.builder.CreateLoad(elementType, arrayPtr);

                // Create the global variable
                llvm::GlobalVariable *var = new llvm::GlobalVariable(
                    *context.module,
                    elementType,
                    false,
                    llvm::GlobalValue::ExternalLinkage,
                    llvm::dyn_cast<llvm::Constant>(loadedValue),
                    llvm::Twine(varName));
            }
            // Check if it's a variable `foo[bar]`
            else if (indexNodeType == NODE_VAR_NAME)
            {
                std::string indexVarName = indexNode->index->data.varName->varName;
                llvm::Value *indexValue = variables.getVariable(indexVarName);
                if (!indexValue)
                {
                    debugger.logMessage("ERROR", __LINE__, "Arrays", "Index variable not found");
                    CONDITION_FAILED;
                }

                llvm::Value *arrayRef = context.namedValues[arrayName];
                llvm::Value *arrayPtr = context.builder.CreateGEP(arrayType, arrayRef, indexValue);
                llvm::Value *loadedValue = context.builder.CreateLoad(elementType, arrayPtr);

                // Create the local variable
                llvm::Value *var = context.builder.CreateAlloca(elementType, loadedValue, varName);

                context.namedValues[varName] = var;
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown index type");
                return;
            }
        }
        else
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown node type");
            return;
        }
    }

    llvm::Value *Arrays::indexArrayForValue(ASTNode *array, int index)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Array", "Indexing Array for Value");

        CryoArrayNode *arrayNode = array->data.array;
        assert(arrayNode != nullptr);

        for (int i = 0; i < arrayNode->elementCount; ++i)
        {
            ASTNode *element = arrayNode->elements[i];
            if (i == index)
            {
                debugger.logMessage("INFO", __LINE__, "Array", "Index Found");
                return compiler.getGenerator().getInitilizerValue(element);
            }
            else
            {
                debugger.logMessage("INFO", __LINE__, "Array", "Index Not Found");
            }
        }
        debugger.logMessage("ERROR", __LINE__, "Array", "Failed to index array for value");
    }

    llvm::Value *Arrays::getIndexExpressionValue(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BackendSymTable &symTable = compiler.getSymTable();
        CryoContext &context = compiler.getContext();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Getting Index Expression Value");

        IndexExprNode *indexNode = node->data.indexExpr;
        assert(indexNode != nullptr);

        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Node Found");

        std::string arrayName = std::string(indexNode->name);
        debugger.logMessage("INFO", __LINE__, "Arrays", "Array Name: " + arrayName);

        // This should be returning the value of the indexed array
        llvm::Value *indexedValue = nullptr;
        ASTNode *array = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrayName);
        if (!array)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Array not found");
            CONDITION_FAILED;
        }
        int elementCount = getArrayLength(array);

        // Get the literal value from the index variable
        ASTNode *literalNode = indexNode->index;
        if (literalNode->metaData->type == NODE_VAR_NAME)
        {
            ASTNode *indexNodeVar = symTable.getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, literalNode->data.varName->varName);
            if (!indexNodeVar)
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Index variable not found");
                CONDITION_FAILED;
            }
            if (indexNodeVar->metaData->type == NODE_LITERAL_EXPR)
            {
                debugger.logMessage("INFO", __LINE__, "Arrays", "Index variable is a literal");
                int _indexValue = compiler.getTypes().getLiteralIntValue(indexNodeVar->data.literal);
                indexedValue = indexArrayForValue(array, _indexValue);

                return indexedValue;
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown index type");
                DEBUG_BREAKPOINT;
            }
        }
        if (literalNode->metaData->type == NODE_LITERAL_EXPR)
        {
            if (literalNode->data.literal->dataType == DATA_TYPE_INT)
            {
                int _indexValue = literalNode->data.literal->value.intValue;
                ASTNode *element = array->data.array->elements[_indexValue];
                debugger.logNode(element);
                llvm::Value *val = compiler.getGenerator().getInitilizerValue(element);
                debugger.logLLVMValue(val);
                return val;
            }
            else
            {
                debugger.logMessage("ERROR", __LINE__, "Arrays", "Unknown literal type");
                CONDITION_FAILED;
            }
        }

        debugger.logMessage("INFO", __LINE__, "Arrays", "Index Expression Value Found");

        return indexedValue;
    }

    void Arrays::isOutOfBoundsException(llvm::Value *array, llvm::Value *index)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        CryoContext &context = compiler.getContext();
        debugger.logMessage("INFO", __LINE__, "Arrays", "Checking for out of bounds exception");

        std::string arrName = array->getName().str();
        ASTNode *arrayNode = compiler.getSymTable().getASTNode(context.currentNamespace, NODE_VAR_DECLARATION, arrName);
        if (!arrayNode)
        {
            debugger.logMessage("ERROR", __LINE__, "Arrays", "Array not found");
            CONDITION_FAILED;
        }

        int ASTarraySize = getArrayLength(arrayNode);
        std::cout << "Array Size: " << ASTarraySize << std::endl;

        llvm::Value *arrSize = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.context), ASTarraySize);
        debugger.logLLVMValue(arrSize);

        // Create basic blocks for in-bounds and out-of-bounds cases
        llvm::Function *currentFunction = context.builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *inBoundsBlock = llvm::BasicBlock::Create(context.context, "in_bounds", currentFunction);
        llvm::BasicBlock *outOfBoundsBlock = llvm::BasicBlock::Create(context.context, "out_of_bounds", currentFunction);
        llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.context, "merge", currentFunction);

        // Check if index is less than array size
        llvm::Value *cmp = context.builder.CreateICmpULT(index, arrSize);
        context.builder.CreateCondBr(cmp, inBoundsBlock, outOfBoundsBlock);

        // Out-of-bounds block
        context.builder.SetInsertPoint(outOfBoundsBlock);
        llvm::Value *undefinedStr = context.builder.CreateGlobalStringPtr("undefined");
        llvm::Function *func = context.module->getFunction("printStr");
        if (func)
        {
            context.builder.CreateCall(func, {undefinedStr});
        }
        llvm::Type *elementType = array->getType();
        // Create an "undefined" value of the appropriate type
        llvm::Value *undefinedValue = llvm::UndefValue::get(elementType);
        context.builder.CreateBr(mergeBlock);
        // In-bounds block
        context.builder.SetInsertPoint(inBoundsBlock);
        llvm::Value *arrayPtr = context.builder.CreateGEP(elementType, array, index);
        llvm::Value *loadedValue = context.builder.CreateLoad(elementType, arrayPtr);
        context.builder.CreateBr(mergeBlock);

        // Merge block
        context.builder.SetInsertPoint(mergeBlock);
        llvm::PHINode *result = context.builder.CreatePHI(elementType, 2);
        result->addIncoming(loadedValue, inBoundsBlock);
        result->addIncoming(undefinedValue, outOfBoundsBlock);

        debugger.logMessage("INFO", __LINE__, "Arrays", "Out of Bounds Exception Checked");

        return;
    }

} // namespace Cryo
