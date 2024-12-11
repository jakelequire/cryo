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
#ifndef GLOBAL_SYMBOL_TABLE_H
#define GLOBAL_SYMBOL_TABLE_H

#ifdef __cplusplus
extern "C"
{
#endif

    // Opaque pointer type for C
    typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

    // C API
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(void);
    // void CryoGlobalSymbolTable_Destroy(CryoGlobalSymbolTable symTable);

    void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);

    bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
    bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);
#define isPrimaryTable(symTable) CryoGlobalSymbolTable_GetIsPrimaryTable(symTable)
#define isDependencyTable(symTable) CryoGlobalSymbolTable_GetIsDependencyTable(symTable)

    void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
    void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);
#define setPrimaryTableStatus(symTable, isPrimary) CryoGlobalSymbolTable_SetPrimaryTableStatus(symTable, isPrimary)
#define setDependencyTableStatus(symTable, isDependency) CryoGlobalSymbolTable_SetDependencyTableStatus(symTable, isDependency)

    void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
    void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable);
#define createPrimarySymbolTable(symTable, namespaceName) CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)
#define initDependencySymbolTable(symTable) CryoGlobalSymbolTable_InitDependencyTable(symTable)

#ifdef __cplusplus
} // C API ----------------------------------------------------------

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>

#include "symbolTable/symdefs.h"
#include "symbolTable/debugSymbols.hpp"
#include "tools/utils/env.h" // getCryoRootPath()
#include "tools/cxx/IDGen.hpp"

typedef struct ASTNode ASTNode;
typedef struct DataType DataType;

struct DebugInfo
{
    std::string buildDir;
    std::string dependencyDir;
    std::string debugDir;
    std::string DBdir;
};

struct TableContext
{
    bool isPrimary;
    bool isDependency;
};

namespace Cryo
{
    class GlobalSymbolTable;
    class SymbolTableDebugger;

    class GlobalSymbolTable
    {
    public:
        GlobalSymbolTable()
        {
            debugInfo = getDebugInfo();
            tableContext = setDefaultContext();
        };
        ~GlobalSymbolTable();

        DebugInfo debugInfo;
        TableContext tableContext;

        SymbolTable *symbolTable = nullptr;
        TypesTable *typeTable = nullptr;

        SymbolTable **dependencyTables; // For C interfacing
        size_t dependencyCount = 0;     //
        size_t dependencyCapacity = 0;  //

        std::vector<SymbolTable *> dependencyTableVector; // Easier for C++ to manage

        bool getIsPrimaryTable(void) { return tableContext.isPrimary; }
        bool getIsDependencyTable(void) { return tableContext.isDependency; }

        void setIsPrimaryTable(bool isPrimary)
        {
            tableContext.isPrimary = isPrimary;
            tableContext.isDependency = !isPrimary;
        }
        void setIsDependencyTable(bool isDependency)
        {
            tableContext.isDependency = isDependency;
            tableContext.isPrimary = !isDependency;
        }

        // -------------------------------------------------------

        void printGlobalTable(GlobalSymbolTable *table);

        void createPrimaryTable(const char *namespaceName);
        SymbolTable *initDependencyTable(void);

        SymbolTable *processProgramNode(ASTNode *node);

    private:
        SymbolTableDebugger *debugger = new SymbolTableDebugger();
        DebugInfo getDebugInfo(void)
        {
            std::string rootDir = getCryoRootPath();
            std::string buildDir = rootDir + "build";
            std::string dependencyDir = buildDir + "/out/deps";
            std::string DBdir = buildDir + "/db";
            std::string debugDir = buildDir + "/debug";

            return {buildDir, dependencyDir, debugDir, DBdir};
        }
        TableContext setDefaultContext(void) { return {false, false}; }

        Symbol *ASTNodeToSymbol(ASTNode *node);
        const char *getRootNamespace(ASTNode *root);

        // -------------------------------------------------------
        // Node Setters

        void addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table);

    protected:
        ScopeBlock *createScopeBlock(const char *name, size_t depth);
        VariableSymbol *createVariableSymbol(const char *name, DataType *type, ASTNode *node, size_t scopeId);
        FunctionSymbol *createFunctionSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node, size_t scopeId);
        TypeSymbol *createTypeSymbol(const char *name, DataType *type, TypeofDataType typeOf, bool isStatic, bool isGeneric, size_t scopeId);
        PropertySymbol *createPropertySymbol(const char *name, DataType *type, ASTNode *node, ASTNode *defaultExpr, bool hasDefaultExpr, bool isStatic, size_t scopeId);
        MethodSymbol *createMethodSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node, bool isStatic, size_t scopeId);

        Symbol *createSymbol(TypeOfSymbol symbolType, void *symbol);
        SymbolTable *createSymbolTable(const char *namespaceName);
        TypesTable *createTypeTable(const char *namespaceName);
    };

    // -------------------------------------------------------
    // C API Implementation
    inline CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create()
    {
        return reinterpret_cast<CryoGlobalSymbolTable *>(new GlobalSymbolTable());
    }

    // inline void CryoGlobalSymbolTable_Destroy(CryoGlobalSymbolTable symTable)
    // {
    //     delete reinterpret_cast<GlobalSymbolTable *>(symTable);
    // }

    inline void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->printGlobalTable(
                reinterpret_cast<GlobalSymbolTable *>(symTable));
        }
    }

    inline bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable)
    {
        return reinterpret_cast<GlobalSymbolTable *>(symTable)->getIsPrimaryTable();
    }

    inline bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable)
    {
        return reinterpret_cast<GlobalSymbolTable *>(symTable)->getIsDependencyTable();
    }

    inline void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary)
    {
        reinterpret_cast<GlobalSymbolTable *>(symTable)->setIsPrimaryTable(isPrimary);
    }

    inline void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency)
    {
        reinterpret_cast<GlobalSymbolTable *>(symTable)->setIsDependencyTable(isDependency);
    }

} // namespace Cryo

#endif // C API -------------------------------------------------------
#endif // GLOBAL_SYMBOL_TABLE_H
