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
#include "cpp/codegen.h"

namespace Cryo
{

    void CryoModules::generateProgram(ASTNode *node)
    {
        CodeGen &codeGenInstance = compiler.getCodeGen();
        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();
        CryoDebugger &cryoDebugger = compiler.getDebugger();

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Program Node");
        if (node->data.program->statementCount == 0)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Program Node has no statements");
            return;
        }
        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Program Statement");
            cryoSyntaxInstance.identifyNodeExpression(node->data.program->statements[i]);
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Program code generation complete");
        return;
    }

    void CryoModules::generateBlock(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Block Node is null");
            return;
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Block Node");
        CodeGen &codeGenInstance = compiler.getCodeGen();
        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();

        for (int i = 0; i < node->data.block->statementCount; i++)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Block Statement");
            cryoSyntaxInstance.identifyNodeExpression(node->data.block->statements[i]);
        }

        return;
    }

    void CryoModules::generateFunctionBlock(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Function Block Node is null");
            return;
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Function Block Node");
        CodeGen &codeGenInstance = compiler.getCodeGen();
        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();
        CryoSyntax &cryoSyntaxInstance = compiler.getSyntax();

        for (int i = 0; i < node->data.functionBlock->statementCount; ++i)
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for Function Block Statement");
            cryoSyntaxInstance.identifyNodeExpression(node->data.functionBlock->statements[i]);
        }

        llvm::BasicBlock *BB = cryoContext.builder.GetInsertBlock();
        if (!BB->getTerminator())
        {
            cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Function Block has no terminator");
            cryoContext.builder.CreateRetVoid();
        }

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Function Block code generation complete");
        return;
    }

    void CryoModules::generateExternalDeclaration(ASTNode *node)
    {
        CryoDebugger &cryoDebugger = compiler.getDebugger();
        if (!node)
        {
            cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "External Declaration Node is null");
            return;
        }

        CryoContext &cryoContext = compiler.getContext();
        CryoTypes &cryoTypesInstance = compiler.getTypes();

        char *functionName = node->data.externNode->externNode->data.functionDecl->name;
        CryoDataType returnTypeData = node->data.externNode->externNode->data.functionDecl->returnType;

        llvm::Type *returnType = cryoTypesInstance.getLLVMType(returnTypeData);
        std::vector<llvm::Type *> paramTypes;

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating code for External Declaration Node");
        for (int i = 0; i < node->data.externNode->externNode->data.functionDecl->paramCount; i++)
        {
            ASTNode *parameter = node->data.externNode->externNode->data.functionDecl->params[i];
            llvm::Type *paramType;
            if (parameter == nullptr)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Parameter Node is null");
                break;
            }
            if (parameter->data.varDecl->type == DATA_TYPE_INT)
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating extern code for type int");
                paramType = cryoTypesInstance.getLLVMType(DATA_TYPE_INT);
                paramTypes.push_back(paramType);
                break;
            }
            if (parameter->data.varDecl->type == DATA_TYPE_STRING)
            {
                cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating extern code for type string");
                paramType = cryoTypesInstance.getLLVMType(DATA_TYPE_STRING);
                paramTypes.push_back(paramType);
                break;
            }
            if (!paramType)
            {
                cryoDebugger.logMessage("ERROR", __LINE__, "Generation", "Parameter type is null");
                return;
            }
        }
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes[0], false);

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "Generating extern code for function");
        llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, functionName, cryoContext.module.get());
        function->setCallingConv(llvm::CallingConv::C);

        cryoDebugger.logMessage("INFO", __LINE__, "Generation", "External Declaration code generation complete");
        return;
    }

} // namespace Cryo
