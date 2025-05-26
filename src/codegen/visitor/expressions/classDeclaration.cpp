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

    void CodeGenVisitor::visitClassDecl(ASTNode *node)
    {
        if (!validateNode(node, "class declaration"))
            return;

        ClassNode *classNode = node->data.classNode;
        std::string className = classNode->name;

        logMessage(LMI, "INFO", "Visitor", "Visiting class declaration: %s", className.c_str());
        logMessage(LMI, "INFO", "Visitor", "Class has %d properties and %d methods",
                   classNode->propertyCount, classNode->methodCount);

        // Create the class struct type in LLVM
        llvm::StructType *classType = llvm::StructType::create(
            context.getInstance().context, "class." + className);

        // Collect all property types and symbols
        std::vector<llvm::Type *> propertyTypes;
        std::vector<IRPropertySymbol> propertySymbols;
        std::vector<IRMethodSymbol> methodSymbols;

        // Process properties by visibility
        processProperties(classNode->publicMembers, propertyTypes, propertySymbols, true);
        processProperties(classNode->privateMembers, propertyTypes, propertySymbols, false);
        processProperties(classNode->protectedMembers, propertyTypes, propertySymbols, false);

        // Process methods by visibility
        processMethods(classNode->publicMembers, methodSymbols, className);
        processMethods(classNode->privateMembers, methodSymbols, className);
        processMethods(classNode->protectedMembers, methodSymbols, className);

        // Finalize class type
        classType->setBody(propertyTypes);

        // Register the class in the symbol table
        IRTypeSymbol typeSymbol = IRSymbolManager::createTypeSymbol(
            classType, "class." + className, propertySymbols, methodSymbols);
        symbolTable->addType(typeSymbol);
        context.getInstance().module->getOrInsertGlobal(className, classType);

        logMessage(LMI, "INFO", "Visitor", "Class type created");
    }

    void CodeGenVisitor::processProperties(
        void *membersVoid,
        std::vector<llvm::Type *> &propertyTypes,
        std::vector<IRPropertySymbol> &propertySymbols,
        bool isPublic)
    {
        // Check if members exists
        if (!membersVoid)
            return;

        // Cast to appropriate member type
        // Since all member structs have the same layout, we can use any of them
        PublicMembers *members = static_cast<PublicMembers *>(membersVoid);

        int propertyStartIndex = propertyTypes.size();

        for (int i = 0; i < members->propertyCount; i++)
        {
            ASTNode *propertyNode = members->properties[i];
            if (!propertyNode || !propertyNode->data.property)
            {
                logMessage(LMI, "ERROR", "Visitor", "Property node or data is null");
                continue;
            }

            std::string propertyName = propertyNode->data.property->name;
            DataType *propertyType = propertyNode->data.property->type;

            logMessage(LMI, "INFO", "Visitor", "Processing property: %s", propertyName.c_str());

            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            if (!llvmType)
            {
                logMessage(LMI, "ERROR", "Visitor", "Failed to get LLVM type for property %s",
                           propertyName.c_str());
                continue;
            }

            propertyTypes.push_back(llvmType);

            int propertyIndex = propertyStartIndex + i;
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(
                llvmType, propertyName, propertyIndex, isPublic);
            propertySymbols.push_back(propertySymbol);
        }
    }

    void CodeGenVisitor::processMethods(
        void *membersVoid,
        std::vector<IRMethodSymbol> &methodSymbols,
        const std::string &className)
    {
        // Check if members exists
        if (!membersVoid)
            return;

        // Cast to appropriate member type
        // Since all member structs have the same layout, we can use any of them
        PublicMembers *members = static_cast<PublicMembers *>(membersVoid);

        for (int i = 0; i < members->methodCount; i++)
        {
            ASTNode *methodNode = members->methods[i];
            if (!methodNode || methodNode->metaData->type != NODE_METHOD || !methodNode->data.method)
            {
                logMessage(LMI, "ERROR", "Visitor", "Invalid method node");
                continue;
            }

            std::string methodName = methodNode->data.method->name;
            logMessage(LMI, "INFO", "Visitor", "Processing method: %s", methodName.c_str());

            DataType *methodType = methodNode->data.method->type;
            if (!methodType)
            {
                logMessage(LMI, "ERROR", "Visitor", "Method type is null for %s", methodName.c_str());
                continue;
            }

            IRMethodSymbol *methodSymbol = context.getInstance().initializer->createClassMethod(
                className, methodNode, methodType);

            if (!methodSymbol)
            {
                logMessage(LMI, "ERROR", "Visitor", "Failed to create method symbol for %s",
                           methodName.c_str());
                continue;
            }

            methodSymbols.push_back(*methodSymbol);
        }
    }

} // namespace Cryo
