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
