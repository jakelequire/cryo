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
    // C Facing Function to work with classes
    void GlobalSymbolTable::initClassDeclaration(const char *className)
    {
        std::cout << "Creating Class Definition for: " << className << std::endl;
        Symbol *classSymbol = createClassDeclarationSymbol(className);
        if (classSymbol)
        {
            std::cout << "Class Declaration Symbol Created" << std::endl;
            addSymbolToCurrentTable(classSymbol);
            std::cout << "Class Declaration Symbol Added to Table" << std::endl;
            initTypeDefinition(classSymbol);
            std::cout << "Class Declaration Type Definition Initialized" << std::endl;
            return;
        }
        else
        {
            std::cout << "Failed to create class declaration symbol" << std::endl;
            return;
        }
    }

    void GlobalSymbolTable::addPropertyToClass(const char *className, ASTNode *property)
    {
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            std::cout << "Failed to find class symbol for adding property" << std::endl;
            return;
        }

        PropertySymbol *propertySymbol = createPropertySymbol(property);
        if (!propertySymbol)
        {
            std::cout << "Failed to create property symbol" << std::endl;
            return;
        }
        updateClassSymbolProperties(classSymbol, propertySymbol, classSymbol->type->propertyCount);

        // Increment the property count
        classSymbol->type->propertyCount++;
        return;
    }

    void GlobalSymbolTable::addMethodToClass(const char *className, ASTNode *method)
    {
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            std::cout << "Failed to find class symbol for adding method" << std::endl;
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
        TypeSymbol *typeSymbol = createIncompleteTypeSymbol(className, CLASS_TYPE);
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
        Symbol *methodSymbol = createSymbol(METHOD_SYMBOL, method);
        size_t methodCap = classSymbol->type->methodCapacity;

        if (methodCount >= methodCap)
        {
            std::cout << "Expanding method capacity" << std::endl;
            classSymbol->type->methods = (Symbol **)realloc(classSymbol->type->methods, methodCap * 2 * sizeof(Symbol *));
            classSymbol->type->methodCapacity = methodCap * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                std::cout << "Method Added to Class Symbol" << std::endl;
                table->symbols[i]->type->methods[methodCount] = methodSymbol;
                return;
            }
        }

        std::cout << "Method Failed to Add to Class Symbol" << std::endl;
        return;
    }

    void GlobalSymbolTable::updateClassSymbolProperties(Symbol *classSymbol, PropertySymbol *property, size_t propertyCount)
    {
        Symbol *propertySymbol = createSymbol(PROPERTY_SYMBOL, property);
        size_t propCapacity = classSymbol->type->propertyCapacity;

        if (propertyCount >= propCapacity)
        {
            std::cout << "Expanding property capacity" << std::endl;
            classSymbol->type->properties = (Symbol **)realloc(classSymbol->type->properties, propCapacity * 2 * sizeof(Symbol *));
            classSymbol->type->propertyCapacity = propCapacity * 2;
        }

        SymbolTable *table = getCurrentSymbolTable();
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                std::cout << "Property Added to Class Symbol" << std::endl;
                table->symbols[i]->type->properties[propertyCount] = propertySymbol;
                return;
            }
        }

        std::cout << "Property Failed to Add to Class Symbol" << std::endl;
        return;
    }

    Symbol *GlobalSymbolTable::getClassSymbol(const char *className)
    {
        // Find the class in the symbol table from the hash
        const char *classNameHash = IDGen::generate64BitHashID(className);
        std::cout << "Generated Hash for Class Name: " << classNameHash << std::endl;

        Symbol *classSymbol = queryCurrentTable(classNameHash, className, TYPE_SYMBOL);
        if (classSymbol)
        {
            std::cout << "Class Symbol Found!" << std::endl;
            return classSymbol;
        }

        std::cout << "Class Symbol not found" << std::endl;
        return nullptr;
    }

    void GlobalSymbolTable::completeClassDeclaration(ASTNode *classNode, const char *className)
    {
        if (!classNode || classNode == nullptr)
        {
            std::cout << "Error: Class Node is null" << std::endl;
            return;
        }
        if (!className || className == nullptr)
        {
            std::cout << "Error: Class Symbol is null" << std::endl;
            return;
        }

        SymbolTable *table = getCurrentSymbolTable();
        if (!table)
        {
            std::cout << "Error: Symbol Table is null" << std::endl;
            return;
        }

        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            std::cout << "Error: Class Symbol not found" << std::endl;
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
        std::cout << "Finding Property: " << propertyName << " in Class: " << className << std::endl;
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            std::cout << "Error: Class Symbol not found" << std::endl;
            return nullptr;
        }

        for (int i = 0; i < classSymbol->type->propertyCount; i++)
        {
            if (classSymbol->type->properties[i]->type->name == propertyName)
            {
                std::cout << "Property Found!" << std::endl;
                return classSymbol->type->properties[i]->type->node;
            }
        }

        std::cerr << "Property not found" << std::endl;
        return nullptr;
    }

    ASTNode *GlobalSymbolTable::findClassMethod(const char *methodName, const char *className)
    {
        std::cout << "Finding Method: " << methodName << " in Class: " << className << std::endl;
        Symbol *classSymbol = getClassSymbol(className);
        if (!classSymbol)
        {
            std::cout << "Error: Class Symbol not found" << std::endl;
            return nullptr;
        }

        for (int i = 0; i < classSymbol->type->methodCount; i++)
        {
            if (classSymbol->type->methods[i]->type->name == methodName)
            {
                std::cout << "Method Found!" << std::endl;
                return classSymbol->type->methods[i]->type->node;
            }
        }

        std::cerr << "Method not found" << std::endl;
        return nullptr;
    }

    // -------------------------------------------------------
    // Updates Symbol Table with the Class Declaration

    void GlobalSymbolTable::addClassDeclarationToTable(Symbol *classSymbol, SymbolTable *table)
    {
        table->symbols[table->count++] = classSymbol;
    }

    void GlobalSymbolTable::updateClassSymbol(Symbol *classSymbol, SymbolTable *table)
    {
        for (int i = 0; i < table->count; i++)
        {
            if (table->symbols[i] == classSymbol)
            {
                table->symbols[i] = classSymbol;
                return;
            }
        }
        std::cout << "Error: Unable to update class symbol in the table." << std::endl;
    }

} // namespace Cryo
