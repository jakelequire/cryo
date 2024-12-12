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

typedef enum TypeOfSymbol
{
    VARIABLE_SYMBOL,
    FUNCTION_SYMBOL,
    TYPE_SYMBOL,
    PROPERTY_SYMBOL,
    METHOD_SYMBOL,
    UNKNOWN_SYMBOL
} TypeOfSymbol;

struct ScopeBlock
{
    const char *namespaceName;
    const char *name;
    size_t id;
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

    size_t scopeId;
};

struct FunctionSymbol
{
    const char *name;
    DataType *returnType;
    DataType **paramTypes;
    size_t paramCount;
    CryoVisibilityType visibility;
    ASTNode *node;

    size_t scopeId;
};

struct TypeSymbol
{
    const char *name;
    DataType *type;
    TypeofDataType typeOf;
    bool isStatic;
    bool isGeneric;

    size_t scopeId;
};

struct PropertySymbol
{
    const char *name;
    DataType *type;
    ASTNode *node;
    ASTNode *defaultExpr;
    bool hasDefaultExpr;
    bool isStatic;

    size_t scopeId;
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

    size_t scopeId;
};

struct Symbol
{
    TypeOfSymbol symbolType;
    union
    {
        VariableSymbol *variable;
        FunctionSymbol *function;
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
    size_t scopeId;
    ScopeBlock *currentScope;
    Symbol **symbols;
};

struct TypesTable
{
    const char *namespaceName;
    size_t count;
    size_t capacity;
    size_t scopeDepth;
    size_t scopeId;
    ScopeBlock *currentScope;
    DataType **types;
};
