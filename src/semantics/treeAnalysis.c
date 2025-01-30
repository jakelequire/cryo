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
#include "semantics/semantics.h"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

void treeAnalysis(struct SemanticAnalyzer *self, ASTNode *node)
{
    __STACK_FRAME__
    if (!node)
    {
        logMessage(LMI, "ERROR", "Semantic Analysis", "Root node is NULL");
        return;
    }

    switch (node->metaData->type)
    {
    case NODE_PROGRAM:
    {
        self->analyzeProgramNode(self, node);
        break;
    }
    case NODE_FUNCTION_DECLARATION:
    {
        self->analyzeFunctionDeclarationNode(self, node);
        break;
    }
    case NODE_VAR_DECLARATION:
    {
        self->analyzeVariableDeclarationNode(self, node);
        break;
    }
    case NODE_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_EXPRESSION:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_BINARY_EXPR:
    {
        self->analyzeBinaryOperationNode(self, node);
        break;
    }
    case NODE_UNARY_EXPR:
    {
        self->analyzeUnaryOperationNode(self, node);
        break;
    }
    case NODE_LITERAL_EXPR:
    {
        self->analyzeLiteralNode(self, node);
        break;
    }
    case NODE_VAR_NAME:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_FUNCTION_CALL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_IF_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_WHILE_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_FOR_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_RETURN_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_BLOCK:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_FUNCTION_BLOCK:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_EXPRESSION_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_ASSIGN:
    {
        self->analyzeAssignmentNode(self, node);
        break;
    }
    case NODE_PARAM_LIST:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_PARAM:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_TYPE:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_STRING_LITERAL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_STRING_EXPRESSION:
    {
        self->analyzeLiteralNode(self, node);
        break;
    }
    case NODE_BOOLEAN_LITERAL:
    {
        self->analyzeLiteralNode(self, node);
        break;
    }
    case NODE_ARRAY_LITERAL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_IMPORT_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_EXTERN_STATEMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_EXTERN_FUNCTION:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_ARG_LIST:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_NAMESPACE:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_INDEX_EXPR:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_VAR_REASSIGN:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_STRUCT_DECLARATION:
    {
        self->analyzeStructDeclarationNode(self, node);
        break;
    }
    case NODE_PROPERTY:
    {
        self->analyzePropertyDeclarationNode(self, node);
        break;
    }
    case NODE_CUSTOM_TYPE:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_SCOPED_FUNCTION_CALL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_EXTERNAL_SYMBOL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_STRUCT_CONSTRUCTOR:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_PROPERTY_ACCESS:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_THIS:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_THIS_ASSIGNMENT:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_PROPERTY_REASSIGN:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_METHOD:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_IDENTIFIER:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_METHOD_CALL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_ENUM:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_GENERIC_DECL:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_GENERIC_INST:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_CLASS:
    {
        self->analyzeClassDeclarationNode(self, node);
        break;
    }
    case NODE_CLASS_CONSTRUCTOR:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_OBJECT_INST:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_NULL_LITERAL:
    {
        self->analyzeLiteralNode(self, node);
        break;
    }
    case NODE_TYPEOF:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_USING:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_MODULE:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    case NODE_UNKNOWN:
    {
        self->unimplementedAnalysis(self);
        break;
    }
    default:
    {
        reportSemanticError(self, "Unknown node type");
        break;
    }
    }
}
