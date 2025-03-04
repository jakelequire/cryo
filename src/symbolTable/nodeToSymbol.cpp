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
#include "symbolTable/globalSymtable.hpp"
#include "diagnostics/diagnostics.h"

#define MAX_SYMBOLS 1024

namespace Cryo
{
    Symbol *GlobalSymbolTable::ASTNodeToSymbol(ASTNode *node)
    {
        __STACK_FRAME__
        if (!node || node == nullptr)
        {
            std::cout << "ASTNodeToSymbol: Node is null" << std::endl;
            return nullptr;
        }

        TypeOfSymbol symbolType = UNKNOWN_SYMBOL;

        CryoNodeType nodeType = node->metaData->type;
        switch (nodeType)
        {
        case NODE_PROGRAM:
        {
            break;
        }

        case NODE_BLOCK:
        {
            break;
        }

        case NODE_FUNCTION_BLOCK:
        {
            int blockCount = node->data.functionBlock->statementCount;
            ASTNode **block = node->data.functionBlock->statements;
            for (int i = 0; i < blockCount; i++)
            {
                Symbol *sym = ASTNodeToSymbol(block[i]);
                if (sym)
                {
                    addSingleSymbolToTable(sym, getCurrentSymbolTable());
                }
            }
            break;
        }

        case NODE_ARG_LIST:
        {
            break;
        }

        case NODE_PARAM_LIST:
        {
            break;
        }

        case NODE_FUNCTION_DECLARATION:
        {

            break;
        }

        case NODE_VAR_DECLARATION:
        {
            VariableSymbol *variableSymbol = createVariableSymbol(node->data.varDecl->name,
                                                                  node->data.varDecl->type,
                                                                  node,
                                                                  0);
            return createSymbol(VARIABLE_SYMBOL, variableSymbol);
        }

        case NODE_PARAM:
        {
            VariableSymbol *variableSymbol = createVariableSymbol(node->data.param->name,
                                                                  node->data.param->type,
                                                                  node,
                                                                  0);
            return createSymbol(VARIABLE_SYMBOL, variableSymbol);
        }

        case NODE_TYPE:
        {
            break;
        }

        case NODE_EXTERN_STATEMENT:
        {
            break;
        }

        case NODE_EXTERN_FUNCTION:
        {
            break;
        }

        case NODE_STRUCT_DECLARATION:
        {
            break;
        }

        case NODE_PROPERTY:
        {
            break;
        }

        case NODE_CUSTOM_TYPE:
        {
            break;
        }

        case NODE_METHOD:
        {
            const char *methodName = node->data.method->name;
            DataType *returnType = node->data.method->functionType;
            size_t paramCount = node->data.method->paramCount;
            CryoVisibilityType visibility = node->data.method->visibility;

            MethodSymbol *methodSymbol = createMethodSymbol(node);

            return createSymbol(METHOD_SYMBOL, methodSymbol);
        }

        case NODE_CLASS:
        {
            break;
        }

        case NODE_ENUM:
        {
            break;
        }

        case NODE_EXTERNAL_SYMBOL:
        {
            break;
        }

        case NODE_NAMESPACE:
        case NODE_INDEX_EXPR:
        case NODE_VAR_REASSIGN:
        case NODE_SCOPED_FUNCTION_CALL:
        case NODE_STRUCT_CONSTRUCTOR:
        case NODE_PROPERTY_ACCESS:
        case NODE_THIS:
        case NODE_THIS_ASSIGNMENT:
        case NODE_PROPERTY_REASSIGN: // Unsure about this one
        case NODE_METHOD_CALL:
        case NODE_GENERIC_DECL: // Unsure about this one
        case NODE_CLASS_CONSTRUCTOR:
        case NODE_OBJECT_INST:
        case NODE_STRING_LITERAL:
        case NODE_ARRAY_LITERAL:
        case NODE_UNARY_EXPR:
        case NODE_IDENTIFIER:
        case NODE_GENERIC_INST:
        case NODE_STRING_EXPRESSION:
        case NODE_STATEMENT:
        case NODE_EXPRESSION:
        case NODE_BINARY_EXPR:
        case NODE_LITERAL_EXPR:
        case NODE_VAR_NAME:
        case NODE_FUNCTION_CALL:
        case NODE_IF_STATEMENT:
        case NODE_WHILE_STATEMENT:
        case NODE_FOR_STATEMENT:
        case NODE_RETURN_STATEMENT:
        case NODE_BOOLEAN_LITERAL:
        case NODE_IMPORT_STATEMENT:
        case NODE_EXPRESSION_STATEMENT:
        case NODE_ASSIGN:
        case NODE_MODULE:
        case NODE_USING:
        {
            // Skip Node.
            const char *nodeString = CryoNodeTypeToString(nodeType);
            return nullptr;
        }
        case NODE_UNKNOWN:
        {
            std::cout << "ASTNodeToSymbol: Unknown Node Type" << std::endl;
            CONDITION_FAILED;
        }
        default:
        {
            std::cout << "ASTNodeToSymbol: Unknown Node Type" << std::endl;
            CONDITION_FAILED;
        }
        }

        return nullptr;
    }

    const char *GlobalSymbolTable::getRootNamespace(ASTNode *root)
    {
        __STACK_FRAME__
        if (!root || root == nullptr)
        {
            std::cout << "getRootNamespace: Node is null" << std::endl;
            return nullptr;
        }

        // Seek for the namespace node and return the string from it
        int nodeCount = 32; // Abitrary number of nodes to check (The namespace should be within the first 32 nodes)
        for (int i = 0; i < nodeCount; i++)
        {
            if (root->data.program->statements[i] != nullptr)
            {
                ASTNode *node = root->data.program->statements[i];
                if (node->metaData->type == NODE_NAMESPACE)
                {
                    std::cout << "Found Namespace Node" << std::endl;
                    return node->data.cryoNamespace->name;
                }
            }
        }
        std::cout << "Unable to find Namespace Node" << std::endl;
        return nullptr;
    }

} // namespace Cryo
