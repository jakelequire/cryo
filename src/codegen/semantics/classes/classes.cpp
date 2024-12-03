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
        std::vector<llvm::Type *> classFields = handleFieldDeclarations(node,
                                                                        classNode->privateMembers,
                                                                        classNode->publicMembers,
                                                                        classNode->protectedMembers);

        // Format the name: "class.<className>"
        std::string classTypeName = "class." + className;

        // Create the class type and register it
        llvm::StructType *classType = llvm::StructType::create(
            compiler.getContext().context,
            classFields,
            classTypeName);

        // Add class type to the symbol table and context
        compiler.getSymTable().addClass(className, classType, classNode, classDataType);

        // Add to the `NamedGlobal` map
        compiler.getContext().module->getOrInsertGlobal(classTypeName, classType);

        // Handle constructor
        if (classNode->constructor)
        {
            ClassConstructorNode *ctorNode = classNode->constructor->data.classConstructor;
            handleClassConstructor(ctorNode, classType);
        }

        // Handle methods
        handleClassMethods(node, className, classType);

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

    void Classes::handleClassConstructor(ClassConstructorNode *ctorNode, llvm::StructType *structType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Class Constructor");

        std::string ctorName = std::string(ctorNode->name);
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Constructor Name: " + ctorName);

        // Create constructor function type
        std::vector<llvm::Type *> paramTypes;
        paramTypes.push_back(structType->getPointerTo()); // 'this' pointer

        // Add constructor parameters
        for (int i = 0; i < ctorNode->argCount; ++i)
        {
            CryoParameterNode *param = ctorNode->args[i]->data.param;
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
            ctorName,
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

        for (int i = 0; i < ctorNode->argCount; ++i)
        {
            llvm::Value *fieldPtr = compiler.getContext().builder.CreateStructGEP(
                structType,
                thisPtr,
                i,
                "field" + std::to_string(i));
            llvm::Value *argValue = argIt++;
            compiler.getContext().builder.CreateStore(argValue, fieldPtr);
        }

        compiler.getContext().builder.CreateRetVoid();

        return;
    }

    void Classes::handleClassMethods(ASTNode *node, std::string className, llvm::StructType *classType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Class Methods");

        ClassNode *classNode = node->data.classNode;
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Class Name: " + className);

        // Public Methods
        if (classNode->publicMembers)
        {
            for (int i = 0; i < classNode->publicMembers->methodCount; ++i)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Public Method");
                ASTNode *method = classNode->publicMembers->methods[i];
                if (!method)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Method is NULL");
                    CONDITION_FAILED;
                }
                createClassMethod(method, classType);
            }
        }

        // Private Methods
        if (classNode->privateMembers)
        {
            for (int i = 0; i < classNode->privateMembers->methodCount; ++i)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Private Method");
                ASTNode *method = classNode->privateMembers->methods[i];
                if (!method)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Method is NULL");
                    CONDITION_FAILED;
                }
                createClassMethod(method, classType);
            }
        }

        // Protected Methods
        if (classNode->protectedMembers)
        {
            for (int i = 0; i < classNode->protectedMembers->methodCount; ++i)
            {
                DevDebugger::logMessage("INFO", __LINE__, "Classes", "Handling Protected Method");
                ASTNode *method = classNode->protectedMembers->methods[i];
                if (!method)
                {
                    DevDebugger::logMessage("ERROR", __LINE__, "Classes", "Method is NULL");
                    CONDITION_FAILED;
                }
                createClassMethod(method, classType);
            }
        }

        return;
    }

    void Classes::createClassMethod(ASTNode *methodNode, llvm::StructType *classType)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Creating Class Method");

        MethodNode *method = methodNode->data.method;
        std::string methodName = std::string(method->name);
        bool isStatic = method->isStatic;
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Method Name: " + methodName);

        // Create parameter types for method
        std::vector<llvm::Type *> paramTypes;

        // Add the struct pointer as the first parameter (this pointer)
        if (!isStatic)
            paramTypes.push_back(classType->getPointerTo());

        for (int i = 0; i < method->paramCount; ++i)
        {
            CryoParameterNode *param = method->params[i]->data.param;
            addParametersToSymTable(method->params[i], param->name);
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

        std::string methodFullName = classType->getName().str() + "." + methodName;
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Method Full Name: " + methodFullName);

        // Create the method function
        llvm::Function *methodFn = llvm::Function::Create(
            methodType,
            llvm::Function::ExternalLinkage,
            llvm::Twine(methodFullName),
            *compiler.getContext().module);

        // Add struct instance as a parameter in the method function
        // Set the name of the struct parameter
        if (!isStatic)
        {
            auto argIt = methodFn->arg_begin();
            argIt->setName("this"); // Name the struct pointer parameter
        }

        methodFn->setName(methodFullName);
        // Set the name of the parameters
        int i = isStatic ? 0 : 1;
        for (auto &arg : methodFn->args())
        {
            std::string paramName = method->params[i]->data.param->name;
            arg.setName(paramName);
            llvm::Value *paramPtr = compiler.getFunctions().createParameter(&arg, paramTypes[i], method->params[i]);
            compiler.getContext().namedValues[paramName] = paramPtr;
            ++i;
        }

        // Add it to the module and set it as the current function
        compiler.getContext().module->getFunctionList().push_back(methodFn);
        compiler.getContext().currentFunction = methodFn;

        // Create entry block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(
            compiler.getContext().context,
            "entry",
            methodFn);
        compiler.getContext().builder.SetInsertPoint(entry);

        // Implementation of method body would go here
        if (method->body)
        {
            compiler.getGenerator().generateBlock(method->body);
        }

        return;
    }

    void Classes::addParametersToSymTable(ASTNode *paramNode, std::string paramName)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Classes", "Adding Parameters to Symbol Table");
        std::string namespaceName = compiler.getContext().currentNamespace;

        // Add the parameter to the symbol table
        compiler.getSymTable().addParameter(namespaceName, paramName, paramNode);

        return;
    }

} // namespace Cryo
