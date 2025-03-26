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
#include "codegen_rewrite/visitor.hpp"

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
            context.module.get());

        symbolTable->setCurrentFunction(function);

        logMessage(LMI, "INFO", "Visitor", "Function prototype created");
        // Create the entry block
        llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context.context, "entry", function);
        context.builder.SetInsertPoint(entryBlock);

        // Add the function to the symbol table
        IRFunctionSymbol funcSymbol = IRSymbolManager::createFunctionSymbol(
            function, funcName, returnTy, funcType, entryBlock, false, false);
        symbolTable->addFunction(funcSymbol);

        // Visit the function body
        visit(node->data.functionDecl->body);

        return;
    }

    void CodeGenVisitor::visitExternFuncDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting extern function declaration...");
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

        return;
    }

    void CodeGenVisitor::visitStructDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting struct declaration...");
        return;
    }

    void CodeGenVisitor::visitClassDecl(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting class declaration...");
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

} // namespace Cryo
