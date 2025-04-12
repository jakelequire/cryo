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
#include "codegen/instances.hpp"

namespace Cryo
{
    // Create an instance of a class or struct
    llvm::Value *Instance::createInstance(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);

        // Check if the node is a struct or class
        if (node->metaData->type == NODE_STRUCT_DECLARATION)
        {
            return createStructInstance(node);
        }

        // If the node is not a struct or class, return nullptr
        return nullptr;
    }

    // Create an instance of a struct
    llvm::Value *Instance::createStructInstance(ASTNode *node)
    {
        ASSERT_NODE_NULLPTR_RET(node);

        // Get struct node data
        StructNode *structNode = node->data.structNode;
        if (!structNode || !structNode->type)
        {
            logMessage(LMI, "ERROR", "Codegen", "Invalid struct node or type");
            return nullptr;
        }

        // Get the struct name
        std::string structName = structNode->name;

        // Check the symbol table if the struct type exists
        llvm::Type *type = context.symbolTable->getLLVMType(structNode->type);
        llvm::StructType *structType = llvm::dyn_cast<llvm::StructType>(type);
        if (!structType)
        {
            logMessage(LMI, "ERROR", "Codegen", "Struct type '%s' not found in symbol table", structName.c_str());
            return nullptr;
        }

        // Create a unique name for the instance
        std::string instanceName = structName + ".instance";

        // Allocate memory for the struct
        llvm::AllocaInst *structAlloca = context.builder.CreateAlloca(structType, nullptr, instanceName);

        // Zero-initialize the struct memory
        // This ensures all fields start with a clean state
        llvm::Value *size = llvm::ConstantExpr::getSizeOf(structType);
        llvm::Value *null = llvm::ConstantInt::get(context.builder.getInt8Ty(), 0);

        // Call memset to zero out the struct memory
        llvm::Function *memsetFunc = context.module->getFunction("llvm.memset.p0.i64");
        if (!memsetFunc)
        {
            // Create the memset declaration if it doesn't exist
            llvm::Type *args[] = {
                llvm::Type::getInt8Ty(context.context), // dest
                context.builder.getInt8Ty(),            // val
                context.builder.getInt64Ty(),           // len
                context.builder.getInt1Ty()             // isvolatile
            };
            llvm::FunctionType *memsetType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(context.context), args, false);
            memsetFunc = llvm::Function::Create(
                memsetType, llvm::Function::ExternalLinkage, "llvm.memset.p0.i64", context.module.get());
        }

        if (memsetFunc)
        {
            llvm::Value *args[] = {
                context.builder.CreateBitCast(structAlloca, llvm::Type::getInt8Ty(context.context)),
                null,
                context.builder.CreateZExtOrTrunc(size, context.builder.getInt64Ty()),
                llvm::ConstantInt::getFalse(context.context) // non-volatile
            };
            context.builder.CreateCall(memsetFunc, args);
        }

        // Now we have a zero-initialized struct, try to call its constructor
        return callConstructor(node, structAlloca, nullptr);
    }

    llvm::Value *Instance::callConstructor(ASTNode *structNode, llvm::Value *instance, ASTNode *constructor)
    {
        ASSERT_NODE_NULLPTR_RET(structNode);
        ASSERT_NODE_NULLPTR_RET(instance);

        // Get struct type and name
        StructNode *structDef = structNode->data.structNode;
        std::string structName = structDef->name;

        // Determine node type (struct or class)
        std::string nodeType;
        if (structNode->metaData->type == NODE_STRUCT_DECLARATION)
        {
            nodeType = "struct";
        }
        else if (structNode->metaData->type == NODE_CLASS)
        {
            nodeType = "class";
        }
        else
        {
            logMessage(LMI, "ERROR", "Codegen", "Node is neither struct nor class");
            return instance;
        }

        // Form the constructor name based on the convention: {struct|class}.{struct_name}.ctor
        std::string constructorName = nodeType + "." + structName + ".ctor";

        // Get the constructor function from the symbol table
        IRFunctionSymbol *constructorFunc = context.symbolTable->findFunction(constructorName);
        if (!constructorFunc)
        {
            logMessage(LMI, "WARNING", "Codegen", "Constructor function '%s' not found", constructorName.c_str());
            return instance; // Return instance without constructor initialization
        }

        // Prepare arguments for constructor call
        std::vector<llvm::Value *> args;

        // First argument is the 'this' pointer (the struct instance)
        // Make sure to pass it as a pointer of the correct type
        llvm::Type *paramType = constructorFunc->function->getFunctionType()->getParamType(0);
        llvm::Value *thisPtr = context.builder.CreateBitCast(instance, paramType, "this.ptr");
        args.push_back(thisPtr);

        // Add other arguments if provided
        if (constructor)
        {
            // Here we need to handle constructor arguments based on your AST structure
            // This is just a placeholder - adjust based on your actual implementation
            StructConstructorNode *structConstructor = constructor->data.structConstructor;
            for (size_t i = 0; i < structConstructor->argCount; i++)
            {
                ASTNode *arg = structConstructor->args[i];
                llvm::Value *argValue = context.initializer->getInitializerValue(arg);
                if (argValue)
                {
                    // Check if we need type conversion
                    if (i + 1 < constructorFunc->function->getFunctionType()->getNumParams())
                    {
                        llvm::Type *expectedType = constructorFunc->function->getFunctionType()->getParamType(i + 1);
                        if (argValue->getType() != expectedType)
                        {
                            // Perform appropriate type conversion
                            argValue = convertToType(argValue, expectedType);
                        }
                    }
                    args.push_back(argValue);
                }
            }
        }

        // Call the constructor
        llvm::CallInst *call = context.builder.CreateCall(constructorFunc->function, args);

        // Return the instance
        return instance;
    }

    // Helper method for type conversion
    llvm::Value *Instance::convertToType(llvm::Value *value, llvm::Type *targetType)
    {
        if (!value || !targetType)
            return value;

        llvm::Type *sourceType = value->getType();
        if (sourceType == targetType)
            return value;

        // Integer -> Integer conversion
        if (sourceType->isIntegerTy() && targetType->isIntegerTy())
        {
            unsigned srcBits = sourceType->getIntegerBitWidth();
            unsigned dstBits = targetType->getIntegerBitWidth();

            if (srcBits < dstBits)
            {
                return context.builder.CreateZExt(value, targetType, "zext");
            }
            else if (srcBits > dstBits)
            {
                return context.builder.CreateTrunc(value, targetType, "trunc");
            }
        }

        // Float -> Float conversion
        if (sourceType->isFloatingPointTy() && targetType->isFloatingPointTy())
        {
            if (sourceType->getPrimitiveSizeInBits() < targetType->getPrimitiveSizeInBits())
            {
                return context.builder.CreateFPExt(value, targetType, "fpext");
            }
            else if (sourceType->getPrimitiveSizeInBits() > targetType->getPrimitiveSizeInBits())
            {
                return context.builder.CreateFPTrunc(value, targetType, "fptrunc");
            }
        }

        // Integer -> Float conversion
        if (sourceType->isIntegerTy() && targetType->isFloatingPointTy())
        {
            // Signed integer to float
            return context.builder.CreateSIToFP(value, targetType, "sitofp");
        }

        // Float -> Integer conversion
        if (sourceType->isFloatingPointTy() && targetType->isIntegerTy())
        {
            // Float to signed integer
            return context.builder.CreateFPToSI(value, targetType, "fptosi");
        }

        // Pointer conversions
        if (sourceType->isPointerTy() && targetType->isPointerTy())
        {
            return context.builder.CreateBitCast(value, targetType, "ptrcast");
        }

        // If we can't handle the conversion, log a warning and return the original value
        logMessage(LMI, "WARNING", "Codegen", "Cannot convert between types %s and %s",
                   sourceType->getTypeID(), targetType->getTypeID());
        return value;
    }

} // namespace Cryo
