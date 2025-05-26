/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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

    llvm::Value *Initializer::generatePropertyAccess(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Initializer", "Generating property access...");

        if (!validatePropertyAccessNode(node))
        {
            logMessage(LMI, "ERROR", "Initializer", "Invalid property access node");
            return nullptr;
        }

        PropertyAccessNode *propertyAccess = node->data.propertyAccess;
        std::string objectTypeName = propertyAccess->objectTypeName;
        std::string propertyName = propertyAccess->propertyName;
        int propertyIndex = propertyAccess->propertyIndex;

        logMessage(LMI, "INFO", "Initializer", "Accessing %s.%s (index: %d)",
                   objectTypeName.c_str(), propertyName.c_str(), propertyIndex);

        // Get the LLVM struct type for the object
        llvm::StructType *llvmStructType = getObjectStructType(objectTypeName);
        if (!llvmStructType)
        {
            logMessage(LMI, "ERROR", "Initializer", "LLVM struct type is null");
            return nullptr;
        }

        // Get the object instance
        llvm::Value *objectInstance = getObjectInstance(propertyAccess->object);
        if (!objectInstance)
        {
            logMessage(LMI, "ERROR", "Initializer", "Object instance is null");
            return nullptr;
        }

        // Get the property type
        llvm::Type *propertyType = getPropertyType(llvmStructType, propertyIndex);
        if (!propertyType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Property type not found for %s.%s",
                       objectTypeName.c_str(), propertyName.c_str());
            return nullptr;
        }

        // Ensure object instance is a pointer
        objectInstance = ensurePointerType(objectInstance, llvmStructType);

        // Access the property
        llvm::Value *accessedProp = accessProperty(objectInstance, llvmStructType, propertyIndex,
                                                   propertyType, objectTypeName, propertyName);
        if (!accessedProp)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to access property %s.%s",
                       objectTypeName.c_str(), propertyName.c_str());
            return nullptr;
        }
        logMessage(LMI, "INFO", "Initializer", "Successfully accessed property %s.%s",
                   objectTypeName.c_str(), propertyName.c_str());
        CodeGenDebug::printLLVMValue(accessedProp);
        return accessedProp;
    }

    bool Initializer::validatePropertyAccessNode(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is null");
            return false;
        }

        if (node->metaData->type != NODE_PROPERTY_ACCESS)
        {
            logMessage(LMI, "ERROR", "Initializer", "Node is not a property access");
            DEBUG_BREAKPOINT;
            return false;
        }

        if (!node->data.propertyAccess->objectType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Object type is null");
            DEBUG_BREAKPOINT;
            return false;
        }

        return true;
    }

    llvm::StructType *Initializer::getObjectStructType(const std::string &objectTypeName)
    {
        IRTypeSymbol *irTypeSymbol = context.getInstance().symbolTable->findType("struct." + objectTypeName);
        if (!irTypeSymbol)
        {
            logMessage(LMI, "ERROR", "Initializer", "IR type symbol not found for %s", objectTypeName.c_str());
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        llvm::Type *llvmType = irTypeSymbol->getType();
        if (!llvmType)
        {
            logMessage(LMI, "ERROR", "Initializer", "LLVM type is null");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        llvm::StructType *llvmStructType = llvm::dyn_cast<llvm::StructType>(llvmType);
        if (!llvmStructType)
        {
            logMessage(LMI, "ERROR", "Initializer", "LLVM type is not a struct type");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Initializer", "Found struct type: %s",
                   llvmStructType->getName().str().c_str());
        return llvmStructType;
    }

    llvm::Value *Initializer::getObjectInstance(ASTNode *objectNode)
    {
        if (objectNode->metaData->type == NODE_VAR_DECLARATION)
        {
            // If the object is a variable declaration, get its value
            std::string varName = objectNode->data.varDecl->name;
            IRVariableSymbol *varSymbol = context.getInstance().symbolTable->findVariable(varName);
            if (!varSymbol)
            {
                logMessage(LMI, "ERROR", "Initializer", "Variable %s not found", varName.c_str());
                DEBUG_BREAKPOINT;
                return nullptr;
            }
            return varSymbol->allocation.getValue();
        }
        llvm::Value *objectInstance = context.getInstance().visitor->getLLVMValue(objectNode);
        if (!objectInstance)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to get object instance");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        return objectInstance;
    }

    llvm::Type *Initializer::getPropertyType(llvm::StructType *structType, int propertyIndex)
    {
        llvm::Type *propertyType = structType->getContainedType(propertyIndex);
        if (!propertyType)
        {
            logMessage(LMI, "ERROR", "Initializer", "Property type not found at index %d", propertyIndex);
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        return propertyType;
    }

    llvm::Value *Initializer::ensurePointerType(llvm::Value *objectInstance, llvm::Type *objectType)
    {
        if (!objectInstance->getType()->isPointerTy())
        {
            // If the object instance is not a pointer, create a pointer to it
            llvm::AllocaInst *objectAlloc = context.getInstance().builder.CreateAlloca(
                objectType, nullptr, "obj.alloc");
            context.getInstance().builder.CreateStore(objectInstance, objectAlloc);
            return objectAlloc; // Return the pointer
        }

        return objectInstance;
    }

    llvm::Value *Initializer::accessProperty(
        llvm::Value *objectInstance,
        llvm::Type *objectType,
        int propertyIndex,
        llvm::Type *propertyType,
        const std::string &objectTypeName,
        const std::string &propertyName)
    {
        // Create a pointer to the property
        llvm::Value *propertyPtr = context.getInstance().builder.CreateStructGEP(
            objectType,
            objectInstance,
            propertyIndex,
            objectTypeName + "." + propertyName);

        if (!propertyPtr)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to create GEP instruction");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        // Load the property value
        llvm::Value *propertyValue = context.getInstance().builder.CreateLoad(
            propertyType, propertyPtr, propertyName + ".load");

        if (!propertyValue)
        {
            logMessage(LMI, "ERROR", "Initializer", "Failed to load property value");
            DEBUG_BREAKPOINT;
            return nullptr;
        }

        logMessage(LMI, "INFO", "Initializer", "Successfully accessed property %s", propertyName.c_str());
        return propertyValue;
    }

} // namespace Cryo
