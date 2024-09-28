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
    /// @brief The entry point to the generation process.
    /// @param node
    void Generator::handleProgram(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Program");

        if (node->data.program->statementCount == 0)
        {
            debugger.logMessage("ERROR", __LINE__, "Generator", "No statements in program");
            return;
        }

        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            std::cout << "Processing Statement " << i + 1 << " of " << node->data.program->statementCount << std::endl;
            parseTree(node->data.program->statements[i]);
        }

        debugger.logMessage("INFO", __LINE__, "Generator", "Program Handled");
        return;
    }
    // -----------------------------------------------------------------------------------------------

    void Generator::handleExternFunction(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Extern Function");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Extern Function Handled");
        return;
    }

    void Generator::handleFunctionDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Declaration");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Function Declaration Handled");
        return;
    }

    void Generator::handleFunctionBlock(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Block");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Function Block Handled");
        return;
    }

    void Generator::handleReturnStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Return Statement");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Return Statement Handled");
        return;
    }

    void Generator::handleFunctionCall(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Function Call");

        functions.handleFunction(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Function Call Handled");
        return;
    }

    void Generator::handleVariableDeclaration(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Variables &variableInstance = compiler.getVariables();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Variable Declaration");

        CryoVariableNode *varDecl = node->data.varDecl;

        bool isRef = varDecl->isReference;
        bool isMutable = varDecl->isMutable;
        if (isRef)
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Reference Variable");
            // A Pointer variable
            variableInstance.handleRefVariable(node);
        }
        if (isMutable)
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Mutable Variable");
            // A Mutable variable
            variableInstance.handleMutableVariable(node);
        }
        else
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Constant Variable");
            // A Constant variable
            variableInstance.handleConstVariable(node);
        }

        // exit(1);
    }

    void Generator::handleBinaryExpression(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        BinaryExpressions &binaryExpressions = compiler.getBinaryExpressions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Binary Expression");

        binaryExpressions.handleBinaryExpression(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "Binary Expression Handled");
    }

    void Generator::handleUnaryExpression(ASTNode *node)
    {
        std::cout << "[CPP] Handling Unary Expression" << std::endl;
        // TODO: Implement
    }

    llvm::Value *Generator::handleLiteralExpression(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Types &types = compiler.getTypes();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Literal Expression");

        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        LiteralNode *literalNode = node->data.literal;

        assert(literalNode != nullptr);
        debugger.logMessage("INFO", __LINE__, "Generator", "Literal Node Found");

        switch (literalNode->dataType)
        {
        case DATA_TYPE_INT:
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Creating Int Constant");
            int intValue = literalNode->value.intValue;
            std::cout << "\n\nLiteral Int Value:" << node->data.literal->value.intValue << std::endl;
            llvmConstant = llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(32, intValue, true));
            std::cout << "\n";

            break;
        }
        case DATA_TYPE_STRING:
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Creating String Constant");
            llvmConstant = llvm::ConstantDataArray::getString(compiler.getContext().context, literalNode->value.stringValue);
            break;
        }
        case DATA_TYPE_FLOAT:
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Creating Float Constant");
            llvmConstant = llvm::ConstantFP::get(compiler.getContext().context, llvm::APFloat(literalNode->value.floatValue));
            break;
        }
        case DATA_TYPE_BOOLEAN:
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Creating Boolean Constant");
            int boolVal;
            bool boolValue = literalNode->value.booleanValue;
            if (boolValue)
            {
                boolVal = 1;
            }
            else
            {
                boolVal = 0;
            }
            llvmConstant = llvm::ConstantInt::get(compiler.getContext().context, llvm::APInt(boolVal, boolVal, true));
            break;
        }
        case DATA_TYPE_VOID:
        {
            debugger.logMessage("INFO", __LINE__, "Generator", "Creating Void Constant");
            llvmConstant = llvm::Constant::getNullValue(llvm::Type::getVoidTy(compiler.getContext().context));
            break;
        }
        default:
            debugger.logMessage("ERROR", __LINE__, "Generator", "Unknown type");
            std::cout << "Received: " << CryoDataTypeToString(literalNode->dataType) << std::endl;
            exit(1);
            break;
        }

        llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

        debugger.logMessage("INFO", __LINE__, "Generator", "Literal Expression Handled");
        return llvmValue;
    }

    void Generator::handleIfStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        IfStatements &ifStatements = compiler.getIfStatements();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling If Statement");

        ifStatements.handleIfStatement(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "If Statement Handled");
        return;
    }

    void Generator::handleWhileStatement(ASTNode *node)
    {
        std::cout << "[CPP] Handling While Statement" << std::endl;
        // TODO: Implement
    }

    void Generator::handleForStatement(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling For Statement");
        Loops &loops = compiler.getLoops();

        loops.handleForLoop(node);

        debugger.logMessage("INFO", __LINE__, "Generator", "For Statement Handled");
    }

    void Generator::handleReassignment(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Variables &variables = compiler.getVariables();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Reassignment");

        variables.handleVariableReassignment(node);

        return;
    }

    void Generator::handleParam(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Functions &functions = compiler.getFunctions();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Parameter");

        // A parameter is a variable declaration, just without the initializer

        return;
    }

    void Generator::handleStruct(ASTNode *node)
    {
        CryoDebugger &debugger = compiler.getDebugger();
        Structs &structs = compiler.getStructs();
        debugger.logMessage("INFO", __LINE__, "Generator", "Handling Struct");

        structs.handleStructDeclaration(node);

        return;
    }

} // namespace Cryo