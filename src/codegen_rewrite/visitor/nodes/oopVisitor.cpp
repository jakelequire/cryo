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
    // Object-oriented features
    void CodeGenVisitor::visitProperty(ASTNode *node) {}
    void CodeGenVisitor::visitMethod(ASTNode *node) {}
    void CodeGenVisitor::visitConstructor(ASTNode *node) {}
    void CodeGenVisitor::visitPropertyAccess(ASTNode *node) {}
    void CodeGenVisitor::visitPropertyReassignment(ASTNode *node) {}
    void CodeGenVisitor::visitThis(ASTNode *node) {}
    
} // namespace Cryo
