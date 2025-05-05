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
#include "codegen/codegen.hpp"

namespace Cryo
{

    llvm::Value *Initializer::getInitializerValue(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);
        std::string nodeTypeStr = CryoNodeTypeToString(node->metaData->type);
        logMessage(LMI, "INFO", "Initializer", "Getting initializer value for node: %s", nodeTypeStr.c_str());

        switch (node->metaData->type)
        {
        case NODE_LITERAL_EXPR:
            return generateLiteralExpr(node);
        case NODE_VAR_NAME:
            return generateVarName(node);
        case NODE_BINARY_EXPR:
            return generateBinaryExpr(node);
        case NODE_FUNCTION_CALL:
            return generateFunctionCall(node);
        case NODE_RETURN_STATEMENT:
            return generateReturnStatement(node);
        case NODE_UNARY_EXPR:
            return generateUnaryExpr(node);
        case NODE_PARAM:
            return generateParam(node);
        case NODE_PROPERTY_ACCESS:
            return generatePropertyAccess(node);
        case NODE_SCOPED_FUNCTION_CALL:
            return generateScopedFunctionCall(node);
        case NODE_VAR_DECLARATION:
            return generateVarDeclaration(node);
        case NODE_OBJECT_INST:
            return generateObjectInst(node);
        case NODE_NULL_LITERAL:
            return generateNullLiteral();
        case NODE_ARRAY_LITERAL:
            return generateArrayLiteral(node);
        case NODE_INDEX_EXPR:
            return generateIndexExpr(node);
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateReturnStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating return statement...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_RETURN_STATEMENT)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a return statement");
            return nullptr;
        }

        return getInitializerValue(node->data.returnStatement->expression);
    }

    llvm::Value *Initializer::generateParam(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating parameter...");
        ASSERT_NODE_NULLPTR_RET(node);
        if (node->metaData->type != NODE_PARAM)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a parameter");
            return nullptr;
        }

        // All parameters are locally allocated variables
        // They are allocated when the function is created and should already be existing in the current scope

        std::string paramName = node->data.param->name;
        logMessage(LMI, "INFO", "Initializer", "Parameter name: %s", paramName.c_str());

        IRVariableSymbol *paramSymbol = context.getInstance().symbolTable->findVariable(paramName);
        if (!paramSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Parameter %s not found", paramName.c_str());
            // Print the symbol table for debugging
            context.getInstance().symbolTable->debugPrint();
            DEBUG_BREAKPOINT;
            return nullptr;
        }
        logMessage(LMI, "INFO", "Initializer", "Parameter %s found", paramName.c_str());

        // Load the parameter if needed
        if (paramSymbol->allocaType == AllocaType::AllocaAndLoad ||
            paramSymbol->allocaType == AllocaType::AllocaLoadStore)
        {
            // paramSymbol->allocation.load(context.getInstance().builder, paramName + ".load");
        }

        return paramSymbol->value;
    }

    llvm::Value *Initializer::generateVarDeclaration(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating variable declaration...");
        DEBUG_BREAKPOINT;
    }

    llvm::Value *Initializer::generateNullLiteral()
    {
        logMessage(LMI, "INFO", "Initializer", "Generating null literal...");
        llvm::Type *nullType = context.getInstance().builder.getInt8Ty();
        llvm::Value *nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(nullType, 0));
        return nullValue;
    }

    llvm::Value *Initializer::generateArrayLiteral(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating array literal...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_ARRAY_LITERAL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not an array literal");
            return nullptr;
        }

        // Handle array literal generation
        int elementCount = node->data.array->elementCount;
        bool isMonomorphic = node->data.array->type->container->type.arrayType->isMonomorphic;
        if (isMonomorphic)
        {
            logMessage(LMI, "INFO", "Initializer", "Generating monomorphic array literal...");
            llvm::Type *elementType = context.getInstance().symbolTable->getLLVMType(node->data.array->type);
            llvm::ArrayType *arrayType = llvm::ArrayType::get(elementType, elementCount);

            // Check if all elements are constants or have constant values
            bool allConstant = true;
            bool isStringArray = false;
            std::vector<llvm::Constant *> constantValues;

            for (int i = 0; i < elementCount && allConstant; ++i)
            {
                llvm::Value *elementValue = getInitializerValue(node->data.array->elements[i]);
                if (!elementValue)
                {
                    logMessage(LMI, "ERROR", "Initializer", "Element value is null at index %s", std::to_string(i).c_str());
                    return nullptr;
                }

                // Check if the element type is a global string literal
                if (llvm::GlobalVariable *globalVar = llvm::dyn_cast<llvm::GlobalVariable>(elementValue))
                {
                    if (globalVar->getName().str().find("g_str") != std::string::npos)
                    {
                        isStringArray = true;
                    }
                }

                // Check if this is a constant value
                if (llvm::Constant *constVal = llvm::dyn_cast<llvm::Constant>(elementValue))
                {
                    constantValues.push_back(constVal);
                }
                else
                {
                    allConstant = false;
                }
            }

            llvm::AllocaInst *arrayAlloca = context.getInstance().builder.CreateAlloca(elementType, nullptr, "array");

            if (allConstant && !isStringArray)
            {
                // Create a constant array aggregate and store it at once
                llvm::Constant *constArray = llvm::ConstantArray::get(arrayType, constantValues);
                context.getInstance().builder.CreateStore(constArray, arrayAlloca);

                logMessage(LMI, "INFO", "Initializer", "Constant array literal generated successfully");
            }
            else if (isStringArray)
            {
                // Handle string array initialization

                // Create the array type: [Nx ptr]
                llvm::ArrayType *arrayTy = llvm::ArrayType::get(llvm::PointerType::get(context.getInstance().builder.getInt8Ty(), 0), elementCount);

                llvm::Constant *stringArray = llvm::ConstantArray::get(arrayTy, constantValues);
                context.getInstance().builder.CreateStore(stringArray, arrayAlloca);

                logMessage(LMI, "INFO", "Initializer", "String array literal generated successfully");
            }
            else
            {
                // Fall back to element-by-element initialization for non-constant values
                for (int i = 0; i < elementCount; ++i)
                {
                    llvm::Value *elementValue = getInitializerValue(node->data.array->elements[i]);

                    // Create a GEP to get a pointer to this element
                    std::vector<llvm::Value *> indices = {
                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.getInstance().context), 0),
                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.getInstance().context), i)};

                    llvm::Value *elementPtr = context.getInstance().builder.CreateGEP(arrayType, arrayAlloca, indices, "array_element_ptr");

                    // Store the element value into the array
                    context.getInstance().builder.CreateStore(elementValue, elementPtr);
                }

                logMessage(LMI, "INFO", "Initializer", "Dynamic array literal generated successfully");
            }
            // Return the array pointer
            return arrayAlloca;
        }
        else
        {
            // Handle heterogeneous arrays if needed
            logMessage(LMI, "WARNING", "Initializer", "Heterogeneous arrays not yet supported");
        }

        DEBUG_BREAKPOINT;
        return nullptr;
    }

    llvm::Value *Initializer::generateIndexExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating index expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_INDEX_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not an index expression");
            return nullptr;
        }

        // Handle index expression generation
        ASTNode *arrayNode = node->data.indexExpr->array;
        llvm::Value *arrayValue = getInitializerValue(arrayNode);
        llvm::Value *indexValue = getInitializerValue(node->data.indexExpr->index);
        llvm::Type *arrayType;

        if (!arrayValue || !indexValue)
        {
            logMessage(LMI, "ERROR", "Initializer", "Array or index value is null");
            CONDITION_FAILED;
        }

        if (arrayNode->metaData->type == NODE_PARAM)
        {
            logMessage(LMI, "INFO", "Initializer", "Array is a parameter");
            // Handle parameter array indexing
            IRVariableSymbol *paramSymbol = context.getInstance().symbolTable->findVariable(arrayNode->data.param->name);
            if (!paramSymbol)
            {
                logMessage(LMI, "ERROR", "Initializer", "Parameter %s not found", arrayNode->data.param->name);
                return nullptr;
            }
            arrayValue = paramSymbol->value;
            arrayType = paramSymbol->type;
        }
        else if (arrayNode->metaData->type == NODE_VAR_NAME)
        {
            logMessage(LMI, "INFO", "Initializer", "Array is a variable");
            // Handle variable array indexing
            IRVariableSymbol *varSymbol = context.getInstance().symbolTable->findVariable(arrayNode->data.varName->varName);
            if (!varSymbol)
            {
                logMessage(LMI, "ERROR", "Initializer", "Variable %s not found", arrayNode->data.varName->varName);
                return nullptr;
            }
            arrayValue = varSymbol->value;
            arrayType = varSymbol->type;
        }
        else
        {
            logMessage(LMI, "ERROR", "Initializer", "Array node type not supported for indexing");
            return nullptr;
        }
        // Create a GEP to get the indexed element
        if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(arrayValue))
        {
            arrayType = allocaInst->getAllocatedType();
        }

        std::vector<llvm::Value *> indices = {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.getInstance().context), 0),
            indexValue};

        if (!arrayValue->getType()->isPointerTy())
        {
            // Create a pointer to the array value
            llvm::PointerType *ptrType = llvm::PointerType::get(arrayValue->getType(), 0);
            arrayValue = context.getInstance().builder.CreateBitCast(arrayValue, ptrType, "array_ptr");
        }
        llvm::Value *elementPtr = context.getInstance().builder.CreateGEP(arrayType, arrayValue, indices, "index_element_ptr");

        return elementPtr;
    }

} // namespace Cryo
