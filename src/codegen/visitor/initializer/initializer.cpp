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
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I8:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(8, node->data.literal->value.intValue, true));
        case PRIM_I16:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(16, node->data.literal->value.intValue, true));
        case PRIM_I32:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(32, node->data.literal->value.intValue, true));
        case PRIM_I64:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(64, node->data.literal->value.intValue, true));
        case PRIM_I128:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(128, node->data.literal->value.intValue, true));
        case PRIM_FLOAT:
            return llvm::ConstantFP::get(context.getInstance().context, llvm::APFloat(node->data.literal->value.floatValue));
        case PRIM_BOOLEAN:
            return llvm::ConstantInt::get(context.getInstance().context, llvm::APInt(1, node->data.literal->value.booleanValue, true));
        case PRIM_STRING:
            return generateStringLiteral(node);
        case PRIM_NULL:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_VOID:
            return llvm::UndefValue::get(llvm::Type::getVoidTy(context.getInstance().context));
        case PRIM_ANY:
            return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(context.getInstance().context), 0));
        case PRIM_OBJECT:
        {
            // Get the object type name
            std::string typeName = literalDataType->typeName;
            logMessage(LMI, "INFO", "Initializer", "Creating object instance of type: %s", typeName.c_str());

            // Get the LLVM type
            llvm::Type *objectType = context.getInstance().symbolTable->getLLVMType(literalDataType);
            if (!objectType)
            {
                logMessage(LMI, "ERROR", "Initializer", "Failed to get LLVM type for %s", typeName.c_str());
                return nullptr;
            }

            // Allocate memory for the struct
            llvm::Value *objectInstance = context.getInstance().builder.CreateAlloca(objectType, nullptr, typeName + ".instance");

            // Create constructor argument based on the literal type
            llvm::Value *ctorArg = nullptr;
            std::string ctorName;

            // Handle different object types
            if (typeName == "String")
            {
                // For String objects, we pass the string literal
                if (node->data.literal->literalType == LITERAL_STRING)
                {
                    // Generate string literal as argument
                    ctorArg = generateStringLiteral(node);
                    ctorName = "struct.String.ctor";
                }
                else
                {
                    // Convert non-string literal to string
                    // (You may need to implement this conversion logic)
                    logMessage(LMI, "WARNING", "Initializer", "Non-string literal used to initialize String");
                }
            }
            else if (typeName == "Int")
            {
                // For Int objects, we pass the integer value
                if (node->data.literal->literalType == LITERAL_INT)
                {
                    ctorArg = llvm::ConstantInt::get(context.getInstance().context,
                                                     llvm::APInt(32, node->data.literal->value.intValue, true));
                    ctorName = "struct.Int.ctor";
                }
                else
                {
                    // Convert non-int literal to int
                    // (You may need to implement this conversion logic)
                    logMessage(LMI, "WARNING", "Initializer", "Non-integer literal used to initialize Int");
                }
            }
            else
            {
                logMessage(LMI, "ERROR", "Initializer", "Unsupported object type: %s", typeName.c_str());
                return objectInstance; // Return uninitialized instance as fallback
            }

            // Call the constructor if we have a valid argument and constructor name
            if (ctorArg && !ctorName.empty())
            {
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
                    context.getInstance().builder.CreateCall(ctorFunc->function, args);
                    logMessage(LMI, "INFO", "Initializer", "Called constructor %s", ctorName.c_str());
                }
                else
                {
                    logMessage(LMI, "WARNING", "Initializer", "Constructor %s not found", ctorName.c_str());
                }
            }

            return objectInstance;
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

                // Get the object type name
                std::string typeName = literalDataType->typeName;
                logMessage(LMI, "INFO", "Initializer", "Creating object instance of type: %s", typeName.c_str());

                // Get the LLVM type
                llvm::Type *objectType = context.getInstance().symbolTable->getLLVMType(literalDataType);
                if (!objectType)
                {
                    logMessage(LMI, "ERROR", "Initializer", "Failed to get LLVM type for %s", typeName.c_str());
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
                    context.getInstance().builder.CreateCall(ctorFunc->function, args);
                    logMessage(LMI, "INFO", "Initializer", "Called constructor %s", ctorName.c_str());
                }
                else
                {
                    logMessage(LMI, "WARNING", "Initializer", "Constructor %s not found", ctorName.c_str());
                }
                break;
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
            // varSymbol->allocation.load(context.getInstance().builder, varName + "_load");
        }
        return varSymbol->allocation.getValue();
    }

    llvm::Value *Initializer::generateBinaryExpr(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating binary expression...");
        ASSERT_NODE_NULLPTR_RET(node);
        // TODO: Generate binary expression
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

        logMessage(LMI, "INFO", "Visitor", "Creating constructor function: %s", ctorFunction->getName().str().c_str());

        // Set the first argument of the function to be `self`
        llvm::Function::arg_iterator argIt = ctorFunction->arg_begin();
        llvm::Value *selfArg = argIt++;
        selfArg->setName("self");

        logMessage(LMI, "INFO", "Visitor", "Constructor function has %d arguments", ctorFunction->arg_size());

        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.getInstance().context, "entry", ctorFunction);
        context.getInstance().builder.SetInsertPoint(entryBlock);

        logMessage(LMI, "INFO", "Visitor", "Creating constructor function body...");

        llvm::AllocaInst *selfAlloc = context.getInstance().builder.CreateAlloca(structType, nullptr, "self");
        context.getInstance().builder.CreateStore(selfArg, selfAlloc);
        AllocaType allocaType = AllocaTypeInference::inferFromNode(node, false);
        IRVariableSymbol selfSymbol = IRSymbolManager::createVariableSymbol(
            selfAlloc, structType->getPointerTo(), "self", allocaType, Allocation());
        context.getInstance().symbolTable->addVariable(selfSymbol);

        logMessage(LMI, "INFO", "Visitor", "Constructor function body created");

        // Insert the constructor function into the module
        context.getInstance().module->getOrInsertFunction(
            llvm::StringRef(structName + "_ctor"), ctorFuncType);

        // Always create a return to `self`
        llvm::Value *selfValue = context.getInstance().builder.CreateLoad(selfAlloc->getType(), selfAlloc, "self");
        context.getInstance().builder.CreateRet(selfValue);
        llvm::verifyFunction(*ctorFunction);

        // Add the constructor function to the symbol table
        IRFunctionSymbol ctorFuncSymbol = IRSymbolManager::createFunctionSymbol(
            ctorFunction, structName + ".ctor", structType->getPointerTo(), ctorFuncType, nullptr, false, false);
        context.getInstance().symbolTable->addFunction(ctorFuncSymbol);

        logMessage(LMI, "INFO", "Visitor", "Constructor function created: %s", ctorFunction->getName().str().c_str());
    }

} // namespace Cryo
