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

    void Classes::handleClassDeclaration(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Class Declaration");

        ClassNode *classNode = node->data.classNode;
        std::string className = classNode->name;
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Class Name: " + className);

        DataType *classDataType = classNode->type;
        if (!classDataType || classDataType == nullptr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Class Data Type is NULL");
            CONDITION_FAILED;
        }
        logVerboseDataType(classDataType);
        DEBUG_BREAKPOINT;

        compiler.getContext().addClassDataType(className, classDataType);

        // Create class type with fields (Public/Private/Protected)
        std::vector<llvm::Type *> classFields = handleFieldDeclarations(node, classNode->privateMembers, classNode->publicMembers, classNode->protectedMembers);

        // Create the class type and register it
        llvm::StructType *classType = llvm::StructType::create(
            compiler.getContext().context,
            classFields,
            className);

        // Add class type to the symbol table and context
        compiler.getSymTable().addClass(className, classType, classNode, classDataType);

        // Add to the `NamedGlobal` map
        compiler.getContext().module->getOrInsertGlobal(className, classType);

        // Handle constructor
        if (classNode->constructor)
        {
            this->handleClassConstructor(node, classType);
        }

        return;
    }

    std::vector<llvm::Type *> Classes::handleFieldDeclarations(ASTNode *classNode, PrivateMembers *privateMembers, PublicMembers *publicMembers, ProtectedMembers *protectedMembers)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Field Declarations");

        std::vector<llvm::Type *> classFields;

        // Handle Public members
        if (publicMembers)
        {
            for (int i = 0; i < publicMembers->propertyCount; ++i)
            {
                ASTNode *property = publicMembers->properties[i];
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        // Handle private members
        if (privateMembers)
        {
            for (int i = 0; i < privateMembers->propertyCount; ++i)
            {
                ASTNode *property = privateMembers->properties[i];
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        // Handle protected members
        if (protectedMembers)
        {
            for (int i = 0; i < protectedMembers->propertyCount; ++i)
            {
                ASTNode *property = protectedMembers->properties[i];
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        return classFields;
    }

    llvm::Type *Classes::getClassFieldType(ASTNode *property)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Getting Class Field Type");

        logASTNode(property);

        PropertyNode *propertyNode = property->data.property;
        DataType *propertyType = propertyNode->type;

        return compiler.getTypes().getType(propertyType, 0);
    }

    llvm::Value *Classes::handleClassConstructor(ASTNode *node, llvm::StructType *structType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Constructor");

        // Create constructor function type
        std::vector<llvm::Type *> paramTypes;
        paramTypes.push_back(structType->getPointerTo()); // 'this' pointer

        // Add constructor parameters
        for (int i = 0; i < node->data.classConstructor->argCount; ++i)
        {
            CryoParameterNode *param = node->data.classConstructor->args[i]->data.param;
            paramTypes.push_back(compiler.getTypes().getType(param->type, 0));
        }

        // Get return type
        llvm::Type *returnType = llvm::Type::getVoidTy(compiler.getContext().context);

        // Create constructor function type
        llvm::FunctionType *constructorType = llvm::FunctionType::get(
            returnType,
            paramTypes,
            false);

        // Create the constructor function
        llvm::Function *constructorFn = llvm::Function::Create(
            constructorType,
            llvm::Function::ExternalLinkage,
            "constructor");

        // Add struct instance as a parameter in the constructor function
        // Set the name of the struct parameter
        auto argIt = constructorFn->arg_begin();
        argIt->setName("this"); // Name the struct pointer parameter

        // Set the name of the constructor arguments
        for (int i = 0; i < node->data.classConstructor->argCount; ++i)
        {
            argIt->setName(node->data.classConstructor->args[i]->data.param->name);
            ++argIt;
        }

        // Create the entry block for the constructor
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            constructorFn);

        // Set the insertion point
        compiler.getContext().builder.SetInsertPoint(entryBlock);

        // Set the current function
        compiler.getContext().currentFunction = constructorFn;

        // Set the 'this' pointer
        llvm::Value *thisPtr = argIt++; // First argument is 'this' pointer

        for (int i = 0; i < node->data.classConstructor->argCount; ++i)
        {
            // Get the argument value
            llvm::Value *argValue = argIt++;

            // Get the field pointer
            llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
                structType,
                thisPtr,
                i,
                "field" + std::to_string(i));

            // Store the argument value in the field pointer
            compiler.getContext().builder.CreateStore(argValue, fieldPtr);
        }

        // Create the return void instruction
        compiler.getContext().builder.CreateRetVoid();

        // Clear the insertion point
        compiler.getContext().builder.ClearInsertionPoint();

        return constructorFn;
    }

} // namespace Cryo
