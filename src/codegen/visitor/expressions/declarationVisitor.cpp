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
    // Declarations
    void CodeGenVisitor::visitFunctionDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting function declaration...");
        assert(node);

        std::string funcName = node->data.functionDecl->name;
        logMessage(LMI, "INFO", "Visitor", "Function Name: %s", funcName.c_str());

        // Create the function prototype
        std::vector<llvm::Type *> argTypes;
        bool isVarArg = false;
        for (size_t i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            ASTNode *param = node->data.functionDecl->params[i];
            DataType *paramDataType = param->data.param->type;
            if (paramDataType->container->objectType == VA_ARGS_OBJ)
            {
                isVarArg = true;
                break;
            }
            llvm::Type *paramType = symbolTable->getLLVMType(paramDataType);
            argTypes.push_back(paramType);
        }
        logMessage(LMI, "INFO", "Visitor", "Function has %d arguments", argTypes.size());

        DataType *functionType = node->data.functionDecl->type;
        DataType *returnType = functionType->container->type.functionType->returnType;

        llvm::Type *returnTy = symbolTable->getLLVMType(returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnTy, argTypes, false);
        if (isVarArg)
        {
            funcType = llvm::FunctionType::get(returnTy, argTypes, true);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating function prototype...");
        // The function signature
        llvm::Function *function = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            funcName,
            context.getInstance().module.get());

        function->setCallingConv(llvm::CallingConv::C);

        // Create the entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.getInstance().context, "entry", function);
        context.getInstance().builder.SetInsertPoint(entryBlock);

        // Add the function to the symbol table
        IRFunctionSymbol funcSymbol = IRSymbolManager::createFunctionSymbol(
            function, funcName, returnTy, funcType, entryBlock, false, false);
        symbolTable->addFunction(funcSymbol);

        for (size_t i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            ASTNode *param = node->data.functionDecl->params[i];
            llvm::Type *paramType = symbolTable->getLLVMType(param->data.param->type);

            // Create a symbol for each parameter for the symbol table
            std::string paramName = param->data.param->name;
            logMessage(LMI, "INFO", "Visitor", "Parameter Name: %s", paramName.c_str());

            // Create the parameter in the function
            llvm::Function::arg_iterator argIt = function->arg_begin();
            llvm::Value *arg = argIt++;
            arg->setName(paramName);
            if (arg->getType()->isPointerTy())
            {
                // Store the pointer to the parameter
                llvm::AllocaInst *allocaInst = context.getInstance().builder.CreateAlloca(
                    paramType, nullptr, paramName + ".alloca");
                context.getInstance().builder.CreateStore(arg, allocaInst);
                arg = allocaInst;
            }
            AllocaType allocaType = AllocaTypeInference::inferFromNode(param, false);
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                function, nullptr, paramType, paramName, allocaType);
            paramSymbol.dataType = param->data.param->type;
            paramSymbol.value = arg;

            symbolTable->addVariable(paramSymbol);
        }

        logMessage(LMI, "INFO", "Visitor", "Function prototype created");

        // Visit the function body (This will also visit the return statement)
        logMessage(LMI, "INFO", "Visitor", "Visiting function body...");
        visit(node->data.functionDecl->body);

        // Remove the basic block
        context.getInstance().builder.ClearInsertionPoint();

        return;
    }

    void CodeGenVisitor::visitExternFuncDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting extern function declaration...");
        // For extern functions, we will just declare a prototype, but not define it.
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        std::string funcName = node->data.externFunction->name;
        logMessage(LMI, "INFO", "Visitor", "Extern Function Name: %s", funcName.c_str());

        // Create the function prototype
        std::vector<llvm::Type *> argTypes;
        bool isVarArg = false;
        for (size_t i = 0; i < node->data.externFunction->paramCount; i++)
        {
            ASTNode *param = node->data.externFunction->params[i];
            if (param->data.param->type->container->objectType == VA_ARGS_OBJ)
            {
                logMessage(LMI, "INFO", "Visitor", "Extern function has varargs");
                isVarArg = true;
                break;
            }
            llvm::Type *paramType = symbolTable->getLLVMType(param->data.param->type);
            argTypes.push_back(paramType);
        }
        logMessage(LMI, "INFO", "Visitor", "Extern function has %d arguments", argTypes.size());

        DataType *functionType = node->data.externFunction->type;
        functionType->debug->printType(functionType);
        DataType *returnType = functionType->container->type.functionType->returnType;

        llvm::Type *returnTy = symbolTable->getLLVMType(returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnTy, argTypes, false);
        if (isVarArg)
        {
            funcType = llvm::FunctionType::get(returnTy, argTypes, true);
        }

        logMessage(LMI, "INFO", "Visitor", "Creating extern function prototype...");

        // The function signature
        llvm::Function *function = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            funcName,
            context.getInstance().module.get());
        function->setCallingConv(llvm::CallingConv::C);
        function->setDoesNotThrow();

        // Add the function to the symbol table
        IRFunctionSymbol funcSymbol = IRSymbolManager::createFunctionSymbol(
            function, funcName, returnTy, funcType, nullptr, false, true);
        symbolTable->addExternFunction(funcSymbol);
        logMessage(LMI, "INFO", "Visitor", "Extern function prototype created");

        return;
    }

    void CodeGenVisitor::visitStructDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting struct declaration...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        std::string structName = node->data.structNode->name;
        std::string fullStructName = "struct." + structName;
        logMessage(LMI, "INFO", "Visitor", "Struct Name: %s", structName.c_str());

        // Create initial empty struct type as prototype
        llvm::StructType *structType = llvm::StructType::create(
            context.getInstance().context,
            fullStructName);

        // Add prototype to symbol table to allow recursive references
        IRTypeSymbol protoType = IRSymbolManager::createProtoTypeSymbol(
            structType,
            fullStructName,
            true);
        context.getInstance().symbolTable->addType(protoType);

        int propertyCount = node->data.structNode->propertyCount;
        int methodCount = node->data.structNode->methodCount;
        logMessage(LMI, "INFO", "Visitor", "Struct has %d properties, %d methods",
                   propertyCount, methodCount);

        // Process properties
        std::vector<llvm::Type *> propertyTypes;
        std::vector<IRPropertySymbol> propertySymbols;

        logMessage(LMI, "INFO", "Visitor", "Processing property symbols...");
        for (int i = 0; i < propertyCount; i++)
        {
            ASTNode *property = node->data.structNode->properties[i];
            DataType *propertyType = property->data.property->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            std::string propertyName = property->data.property->name;

            if (!llvmType)
            {
                logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for property %s",
                           propertyName.c_str());
                return;
            }

            propertyTypes.push_back(llvmType);
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(
                llvmType,
                propertyName,
                i,
                true);
            propertySymbols.push_back(propertySymbol);
        }

        // Set the struct body now that we have all property types
        structType->setBody(propertyTypes);

        // Process methods
        std::vector<IRMethodSymbol> methodSymbols;
        logMessage(LMI, "INFO", "Visitor", "Processing method symbols...");
        for (int i = 0; i < methodCount; i++)
        {
            ASTNode *method = node->data.structNode->methods[i];
            DataType *methodType = method->data.method->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(methodType);
            std::string methodName = method->data.method->name;

            if (!llvmType)
            {
                logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for method %s",
                           methodName.c_str());
                return;
            }

            // Create function symbol for the method
            IRFunctionSymbol functionSymbol = IRSymbolManager::createFunctionSymbol(
                nullptr,
                methodName,
                llvmType,
                nullptr,
                nullptr,
                false,
                false);

            // Create method symbol
            IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
                functionSymbol,
                false,  // isVirtual
                false,  // isAbstract
                false,  // isOverride
                false,  // isStatic
                0,      // vtableIndex
                nullptr // parentClass
            );

            methodSymbols.push_back(methodSymbol);
        }

        // Remove prototype and add complete type
        context.getInstance().symbolTable->removeType(fullStructName);

        // Create complete type symbol with properties and methods
        IRTypeSymbol typeSymbol = IRSymbolManager::createTypeSymbol(
            structType,
            fullStructName,
            propertySymbols,
            methodSymbols);
        context.getInstance().symbolTable->addType(typeSymbol);

        // Register struct globally
        context.getInstance().module->getOrInsertGlobal(structName, structType);

        // Handle constructor if present
        if (node->data.structNode->constructor)
        {
            logMessage(LMI, "INFO", "Visitor", "Generating constructor for struct %s",
                       structName.c_str());
            context.getInstance().symbolTable->enterConstructorInstance();
            context.getInstance().initializer->generateStructConstructor(
                node->data.structNode->constructor,
                structType);
            context.getInstance().symbolTable->exitConstructorInstance();
        }
        else
        {
            logMessage(LMI, "INFO", "Visitor", "No constructor for struct %s",
                       structName.c_str());
            DEBUG_BREAKPOINT;
        }

        logMessage(LMI, "INFO", "Visitor", "Struct declaration complete for %s",
                   structName.c_str());
    }

    void CodeGenVisitor::visitEnumDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting enum declaration...");
        DEBUG_BREAKPOINT;
        return;
    }

    void CodeGenVisitor::visitGenericDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting generic declaration...");
        DEBUG_BREAKPOINT;
        return;
    }

    void CodeGenVisitor::visitTypeDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting type declaration...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }
        if (node->metaData->type != NODE_TYPE)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not a type declaration");
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Processing type declaration...");

        std::string typeName = node->data.typeDecl->name;
        logMessage(LMI, "INFO", "Visitor", "Type Name: %s", typeName.c_str());

        DataType *type = node->data.typeDecl->type;
        TypeofDataType typeOf = type->container->typeOf;
        switch (typeOf)
        {
        case PRIM_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is a primitive type");
            break;
        case ARRAY_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is an array type");
            break;
        case ENUM_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is an enum type");
            break;
        case FUNCTION_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is a function type");
            break;
        case GENERIC_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is a generic type");
            break;
        case OBJECT_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is an object type");
            break;
        case TYPE_DEF:
            logMessage(LMI, "INFO", "Visitor", "Type is a type definition");
            break;
        case UNKNOWN_TYPE:
            logMessage(LMI, "INFO", "Visitor", "Type is an unknown type");
            break;
        default:
            logMessage(LMI, "ERROR", "Visitor", "Unknown type of data type");
            break;
        }

        logMessage(LMI, "INFO", "Visitor", "Type declaration complete");

        return;
    }

    llvm::Value *CodeGenVisitor::convertValueToTargetType(llvm::Value *value, DataType *sourceType, DataType *targetType)
    {
        if (!value || !sourceType || !targetType)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Null input to type conversion");
            return value;
        }

        logMessage(LMI, "INFO", "CodeGenVisitor", "Converting value from %s to %s",
                   sourceType->typeName, targetType->typeName);

        // Get LLVM types
        llvm::Type *sourceLLVMType = symbolTable->getLLVMType(sourceType);
        llvm::Type *targetLLVMType = symbolTable->getLLVMType(targetType);

        if (!sourceLLVMType || !targetLLVMType)
        {
            logMessage(LMI, "ERROR", "CodeGenVisitor", "Failed to get LLVM types for conversion");
            return value;
        }

        // Case 1: String to str conversion (extract pointer from struct)
        if (strcmp(sourceType->typeName, "String") == 0 &&
            (strcmp(targetType->typeName, "str") == 0 || targetType->container->primitive == PRIM_STR))
        {

            // Extract the char* field from the String struct
            if (value->getType()->isPointerTy())
            {
                // If we have a pointer to a String struct
                llvm::StructType *stringType = llvm::StructType::getTypeByName(
                    context.getInstance().context, "struct.String");

                llvm::Value *ptrField = builder.CreateStructGEP(
                    stringType, value, 0, "string.ptr");

                return builder.CreateLoad(builder.getInt8Ty(), ptrField, "str.extract");
            }
            else if (value->getType()->isStructTy())
            {
                // If we have a String struct by value, create a temporary and extract
                llvm::AllocaInst *tempAlloca = builder.CreateAlloca(
                    value->getType(), nullptr, "string.temp");
                builder.CreateStore(value, tempAlloca);

                llvm::Value *ptrField = builder.CreateStructGEP(
                    value->getType(), tempAlloca, 0, "string.ptr");

                return builder.CreateLoad(builder.getInt8Ty(), ptrField, "str.extract");
            }
        }

        // Case 2: str to String conversion (wrap pointer in struct)
        if ((strcmp(sourceType->typeName, "str") == 0 || sourceType->container->primitive == PRIM_STR) &&
            strcmp(targetType->typeName, "String") == 0)
        {

            // Create a String struct to wrap the char* pointer
            llvm::StructType *stringType = llvm::StructType::getTypeByName(
                context.getInstance().context, "struct.String");

            llvm::AllocaInst *stringAlloca = builder.CreateAlloca(
                stringType, nullptr, "string.temp");

            llvm::Value *ptrField = builder.CreateStructGEP(
                stringType, stringAlloca, 0, "string.ptr");

            // If value is already a pointer, store it directly
            if (value->getType()->isPointerTy())
            {
                if (value->getType() == builder.getInt8Ty())
                {
                    builder.CreateStore(value, ptrField);
                }
                else
                {
                    // Load the pointer value if needed
                    llvm::Value *ptrValue = builder.CreateLoad(
                        builder.getInt8Ty(), value, "ptr.load");
                    builder.CreateStore(ptrValue, ptrField);
                }
            }

            // Return the String struct by value or pointer based on need
            if (targetLLVMType->isPointerTy())
            {
                return stringAlloca; // Return pointer to the struct
            }
            else
            {
                return builder.CreateLoad(stringType, stringAlloca, "string.load"); // Return struct by value
            }
        }

        // Case 3: Pointer to value conversion (general case)
        if (value->getType()->isPointerTy() && !targetLLVMType->isPointerTy())
        {
            // Determine the type to load
            llvm::Type *loadType = nullptr;
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(value))
            {
                loadType = allocaInst->getAllocatedType();
            }
            else if (sourceLLVMType->isPointerTy())
            {
                loadType = sourceLLVMType;
            }
            else
            {
                loadType = targetLLVMType;
            }

            return builder.CreateLoad(loadType, value, "value.load");
        }

        // Case 4: Value to pointer conversion (allocation)
        if (!value->getType()->isPointerTy() && targetLLVMType->isPointerTy())
        {
            llvm::AllocaInst *tempAlloca = builder.CreateAlloca(
                value->getType(), nullptr, "value.temp");
            builder.CreateStore(value, tempAlloca);
            return tempAlloca;
        }

        // Case 5: Standard primitive type conversions
        if (value->getType() != targetLLVMType)
        {
            // Integer to integer conversion
            if (value->getType()->isIntegerTy() && targetLLVMType->isIntegerTy())
            {
                unsigned srcBits = value->getType()->getIntegerBitWidth();
                unsigned dstBits = targetLLVMType->getIntegerBitWidth();

                if (srcBits < dstBits)
                    return builder.CreateZExt(value, targetLLVMType, "int.extend");
                else if (srcBits > dstBits)
                    return builder.CreateTrunc(value, targetLLVMType, "int.truncate");
            }

            // Float to float conversion
            if (value->getType()->isFloatingPointTy() && targetLLVMType->isFloatingPointTy())
            {
                if (value->getType()->getPrimitiveSizeInBits() < targetLLVMType->getPrimitiveSizeInBits())
                    return builder.CreateFPExt(value, targetLLVMType, "float.extend");
                else if (value->getType()->getPrimitiveSizeInBits() > targetLLVMType->getPrimitiveSizeInBits())
                    return builder.CreateFPTrunc(value, targetLLVMType, "float.truncate");
            }

            // Integer to float
            if (value->getType()->isIntegerTy() && targetLLVMType->isFloatingPointTy())
                return builder.CreateSIToFP(value, targetLLVMType, "int.to.float");

            // Float to integer
            if (value->getType()->isFloatingPointTy() && targetLLVMType->isIntegerTy())
                return builder.CreateFPToSI(value, targetLLVMType, "float.to.int");

            // Pointer conversions
            if (value->getType()->isPointerTy() && targetLLVMType->isPointerTy())
                return builder.CreateBitCast(value, targetLLVMType, "ptr.cast");
        }

        // If no conversion needed or possible, return original value
        return value;
    }

    void CodeGenVisitor::visitScopedFunctionCall(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting scoped function call...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
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
            return;
        }

        // Process arguments
        std::vector<llvm::Value *> args;
        int argCount = node->data.scopedFunctionCall->argCount;
        for (int i = 0; i < argCount; i++)
        {
            args.push_back(context.getInstance().initializer->getInitializerValue(node->data.scopedFunctionCall->args[i]));
        }

        logMessage(LMI, "INFO", "Initializer", "Function call: %s", fullFunctionName.c_str());
        llvm::Function *function = funcSymbol->function;
        if (!function)
        {
            logMessage(LMI, "ERROR", "Initializer", "Function %s not found", fullFunctionName.c_str());
            CONDITION_FAILED;
            return;
        }

        // Create the function call
        context.getInstance().builder.CreateCall(function, args);

        return;
    }

    void CodeGenVisitor::visitBreakStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting break statement...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        // Break statement logic
        llvm::BasicBlock *breakBlock = context.getInstance().symbolTable->getBreakBlock();
        if (breakBlock)
        {
            context.getInstance().builder.CreateBr(breakBlock);
        }
        else
        {
            logMessage(LMI, "ERROR", "Visitor", "No break block found");
            CONDITION_FAILED;
        }

        return;
    }

    void CodeGenVisitor::visitContinueStatement(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting continue statement...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }
        // Continue statement logic
        llvm::BasicBlock *continueBlock = context.getInstance().symbolTable->getContinueBlock();
        if (continueBlock)
        {
            context.getInstance().builder.CreateBr(continueBlock);
            context.getInstance().builder.SetInsertPoint(continueBlock);
        }
        else
        {
            logMessage(LMI, "ERROR", "Visitor", "No continue block found");
            CONDITION_FAILED;
        }
        return;
    }

} // namespace Cryo
