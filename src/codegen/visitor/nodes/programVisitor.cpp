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
    // Program structure
    void CodeGenVisitor::visitProgram(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting program...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Root node is null");
            return;
        }

        for (size_t i = 0; i < node->data.program->statementCount; i++)
        {
            visit(node->data.program->statements[i]);
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting program complete!");
        return;
    }

    void CodeGenVisitor::visitModule(ASTNode *node)
    {
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Root node is null");
            return;
        }
        if (node->metaData->type != NODE_MODULE)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not a module");
            return;
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting module...");
        for (size_t i = 0; i < node->data.module->statementCount; i++)
        {
            std::string nodeTypeStr = CryoNodeTypeToString(node->data.module->statements[i]->metaData->type);
            logMessage(LMI, "INFO", "Visitor", "Visiting node: %s", nodeTypeStr.c_str());
            if (node->metaData->type == NODE_NAMESPACE)
            {
                // Skip namespace nodes
            }
            else
            {
                visit(node->data.module->statements[i]);
            }
        }
        logMessage(LMI, "INFO", "Visitor", "Visiting module complete!");
        return;
    }

    void CodeGenVisitor::visitFunctionBlock(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting function block...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Root node is null");
            return;
        }

        if (node->metaData->type != NODE_FUNCTION_BLOCK)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not a function block");
            return;
        }

        // Make sure that the entry point has been set from the function declaration.
        if (!context.getInstance().builder.GetInsertBlock())
        {
            logMessage(LMI, "ERROR", "Visitor", "Entry point is not set");
            CONDITION_FAILED;
        }

        for (size_t i = 0; i < node->data.functionBlock->statementCount; i++)
        {
            visit(node->data.functionBlock->statements[i]);
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting function block complete!");
        return;
    }

    void CodeGenVisitor::visitBlock(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting block...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Root node is null");
            return;
        }

        if (node->metaData->type != NODE_BLOCK)
        {
            logMessage(LMI, "ERROR", "Visitor", "Node is not a block");
            return;
        }

        for (size_t i = 0; i < node->data.block->statementCount; i++)
        {
            visit(node->data.block->statements[i]);
        }

        logMessage(LMI, "INFO", "Visitor", "Visiting block complete!");
        return;
    }

    // Skip namespace nodes.
    void CodeGenVisitor::visitNamespace(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Skipping namespace node...");
        return;
    }

    // Skip import nodes.
    void CodeGenVisitor::visitImport(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Skipping import node...");
        return;
    }

    // Skip using nodes.
    void CodeGenVisitor::visitUsing(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Skipping using node...");
        return;
    }

} // namespace Cryo
