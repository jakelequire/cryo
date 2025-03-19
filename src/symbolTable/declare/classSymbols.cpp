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
    // C Facing Function to work with classes
    void GlobalSymbolTable::initClassDeclaration(const char *className)
    {
        __STACK_FRAME__
        Symbol *classSymbol = createClassDeclarationSymbol(className);
        if (classSymbol)
        {
            addSymbolToCurrentTable(classSymbol);
            initTypeDefinition(classSymbol);
            logMessage(LMI, "INFO", "Symbol Table", "Initialized Class Declaration", "Class Name", className);
            return;
        }
        else
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to Initialize Class Declaration", "Class Name", className);
            return;
        }
    }

    void GlobalSymbolTable::addPropertyToClass(const char *className, ASTNode *property)
    {
        __STACK_FRAME__
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to find class symbol for adding property", "Class Name", className);
            return;
        }

        PropertySymbol *propertySymbol = createPropertySymbol(property);
        if (!propertySymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to create property symbol", "Class Name", className);
            return;
        }
        updateClassSymbolProperties(classSymbol, propertySymbol, classSymbol->type->propertyCount);

        // Increment the property count
        classSymbol->type->propertyCount++;
        return;
    }

    void GlobalSymbolTable::addMethodToClass(const char *className, ASTNode *method)
    {
        __STACK_FRAME__
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to find class symbol for adding method", "Class Name", className);
            return;
        }

        MethodSymbol *methodSymbol = createMethodSymbol(method);
        updateClassSymbolMethods(classSymbol, methodSymbol, classSymbol->type->methodCount);

        // Increment the method count
        classSymbol->type->methodCount++;
        return;
    }

    // Class Symbol Management

    Symbol *GlobalSymbolTable::createClassDeclarationSymbol(const char *className)
    {
        __STACK_FRAME__
        TypeSymbol *typeSymbol = createIncompleteTypeSymbol(className, OBJECT_TYPE);
        Symbol *classSymbol = new Symbol();
        classSymbol->symbolType = TYPE_SYMBOL;
        classSymbol->type = typeSymbol;
        classSymbol->type->name = className;
        classSymbol->type->properties = (Symbol **)malloc(sizeof(Symbol *) * MAX_PROPERTY_COUNT);
        classSymbol->type->methods = (Symbol **)malloc(sizeof(Symbol *) * MAX_METHOD_COUNT);
        classSymbol->type->propertyCapacity = MAX_PROPERTY_COUNT;
        classSymbol->type->methodCapacity = MAX_METHOD_COUNT;
        classSymbol->type->propertyCount = 0;
        classSymbol->type->methodCount = 0;
        classSymbol->type->scopeId = IDGen::generate64BitHashID(className);

        return classSymbol;
    }

    void GlobalSymbolTable::updateClassSymbolMethods(Symbol *classSymbol, MethodSymbol *method, size_t methodCount)
    {
        __STACK_FRAME__
        Symbol *methodSymbol = createSymbol(METHOD_SYMBOL, method);
        size_t methodCap = classSymbol->type->methodCapacity;

        if (methodCount >= methodCap)
        {
            logMessage(LMI, "INFO", "Symbol Table", "Expanding method capacity", "Class Name", classSymbol->type->name);
            classSymbol->type->methods = (Symbol **)realloc(classSymbol->type->methods, methodCap * 2 * sizeof(Symbol *));
            classSymbol->type->methodCapacity = methodCap * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                table->symbols[i]->type->methods[methodCount] = methodSymbol;
                return;
            }
        }

        logMessage(LMI, "ERROR", "Symbol Table", "Method Failed to Add to Class Symbol", "Class Name", classSymbol->type->name);
        return;
    }

    void GlobalSymbolTable::updateClassSymbolProperties(Symbol *classSymbol, PropertySymbol *property, size_t propertyCount)
    {
        __STACK_FRAME__
        Symbol *propertySymbol = createSymbol(PROPERTY_SYMBOL, property);
        size_t propCapacity = classSymbol->type->propertyCapacity;

        if (propertyCount >= propCapacity)
        {
            logMessage(LMI, "INFO", "Symbol Table", "Expanding property capacity", "Class Name", classSymbol->type->name);
            classSymbol->type->properties = (Symbol **)realloc(classSymbol->type->properties, propCapacity * 2 * sizeof(Symbol *));
            classSymbol->type->propertyCapacity = propCapacity * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                table->symbols[i]->type->properties[propertyCount] = propertySymbol;
                return;
            }
        }

        logMessage(LMI, "ERROR", "Symbol Table", "Property Failed to Add to Class Symbol", "Class Name", classSymbol->type->name);
        return;
    }

    Symbol *GlobalSymbolTable::getClassSymbol(const char *className)
    {
        __STACK_FRAME__
        // Find the class in the symbol table from the hash
        const char *classNameHash = IDGen::generate64BitHashID(className);

        Symbol *classSymbol = queryCurrentTable(classNameHash, className, TYPE_SYMBOL);
        if (classSymbol)
        {
            return classSymbol;
        }

        logMessage(LMI, "ERROR", "Symbol Table", "Class Symbol not found", "Class Name", className);
        return nullptr;
    }

    void GlobalSymbolTable::completeClassDeclaration(ASTNode *classNode, const char *className)
    {
        __STACK_FRAME__
        if (!classNode || classNode == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Class Node is null");
            return;
        }
        if (!className || className == nullptr)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Class Name is null");
            return;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Symbol Table is null");
            return;
        }

        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Class Symbol not found", "Class Name", className);
            return;
        }

        // Add the node to the class symbol
        classSymbol->type->node = classNode;
        classSymbol->type->type = classNode->data.classNode->type;

        // Update the class symbol in the table
        updateClassSymbol(classSymbol, table);

        // Add the class declaration to the Types Table
        addDataTypeSymbol(classSymbol);

        completeTypeDefinition(classSymbol, className);

        return;
    }

    ASTNode *GlobalSymbolTable::findClassProperty(const char *propertyName, const char *className)
    {
        __STACK_FRAME__
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Class Symbol not found", "Class Name", className);
            return nullptr;
        }

        for (int i = 0; i < classSymbol->type->propertyCount; i++)
        {
            if (classSymbol->type->properties[i]->type->name == propertyName)
            {
                return classSymbol->type->properties[i]->type->node;
            }
        }

        logMessage(LMI, "ERROR", "Symbol Table", "Property not found", "Property Name", propertyName);
        return nullptr;
    }

    ASTNode *GlobalSymbolTable::findClassMethod(const char *methodName, const char *className)
    {
        __STACK_FRAME__
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Class Symbol not found", "Class Name", className);
            return nullptr;
        }

        for (int i = 0; i < classSymbol->type->methodCount; i++)
        {
            if (classSymbol->type->methods[i]->type->name == methodName)
            {
                return classSymbol->type->methods[i]->type->node;
            }
        }

        logMessage(LMI, "ERROR", "Symbol Table", "Method not found", "Method Name", methodName);
        return nullptr;
    }

    // -------------------------------------------------------
    // Updates Symbol Table with the Class Declaration

    void GlobalSymbolTable::addClassDeclarationToTable(Symbol *classSymbol, SymbolTable *table)
    {
        __STACK_FRAME__
        table->symbols[table->count++] = classSymbol;
    }

    void GlobalSymbolTable::updateClassSymbol(Symbol *classSymbol, SymbolTable *table)
    {
        __STACK_FRAME__
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                table->symbols[i] = classSymbol;
                return;
            }
        }
        logMessage(LMI, "ERROR", "Symbol Table", "Failed to update class symbol in table");
    }

} // namespace Cryo
