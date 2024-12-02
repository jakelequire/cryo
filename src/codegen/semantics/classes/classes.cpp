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
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Public Member");
                ASTNode *property = publicMembers->properties[i];
                if (!property)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Property is NULL");
                    CONDITION_FAILED;
                }
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        // Handle private members
        if (privateMembers)
        {
            for (int i = 0; i < privateMembers->propertyCount; ++i)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Private Member");
                ASTNode *property = privateMembers->properties[i];
                if (!property)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Property is NULL");
                    CONDITION_FAILED;
                }
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        // Handle protected members
        if (protectedMembers)
        {
            for (int i = 0; i < protectedMembers->propertyCount; ++i)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Protected Member");
                ASTNode *property = protectedMembers->properties[i];
                if (!property)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Property is NULL");
                    CONDITION_FAILED;
                }
                llvm::Type *fieldType = getClassFieldType(property);
                classFields.push_back(fieldType);
            }
        }

        return classFields;
    }

    llvm::Type *Classes::getClassFieldType(ASTNode *property)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Getting Class Field Type");

        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Getting Property Node");
        PropertyNode *propertyNode = property->data.property;
        DataType *propertyType = propertyNode->type;

        return compiler.getTypes().getType(propertyType, 0);
    }

    llvm::Value *Classes::handleClassConstructor(ASTNode *node, llvm::StructType *structType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Constructor");

        if (!structType || structType == nullptr)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Struct Type is NULL");
            CONDITION_FAILED;
        }

        // Create constructor function type
        std::vector<llvm::Type *> paramTypes;
        paramTypes.push_back(structType->getPointerTo()); // 'this' pointer

        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Getting Constructor Node");

        // Add constructor parameters
        int argCount = node->data.classNode->constructor->data.classConstructor->argCount;
        for (int i = 0; i < argCount; ++i)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Classes", "Adding Constructor Parameter");
            CryoParameterNode *param = node->data.classConstructor->args[i]->data.param;
            DevDebugger::logMessage("INFO", __LINE__, "Classes", "Getting Parameter Type");
            paramTypes.push_back(compiler.getTypes().getType(param->type, 0));
            DevDebugger::logMessage("INFO", __LINE__, "Classes", "Constructor Parameter Added");
        }

        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Creating Constructor Function Type");
        // Create constructor function type
        llvm::FunctionType *constructorType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(compiler.getContext().context),
            paramTypes,
            false);

        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Creating Constructor Function");
        // Create the constructor function
        llvm::Function *constructorFn = llvm::Function::Create(
            constructorType,
            llvm::Function::ExternalLinkage,
            "constructor");

        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Adding Constructor Function to Module");
        // Create entry block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            constructorFn);
        compiler.getContext().builder.SetInsertPoint(entry);

        // Add struct instance as a parameter in the constructor function
        // Set the name of the struct parameter
        auto argIt = constructorFn->arg_begin();
        llvm::Value *thisPtr = argIt++; // First argument is 'this' pointer
        argIt->setName("this");         // Name the struct pointer parameter

        // node->data.classConstructor->argCount
        // Set the name of the constructor arguments
        for (int i = 0; i < node->data.classConstructor->argCount; ++i)
        {
            PropertyNode *prop = node->data.classConstructor->args[i]->data.property;
            llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
                structType,
                thisPtr,
                i,
                "field" + std::to_string(i));
            llvm::Value *argValue = argIt++;
            compiler.getContext().builder.CreateStore(argValue, fieldPtr);
        }

        compiler.getContext().builder.CreateRetVoid();

        return nullptr;
    }

} // namespace Cryo
