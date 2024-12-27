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
#pragma once

#include "frontend/AST.h"
#include "frontend/dataTypes.h"

#define MAX_SYMBOLS 1024

typedef struct ASTNode ASTNode;
typedef struct DataType DataType;
typedef struct Symbol Symbol;

typedef enum TypeOfSymbol
{
    VARIABLE_SYMBOL,
    FUNCTION_SYMBOL,
    EXTERN_SYMBOL,
    TYPE_SYMBOL,
    PROPERTY_SYMBOL,
    METHOD_SYMBOL,
    UNKNOWN_SYMBOL
} TypeOfSymbol;

struct ScopeBlock
{
    const char *namespaceName;
    const char *name;
    const char *id;
    size_t depth;

    // Scope hierarchy
    struct ScopeBlock *parent;
    struct ScopeBlock **children;
    size_t childCount;
    size_t childCapacity;
};

struct VariableSymbol
{
    const char *name;
    DataType *type;
    ASTNode *node;
    bool isParam;

    const char *scopeId;
};

struct FunctionSymbol
{
    const char *name;
    DataType *returnType;
    DataType **paramTypes;
    size_t paramCount;
    CryoVisibilityType visibility;
    ASTNode *node;

    const char *parentScopeID;
    const char *functionScopeId;
};

struct ExternSymbol
{
    const char *name;
    DataType *returnType;
    DataType **paramTypes;
    size_t paramCount;
    ASTNode *node;
    CryoNodeType nodeType;
    CryoVisibilityType visibility;

    const char *scopeId;
};

struct TypeSymbol
{
    const char *name;
    DataType *type;
    TypeofDataType typeOf;
    bool isStatic;
    bool isGeneric;
    ASTNode *node;
    Symbol **properties;
    int propertyCount;
    int propertyCapacity;
    Symbol **methods;
    int methodCount;
    int methodCapacity;

    const char *scopeId;
    const char *parentNameID;
};

struct PropertySymbol
{
    const char *name;
    DataType *type;
    ASTNode *node;
    ASTNode *defaultExpr;
    bool hasDefaultExpr;
    bool isStatic;

    const char *scopeId;
};

struct MethodSymbol
{
    const char *name;
    DataType *returnType;
    DataType **paramTypes;
    size_t paramCount;
    CryoVisibilityType visibility;
    ASTNode *node;
    bool isStatic;

    const char *scopeId;
};

struct Symbol
{
    TypeOfSymbol symbolType;
    union
    {
        VariableSymbol *variable;
        FunctionSymbol *function;
        ExternSymbol *externSymbol;
        TypeSymbol *type;
        PropertySymbol *property;
        MethodSymbol *method;
    };
};

struct SymbolTable
{
    const char *namespaceName;
    size_t count;
    size_t capacity;
    size_t scopeDepth;
    const char *scopeId;
    ScopeBlock *currentScope;
    Symbol **symbols;
};

struct TypesTable
{
    const char *namespaceName;
    size_t count;
    size_t capacity;
    size_t scopeDepth;
    const char *scopeId;
    ScopeBlock *currentScope;
    DataType **types;
    TypeSymbol **typeSymbols;
};
