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
        default:
            logMessage(LMI, "ERROR", "Initializer", "Unhandled node type: %s", nodeTypeStr.c_str());
            return nullptr;
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

    llvm::Value *Initializer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_BINARY_EXPR)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a binary expression");
            return nullptr;
        }

        CryoOperatorType opType = node->data.bin_op->op;

        // First generate the left and right operands
        llvm::Value *lhs = getInitializerValue(node->data.bin_op->left);
        llvm::Value *rhs = getInitializerValue(node->data.bin_op->right);
        if (!lhs || !rhs)
        {
            logMessage(LMI, "ERROR", "Initializer", "Left or right operand is null");
            return nullptr;
        }

        // Special handling for pointer null comparisons
        if ((opType == OPERATOR_EQ || opType == OPERATOR_NEQ) &&
            (lhs->getType()->isPointerTy() || rhs->getType()->isPointerTy()))
        {
            DataType *leftType = DTM->astInterface->getTypeofASTNode(node->data.bin_op->left);
            DataType *rightType = DTM->astInterface->getTypeofASTNode(node->data.bin_op->right);

            bool isNullComparison = false;
            bool isLeftNull = false;
            bool isRightNull = false;

            // Check if one side is null
            if (rightType && (rightType->container->primitive == PRIM_NULL ||
                              strcmp(rightType->typeName, "null") == 0))
            {
                isNullComparison = true;
                isRightNull = true;
            }
            else if (leftType && (leftType->container->primitive == PRIM_NULL ||
                                  strcmp(leftType->typeName, "null") == 0))
            {
                isNullComparison = true;
                isLeftNull = true;
            }

            if (isNullComparison)
            {
                // Determine which is the pointer
                llvm::Value *ptrValue = isLeftNull ? rhs : lhs;

                if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptrValue))
                {
                    logMessage(LMI, "INFO", "Initializer", "Loading pointer value for null comparison");
                    ptrValue = context.getInstance().builder.CreateLoad(
                        allocaInst->getAllocatedType(),
                        ptrValue,
                        "ptr_value");
                }

                // Create null pointer constant of the appropriate type
                llvm::PointerType *ptrType = nullptr;
                if (ptrValue->getType()->isPointerTy())
                {
                    ptrType = llvm::cast<llvm::PointerType>(ptrValue->getType());
                }
                else
                {
                    // Fallback to generic pointer
                    ptrType = llvm::PointerType::get(context.getInstance().context, 0);
                }

                llvm::Value *nullPtr = llvm::ConstantPointerNull::get(ptrType);

                // Create the comparison
                if (opType == OPERATOR_EQ)
                {
                    return context.getInstance().builder.CreateICmpEQ(ptrValue, nullPtr, "ptr_eq_null");
                }
                else
                {
                    return context.getInstance().builder.CreateICmpNE(ptrValue, nullPtr, "ptr_ne_null");
                }
            }
        }

        // Handle standard operators
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
        case OPERATOR_SUB_ASSIGN:
        case OPERATOR_MUL_ASSIGN:
        case OPERATOR_DIV_ASSIGN:
        case OPERATOR_MOD_ASSIGN:
        case OPERATOR_AND_ASSIGN:
        case OPERATOR_OR_ASSIGN:
        case OPERATOR_XOR_ASSIGN:
            logMessage(LMI, "ERROR", "Initializer", "Assignment operator is not supported in expressions");
            return nullptr;
        case OPERATOR_INCREMENT:
        case OPERATOR_DECREMENT:
            logMessage(LMI, "ERROR", "Initializer", "Increment/decrement operator is not supported in expressions");
            return nullptr;
        case OPERATOR_NA:
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

    IRMethodSymbol *Initializer::createClassMethod(const std::string &className, ASTNode *method, DataType *methodDataType)
    {
        ASSERT_NODE_NULLPTR_RET(method);
        if (method->metaData->type != NODE_METHOD)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Node is not a method");
            CONDITION_FAILED;
            return nullptr;
        }
        logMessage(LMI, "INFO", "Visitor", "Creating method: %s", method->data.method->name);
        // Create the function prototype
        std::vector<llvm::Type *> argTypes;
        bool isVarArg = false;
        for (size_t i = 0; i < method->data.method->paramCount; i++)
        {
            logMessage(LMI, "INFO", "Visitor", "Creating method parameter %d: %s", i, method->data.method->params[i]->data.param->name);
            ASTNode *param = method->data.method->params[i];
            DataType *paramDataType = param->data.param->type;
            if (paramDataType->container->objectType == VA_ARGS_OBJ)
            {
                isVarArg = true;
                break;
            }
            llvm::Type *paramType = context.getInstance().symbolTable->getLLVMType(paramDataType);
            argTypes.push_back(paramType);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating method function prototype...");
        DataType *functionDataType = method->data.method->type;
        if (!functionDataType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Function data type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Function data type: %s", functionDataType->typeName);
        DataType *returnType = functionDataType->container->type.functionType->returnType;
        if (!returnType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Return type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Visitor", "Method return type: %s", returnType->typeName);
        llvm::Type *methodReturnType = context.getInstance().symbolTable->getLLVMType(returnType);
        if (!methodReturnType)
        {
            logMessage(LMI, "ERROR", "CodeGen", "Method return type is null");
            CONDITION_FAILED;
            return nullptr;
        }

        llvm::FunctionType *methodFuncType = llvm::FunctionType::get(
            methodReturnType, argTypes, isVarArg);
        if (isVarArg)
        {
            methodFuncType = llvm::FunctionType::get(
                methodReturnType, argTypes, true);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating method function prototype...");
        // The function signature
        llvm::Function *methodFunction = llvm::Function::Create(
            methodFuncType,
            llvm::Function::ExternalLinkage,
            className + "." + method->data.method->name,
            context.getInstance().module.get());

        methodFunction->setCallingConv(llvm::CallingConv::C);
        methodFunction->setDoesNotThrow();
        methodFunction->setName(className + "." + method->data.method->name);

        logMessage(LMI, "INFO", "Visitor", "Method function prototype created: %s", methodFunction->getName().str().c_str());
        // Add the method to the symbol table
        IRFunctionSymbol methodFnSymbol = IRSymbolManager::createFunctionSymbol(
            methodFunction,
            className + "." + method->data.method->name,
            methodReturnType,
            methodFuncType,
            nullptr, false, false);

        context.getInstance().symbolTable->addFunction(methodFnSymbol);
        logMessage(LMI, "INFO", "Visitor", "Adding method to symbol table: %s", methodFunction->getName().str().c_str());
        IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
            methodFnSymbol,
            false, false, false, false, 0, nullptr);

        logMessage(LMI, "INFO", "Visitor", "Setting method params...");
        for (size_t i = 0; i < method->data.method->paramCount; i++)
        {
            ASTNode *param = method->data.method->params[i];
            llvm::Type *paramType = context.getInstance().symbolTable->getLLVMType(param->data.param->type);

            // Create a symbol for each parameter for the symbol table
            std::string paramName = param->data.param->name;
            logMessage(LMI, "INFO", "Visitor", "Parameter Name: %s", paramName.c_str());
            AllocaType allocaType = AllocaTypeInference::inferFromNode(param, false);
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                methodFunction, nullptr, paramType, paramName, allocaType);

            // Create the parameter in the function
            llvm::Function::arg_iterator argIt = methodFunction->arg_begin();
            llvm::Value *arg = argIt++;
            arg->setName(paramName);
            paramSymbol.value = arg;
            context.getInstance().symbolTable->addVariable(paramSymbol);
        }

        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.getInstance().context, "entry", methodFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);
        logMessage(LMI, "INFO", "Visitor", "Creating method function: %s", methodFunction->getName().str().c_str());
        context.getInstance().symbolTable->addFunction(methodFnSymbol);

        context.getInstance().visitor->visit(method->data.method->body);

        // Clear the current function
        context.getInstance().builder.ClearInsertionPoint();

        logMessage(LMI, "INFO", "Visitor", "Method function created: %s", methodFunction->getName().str().c_str());
        return &(methodSymbol);
    }

    llvm::Value *Initializer::generateScopedFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating scoped function call...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_SCOPED_FUNCTION_CALL)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a scoped function call");
            return nullptr;
        }

        std::string scopeName = node->data.scopedFunctionCall->scopeName;
        std::string functionName = node->data.scopedFunctionCall->functionName;
        std::string fullFunctionName = scopeName + "." + functionName;

        logMessage(LMI, "INFO", "Initializer", "Scoped function call: %s", fullFunctionName.c_str());
        // Find the function in the symbol table
        IRFunctionSymbol *funcSymbol = context.getInstance().symbolTable->findFunction(fullFunctionName);
        if (!funcSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", fullFunctionName.c_str());
            CONDITION_FAILED;
            return nullptr;
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.scopedFunctionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            args.push_back(getInitializerValue(node->data.scopedFunctionCall->args[i]));
        }

        logMessage(LMI, "INFO", "Initializer", "Function call: %s", fullFunctionName.c_str());
        llvm::Function *function = funcSymbol->function;
        if (!function)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", fullFunctionName.c_str());
            CONDITION_FAILED;
            return nullptr;
        }

        // Create the function call
        llvm::Value *call = context.getInstance().builder.CreateCall(function, args, fullFunctionName);
        if (!call)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function call failed");
            CONDITION_FAILED;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Initializer", "Function call created: %s", call->getName().str().c_str());
        return call;
    }

    llvm::Value *Initializer::generateVarDeclaration(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating variable declaration...");
        DEBUG_BREAKPOINT;
    }

    llvm::Value *Initializer::generateObjectInst(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating object instance...");
        ASSERT_NODE_NULLPTR_RET(node);

        if (node->metaData->type != NODE_OBJECT_INST)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not an object instance");
            return nullptr;
        }

        // Get the object type name
        std::string objectTypeName = node->data.objectNode->name;
        logMessage(LMI, "INFO", "Initializer", "Object type name: %s", objectTypeName.c_str());
        bool isNewObject = node->data.objectNode->isNewInstance;
        // Get the object type
        DataType *objectType = node->data.objectNode->objType;
        if (!objectType)
        {
            logMessage(LMI, "ERROR", "Visitor", "Object type is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        // Get the LLVM type
        IRTypeSymbol *irTypeSymbol = context.getInstance().symbolTable->findType("struct." + objectTypeName);
        if (!irTypeSymbol)
        {
            logMessage(LMI, "ERROR", "Visitor", "IR type symbol is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }
        if (isNewObject)
        {
            logMessage(LMI, "INFO", "Visitor", "Creating new object instance: %s", objectTypeName.c_str());
            llvm::StructType *llvmStructType = llvm::dyn_cast<llvm::StructType>(irTypeSymbol->getType());
            if (!llvmStructType)
            {
                logMessage(LMI, "ERROR", "Visitor", "LLVM struct type is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }
            logMessage(LMI, "INFO", "Visitor", "LLVM Struct Type: %s", llvmStructType->getName().str().c_str());

            IRFunctionSymbol *ctorFuncSymbol = context.getInstance().symbolTable->findFunction("struct." + objectTypeName + ".ctor");
            if (!ctorFuncSymbol)
            {
                logMessage(LMI, "ERROR", "Visitor", "Constructor function symbol is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }

            // Prepare the arguments for the constructor
            std::vector<llvm::Value *> ctorArgs;
            int argCount = node->data.objectNode->argCount;
            for (int i = 0; i < argCount; i++)
            {
                ctorArgs.push_back(getInitializerValue(node->data.objectNode->args[i]));
            }

            // Create the object instance
            llvm::Value *objectInstance = context.getInstance().builder.CreateCall(
                ctorFuncSymbol->function, ctorArgs, objectTypeName + ".ctor");
            if (!objectInstance)
            {
                logMessage(LMI, "ERROR", "Visitor", "Object instance is null");
                DEBUG_BREAKPOINT;
                return nullptr;
            }

            return objectInstance;
        }

        DEBUG_BREAKPOINT;

        return nullptr;
    }

    llvm::Value *Initializer::generateNullLiteral()
    {
        logMessage(LMI, "INFO", "Initializer", "Generating null literal...");
        llvm::Type *nullType = context.getInstance().builder.getInt8Ty();
        llvm::Value *nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(nullType, 0));
        return nullValue;
    }

} // namespace Cryo
