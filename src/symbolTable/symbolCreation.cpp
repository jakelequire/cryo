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

    ScopeBlock *GlobalSymbolTable::createScopeBlock(const char *name, size_t depth)
    {
        ScopeBlock *block = new ScopeBlock();
        block->name = name;
        block->depth = depth;
        block->id = IDGen::generate32BitIntID();
        block->childCount = 0;
        block->childCapacity = 0;
        block->children = nullptr;
        block->parent = nullptr;
        return block;
    }

    VariableSymbol *GlobalSymbolTable::createVariableSymbol(const char *name, DataType *type, ASTNode *node, size_t scopeId)
    {
        VariableSymbol *symbol = new VariableSymbol();
        symbol->name = name;
        symbol->type = type;
        symbol->node = node;
        symbol->scopeId = scopeId;
        return symbol;
    }

    FunctionSymbol *GlobalSymbolTable::createFunctionSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node)
    {
        FunctionSymbol *symbol = new FunctionSymbol();
        symbol->name = name;
        symbol->returnType = returnType;
        symbol->paramTypes = paramTypes;
        symbol->paramCount = paramCount;
        symbol->visibility = visibility;
        symbol->node = node;
        symbol->scopeId = IDGen::generate32BitIntID();
        return symbol;
    }

    TypeSymbol *GlobalSymbolTable::createTypeSymbol(const char *name, DataType *type, TypeofDataType typeOf, bool isStatic, bool isGeneric, size_t scopeId)
    {
        TypeSymbol *symbol = new TypeSymbol();
        symbol->name = name;
        symbol->type = type;
        symbol->typeOf = typeOf;
        symbol->isStatic = isStatic;
        symbol->isGeneric = isGeneric;
        symbol->scopeId = scopeId;
        return symbol;
    }

    PropertySymbol *GlobalSymbolTable::createPropertySymbol(const char *name, DataType *type, ASTNode *node, ASTNode *defaultExpr, bool hasDefaultExpr, bool isStatic, size_t scopeId)
    {
        PropertySymbol *symbol = new PropertySymbol();
        symbol->name = name;
        symbol->type = type;
        symbol->node = node;
        symbol->defaultExpr = defaultExpr;
        symbol->hasDefaultExpr = hasDefaultExpr;
        symbol->isStatic = isStatic;
        symbol->scopeId = scopeId;
        return symbol;
    }

    MethodSymbol *GlobalSymbolTable::createMethodSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node, bool isStatic, size_t scopeId)
    {
        MethodSymbol *symbol = new MethodSymbol();
        symbol->name = name;
        symbol->returnType = returnType;
        symbol->paramTypes = paramTypes;
        symbol->paramCount = paramCount;
        symbol->visibility = visibility;
        symbol->node = node;
        symbol->isStatic = isStatic;
        symbol->scopeId = scopeId;
        return symbol;
    }

    Symbol *GlobalSymbolTable::createSymbol(TypeOfSymbol symbolType, void *symbol)
    {
        Symbol *symbolNode = new Symbol();
        symbolNode->symbolType = symbolType;
        switch (symbolType)
        {
        case VARIABLE_SYMBOL:
            symbolNode->variable = (VariableSymbol *)symbol;
            break;
        case FUNCTION_SYMBOL:
            symbolNode->function = (FunctionSymbol *)symbol;
            break;
        case TYPE_SYMBOL:
            symbolNode->type = (TypeSymbol *)symbol;
            break;
        case PROPERTY_SYMBOL:
            symbolNode->property = (PropertySymbol *)symbol;
            break;
        case METHOD_SYMBOL:
            symbolNode->method = (MethodSymbol *)symbol;
            break;
        default:
            break;
        }
        return symbolNode;
    }

    SymbolTable *GlobalSymbolTable::createSymbolTable(const char *namespaceName)
    {
        SymbolTable *table = new SymbolTable();
        table->namespaceName = namespaceName;
        table->count = 0;
        table->capacity = MAX_SYMBOLS;
        table->scopeDepth = 0;
        table->scopeId = IDGen::generate32BitIntID();
        table->currentScope = nullptr;
        table->symbols = (Symbol **)malloc(sizeof(Symbol *) * table->capacity);
        return table;
    }

    TypesTable *GlobalSymbolTable::createTypeTable(const char *namespaceName)
    {
        TypesTable *table = new TypesTable();
        table->namespaceName = namespaceName;
        table->count = 0;
        table->capacity = 0;
        table->scopeDepth = 0;
        table->scopeId = 0;
        table->currentScope = nullptr;
        table->types = nullptr;
        return table;
    }

}
