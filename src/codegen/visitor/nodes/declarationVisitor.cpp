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
        for (size_t i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            ASTNode *param = node->data.functionDecl->params[i];
            llvm::Type *paramType = symbolTable->getLLVMType(param->data.param->type);
            argTypes.push_back(paramType);
        }
        logMessage(LMI, "INFO", "Visitor", "Function has %d arguments", argTypes.size());

        DataType *functionType = node->data.functionDecl->type;
        DataType *returnType = functionType->container->type.functionType->returnType;

        llvm::Type *returnTy = symbolTable->getLLVMType(returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnTy, argTypes, false);

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

        for (size_t i = 0; i < node->data.functionDecl->paramCount; i++)
        {
            ASTNode *param = node->data.functionDecl->params[i];
            llvm::Type *paramType = symbolTable->getLLVMType(param->data.param->type);

            // Create a symbol for each parameter for the symbol table
            std::string paramName = param->data.param->name;
            logMessage(LMI, "INFO", "Visitor", "Parameter Name: %s", paramName.c_str());
            AllocaType allocaType = AllocaTypeInference::inferFromNode(param, false);
            IRVariableSymbol paramSymbol = IRSymbolManager::createVariableSymbol(
                function, nullptr, paramType, paramName, allocaType);

            // Create the parameter in the function
            llvm::Function::arg_iterator argIt = function->arg_begin();
            llvm::Value *arg = argIt++;
            arg->setName(paramName);

            // Allocate space for the parameter in the function's entry block
            paramSymbol.allocation = Allocation::createLocal(context.getInstance().builder, paramType, paramName, arg);

            // Add the parameter to the symbol table
            symbolTable->addVariable(paramSymbol);
        }

        logMessage(LMI, "INFO", "Visitor", "Function prototype created");

        // Add the function to the symbol table
        IRFunctionSymbol funcSymbol = IRSymbolManager::createFunctionSymbol(
            function, funcName, returnTy, funcType, entryBlock, false, false);
        symbolTable->addFunction(funcSymbol);
        symbolTable->setCurrentFunction(&funcSymbol);

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
        for (size_t i = 0; i < node->data.externFunction->paramCount; i++)
        {
            ASTNode *param = node->data.externFunction->params[i];
            llvm::Type *paramType = symbolTable->getLLVMType(param->data.param->type);
            argTypes.push_back(paramType);
        }
        logMessage(LMI, "INFO", "Visitor", "Extern function has %d arguments", argTypes.size());

        DataType *functionType = node->data.externFunction->type;
        functionType->debug->printType(functionType);
        DataType *returnType = functionType->container->type.functionType->returnType;

        llvm::Type *returnTy = symbolTable->getLLVMType(returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnTy, argTypes, false);

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

    void CodeGenVisitor::visitVarDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting variable declaration...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        std::string varName = node->data.varDecl->name;
        logMessage(LMI, "INFO", "Visitor", "Variable Name: %s", varName.c_str());

        DataType *varType = node->data.varDecl->type;
        llvm::Type *llvmType = symbolTable->getLLVMType(varType);

        llvm::Value *initVal = nullptr;
        if (node->data.varDecl->initializer)
        {
            logMessage(LMI, "INFO", "Visitor", "Variable has an initialization expression");
            initVal = getLLVMValue(node->data.varDecl->initializer);
        }
        else
        {
            logMessage(LMI, "INFO", "Visitor", "Variable has no initialization expression");
        }

        llvm::AllocaInst *alloca = context.getInstance().builder.CreateAlloca(llvmType, nullptr, varName);
        AllocaType allocaType = AllocaTypeInference::inferFromNode(node, false);
        IRVariableSymbol varSymbol = IRSymbolManager::createVariableSymbol(alloca, llvmType, varName, allocaType, Allocation());
        symbolTable->addVariable(varSymbol);

        if (initVal)
        {
            context.builder.CreateStore(initVal, alloca);
        }

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
        logMessage(LMI, "INFO", "Visitor", "Struct Name: %s", structName.c_str());

        int propertyCount = node->data.structNode->propertyCount;
        logMessage(LMI, "INFO", "Visitor", "Struct has %d properties", propertyCount);

        int methodCount = node->data.structNode->methodCount;

        std::vector<llvm::Type *> propertyTypes;
        std::vector<IRPropertySymbol> propertySymbols;
        std::vector<IRMethodSymbol> methodSymbols;

        logMessage(LMI, "INFO", "Visitor", "Processing property symbols...");
        for (int i = 0; i < propertyCount; i++)
        {
            ASTNode *property = node->data.structNode->properties[i];
            DataType *propertyType = property->data.property->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            std::string propertyName = property->data.property->name;

            propertyTypes.push_back(llvmType);
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(llvmType, propertyName, i, true);
            propertySymbols.push_back(propertySymbol);
        }

        logMessage(LMI, "INFO", "Visitor", "Processing method symbols...");
        for (int i = 0; i < methodCount; i++)
        {
            ASTNode *method = node->data.structNode->methods[i];
            DataType *methodType = method->data.method->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(methodType);
            std::string methodName = method->data.method->name;

            IRFunctionSymbol functionSymbol = IRSymbolManager::createFunctionSymbol(
                nullptr, methodName, llvmType, nullptr, nullptr, false, false);
            IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
                functionSymbol, false, false, false, false, 0, nullptr);
            methodSymbols.push_back(methodSymbol);
        }

        llvm::StructType *structType = llvm::StructType::create(context.getInstance().context, "struct." + structName);
        structType->setBody(propertyTypes);

        IRTypeSymbol typeSymbol = IRSymbolManager::createTypeSymbol(
            structType, "struct." + structName, propertySymbols, methodSymbols);
        symbolTable->addType(typeSymbol);

        context.getInstance().module->getOrInsertGlobal(structName, structType);
        logMessage(LMI, "INFO", "Visitor", "Struct type created");

        // Create the constructor if it exists
        if (node->data.structNode->constructor)
        {
            logMessage(LMI, "INFO", "Visitor", "Struct has a constructor");
            context.getInstance().initializer->generateStructConstructor(node->data.structNode->constructor, structType);
        }

        logMessage(LMI, "INFO", "Visitor", "Struct declaration complete");
        return;
    }

    void CodeGenVisitor::visitClassDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting class declaration...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is null");
            return;
        }

        std::string className = node->data.classNode->name;
        logMessage(LMI, "INFO", "Visitor", "Class Name: %s", className.c_str());

        llvm::StructType *classType = llvm::StructType::create(context.getInstance().context, className);

        int propertyCount = node->data.classNode->propertyCount;
        logMessage(LMI, "INFO", "Visitor", "Class has %d properties", propertyCount);
        int methodCount = node->data.classNode->methodCount;
        logMessage(LMI, "INFO", "Visitor", "Class has %d methods", methodCount);

        // Create the class type
        std::vector<llvm::Type *> propertyTypes;
        std::vector<IRPropertySymbol> propertySymbols;
        std::vector<IRMethodSymbol> methodSymbols;

        // Loop through the public properties
        for (int i = 0; i < node->data.classNode->publicMembers->propertyCount; i++)
        {
            ASTNode *publicProperties = node->data.classNode->publicMembers->properties[i];
            DataType *propertyType = publicProperties->data.property->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            std::string propertyName = publicProperties->data.property->name;
            propertyTypes.push_back(llvmType);
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(llvmType, propertyName, i, true);
            propertySymbols.push_back(propertySymbol);
        }

        // Loop through the private properties
        for (int i = 0; i < node->data.classNode->privateMembers->propertyCount; i++)
        {
            ASTNode *privateProperties = node->data.classNode->privateMembers->properties[i];
            DataType *propertyType = privateProperties->data.property->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            std::string propertyName = privateProperties->data.property->name;
            propertyTypes.push_back(llvmType);
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(llvmType, propertyName, i, false);
            propertySymbols.push_back(propertySymbol);
        }

        // Loop through the protected properties
        for (int i = 0; i < node->data.classNode->protectedMembers->propertyCount; i++)
        {
            ASTNode *protectedProperties = node->data.classNode->protectedMembers->properties[i];
            DataType *propertyType = protectedProperties->data.property->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(propertyType);
            std::string propertyName = protectedProperties->data.property->name;
            propertyTypes.push_back(llvmType);
            IRPropertySymbol propertySymbol = IRSymbolManager::createPropertySymbol(llvmType, propertyName, i, false);
            propertySymbols.push_back(propertySymbol);
        }

        // Loop through the public methods
        for (int i = 0; i < node->data.classNode->publicMembers->methodCount; i++)
        {
            ASTNode *publicMethods = node->data.classNode->publicMembers->methods[i];
            DataType *methodType = publicMethods->data.method->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(methodType);
            std::string methodName = publicMethods->data.method->name;
            IRFunctionSymbol functionSymbol = IRSymbolManager::createFunctionSymbol(
                nullptr, methodName, llvmType, nullptr, nullptr, false, false);
            IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
                functionSymbol, false, false, false, false, 0, nullptr);
            methodSymbols.push_back(methodSymbol);
        }

        // Loop through the private methods
        for (int i = 0; i < node->data.classNode->privateMembers->methodCount; i++)
        {
            ASTNode *privateMethods = node->data.classNode->privateMembers->methods[i];
            DataType *methodType = privateMethods->data.method->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(methodType);
            std::string methodName = privateMethods->data.method->name;
            IRFunctionSymbol functionSymbol = IRSymbolManager::createFunctionSymbol(
                nullptr, methodName, llvmType, nullptr, nullptr, false, false);
            IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
                functionSymbol, false, false, false, false, 0, nullptr);
            methodSymbols.push_back(methodSymbol);
        }

        // Loop through the protected methods
        for (int i = 0; i < node->data.classNode->protectedMembers->methodCount; i++)
        {
            ASTNode *protectedMethods = node->data.classNode->protectedMembers->methods[i];
            DataType *methodType = protectedMethods->data.method->type;
            llvm::Type *llvmType = symbolTable->getLLVMType(methodType);
            std::string methodName = protectedMethods->data.method->name;
            IRFunctionSymbol functionSymbol = IRSymbolManager::createFunctionSymbol(
                nullptr, methodName, llvmType, nullptr, nullptr, false, false);
            IRMethodSymbol methodSymbol = IRSymbolManager::createMethodSymbol(
                functionSymbol, false, false, false, false, 0, nullptr);
            methodSymbols.push_back(methodSymbol);
        }

        // Create the class type
        classType->setBody(propertyTypes);

        IRTypeSymbol typeSymbol = IRSymbolManager::createTypeSymbol(
            classType, className, propertySymbols, methodSymbols);
        symbolTable->addType(typeSymbol);
        context.getInstance().module->getOrInsertGlobal(className, classType);

        logMessage(LMI, "INFO", "Visitor", "Class type created");

        return;
    }

    void CodeGenVisitor::visitEnumDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting enum declaration...");
        return;
    }

    void CodeGenVisitor::visitGenericDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting generic declaration...");
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

} // namespace Cryo
