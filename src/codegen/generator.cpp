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
    /// @brief The entry point to the generation process.
    /// @param node
    void Generator::handleProgram(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Program");

        if (node->data.program->statementCount == 0)
        {
            DevDebugger::logMessage("ERROR", __LINE__, "Generator", "No statements in program");
            return;
        }

        // First, process all import statements
        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            ASTNode *statement = node->data.program->statements[i];
            if (statement->metaData->type == NODE_IMPORT_STATEMENT)
            {
                std::cout << "Processing Import Statement " << i + 1 << std::endl;

                parseTree(statement);
            }
        }

        // Then, process all other statements
        for (int i = 0; i < node->data.program->statementCount; ++i)
        {
            ASTNode *statement = node->data.program->statements[i];
            if (statement->metaData->type != NODE_IMPORT_STATEMENT)
            {
                std::cout << "Processing Statement " << i + 1 << " of " << node->data.program->statementCount << std::endl;
                parseTree(statement);
            }
        }

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Program Handled");
    }
    // -----------------------------------------------------------------------------------------------

    void Generator::handleExternFunction(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Extern Function");

        functions.handleFunction(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Extern Function Handled");
        return;
    }

    void Generator::handleFunctionDeclaration(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Function Declaration");

        functions.handleFunction(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Function Declaration Handled");
        return;
    }

    void Generator::handleFunctionBlock(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Function Block");

        functions.handleFunction(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Function Block Handled");
        return;
    }

    void Generator::handleReturnStatement(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Return Statement");

        functions.handleFunction(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Return Statement Handled");
        return;
    }

    void Generator::handleFunctionCall(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Function Call");

        functions.handleFunction(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Function Call Handled");
        return;
    }

    void Generator::handleVariableDeclaration(ASTNode *node)
    {
        Variables &variableInstance = compiler.getVariables();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Variable Declaration");

        CryoVariableNode *varDecl = node->data.varDecl;

        bool isRef = varDecl->isReference;
        bool isMutable = varDecl->isMutable;
        if (isRef)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Reference Variable");
            // A Pointer variable
            variableInstance.handleRefVariable(node);
        }
        if (isMutable)
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Mutable Variable");
            // A Mutable variable
            variableInstance.handleMutableVariable(node);
        }
        else
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Constant Variable");
            // A Constant variable
            variableInstance.createLocalVariable(node);
        }

        // exit(1);
    }

    void Generator::handleBinaryExpression(ASTNode *node)
    {
        BinaryExpressions &binaryExpressions = compiler.getBinaryExpressions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Binary Expression");

        binaryExpressions.handleBinaryExpression(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Binary Expression Handled");
    }

    void Generator::handleUnaryExpression(ASTNode *node)
    {
        std::cout << "[CPP] Handling Unary Expression" << std::endl;
        // TODO: Implement
    }

    llvm::Value *Generator::handleLiteralExpression(ASTNode *node)
    {
        OldTypes &types = compiler.getTypes();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Literal Expression");

        llvm::Value *llvmValue = nullptr;
        llvm::Constant *llvmConstant = nullptr;
        LiteralNode *literalNode = node->data.literal;

        assert(literalNode != nullptr);
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Literal Node Found");

        DataType *dataType = literalNode->type;
        switch (dataType->container.primitive)
        {
        case PRIM_INT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating Int Constant");
            int intValue = literalNode->value.intValue;
            std::cout << "\n\nLiteral Int Value:" << node->data.literal->value.intValue << std::endl;
            llvm::Type *ty = compiler.getTypes().getType(dataType, 0);
            llvmConstant = llvm::ConstantInt::get(ty, intValue);
            std::cout << "\n";

            break;
        }
        case PRIM_STRING:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating String Constant");
            llvmConstant = llvm::ConstantDataArray::getString(compiler.getContext().context, literalNode->value.stringValue);
            break;
        }
        case PRIM_FLOAT:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating Float Constant");
            llvmConstant = llvm::ConstantFP::get(compiler.getContext().context, llvm::APFloat(literalNode->value.floatValue));
            break;
        }
        case PRIM_BOOLEAN:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating Boolean Constant");
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
        case PRIM_NULL:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating Null Constant");
            llvmConstant = llvm::ConstantPointerNull::get(llvm::PointerType::get(compiler.getTypes().getType(dataType, 0), 0));
            break;
        }
        case PRIM_VOID:
        {
            DevDebugger::logMessage("INFO", __LINE__, "Generator", "Creating Void Constant");
            llvmConstant = llvm::Constant::getNullValue(llvm::Type::getVoidTy(compiler.getContext().context));
            break;
        }
        default:
            DevDebugger::logMessage("ERROR", __LINE__, "Generator", "Unknown type");
            std::cout << "Received: " << DataTypeToString(literalNode->type) << std::endl;
            exit(1);
            break;
        }

        llvmValue = llvm::dyn_cast<llvm::Value>(llvmConstant);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Literal Expression Handled");
        return llvmValue;
    }

    void Generator::handleIfStatement(ASTNode *node)
    {
        IfStatements &ifStatements = compiler.getIfStatements();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling If Statement");

        ifStatements.handleIfStatement(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "If Statement Handled");
        return;
    }

    void Generator::handleWhileStatement(ASTNode *node)
    {
        WhileStatements &whileStatements = compiler.getWhileStatements();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling While Statement");

        whileStatements.handleWhileLoop(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "While Statement Handled");
        return;
    }

    void Generator::handleForStatement(ASTNode *node)
    {
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling For Statement");
        Loops &loops = compiler.getLoops();

        loops.handleForLoop(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "For Statement Handled");
    }

    void Generator::handleReassignment(ASTNode *node)
    {
        Variables &variables = compiler.getVariables();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Reassignment");

        variables.handleVariableReassignment(node);

        return;
    }

    void Generator::handleParam(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Parameter");

        // A parameter is a variable declaration, just without the initializer

        return;
    }

    void Generator::handleStruct(ASTNode *node)
    {
        Structs &structs = compiler.getStructs();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Struct");

        structs.handleStructDeclaration(node);

        return;
    }

    void Generator::handleImportStatement(ASTNode *node)
    {
        Imports &imports = compiler.getImports();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Import Statement");

        imports.handleImportStatement(node);

        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Import Statement Handled");

        return;
    }

    void Generator::handleScopedFunctionCall(ASTNode *node)
    {
        Functions &functions = compiler.getFunctions();
        DevDebugger::logMessage("INFO", __LINE__, "Generator", "Handling Scoped Function Call");

        functions.handleFunction(node);

        return;
    }

    void Generator::addCommentToIR(const std::string &comment)
    {
        llvm::IRBuilder<> &Builder = compiler.getContext().builder;
        llvm::LLVMContext &Context = Builder.getContext();

        // Create a metadata string
        llvm::MDString *commentMD = llvm::MDString::get(Context, comment);

        // Create a metadata node
        llvm::MDNode *node = llvm::MDNode::get(Context, commentMD);

        // Create a named metadata node in the module
        llvm::NamedMDNode *namedNode = compiler.getContext().module->getOrInsertNamedMetadata("comments");

        // Add the comment metadata to the named node
        namedNode->addOperand(node);
    }

} // namespace Cryo
