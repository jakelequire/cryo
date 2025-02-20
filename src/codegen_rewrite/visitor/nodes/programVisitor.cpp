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

    void CodeGenVisitor::visitFunctionBlock(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Visiting function block...");
        if (!node)
        {
            logMessage(LMI, "ERROR", "Visitor", "Root node is null");
            return;
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

    // Skip module nodes.
    void CodeGenVisitor::visitModule(ASTNode *node)
    {
        logMessage(LMI, "INFO", "Visitor", "Skipping module node...");
        return;
    }

} // namespace Cryo
