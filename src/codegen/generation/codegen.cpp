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
#include "codegen/generation/codegen.hpp"

namespace Cryo
{
    void CodeGen::initCodeGen()
    {
    }
    void CodeGen::initCodeGen(const std::string &moduleName)
    {
    }
    void CodeGen::initCodeGen(const std::string &moduleName, const std::string &outputDir)
    {
    }
    void CodeGen::generateModuleFromAST(ASTNode *root)
    {
        assert(root != nullptr && "Root ASTNode is null");
    }

    void CodeGen::recursiveASTTraversal(ASTNode *root)
    {
        // Cryo// debugger &// debugger = compiler.get// debugger();
        DevDebugger::logMessage("INFO", __LINE__, "CodeGen", "Parsing Tree");

        if (!root)
        {
            // debugger.logMessage("ERROR", __LINE__, "CodeGen", "Root is null!");
            CONDITION_FAILED;
        }

        // Generator &generator = this->compiler.getGenerator();

        switch (root->metaData->type)
        {
        case NODE_PROGRAM:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Program");
            // generator.handleProgram(root);
            break;
        case NODE_IMPORT_STATEMENT:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Import Statement");
            // generator.handleImportStatement(root);
            break;
        case NODE_FUNCTION_DECLARATION:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Declaration");
            // generator.handleFunctionDeclaration(root);
            break;
        case NODE_FUNCTION_BLOCK:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Block");
            // generator.handleFunctionBlock(root);
            break;
        case NODE_RETURN_STATEMENT:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Return Statement");
            // generator.handleReturnStatement(root);
            break;
        case NODE_FUNCTION_CALL:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Function Call");
            // generator.handleFunctionCall(root);
            break;
        case NODE_VAR_DECLARATION:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Declaration");
            // generator.handleVariableDeclaration(root);
            break;
        case NODE_BINARY_EXPR:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Binary Expression");
            // generator.handleBinaryExpression(root);
            break;
        case NODE_UNARY_EXPR:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Unary Expression");
            // generator.handleUnaryExpression(root);
            break;
        case NODE_LITERAL_EXPR:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Literal Expression");
            // generator.handleLiteralExpression(root);
            break;
        case NODE_IF_STATEMENT:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling If Statement");
            // generator.handleIfStatement(root);
            break;
        case NODE_WHILE_STATEMENT:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling While Statement");
            // generator.handleWhileStatement(root);
            break;
        case NODE_FOR_STATEMENT:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling For Statement");
            // generator.handleForStatement(root);
            break;
        case NODE_EXTERN_FUNCTION:
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Extern Function");
            // generator.handleExternFunction(root);
            break;
        case NODE_NAMESPACE:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Namespace");
            // set the module name
            std::string moduleName = std::string(root->data.cryoNamespace->name);
            std::cout << "Module Name: " << moduleName << std::endl;
            // compiler.getContext().module->setModuleIdentifier(moduleName);
            // Set the namespace name
            break;
        }
        case NODE_BLOCK:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Block");
            ASTNode **statements = root->data.block->statements;
            int statementCount = root->data.block->statementCount;
            for (int i = 0; i < statementCount; i++)
            {
                // generator.parseTree(statements[i]);
            }
            break;
        }
        case NODE_INDEX_EXPR:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Index Expression");
            break;
        }
        case NODE_VAR_REASSIGN:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Reassignment");
            // generator.handleReassignment(root);
            break;
        }
        case NODE_VAR_NAME:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Variable Name");
            break;
        }
        case NODE_STRUCT_DECLARATION:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Struct Declaration");
            // generator.handleStruct(root);
            break;
        }
        case NODE_SCOPED_FUNCTION_CALL:
        {
            // debugger.logMessage("INFO", __LINE__, "CodeGen", "Handling Scoped Function Call");
            // generator.handleScopedFunctionCall(root);
            break;
        }
        default:
            // debugger.logMessage("ERROR", __LINE__, "CodeGen", "Unknown Node Type");
            std::cout << "Received: " << CryoNodeTypeToString(root->metaData->type) << std::endl;
            CONDITION_FAILED;
            break;
        }

        // // debugger.logMessage("INFO", __LINE__, "CodeGen", "Tree Parsed");

        return;
    }

}
