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
#include "tools/utils/c_logger.h"
#include "symbolTable/globalSymtable.hpp"
#include "tools/logger/logger_config.h"
#include "diagnostics/diagnostics.h"

namespace Cryo
{

    void GlobalSymbolTable::initStructDeclaration(const char *structName, const char *parentNameID)
    {
        __STACK_FRAME__
        Symbol *structSymbol = createStructDeclarationSymbol(structName, parentNameID);
        if (structSymbol)
        {
            addSymbolToCurrentTable(structSymbol);
            initTypeDefinition(structSymbol);
            return;
        }
        else
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to create struct declaration symbol");
            return;
        }
    }

    Symbol *GlobalSymbolTable::createStructDeclarationSymbol(const char *structName, const char *parentNameID)
    {
        __STACK_FRAME__
        if (!structName || structName == nullptr)
        {
            return nullptr;
        }

        TypeSymbol *typeSymbol = createIncompleteTypeSymbol(structName, STRUCT_TYPE);
        Symbol *structSymbol = new Symbol();
        structSymbol->symbolType = TYPE_SYMBOL;
        structSymbol->type = typeSymbol;
        structSymbol->type->properties = (Symbol **)malloc(sizeof(Symbol *) * MAX_PROPERTY_COUNT);
        structSymbol->type->methods = (Symbol **)malloc(sizeof(Symbol *) * MAX_METHOD_COUNT);
        structSymbol->type->propertyCapacity = MAX_PROPERTY_COUNT;
        structSymbol->type->methodCapacity = MAX_METHOD_COUNT;
        structSymbol->type->propertyCount = 0;
        structSymbol->type->methodCount = 0;
        structSymbol->type->scopeId = IDGen::generate64BitHashID(structName);
        structSymbol->type->name = structName;
        structSymbol->type->parentNameID = parentNameID;

        return structSymbol;
    }

    void GlobalSymbolTable::addPropertyToStruct(const char *structName, ASTNode *property)
    {
        __STACK_FRAME__
        Symbol *structSymbol = getStructSymbol(structName);
        if (!structSymbol)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to find struct symbol for adding property");
            return;
        }

        PropertySymbol *propertySymbol = createPropertySymbol(property);
        updateStructSymbolProperties(structSymbol, propertySymbol, structSymbol->type->propertyCount);

        // Increment the property count
        structSymbol->type->propertyCount++;
        return;
    }

    void GlobalSymbolTable::addMethodToStruct(const char *structName, ASTNode *method)
    {
        __STACK_FRAME__
        Symbol *structSymbol = getStructSymbol(structName);
        if (!structSymbol)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Failed to find struct symbol for adding method");
            return;
        }

        MethodSymbol *methodSymbol = createMethodSymbol(method);
        updateStructSymbolMethods(structSymbol, methodSymbol, structSymbol->type->methodCount);

        structSymbol->type->methodCount++;
        return;
    }

    Symbol *GlobalSymbolTable::getStructSymbol(const char *structName)
    {
        __STACK_FRAME__
        // Find the Struct in the symbol table from the hash
        const char *structHashName = IDGen::generate64BitHashID(structName);

        Symbol *structSymbol = queryCurrentTable(structHashName, structName, TYPE_SYMBOL);
        if (structSymbol)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Found Struct Symbol");
            return structSymbol;
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Failed to find Struct Symbol");
        return nullptr;
    }

    void GlobalSymbolTable::updateStructSymbolMethods(Symbol *structSymbol, MethodSymbol *method, size_t methodCount)
    {
        __STACK_FRAME__
        Symbol *methodSymbol = createSymbol(METHOD_SYMBOL, method);
        size_t methodCap = structSymbol->type->methodCapacity;

        if (methodCount >= methodCap)
        {
            logMessage(LMI, "INFO", "SymbolTable", "Expanding method capacity");
            structSymbol->type->methods = (Symbol **)realloc(structSymbol->type->methods, methodCap * 2 * sizeof(Symbol *));
            structSymbol->type->methodCapacity = methodCap * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == structSymbol)
            {
                table->symbols[i]->type->methods[methodCount] = methodSymbol;
                return;
            }
        }

        // Increment the method count
        structSymbol->type->methodCount++;

        logMessage(LMI, "ERROR", "SymbolTable", "Method Failed to Add to Struct Symbol");
        return;
    }

    void GlobalSymbolTable::updateStructSymbolProperties(Symbol *structSymbol, PropertySymbol *property, size_t propertyCount)
    {
        __STACK_FRAME__
        Symbol *propertySymbol = createSymbol(PROPERTY_SYMBOL, property);
        size_t propCapacity = structSymbol->type->propertyCapacity;

        if (propertyCount >= propCapacity)
        {
            std::cout << "Expanding property capacity" << std::endl;
            structSymbol->type->properties = (Symbol **)realloc(structSymbol->type->properties, propCapacity * 2 * sizeof(Symbol *));
            structSymbol->type->propertyCapacity = propCapacity * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == structSymbol)
            {
                table->symbols[i]->type->properties[propertyCount] = propertySymbol;
                return;
            }
        }

        logMessage(LMI, "ERROR", "SymbolTable", "Property Failed to Add to Struct Symbol");
        return;
    }

    void GlobalSymbolTable::completeStructDeclaration(ASTNode *structNode, const char *structName)
    {
        __STACK_FRAME__
        if (!structNode || structNode == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Struct Node is null");
            return;
        }
        if (!structName || structName == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Struct Name is null");
            return;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return;
        }

        Symbol *structSymbol = getStructSymbol(structName);
        if (!structSymbol)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Struct Symbol is null");
            return;
        }

        // Add the node to the Struct symbol
        structSymbol->type->node = structNode;
        structSymbol->type->type = structNode->data.structNode->type;

        // Update the Struct symbol in the table
        updateStructSymbol(structSymbol, table);
        completeTypeDefinition(structSymbol, structName);

        logMessage(LMI, "INFO", "SymbolTable", "Completed Struct Declaration");
        return;
    }

    // -------------------------------------------------------
    // Updates Symbol Table with the Struct Declaration

    void GlobalSymbolTable::addStructDeclarationToTable(Symbol *structSymbol, SymbolTable *table)
    {
        __STACK_FRAME__
        table->symbols[table->count++] = structSymbol;
    }

    void GlobalSymbolTable::updateStructSymbol(Symbol *structSymbol, SymbolTable *table)
    {
        __STACK_FRAME__
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == structSymbol)
            {
                table->symbols[i] = structSymbol;
                return;
            }
        }
        std::cout << "Error: Unable to update Struct symbol in the table." << std::endl;
    }

    bool GlobalSymbolTable::doesStructSymbolExist(const char *name, SymbolTable *table)
    {
        __STACK_FRAME__
        if (!name || name == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Struct Name is null");
            return false;
        }
        if (!table || table == nullptr)
        {
            logMessage(LMI, "ERROR", "SymbolTable", "Symbol Table is null");
            return false;
        }

        for (size_t i = 0; i < table->count; i++)
        {
            if (table->symbols[i]->symbolType == TYPE_SYMBOL)
            {
                TypeSymbol *typeSymbol = table->symbols[i]->type;
                if (strcmp(typeSymbol->name, name) == 0)
                {
                    return true;
                }
            }
        }

        return false;
    }

} // namespace Cryo
