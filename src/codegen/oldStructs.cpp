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

        StructNode *structNode = node->data.structNode;
        std::string structName = structNode->name;
        DataType *structDataType = structNode->type;
        logVerboseDataType(structDataType);

        compiler.getContext().addStructDataType(structName, structDataType);

        // Create struct type with fields
        std::vector<llvm::Type *> structFields;
        for (int i = 0; i < structNode->propertyCount; ++i)
        {
            PropertyNode *property = structNode->properties[i]->data.property;
            llvm::Type *fieldType = getStructFieldType(property);
            structFields.push_back(fieldType);
        }

        // Create the struct type and register it
        llvm::StructType *structType = llvm::StructType::create(
            compiler.getContext().context,
            structFields,
            structName);

        // Add struct type to the symbol table and context
        compiler.getSymTable().addStruct(structName, structType, structNode, structDataType);
        compiler.getContext().addStructToInstance(structName, structType);

        // Add to the `NamedGlobal` map
        compiler.getContext().module->getOrInsertGlobal(structName, structType);

        if (structNode->constructor)
        {
            handleStructConstructor(structNode, structType);
        }

        for (int i = 0; i < structNode->methodCount; ++i)
        {
            ASTNode *methodNode = structNode->methods[i];
            handleMethod(methodNode, structName, structType);
        }
    }

    void Structs::handleStructConstructor(StructNode *node, llvm::StructType *structType)
    {
        ASTNode *constructor = node->constructor;
        std::string structName = std::string(node->name);

        // Create constructor function type
        std::vector<llvm::Type *> paramTypes;
        paramTypes.push_back(structType->getPointerTo()); // 'this' pointer

        // Add constructor parameters
        for (int i = 0; i < constructor->data.structConstructor->argCount; ++i)
        {
            CryoParameterNode *param = constructor->data.structConstructor->args[i]->data.param;
            paramTypes.push_back(compiler.getTypes().getType(param->type, 0));
        }

        // Create constructor function
        llvm::FunctionType *ctorType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(compiler.getContext().context),
            paramTypes,
            false);

        llvm::Function *ctorFunc = llvm::Function::Create(
            ctorType,
            llvm::Function::ExternalLinkage,
            structName + ".constructor",
            *compiler.getContext().module);

        // Create entry block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            ctorFunc);
        compiler.getContext().builder.SetInsertPoint(entry);

        // Initialize fields
        auto argIt = ctorFunc->arg_begin();
        llvm::Value *thisPtr = argIt++; // First argument is 'this' pointer

        for (int i = 0; i < node->propertyCount; ++i)
        {
            PropertyNode *prop = node->properties[i]->data.property;
            llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
                structType,
                thisPtr,
                i,
                "field" + std::to_string(i));
            llvm::Value *argValue = argIt++;
            compiler.getContext().builder.CreateStore(argValue, fieldPtr);
        }

        compiler.getContext().builder.CreateRetVoid();
    }

    void Structs::handleMethod(ASTNode *methodNode, const std::string &structName, llvm::StructType *structType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Structs", "Handling Method");

        MethodNode *method = methodNode->data.method;
        std::string methodName = structName + "." + method->name;

        // Create parameter types for method
        std::vector<llvm::Type *> paramTypes;

        // Add the struct pointer as the first parameter (this pointer)
        paramTypes.push_back(structType->getPointerTo());

        for (int i = 0; i < method->paramCount; ++i)
        {
            CryoParameterNode *param = method->params[i]->data.param;
            paramTypes.push_back(compiler.getTypes().getType(param->type, 0));
        }

        // Get return type
        llvm::Type *returnType = compiler.getTypes().getType(method->type, 0);
        if (method->type->container->baseType == PRIMITIVE_TYPE && method->type->container->primitive == PRIM_STRING)
        {
            returnType = returnType->getPointerTo();
        }

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

        // Add struct instance as a parameter in the method function
        // Set the name of the struct parameter
        auto argIt = methodFn->arg_begin();

        // Add it to the module and set it as the current function
        compiler.getContext().module->getFunctionList().push_back(methodFn);
        compiler.getContext().currentFunction = methodFn;

        // Create entry block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            methodFn);
        compiler.getContext().builder.SetInsertPoint(entry);

        if (method->body)
        {
            // Handle method body
            compiler.getGenerator().generateBlock(method->body);
        }

        // Clear the insertion point
        compiler.getContext().builder.ClearInsertionPoint();
        compiler.getContext().currentFunction = nullptr;
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

    llvm::Value *Structs::createStructInstance(ASTNode *node)
    {
        CryoVariableNode *varDecl = node->data.varDecl;
        std::string structName = varDecl->type->container->custom.structDef->name;

        // Get struct type
        llvm::StructType *structType = compiler.getContext().structTypes[structName];

        // Allocate memory for struct
        llvm::Value *structPtr = compiler.getContext().builder.CreateAlloca(
            structType,
            nullptr,
            varDecl->name);

        // If there's an initializer, handle it
        if (varDecl->initializer)
        {
            // For implicit constructor call with single value
            if (varDecl->initializer->metaData->type == NODE_LITERAL_EXPR)
            {
                llvm::Value *initValue = compiler.getGenerator().getInitilizerValue(varDecl->initializer);

                // Call constructor
                std::vector<llvm::Value *> args;
                args.push_back(structPtr);
                args.push_back(initValue);

                llvm::Function *ctor = compiler.getContext().module->getFunction(structName + ".constructor");
                compiler.getContext().builder.CreateCall(ctor, args);
            }
        }

        return structPtr;
    }

};
