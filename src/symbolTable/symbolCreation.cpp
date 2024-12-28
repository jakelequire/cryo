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
        block->id = IDGen::generate64BitHashID(name);
        block->childCount = 0;
        block->childCapacity = 0;
        block->children = nullptr;
        block->parent = nullptr;
        return block;
    }

    VariableSymbol *GlobalSymbolTable::createVariableSymbol(const char *name, DataType *type, ASTNode *node, const char *scopeId)
    {
        VariableSymbol *symbol = new VariableSymbol();
        symbol->name = name;
        symbol->type = type;
        symbol->node = node;
        symbol->isParam = isParamSymbol(node);
        symbol->scopeId = scopeId;
        return symbol;
    }

    FunctionSymbol *GlobalSymbolTable::createFunctionSymbol(const char *name, const char *parentScopeID, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node)
    {
        FunctionSymbol *symbol = new FunctionSymbol();
        symbol->name = name;
        symbol->returnType = returnType;
        symbol->paramTypes = paramTypes;
        symbol->paramCount = paramCount;
        symbol->visibility = visibility;
        symbol->node = node;
        symbol->functionScopeId = IDGen::generate64BitHashID(name);
        symbol->parentScopeID = parentScopeID;
        return symbol;
    }

    ExternSymbol *GlobalSymbolTable::createExternSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoNodeType nodeType, CryoVisibilityType visibility, const char *scopeId)
    {
        ExternSymbol *symbol = new ExternSymbol();
        symbol->name = name;
        symbol->returnType = returnType;
        symbol->paramTypes = paramTypes;
        symbol->paramCount = paramCount;
        symbol->nodeType = nodeType;
        symbol->visibility = visibility;
        symbol->scopeId = scopeId;
        return symbol;
    }

    TypeSymbol *GlobalSymbolTable::createTypeSymbol(const char *name, ASTNode *node, DataType *type, TypeofDataType typeOf, bool isStatic, bool isGeneric, const char *scopeId)
    {
        TypeSymbol *symbol = new TypeSymbol();
        symbol->name = name;
        symbol->type = type;
        symbol->typeOf = typeOf;
        symbol->isStatic = isStatic;
        symbol->isGeneric = isGeneric;
        symbol->scopeId = scopeId;
        symbol->node = node;

        symbol->propertyCapacity = MAX_PROPERTY_COUNT;
        symbol->methodCapacity = MAX_METHOD_COUNT;
        symbol->propertyCount = 0;
        symbol->methodCount = 0;
        symbol->properties = (Symbol **)malloc(sizeof(Symbol *) * MAX_PROPERTY_COUNT);
        symbol->methods = (Symbol **)malloc(sizeof(Symbol *) * MAX_METHOD_COUNT);

        symbol->parentNameID = nullptr;

        return symbol;
    }

    TypeSymbol *GlobalSymbolTable::createIncompleteTypeSymbol(const char *name, TypeofDataType typeOf)
    {
        TypeSymbol *symbol = new TypeSymbol();
        symbol->name = name;
        symbol->type = nullptr;
        symbol->typeOf = typeOf;
        symbol->isStatic = false;
        symbol->isGeneric = false;
        symbol->scopeId = IDGen::generate64BitHashID(name);

        symbol->propertyCapacity = MAX_PROPERTY_COUNT;
        symbol->methodCapacity = MAX_METHOD_COUNT;
        symbol->propertyCount = 0;
        symbol->methodCount = 0;
        symbol->properties = (Symbol **)malloc(sizeof(Symbol *) * MAX_PROPERTY_COUNT);
        symbol->methods = (Symbol **)malloc(sizeof(Symbol *) * MAX_METHOD_COUNT);

        symbol->parentNameID = nullptr;

        return symbol;
    }

    PropertySymbol *GlobalSymbolTable::createPropertySymbol(ASTNode *propNode)
    {
        if (propNode->metaData->type != NODE_PROPERTY)
        {
            std::cout << "Error: Failed to create Property Symbol, typeof node mismatch." << std::endl;
            return nullptr;
        }
        PropertyNode *prop = propNode->data.property;
        PropertySymbol *symbol = new PropertySymbol();

        symbol->name = prop->name;
        symbol->type = prop->type;
        symbol->node = propNode;
        symbol->defaultExpr = nullptr;
        symbol->hasDefaultExpr = prop->defaultProperty;
        symbol->isStatic = false;
        symbol->scopeId = IDGen::generate64BitHashID(prop->parentName);
        return symbol;
    }

    MethodSymbol *GlobalSymbolTable::createMethodSymbol(ASTNode *methodNode)
    {
        if (methodNode->metaData->type != NODE_METHOD)
        {
            std::cout << "Error: Failed to create Method Symbol, typeof node mismatch" << std::endl;
            return nullptr;
        }

        MethodNode *method = methodNode->data.method;
        MethodSymbol *symbol = new MethodSymbol();

        symbol->name = (const char *)strdup(method->name);
        symbol->returnType = method->functionType;
        symbol->node = methodNode;
        symbol->visibility = method->visibility;
        symbol->isStatic = method->isStatic;
        symbol->scopeId = IDGen::generate64BitHashID(method->parentName);
        symbol->paramCount = method->paramCount;
        symbol->paramTypes = nullptr;

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
        case EXTERN_SYMBOL:
            symbolNode->externSymbol = (ExternSymbol *)symbol;
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
        table->scopeId = IDGen::generate64BitHashID(namespaceName);
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
        table->scopeId = IDGen::generate64BitHashID(namespaceName);
        table->currentScope = nullptr;
        table->types = nullptr;
        return table;
    }

    bool GlobalSymbolTable::isParamSymbol(ASTNode *node)
    {
        CryoNodeType nodeType = node->metaData->type;
        if (nodeType == NODE_PARAM)
        {
            return true;
        }
        return false;
    }

}
