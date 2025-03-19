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

    ScopeBlock *GlobalSymbolTable::createScopeBlock(const char *name, size_t depth)
    {
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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

    FunctionSymbol *GlobalSymbolTable::createIncompleteFunctionSymbol(const char *name, const char *parentScopeID, DataType *returnType, DataType **paramTypes, size_t paramCount)
    {
        __STACK_FRAME__
        FunctionSymbol *symbol = new FunctionSymbol();
        symbol->name = name;
        symbol->returnType = returnType;
        symbol->paramTypes = paramTypes;
        symbol->paramCount = paramCount;
        symbol->visibility = VISIBILITY_PUBLIC;
        symbol->node = nullptr;
        symbol->functionScopeId = IDGen::generate64BitHashID(name);
        symbol->parentScopeID = parentScopeID;
        return symbol;
    }

    ExternSymbol *GlobalSymbolTable::createExternSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoNodeType nodeType, CryoVisibilityType visibility, const char *scopeId)
    {
        __STACK_FRAME__
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

    TypeSymbol *GlobalSymbolTable::createTypeSymbol(const char *name, ASTNode *node, DataType *type,
                                                    TypeofDataType typeOf, bool isStatic, bool isGeneric,
                                                    const char *scopeId)
    {
        __STACK_FRAME__
        if (!name)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Cannot create type symbol with null name");
            return nullptr;
        }

        TypeSymbol *symbol = (TypeSymbol *)malloc(sizeof(TypeSymbol));
        if (!symbol)
        {
            logMessage(LMI, "ERROR", "Symbol Table", "Failed to allocate memory for type symbol");
            return nullptr;
        }

        // Initialize all fields to avoid undefined behavior
        symbol->name = strdup(name);
        symbol->node = node;
        symbol->type = type;
        symbol->typeOf = typeOf;
        symbol->isStatic = isStatic;
        symbol->isGeneric = isGeneric;
        symbol->scopeId = scopeId ? strdup(scopeId) : nullptr;
        symbol->parentNameID = nullptr;

        // Initialize property and method arrays
        symbol->properties = (Symbol **)malloc(sizeof(Symbol *) * PROPERTY_CAPACITY);
        symbol->propertyCount = 0;
        symbol->propertyCapacity = PROPERTY_CAPACITY;

        symbol->methods = (Symbol **)malloc(sizeof(Symbol *) * METHOD_CAPACITY);
        symbol->methodCount = 0;
        symbol->methodCapacity = METHOD_CAPACITY;

        // Initialize generic type information
        symbol->isGenericType = isGeneric;
        if (isGeneric)
        {
            symbol->generics.typeParameters = nullptr;
            symbol->generics.paramCount = 0;
            symbol->generics.typeArguments = nullptr;
            symbol->generics.argCount = 0;
            symbol->generics.baseGenericType = nullptr;
        }

        logMessage(LMI, "INFO", "Symbol Table", "Created type symbol", "Name", name,
                   "Type", DTM->debug->dataTypeToString(type));
        return symbol;
    }

    TypeSymbol *GlobalSymbolTable::createIncompleteTypeSymbol(const char *name, TypeofDataType typeOf)
    {
        __STACK_FRAME__
        TypeSymbol *symbol = new TypeSymbol();
        symbol->name = name;
        symbol->type = nullptr; // Need to put a temp DataType here
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
        __STACK_FRAME__
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
        __STACK_FRAME__
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

        const char *methodName = method->name;
        const char *methodID = IDGen::generate64BitHashID(methodName);
        pushNewScopePair(methodName, methodID);

        return symbol;
    }

    Symbol *GlobalSymbolTable::createSymbol(TypeOfSymbol symbolType, void *symbol)
    {
        __STACK_FRAME__
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
        __STACK_FRAME__
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
        __STACK_FRAME__
        TypesTable *table = new TypesTable();
        table->namespaceName = namespaceName;
        table->count = 0;
        table->capacity = MAX_TYPE_SYMBOLS;
        table->scopeDepth = 0;
        table->scopeId = IDGen::generate64BitHashID(namespaceName);
        table->currentScope = nullptr;
        table->types = (DataType **)malloc(sizeof(DataType *) * MAX_TYPE_SYMBOLS);
        table->typeSymbols = (TypeSymbol **)malloc(sizeof(TypeSymbol *) * MAX_TYPE_SYMBOLS);
        return table;
    }

    bool GlobalSymbolTable::isParamSymbol(ASTNode *node)
    {
        __STACK_FRAME__
        CryoNodeType nodeType = node->metaData->type;
        if (nodeType == NODE_PARAM)
        {
            return true;
        }
        return false;
    }

}
