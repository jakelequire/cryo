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
    void Structs::handleStructDeclaration(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Handling Struct Declaration");
        DevDebugger::logNode(node);

        StructNode *structNode = node->data.structNode;
        if (!structNode)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Structs", "StructNode is null");
            CONDITION_FAILED;
        }

        std::string structName = structNode->name;
        if (structName.empty())
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Structs", "Struct name is empty");
            CONDITION_FAILED;
        }
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Struct name: " + structName);

        // Create struct type with fields
        std::vector<llvm::Type *> structFields;
        for (int i = 0; i < structNode->propertyCount; ++i)
        {
            PropertyNode *property = structNode->properties[i]->data.property;
            llvm::Type *fieldType = getStructFieldType(property);
            structFields.push_back(fieldType);
        }

        // Create the struct type
        llvm::StructType *structType = llvm::StructType::create(
            compiler.getContext().context,
            structFields,
            "struct." + structName);

        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Struct type created");
        DevDebugger::logLLVMStruct(structType);

        // Add struct type to the symbol table first
        compiler.getSymTable().addStruct(structName, structType, structNode);

        // Handle constructor if present
        if (structNode->constructor)
        {
            handleConstructor(structNode, structType);
        }

        // Handle methods
        for (int i = 0; i < structNode->methodCount; ++i)
        {
            handleMethod(structNode->methods[i], structType, structName);
        }
    }

    void Structs::handleConstructor(StructNode *node, llvm::StructType *structType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Handling Constructor");

        ASTNode *constructor = node->constructor;
        if (!constructor)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Structs", "Constructor node is null");
            CONDITION_FAILED;
        }

        StructConstructorNode *constructorNode = constructor->data.structConstructor;
        std::string structName = constructorNode->name;
        std::string constructorName = structName + "::Constructor";

        // Create parameter types for constructor
        std::vector<llvm::Type *> paramTypes;
        // First parameter is always the struct pointer (this)
        paramTypes.push_back(structType->getPointerTo());

        // Add parameter types from constructor arguments
        for (int i = 0; i < constructorNode->argCount; ++i)
        {
            CryoParameterNode *param = constructorNode->args[i]->data.param;
            llvm::Type *paramType = compiler.getTypes().getType(param->type, 0);
            paramTypes.push_back(paramType);
        }

        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Creating constructor function");

        // Create constructor function type
        llvm::FunctionType *constructorFnType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(compiler.getContext().context),
            paramTypes,
            false);

        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Constructor function type created");

        // Create the constructor function
        llvm::Function *constructorFn = llvm::Function::Create(
            constructorFnType,
            llvm::Function::ExternalLinkage,
            constructorName);

        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Constructor function created");

        // Add function attributes
        constructorFn->addFnAttr(llvm::Attribute::NoUnwind);
        constructorFn->addFnAttr(llvm::Attribute::UWTable);

        // Create entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            constructorFn);
        compiler.getContext().builder.SetInsertPoint(entryBlock);

        // Get constructor parameters
        auto argIt = constructorFn->arg_begin();
        llvm::Value *thisPtr = argIt++; // First argument is 'this' pointer

        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Creating constructor body");

        DevDebugger::logLLVMStruct(structType);

        // Store constructor parameters in struct fields
        for (int i = 0; i < constructorNode->argCount; ++i)
        {
            llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
                structType,
                thisPtr,
                i,
                "field" + std::to_string(i));

            llvm::Value *argValue = argIt++;
            compiler.getContext().builder.CreateStore(argValue, fieldPtr);
        }

        // Create return void
        compiler.getContext().builder.CreateRetVoid();

        // Add to symbol table
        compiler.getSymTable().updateStructConstructor(
            compiler.getContext().currentNamespace,
            structName,
            constructorFn);
    }

    void Structs::handleMethod(ASTNode *methodNode, llvm::StructType *structType,
                               const std::string &structName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Handling Method");

        MethodNode *method = methodNode->data.method;
        std::string methodName = structName + "::" + method->name;

        // Create parameter types for method
        std::vector<llvm::Type *> paramTypes;
        paramTypes.push_back(structType->getPointerTo()); // 'this' pointer

        for (int i = 0; i < method->paramCount; ++i)
        {
            CryoParameterNode *param = method->params[i]->data.param;
            paramTypes.push_back(compiler.getTypes().getType(param->type, 0));
        }

        // Get return type
        llvm::Type *returnType = compiler.getTypes().getType(method->type, 0);

        // Create method function type
        llvm::FunctionType *methodType = llvm::FunctionType::get(
            returnType,
            paramTypes,
            false);

        // Create the method function
        llvm::Function *methodFn = llvm::Function::Create(
            methodType,
            llvm::Function::ExternalLinkage,
            methodName);

        // Add function attributes
        methodFn->addFnAttr(llvm::Attribute::NoUnwind);
        methodFn->addFnAttr(llvm::Attribute::UWTable);

        // Create entry block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            methodFn);
        compiler.getContext().builder.SetInsertPoint(entry);

        // Implementation of method body would go here
        // You'll need to implement this based on your AST node structure

        // Add return statement
        if (returnType->isVoidTy())
        {
            compiler.getContext().builder.CreateRetVoid();
        }
        else
        {
            // For non-void methods, you'll need to implement the proper return
            // based on your method body implementation
            llvm::Value *returnValue = llvm::UndefValue::get(returnType);
            compiler.getContext().builder.CreateRet(returnValue);
        }
    }

    llvm::Type *Structs::getStructFieldType(PropertyNode *property)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Getting Struct Field Type");

        if (!property)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Structs", "Property is null");
            CONDITION_FAILED;
        }

        DataType *dataType = property->type;
        std::string dataTypeStr = DataTypeToString(dataType);
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Data Type: " + dataTypeStr);

        std::string propertyName = property->name;
        if (propertyName.empty())
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Structs", "Property name is empty");
            CONDITION_FAILED;
        }

        return compiler.getTypes().getType(dataType, 0);
    }
};
