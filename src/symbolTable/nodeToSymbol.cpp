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

#define MAX_SYMBOLS 1024

namespace Cryo
{
    SymbolTable *GlobalSymbolTable::processProgramNode(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            std::cout << "processProgramNode: Node is null" << std::endl;
            return nullptr;
        }

        const char *namespaceName = getRootNamespace(node);
        if (!namespaceName)
        {
            std::cout << "processProgramNode: Namespace is null" << std::endl;
            return nullptr;
        }

        // Create a new Symbol Table
        SymbolTable *table = createSymbolTable(namespaceName);
        if (!table)
        {
            std::cout << "processProgramNode: Failed to create Symbol Table" << std::endl;
            return nullptr;
        }

        // Process the Program Node
        Symbol **symbols = (Symbol **)malloc(sizeof(Symbol *) * MAX_SYMBOLS);
        int symbolCount = 0;
        do
        {
            Symbol *symbol = ASTNodeToSymbol(node);
            if (symbol)
            {
                symbols[symbolCount] = symbol;
                symbolCount++;
            }
        } while (node->data.program->statements[symbolCount] != nullptr);
    }

    Symbol *GlobalSymbolTable::ASTNodeToSymbol(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            std::cout << "ASTNodeToSymbol: Node is null" << std::endl;
            return nullptr;
        }

        CryoNodeType nodeType = node->metaData->type;
        switch (nodeType)
        {
        case NODE_PROGRAM:
        {
            std::cout << "ASTNodeToSymbol: Processing Program Node" << std::endl;
            break;
        }
        case NODE_FUNCTION_DECLARATION:
        {
        }
        case NODE_VAR_DECLARATION:
        {
        }
        case NODE_BLOCK:
        {
        }
        case NODE_FUNCTION_BLOCK:
        {
        }
        case NODE_EXPRESSION_STATEMENT:
        {
        }
        case NODE_ASSIGN:
        {
        }
        case NODE_PARAM_LIST:
        {
        }
        case NODE_PARAM:
        {
        }
        case NODE_TYPE:
        {
        }
        case NODE_EXTERN_STATEMENT:
        {
        }
        case NODE_EXTERN_FUNCTION:
        {
        }
        case NODE_ARG_LIST:
        {
        }
        case NODE_STRUCT_DECLARATION:
        {
        }
        case NODE_PROPERTY:
        {
        }
        case NODE_CUSTOM_TYPE:
        {
        }
        case NODE_METHOD:
        {
        }
        case NODE_ENUM:
        {
        }
        case NODE_CLASS:
        {
        }
        case NODE_EXTERNAL_SYMBOL:
        {
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
        {
            // Skip Node.
            const char *nodeString = CryoNodeTypeToString(nodeType);
            std::cout << "ASTNodeToSymbol: Skipping Node: " << nodeString << std::endl;
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
