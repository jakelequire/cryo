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
        std::string classNameStr(className);
        Symbol *classSymbol = createClassDeclarationSymbol(classNameStr);
        if (classSymbol)
        {
            addSymbolToCurrentTable(classSymbol);
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
        updateClassSymbolProperties(classSymbol, propertySymbol, classSymbol->type->propertyCount + 1);

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
        updateClassSymbolMethods(classSymbol, methodSymbol, classSymbol->type->methodCount + 1);

        // Increment the method count
        classSymbol->type->methodCount++;

        return;
    }

    // Class Symbol Management

    Symbol *GlobalSymbolTable::createClassDeclarationSymbol(std::string className)
    {
        TypeSymbol *typeSymbol = createIncompleteTypeSymbol(className.c_str(), CLASS_TYPE);
        Symbol *classSymbol = new Symbol();
        classSymbol->symbolType = TYPE_SYMBOL;
        classSymbol->type = typeSymbol;

        return classSymbol;
    }

    Symbol *GlobalSymbolTable::updateClassSymbolMethods(Symbol *classSymbol, MethodSymbol *method, size_t methodCount)
    {
        Symbol *methodSymbol = createSymbol(METHOD_SYMBOL, method);
        size_t methodCap = classSymbol->type->methodCapacity;

        if (methodCount >= methodCap)
        {
            std::cout << "Expanding method capacity" << std::endl;
            classSymbol->type->methods = (Symbol **)realloc(classSymbol->type->methods, methodCap * 2 * sizeof(Symbol *));
            classSymbol->type->methodCapacity = methodCap * 2;
        }

        classSymbol->type->methods[methodCount] = methodSymbol;

        std::cout << "Method Added to Class Symbol" << std::endl;
        return classSymbol;
    }

    Symbol *GlobalSymbolTable::updateClassSymbolProperties(Symbol *classSymbol, PropertySymbol *property, size_t propertyCount)
    {
        Symbol *propertySymbol = createSymbol(PROPERTY_SYMBOL, property);
        size_t propCapacity = classSymbol->type->propertyCapacity;

        if (propertyCount >= propCapacity)
        {
            std::cout << "Expanding property capacity" << std::endl;
            classSymbol->type->properties = (Symbol **)realloc(classSymbol->type->properties, propCapacity * 2 * sizeof(Symbol *));
            classSymbol->type->propertyCapacity = propCapacity * 2;
        }

        classSymbol->type->properties[propertyCount] = propertySymbol;

        std::cout << "Property Added to Class Symbol" << std::endl;
        return classSymbol;
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

    // -------------------------------------------------------
    // Updates Symbol Table with Class Declaration

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
