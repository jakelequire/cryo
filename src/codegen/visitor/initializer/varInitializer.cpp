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

    llvm::Value *Initializer::generateVarName(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating variable name...");
        ASSERT_NODE_NULLPTR_RET(node);
        if (node->metaData->type != NODE_VAR_NAME)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a variable name");
            return nullptr;
        }

        std::string varName = node->data.varName->varName;
        logMessage(LMI, "INFO", "Initializer", "Variable name: %s", varName.c_str());

        IRVariableSymbol *varSymbol = getSymbolTable()->findVariable(varName);
        if (!varSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Variable %s not found", varName.c_str());
            // Print the symbol table for debugging
            context.getInstance().symbolTable->debugPrint();
            return nullptr;
        }
        DataType *varDataType = varSymbol->dataType;
        if (!varDataType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Variable %s has no data type", varName.c_str());
            return nullptr;
        }
        if (varDataType->container->typeOf == OBJECT_TYPE)
        {
            logMessage(LMI, "INFO", "Initializer", "Variable %s is an object type", varName.c_str());
            if (varSymbol->value && varSymbol->value->getType()->isPointerTy())
            {
                return varSymbol->value;
            }
            else
            {
                logMessage(LMI, "ERROR", "Initializer", "Variable %s is not a pointer type", varName.c_str());
                return nullptr;
            }
        }

        logMessage(LMI, "INFO", "Initializer", "Variable %s found", varName.c_str());
        return varSymbol->value;
    }

    void CodeGenVisitor::visitVarDecl(ASTNode *node)
    {
        if (!validateNode(node, "variable declaration"))
            return;

        std::string varName = node->data.varDecl->name;
        DataType *varType = node->data.varDecl->type;
        ASTNode *initializer = node->data.varDecl->initializer;
        bool noInitializer = node->data.varDecl->noInitializer;

        if (noInitializer)
        {
            logMessage(LMI, "INFO", "Visitor", "Variable %s has no initializer", varName.c_str());
            varType->debug->printVerbosType(varType);
            handleNoInitializer(varName, varType);
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting variable declaration: %s", varName.c_str());
        varType->debug->printVerbosType(varType);
        initializer->print(initializer);
        // Handle variable based on its type

        // String type with literal initializer
        if (isStringWithLiteralInitializer(varType, initializer))
        {
            logMessage(LMI, "INFO", "Visitor", "Handling String variable with literal initializer: %s", varName.c_str());
            handleStringVarDecl(varName, varType, initializer);
        }
        // Struct or Object type with initializer
        else if (isStructWithInitializer(varType, initializer))
        {
            logMessage(LMI, "INFO", "Visitor", "Handling struct variable with initializer: %s", varName.c_str());
            handleStructVarDecl(varName, varType, initializer);
        }
        // Array type with initializer
        else if (isArrayWithInitializer(varType, initializer))
        {
            logMessage(LMI, "INFO", "Visitor", "Handling array variable with initializer: %s", varName.c_str());
            handleArrayInitializer(varName, varType, initializer);
        }
        // Is an index expression
        else if (isIndexExpression(initializer))
        {
            logMessage(LMI, "INFO", "Visitor", "Handling index expression: %s", varName.c_str());
            handleIndexExpression(varName, varType, initializer);
        }
        else
        {
            logMessage(LMI, "INFO", "Visitor", "Handling primitive variable declaration: %s", varName.c_str());
            handlePrimitiveVarDecl(varName, varType, initializer);
        }
    }

    // Helper methods

    bool CodeGenVisitor::validateNode(ASTNode *node, const char *nodeType)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return false;
        }
        return true;
    }

    bool CodeGenVisitor::isStringWithLiteralInitializer(DataType *varType, ASTNode *initializer)
    {
        return std::string(varType->typeName) == "String" &&
               initializer &&
               initializer->metaData->type == NODE_LITERAL_EXPR &&
               initializer->data.literal->literalType == LITERAL_STRING;
    }

    bool CodeGenVisitor::isStructWithInitializer(DataType *varType, ASTNode *initializer)
    {
        return varType->container->typeOf == OBJECT_TYPE && initializer;
    }

    bool CodeGenVisitor::isArrayWithInitializer(DataType *varType, ASTNode *initializer)
    {
        return varType->container->primitive == PRIM_ARRAY && initializer;
    }

    bool CodeGenVisitor::isIndexExpression(ASTNode *node)
    {
        return node->metaData->type == NODE_INDEX_EXPR;
    }

    void CodeGenVisitor::handleStringVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling String variable with string literal initializer");

        // Get the string literal value
        std::string strValue = initializer->data.literal->value.stringValue;

        // Get or create the global string constant
        llvm::Value *globalStr = symbolTable->getOrCreateGlobalString(strValue);

        // Get the String struct type
        llvm::StructType *stringType = llvm::StructType::getTypeByName(
            context.getInstance().context, "struct.String");
        if (!stringType)
        {
            logMessage(LMI, "ERROR", "Visitor", "String struct type not found");
            return;
        }

        // Create an alloca for the String struct
        llvm::AllocaInst *stringAlloca = builder.CreateAlloca(stringType, nullptr, varName);
        stringAlloca->setAlignment(llvm::Align(8));

        // Store the string pointer into the struct
        llvm::Value *ptrField = builder.CreateStructGEP(stringType, stringAlloca, 0, varName + ".ptr");
        builder.CreateStore(globalStr, ptrField);

        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, stringAlloca, stringType->getPointerTo(), AllocaType::AllocaOnly);

        logMessage(LMI, "INFO", "Visitor", "String variable %s initialized with literal", varName.c_str());
    }

    void CodeGenVisitor::handleStructVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling object/struct variable with initializer");

        // Get the LLVM type for the struct
        llvm::Type *llvmType = symbolTable->getLLVMType(varType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for %s", varType->typeName);
            return;
        }

        // Create the alloca for the struct
        llvm::AllocaInst *structAlloca = builder.CreateAlloca(llvmType, nullptr, varName);
        structAlloca->setAlignment(llvm::Align(8));

        // Handle different types of initializers
        llvm::Value *initVal = nullptr;

        if (initializer->metaData->type == NODE_OBJECT_INST)
        {
            initVal = generateObjectInstance(structAlloca, initializer);
        }
        else
        {
            initVal = getLLVMValue(initializer);
            storeInitializerValue(initVal, structAlloca, llvmType);
        }

        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, structAlloca, llvmType->getPointerTo(),
                                 AllocaTypeInference::inferFromNode(initializer, false));

        logMessage(LMI, "INFO", "Visitor", "Struct variable %s initialized", varName.c_str());
    }

    llvm::Value *CodeGenVisitor::generateObjectInstance(llvm::AllocaInst *structAlloca, ASTNode *initializer)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling object instance initializer");

        llvm::Value *objectInstance = context.getInstance().initializer->generateObjectInst(
            structAlloca, initializer);

        if (!objectInstance)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to generate object instance");
            CONDITION_FAILED;
            return nullptr;
        }

        // Store the object instance into the alloca
        builder.CreateStore(objectInstance, structAlloca);
        return objectInstance;
    }

    void CodeGenVisitor::storeInitializerValue(llvm::Value *initVal, llvm::AllocaInst *allocaInst, llvm::Type *llvmType)
    {
        if (!initVal)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get initializer value");
            CONDITION_FAILED;
            return;
        }

        // Check if the initializer is a pointer to a struct
        if (initVal->getType()->isPointerTy())
        {
            llvm::Value *loadedStruct = builder.CreateLoad(llvmType, initVal, "struct.load");
            builder.CreateStore(loadedStruct, allocaInst);
        }
        else
        {
            // If it's already a struct value, just store it
            builder.CreateStore(initVal, allocaInst);
        }
    }

    void CodeGenVisitor::handlePrimitiveVarDecl(const std::string &varName, DataType *varType, ASTNode *initializer)
    {
        llvm::Type *llvmType = symbolTable->getLLVMType(varType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for %s", varType->typeName);
            return;
        }

        // Create the alloca
        llvm::AllocaInst *allocaInst = builder.CreateAlloca(llvmType, nullptr, varName);
        setAppropriateAlignment(allocaInst, llvmType);

        // Handle initializer if present
        if (initializer)
        {
            handlePrimitiveInitializer(allocaInst, initializer, llvmType);
        }

        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, allocaInst, llvmType->getPointerTo(),
                                 AllocaTypeInference::inferFromNode(initializer, false));

        logMessage(LMI, "INFO", "Visitor", "Variable %s initialized", varName.c_str());
    }

    void CodeGenVisitor::setAppropriateAlignment(llvm::AllocaInst *allocaInst, llvm::Type *llvmType)
    {
        if (llvmType->isIntegerTy())
        {
            unsigned bitWidth = llvmType->getIntegerBitWidth();
            if (bitWidth <= 8)
                allocaInst->setAlignment(llvm::Align(1));
            else if (bitWidth <= 16)
                allocaInst->setAlignment(llvm::Align(2));
            else if (bitWidth <= 32)
                allocaInst->setAlignment(llvm::Align(4));
            else
                allocaInst->setAlignment(llvm::Align(8));
        }
        else if (llvmType->isFloatingPointTy())
        {
            allocaInst->setAlignment(llvmType->isFloatTy() ? llvm::Align(4) : llvm::Align(8));
        }
        else if (llvmType->isPointerTy() || llvmType->isStructTy())
        {
            allocaInst->setAlignment(llvm::Align(8));
        }
        else
        {
            allocaInst->setAlignment(llvm::Align(8)); // Default alignment
        }
    }

    void CodeGenVisitor::handlePrimitiveInitializer(llvm::AllocaInst *allocaInst, ASTNode *initializer, llvm::Type *llvmType)
    {
        llvm::Value *initVal = nullptr;

        // Log the initializer type before processing
        logMessage(LMI, "DEBUG", "Visitor", "Processing initializer of type: %s",
                   CryoNodeTypeToString(initializer->metaData->type));

        if (initializer->metaData->type == NODE_LITERAL_EXPR &&
            initializer->data.literal->literalType == LITERAL_STRING &&
            initializer->data.literal->type->container->primitive == PRIM_STR)
        {
            logMessage(LMI, "INFO", "Visitor", "String literal initializer detected");
            initVal = context.getInstance().initializer->generateStringInitializer(initializer);
        }
        else if (initializer->metaData->type == NODE_BINARY_EXPR)
        {
            logMessage(LMI, "DEBUG", "Visitor", "Binary expression initializer detected with operator: %s",
                       CryoOperatorTypeToString(initializer->data.bin_op->op));
            initVal = getLLVMValue(initializer);
        }
        else
        {
            logMessage(LMI, "INFO", "Visitor", "Variable has an initialization expression");
            initVal = getLLVMValue(initializer);
        }

        if (initVal)
        {
            // Handle type conversions if needed
            if (initVal->getType() != llvmType)
            {
                logMessage(LMI, "INFO", "Visitor", "Initializer type doesn't match variable type, attempting conversion");
                initVal = convertInitializerType(initVal, llvmType);
            }

            // Store the initializer value with logging
            llvm::StoreInst *storeInst = builder.CreateStore(initVal, allocaInst);
            logMessage(LMI, "DEBUG", "Visitor", "Stored value %p in alloca %p", initVal, allocaInst);
        }
        else
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get initializer value");
        }
    }

    void CodeGenVisitor::handleArrayInitializer(const std::string &varName, DataType *varType, ASTNode *initializer)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling array variable with initializer");

        llvm::Type *llvmType = symbolTable->getLLVMType(varType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for %s", varType->typeName);
            return;
        }

        // Get the array type from the initializer
        llvm::ArrayType *arrayType = context.getInstance().symbolTable->getLLVMArrayType(initializer);
        llvm::Value *arrayValue = context.getInstance().initializer->generateArrayLiteral(initializer);
        if (!arrayValue)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to generate array literal");
            return;
        }

        arrayValue->setName(varName);
        llvm::AllocaInst *arrAlloca = llvm::dyn_cast<llvm::AllocaInst>(arrayValue);
        if (!arrAlloca)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to create array alloca");
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Array variable %s initialized", varName.c_str());
        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, arrAlloca, arrayType,
                                 AllocaTypeInference::inferFromNode(initializer, false));
        logMessage(LMI, "INFO", "Visitor", "Array variable %s initialized", varName.c_str());
    }

    void CodeGenVisitor::handleIndexExpression(const std::string &varName, DataType *varType, ASTNode *initializer)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling index expression");

        llvm::Value *indexValue = context.getInstance().initializer->generateIndexExpr(initializer);
        if (!indexValue)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to generate index expression");
            return;
        }

        llvm::Type *llvmType = symbolTable->getLLVMType(varType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for %s", varType->typeName);
            return;
        }

        llvm::AllocaInst *varAlloca = context.getInstance().builder.CreateAlloca(
            llvmType, nullptr, varName);

        // Store the index value into the alloca
        if (indexValue->getType()->isPointerTy())
        {
            llvm::Value *loadedIndex = builder.CreateLoad(indexValue->getType(), indexValue, "index.load");
            builder.CreateStore(loadedIndex, varAlloca);
        }
        else
        {
            builder.CreateStore(indexValue, varAlloca);
        }
        // Set the alignment for the alloca
        setAppropriateAlignment(varAlloca, llvmType);

        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, varAlloca, llvmType->getPointerTo(),
                                 AllocaTypeInference::inferFromNode(initializer, false));
        logMessage(LMI, "INFO", "Visitor", "Index expression variable %s initialized", varName.c_str());
    }

    void CodeGenVisitor::handleNoInitializer(const std::string &varName, DataType *varType)
    {
        logMessage(LMI, "INFO", "Visitor", "Handling variable with no initializer");

        llvm::Type *llvmType = symbolTable->getLLVMType(varType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for %s", varType->typeName);
            return;
        }

        // Create the alloca
        llvm::AllocaInst *allocaInst = builder.CreateAlloca(llvmType, nullptr, varName);
        setAppropriateAlignment(allocaInst, llvmType);

        // Create and add variable symbol
        addVariableToSymbolTable(varName, varType, allocaInst, llvmType->getPointerTo(),
                                 AllocaTypeInference::inferFromNode(nullptr, false));

        logMessage(LMI, "INFO", "Visitor", "Variable %s initialized with no initializer", varName.c_str());
    }

    llvm::Value *CodeGenVisitor::convertInitializerType(llvm::Value *initVal, llvm::Type *targetType)
    {
        // Handle simple pointer to primitive type (loading)
        if (initVal->getType()->isPointerTy() && !targetType->isPointerTy())
        {
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(initVal))
            {
                llvm::Type *pointeeType = allocaInst->getAllocatedType();
                if (pointeeType == targetType)
                {
                    return builder.CreateLoad(pointeeType, initVal, "init.load");
                }
            }
        }

        // Return original value if no conversion was applied
        return initVal;
    }

    void CodeGenVisitor::addVariableToSymbolTable(const std::string &varName, DataType *varType,
                                                  llvm::Value *value, llvm::Type *pointerType,
                                                  AllocaType allocaType)
    {
        IRVariableSymbol varSymbol = IRSymbolManager::createVariableSymbol(
            nullptr, value, pointerType, varName, allocaType);
        varSymbol.dataType = varType;
        varSymbol.value = value;

        symbolTable->addVariable(varSymbol);
    }

} // namespace Cryo
