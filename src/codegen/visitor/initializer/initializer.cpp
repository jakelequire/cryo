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
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateLiteralExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating literal expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a literal expression");

            return nullptr;
        }

        DataType *literalDataType = node->data.literal->type;
        logMessage(LMI, "INFO", "Initializer", "Literal data type: %s", DTM->debug->dataTypeToString(literalDataType));

        TypeofDataType typeofDataType = literalDataType->container->typeOf;
        logMessage(LMI, "INFO", "Initializer", "Type of data: %s", DTM->debug->typeofDataTypeToString(typeofDataType));

        PrimitiveDataType primitiveType = literalDataType->container->primitive;
        logMessage(LMI, "INFO", "Initializer", "Primitive type: %s", DTM->debug->primitiveDataTypeToString(primitiveType));

        if (typeofDataType == OBJECT_TYPE)
        {
            logMessage(LMI, "INFO", "Initializer", "Creating object from literal");
            return generateLiteralObject(node);
        }

        node->print(node);

        switch (literalDataType->container->primitive)
        {
        case PRIM_INT:
            return this->createIntegerLiteral(32, node->data.literal->value.intValue, true);
        case PRIM_I8:
            return this->createIntegerLiteral(8, node->data.literal->value.intValue, true);
        case PRIM_I16:
            return this->createIntegerLiteral(16, node->data.literal->value.intValue, true);
        case PRIM_I32:
            return this->createIntegerLiteral(32, node->data.literal->value.intValue, true);
        case PRIM_I64:
            return this->createIntegerLiteral(64, node->data.literal->value.intValue, true);
        case PRIM_I128:
            return this->createIntegerLiteral(128, node->data.literal->value.intValue, true);
        case PRIM_FLOAT:
            return llvm::ConstantFP::get(context.getInstance().context, llvm::APFloat(node->data.literal->value.floatValue));
        case PRIM_BOOLEAN:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(1, node->data.literal->value.booleanValue, true));
        case PRIM_STRING:
            return generateStringLiteral(node);
        case PRIM_STR:
            return generateStringLiteral(node);
        case PRIM_NULL:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_VOID:
            return llvm::UndefValue::get(llvm::Type::getVoidTy(context.getInstance().context));
        case PRIM_ANY:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_OBJECT:
        {
            logMessage(LMI, "INFO", "Initializer", "Creating object from literal");
            llvm::Value *objPtr = generateLiteralObject(node);
            if (!objPtr)
            {
                logMessage(LMI, "ERROR", "Initializer", "Failed to create object from literal");
                CONDITION_FAILED;
            }
        }
        case PRIM_AUTO:
            return nullptr;
        case PRIM_UNDEFINED:
            return nullptr;
        case PRIM_UNKNOWN:
            return nullptr;
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled literal type: %s", DTM->debug->dataTypeToString(literalDataType));
            return nullptr;
        }

        std::cerr << "Unhandled literal type: " << DTM->debug->dataTypeToString(literalDataType) << std::endl;
        return nullptr;
    }

    // This function will always return the dereferenced value of the literal object, not a pointer
    // to the object. This is because the literal object is not a pointer type by default.
    // The callee is responsible for handling the pointer type if needed.
    llvm::Value *Initializer::generateLiteralObject(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_LITERAL_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a literal expression");
            CONDITION_FAILED;
            return nullptr;
        }

        DataType *literalDataType = node->data.literal->type;
        logMessage(LMI, "INFO", "Initializer", "Literal Object data type: %s", DTM->debug->dataTypeToString(literalDataType));

        TypeofDataType typeofDataType = literalDataType->container->typeOf;
        logMessage(LMI, "INFO", "Initializer", "Type of data: %s", DTM->debug->typeofDataTypeToString(typeofDataType));

        if (typeofDataType == OBJECT_TYPE)
        {
            switch (node->data.literal->literalType)
            {
            case LITERAL_INT:
            {
                logMessage(LMI, "INFO", "Initializer", "Creating object from integer literal");
                node->print(node);
                // Get the object type name
                std::string typeName = literalDataType->typeName;
                logMessage(LMI, "INFO", "Initializer", "Creating object instance of type: %s", typeName.c_str());

                // Get the LLVM type
                llvm::Type *objectType = context.getInstance().symbolTable->getLLVMType(literalDataType);
                if (!objectType)
                {
                    logMessage(LMI, "ERROR", "Initializer", "Failed to get LLVM type for %s", typeName.c_str());
                    CONDITION_FAILED;
                    return nullptr;
                }

                // Allocate memory for the struct
                llvm::Value *objectInstance = context.getInstance().builder.CreateAlloca(objectType, nullptr, typeName + ".instance");
                // Create constructor argument based on the literal type
                llvm::Value *ctorArg = llvm::ConstantInt::get(context.getInstance().context,
                                                              llvm::APInt(32, node->data.literal->value.intValue, true));
                std::string ctorName = "struct." + typeName + ".ctor";

                // Find the constructor function
                IRFunctionSymbol *ctorFunc = context.getInstance().symbolTable->findFunction(ctorName);
                if (ctorFunc)
                {
                    // Prepare constructor arguments
                    std::vector<llvm::Value *> args;
                    // First arg is always 'this' pointer
                    args.push_back(objectInstance);
                    // Add the literal value as second arg
                    args.push_back(ctorArg);

                    // Call the constructor
                    context.getInstance().builder.CreateCall(ctorFunc->function, args, "call_ctor." + typeName);
                    logMessage(LMI, "INFO", "Initializer", "Called constructor %s", ctorName.c_str());
                }
                else
                {
                    logMessage(LMI, "WARNING", "Initializer", "Constructor %s not found", ctorName.c_str());
                }

                // Load the object instance
                llvm::Value *objectValue = context.getInstance().builder.CreateLoad(objectType, objectInstance, "load." + typeName);
                logMessage(LMI, "INFO", "Initializer", "Loaded object instance: %s", objectValue->getName().str().c_str());
                return objectValue;
            }
            case LITERAL_STRING:
            {
                logMessage(LMI, "INFO", "Initializer", "Creating object from string literal");
                break;
            }
            case LITERAL_FLOAT:
            {
                logMessage(LMI, "INFO", "Initializer", "Creating object from float literal");
                break;
            }
            case LITERAL_BOOLEAN:
            {
                logMessage(LMI, "INFO", "Initializer", "Creating object from boolean literal");
                break;
            }
            default:
            {
                logMessage(LMI, "ERROR", "Initializer", "Unhandled literal type: %s", DTM->debug->literalTypeToString(node->data.literal->literalType));
                return nullptr;
            }
            }
        }
    }

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

        // Load the variable if needed
        if (varSymbol->allocaType == AllocaType::AllocaAndLoad ||
            varSymbol->allocaType == AllocaType::AllocaLoadStore)
        {
            varSymbol->allocation.load(context.getInstance().builder, varName + "_load");
        }

        logMessage(LMI, "INFO", "Initializer", "Variable %s found", varName.c_str());
        return varSymbol->value;
    }

    llvm::Value *Initializer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate binary expression
        context.getInstance().module->print(llvm::errs(), nullptr);

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a binary expression");
            return nullptr;
        }

        llvm::Value *lhs = getInitializerValue(node->data.bin_op->left);
        llvm::Value *rhs = getInitializerValue(node->data.bin_op->right);
        if (!lhs || !rhs)
        {
            logMessage(LMI, "ERROR", "Initializer", "Left or right operand is null");
            return nullptr;
        }

        // Get the operator type
        CryoOperatorType opType = node->data.bin_op->op;
        logMessage(LMI, "INFO", "Initializer", "Binary operator: %s", CryoOperatorTypeToString(opType));
        switch (opType)
        {
        case OPERATOR_ADD:
            return context.getInstance().builder.CreateAdd(lhs, rhs, "add");
        case OPERATOR_SUB:
            return context.getInstance().builder.CreateSub(lhs, rhs, "sub");
        case OPERATOR_MUL:
            return context.getInstance().builder.CreateMul(lhs, rhs, "mul");
        case OPERATOR_DIV:
            return context.getInstance().builder.CreateSDiv(lhs, rhs, "div");
        case OPERATOR_MOD:
            return context.getInstance().builder.CreateSRem(lhs, rhs, "mod");
        case OPERATOR_AND:
            return context.getInstance().builder.CreateAnd(lhs, rhs, "and");
        case OPERATOR_OR:
            return context.getInstance().builder.CreateOr(lhs, rhs, "or");
        case OPERATOR_XOR:
            return context.getInstance().builder.CreateXor(lhs, rhs, "xor");
        case OPERATOR_LSHIFT:
            return context.getInstance().builder.CreateShl(lhs, rhs, "lshift");
        case OPERATOR_RSHIFT:
            return context.getInstance().builder.CreateLShr(lhs, rhs, "rshift");
        case OPERATOR_LT:
            return context.getInstance().builder.CreateICmpSLT(lhs, rhs, "lt");
        case OPERATOR_GT:
            return context.getInstance().builder.CreateICmpSGT(lhs, rhs, "gt");
        case OPERATOR_LTE:
            return context.getInstance().builder.CreateICmpSLE(lhs, rhs, "lte");
        case OPERATOR_GTE:
            return context.getInstance().builder.CreateICmpSGE(lhs, rhs, "gte");
        case OPERATOR_EQ:
            return context.getInstance().builder.CreateICmpEQ(lhs, rhs, "eq");
        case OPERATOR_NEQ:
            return context.getInstance().builder.CreateICmpNE(lhs, rhs, "neq");
        case OPERATOR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_ADD_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_SUB_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_MUL_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_DIV_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_MOD_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_AND_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_OR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_XOR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_INCREMENT:
            logMessage(LMI, "ERROR", "Initializer", "Increment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_DECREMENT:
            logMessage(LMI, "ERROR", "Initializer", "Decrement operator is not supported in expressions");
            return nullptr;
        case OPERATOR_NA:
            logMessage(LMI, "ERROR", "Initializer", "Operator not applicable");
            return nullptr;
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled operator type: %s", CryoOperatorTypeToString(opType));
            return nullptr;
        }
    }

    llvm::Value *Initializer::generateFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating function call...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_FUNCTION_CALL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a function call");
            return nullptr;
        }

        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = getSymbolTable()->findFunction(node->data.functionCall->name);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", node->data.functionCall->name);
            // Check the DTM to see if the function exists
            DataType *functionDataType = DTM->symbolTable->lookup(DTM->symbolTable, node->data.functionCall->name);
            if (functionDataType)
            {
                logMessage(LMI, "INFO", "Initializer", "Function %s found in DTM", node->data.functionCall->name);
            }
            else
            {
                return nullptr;
            }
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.functionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            args.push_back(getInitializerValue(node->data.functionCall->args[i]));
        }
        std::string funcName = node->data.functionCall->name;
        logMessage(LMI, "INFO", "Initializer", "Function call: %s", funcName.c_str());

        return context.getInstance().builder.CreateCall(funcSymbol->function, args, funcName);
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

    llvm::Value *Initializer::generateUnaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating unary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate unary expression
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

    llvm::Value *Initializer::generatePropertyAccess(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating property access...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_PROPERTY_ACCESS)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a property access");
            return nullptr;
        }

        // Get the object type name
        std::string objectTypeName = node->data.propertyAccess->object->data.varName->varName;
        logMessage(LMI, "INFO", "Initializer", "Object type name: %s", objectTypeName.c_str());

        // Get the property name
        std::string propertyName = node->data.propertyAccess->propertyName;
        logMessage(LMI, "INFO", "Initializer", "Property name: %s", propertyName.c_str());

        // Get the property index
        int propertyIndex = node->data.propertyAccess->propertyIndex;
        logMessage(LMI, "INFO", "Initializer", "Property index: %d", propertyIndex);

        // Get the object type
        DataType *objectType = node->data.propertyAccess->objectType;
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Object type is null");
            return nullptr;
        }

        // Get the LLVM type
        llvm::Type *llvmType = context.getInstance().symbolTable->getLLVMType(objectType);
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Visitor", "LLVM type is null");
            return nullptr;
        }
        logMessage(LMI, "INFO", "Visitor", "LLVM Type: %s", llvmType->getStructName().str().c_str());

        IRTypeSymbol *typeSymbol = context.getInstance().symbolTable->findType(objectTypeName);
        if (!typeSymbol)
        {
            logMessage(LMI, "ERROR", "Visitor", "Type symbol is null");
            return nullptr;
        }

        // Get the object instance
        llvm::Value *objectInstance = getInitializerValue(node->data.propertyAccess->object);
        if (!objectInstance)
        {
            logMessage(LMI, "ERROR", "Visitor", "Object instance is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Object Instance: %s", objectInstance->getName().str().c_str());

        llvm::Value *propertyPtr = context.getInstance().builder.CreateStructGEP(
            llvmType, objectInstance, propertyIndex, propertyName);
        if (!propertyPtr)
        {
            logMessage(LMI, "ERROR", "Visitor", "Property pointer is null");
            return nullptr;
        }
        context.getInstance().module->print(llvm::errs(), nullptr);

        logMessage(LMI, "INFO", "Visitor", "Property Pointer: %s", propertyPtr->getName().str().c_str());
        // Load the property value
        llvm::Value *propertyValue = context.getInstance().builder.CreateLoad(llvmType, propertyPtr, propertyName + ".load");
        if (!propertyValue)
        {
            logMessage(LMI, "ERROR", "Visitor", "Property value is null");
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Property Value: %s", propertyValue->getName().str().c_str());
        return propertyValue;
    }

    void Initializer::generateStructConstructor(ASTNode *node, llvm::StructType *structType)
    {
        logMessage(LMI, "INFO", "Visitor", "Generating struct constructor...");
        node->print(node);
        std::string structName = structType->getName().str();

        int cTorArgs = node->data.structConstructor->argCount;
        ASTNode **ctorArgs = node->data.structConstructor->args;
        std::vector<llvm::Type *> ctorArgTypes;

        for (int i = 0; i < cTorArgs; i++)
        {
            DataType *argType = ctorArgs[i]->data.param->type;
            logMessage(LMI, "INFO", "Visitor", "Constructor argument %d type: %s", i, argType->typeName);
            argType->debug->printType(argType);
            llvm::Type *llvmArgType = context.getInstance().symbolTable->getLLVMType(argType);
            ctorArgTypes.push_back(llvmArgType);
        }

        // Self type is the first argument
        ctorArgTypes.insert(ctorArgTypes.begin(), structType->getPointerTo());

        llvm::FunctionType *ctorFuncType = llvm::FunctionType::get(
            structType->getPointerTo(), ctorArgTypes, false);

        llvm::Function *ctorFunction = llvm::Function::Create(
            ctorFuncType, llvm::Function::ExternalLinkage, structName + ".ctor", context.getInstance().module.get());
        ctorFunction->setCallingConv(llvm::CallingConv::C);
        ctorFunction->setDoesNotThrow();

        // Add the constructor function to the symbol table
        IRFunctionSymbol ctorFuncSymbol = IRSymbolManager::createFunctionSymbol(
            ctorFunction, structName + ".ctor", structType->getPointerTo(), ctorFuncType, nullptr, false, false);
        context.getInstance().symbolTable->addFunction(ctorFuncSymbol);
        context.getInstance().symbolTable->setCurrentFunction(&ctorFuncSymbol);

        logMessage(LMI, "INFO", "Visitor", "Creating constructor function: %s", ctorFunction->getName().str().c_str());

        // Set the first argument of the function to be `self`
        llvm::Function::arg_iterator argIt = ctorFunction->arg_begin();
        llvm::Value *selfArg = argIt++;
        selfArg->setName("self");

        logMessage(LMI, "INFO", "Visitor", "Constructor function has %d arguments", ctorFunction->arg_size());

        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.getInstance().context, "entry", ctorFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);

        logMessage(LMI, "INFO", "Visitor", "Creating constructor function body...");

        std::string selfName = "self.alloc";
        llvm::AllocaInst *selfAlloc = context.getInstance().builder.CreateAlloca(structType, nullptr, selfName);
        context.getInstance().builder.CreateStore(selfArg, selfAlloc);
        AllocaType allocaType = AllocaTypeInference::inferFromNode(node, false);
        IRVariableSymbol selfSymbol = IRSymbolManager::createVariableSymbol(
            selfAlloc, structType->getPointerTo(), selfName, allocaType, Allocation());
        context.getInstance().symbolTable->addVariable(selfSymbol);

        // Now we can process the constructor arguments
        for (int i = 0; i < cTorArgs; i++)
        {
            llvm::Value *arg = argIt++;
            std::string argName = ctorArgs[i]->data.param->name;
            logMessage(LMI, "INFO", "Visitor", "Constructor argument %d name: %s", i, argName.c_str());

            // Create a new alloca for the argument
            llvm::AllocaInst *argAlloc = context.getInstance().builder.CreateAlloca(ctorArgTypes[i], nullptr, argName);
            context.getInstance().builder.CreateStore(arg, argAlloc);

            AllocaType allocaType = AllocaTypeInference::inferFromNode(ctorArgs[i], false);
            IRVariableSymbol argSymbol = IRSymbolManager::createVariableSymbol(
                argAlloc, ctorArgTypes[i], argName, allocaType, Allocation());
            context.getInstance().symbolTable->addVariable(argSymbol);
        }

        // Insert the constructor function into the module
        context.getInstance().module->getOrInsertFunction(
            llvm::StringRef(structName + ".ctor"), ctorFuncType);

        // Visit the constructor body
        ASTNode *constructorBody = node->data.structConstructor->constructorBody;
        if (constructorBody)
        {
            logMessage(LMI, "INFO", "Visitor", "Visiting constructor body...");
            context.getInstance().visitor->visit(constructorBody);
        }
        else
        {
            logMessage(LMI, "ERROR", "Visitor", "Constructor body is null");
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Constructor function body created");

        // Always create a return to `self`
        context.getInstance().builder.CreateRet(selfAlloc);
        llvm::verifyFunction(*ctorFunction);

        context.getInstance().symbolTable->exitFunctionScope();

        logMessage(LMI, "INFO", "Visitor", "Constructor function created: %s", ctorFunction->getName().str().c_str());
    }

    llvm::Value *Initializer::derefValuePointer(llvm::Value *value)
    {
        if (!value)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Value is null");
            return nullptr;
        }

        llvm::Type *type = value->getType();
        if (type->isPointerTy())
        {
            logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", value->getName().str().c_str());
            // Dereference the pointer
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(value);
            if (inst)
            {
                CodeGenDebug::printLLVMInstruction(inst);
                /*
                >>===-----------<LLVM Instruction Node>-----------===<<
                Inst:   %self.alloc = alloca %struct.Int, align 8
                Address: 0x555557f2d7b0
                >>===---------------------------------------------===<<
                */
                // If we print out something like the above, then we need to get the type of the alloca used
                // to create the pointer. This is because the alloca is not a pointer type, but a struct type.
                // So we need to get the type of the alloca and use that instead of the pointer type.
                inst->printAsOperand(llvm::errs(), false);
                llvm::Type *allocaType = nullptr;

                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(inst))
                {
                    allocaType = allocaInst->getAllocatedType();
                    CodeGenDebug::printLLVMAllocaInst(allocaInst);
                }
                else
                {
                    logMessage(LMI, "ERROR", "CodeGen", "Value is not an alloca instruction");
                    return nullptr;
                }

                CodeGenDebug::printLLVMType(allocaType);
                logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", allocaType->getStructName().str().c_str());
                return context.getInstance().builder.CreateLoad(allocaType, value, value->getName() + ".deref");
            }
        }
        return value;
    }

    llvm::Type *Initializer::derefValueForType(llvm::Value *value)
    {
        if (!value)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Value is null");
            return nullptr;
        }

        llvm::Type *type = value->getType();
        if (type->isPointerTy())
        {
            logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", value->getName().str().c_str());
            // Dereference the pointer
            llvm::Instruction *inst = llvm::dyn_cast<llvm::Instruction>(value);
            if (inst)
            {
                CodeGenDebug::printLLVMInstruction(inst);
                /*
                >>===-----------<LLVM Instruction Node>-----------===<<
                Inst:   %self.alloc = alloca %struct.Int, align 8
                Address: 0x555557f2d7b0
                >>===---------------------------------------------===<<
                */
                // If we print out something like the above, then we need to get the type of the alloca used
                // to create the pointer. This is because the alloca is not a pointer type, but a struct type.
                // So we need to get the type of the alloca and use that instead of the pointer type.
                inst->printAsOperand(llvm::errs(), false);
                llvm::Type *allocaType = nullptr;

                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(inst))
                {
                    allocaType = allocaInst->getAllocatedType();
                    CodeGenDebug::printLLVMAllocaInst(allocaInst);
                }
                else
                {
                    logMessage(LMI, "ERROR", "CodeGen", "Value is not an alloca instruction");
                    return nullptr;
                }

                CodeGenDebug::printLLVMType(allocaType);
                logMessage(LMI, "INFO", "CodeGen", "Dereferencing pointer: %s", allocaType->getStructName().str().c_str());
                return allocaType;
            }
        }
        return type;
    }

} // namespace Cryo
