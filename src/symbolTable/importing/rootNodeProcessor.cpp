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

namespace Cryo
{

    void GlobalSymbolTable::handleRootNodeImport(ASTNode *node)
    {
        if (!node || node == nullptr)
        {
            std::cerr << "handleRootNodeImport: Node is null" << std::endl;
            return;
        }

        const char *namespaceName = getNamespaceNameFromRootNode(node);
        if (!namespaceName || namespaceName == nullptr)
        {
            std::cerr << "handleRootNodeImport: Namespace name is null" << std::endl;
            return;
        }

        SymbolTable *table = createNewImportTable(namespaceName);
        if (!table)
        {
            std::cerr << "handleRootNodeImport: Failed to create new import table" << std::endl;
            return;
        }

        // loopRootNode(node, table, (char *)namespaceName);

        // std::cout << "\n\n IMPORT TABLE: " << namespaceName << std::endl;
        // debugger->logSymbolTable(table);
        // std::cout << "\n\n";

        SymbolTable *curTable = getCurrentSymbolTable();
        if (!curTable || curTable == nullptr)
        {
            std::cerr << "handleRootNodeImport: Current table is null" << std::endl;
            return;
        }

        std::cout << "CURRENT TABLE: " << curTable->namespaceName << std::endl;
        debugger->logSymbolTable(curTable);

        completeDependencyTable();

        std::cout << "<!> Completed Root Node Import" << std::endl;

        return;
    }

    SymbolTable *GlobalSymbolTable::createNewImportTable(const char *namespaceName)
    {
        SymbolTable *table = createSymbolTable(namespaceName);
        if (!table)
        {
            std::cerr << "Failed to create new import table" << std::endl;
            return nullptr;
        }

        return table;
    }

    void GlobalSymbolTable::loopRootNode(ASTNode *node, SymbolTable *table, char *currentScopeID)
    {
        if (!node || node == nullptr)
        {
            std::cerr << "loopRootNode: Node is null" << std::endl;
            return;
        }

        if (!table || table == nullptr)
        {
            std::cerr << "loopRootNode: Table is null" << std::endl;
            return;
        }

        switch (node->metaData->type)
        {
        case NODE_PROGRAM:
        {
            for (size_t i = 0; i < node->data.program->statementCount; i++)
            {
                ASTNode *child = node->data.program->statements[i];
                loopRootNode(child, table, currentScopeID);
            }
            break;
        }
        case NODE_NAMESPACE:
        {
            // TODO: Handle namespace node
            break;
        }
        case NODE_VAR_DECLARATION:
        {
            // Add the variable to the table
            VariableSymbol *variable = createVariableSymbol(
                node->data.varDecl->name,
                node->data.varDecl->type,
                node,
                currentScopeID);
            if (!variable)
            {
                std::cerr << "loopRootNode: Failed to create variable symbol" << std::endl;
                return;
            }
            Symbol *symbol = wrapSubSymbol(VARIABLE_SYMBOL, variable);
            addSymbolToTable(symbol, table);
            table->count++;
            break;
        }
        case NODE_FUNCTION_DECLARATION:
        {
            // Add the function to the table
            FunctionSymbol *function = createFunctionSymbol(
                node->data.functionDecl->name,
                node->data.functionDecl->parentScopeID,
                node->data.functionDecl->type,
                node->data.functionDecl->paramTypes,
                node->data.functionDecl->paramCount,
                node->data.functionDecl->visibility,
                node);
            if (!function)
            {
                std::cerr << "loopRootNode: Failed to create function symbol" << std::endl;
                return;
            }
            Symbol *symbol = wrapSubSymbol(FUNCTION_SYMBOL, function);
            addSymbolToTable(symbol, table);
            table->count++;

            currentScopeID = (char *)function->functionScopeId;

            // Loop through the function body
            int statementCount = node->data.functionDecl->body->data.functionBlock->statementCount;
            for (int i = 0; i < statementCount; i++)
            {
                ASTNode *child = node->data.functionDecl->body->data.functionBlock->statements[i];
                loopRootNode(child, table, currentScopeID);
            }

            break;
        }
        case NODE_EXTERN_FUNCTION:
        {
            // Add the extern function to the table
            ExternSymbol *externSymbol = createExternSymbol(
                node->data.externFunction->name,
                node->data.externFunction->type,
                getTypeArrayFromASTNode(node->data.externFunction->params, node->data.externFunction->paramCount),
                node->data.externFunction->paramCount,
                node->metaData->type,
                VISIBILITY_PUBLIC,
                currentScopeID);
            if (!externSymbol)
            {
                std::cerr << "loopRootNode: Failed to create extern symbol" << std::endl;
                return;
            }
            Symbol *symbol = wrapSubSymbol(EXTERN_SYMBOL, externSymbol);
            addSymbolToTable(symbol, table);
            table->count++;
            break;
        }
        case NODE_STRUCT_DECLARATION:
        {
            // Add the struct to the table
            TypeSymbol *typeSymbol = createTypeSymbol(
                node->data.structNode->name,
                node,
                node->data.structNode->type,
                STRUCT_TYPE,
                false,
                false,
                currentScopeID);
            if (!typeSymbol)
            {
                std::cerr << "loopRootNode: Failed to create struct symbol" << std::endl;
                return;
            }
            Symbol *symbol = wrapSubSymbol(TYPE_SYMBOL, typeSymbol);
            addSymbolToTable(symbol, table);
            table->count++;

            // Add the properties to the table
            for (size_t i = 0; i < node->data.structNode->propertyCount; i++)
            {
                PropertySymbol *property = createPropertySymbol(node->data.structNode->properties[i]);
                if (!property)
                {
                    std::cerr << "loopRootNode: Failed to create property symbol" << std::endl;
                    return;
                }
                Symbol *propSymbol = wrapSubSymbol(PROPERTY_SYMBOL, property);
                addSymbolToTable(propSymbol, table);
                table->count++;
            }

            // Add the methods to the table
            for (size_t i = 0; i < node->data.structNode->methodCount; i++)
            {
                MethodSymbol *method = createMethodSymbol(node->data.structNode->methods[i]);
                if (!method)
                {
                    std::cerr << "loopRootNode: Failed to create method symbol" << std::endl;
                    return;
                }
                Symbol *methodSymbol = wrapSubSymbol(METHOD_SYMBOL, method);
                addSymbolToTable(methodSymbol, table);
                table->count++;
            }

            break;
        }
        case NODE_CLASS:
        {
            // Add the class to the table
            TypeSymbol *typeSymbol = createTypeSymbol(
                node->data.classNode->name,
                node,
                node->data.classNode->type,
                CLASS_TYPE,
                false,
                false,
                currentScopeID);
            if (!typeSymbol)
            {
                std::cerr << "loopRootNode: Failed to create class symbol" << std::endl;
                return;
            }
            Symbol *symbol = wrapSubSymbol(TYPE_SYMBOL, typeSymbol);
            addSymbolToTable(symbol, table);
            table->count++;

            break;
        }

        default:
            break;
        }
    }

} // namespace Cryo
