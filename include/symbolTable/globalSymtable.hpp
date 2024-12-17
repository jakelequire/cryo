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

    typedef struct ASTNode ASTNode;

    // Opaque pointer type for C
    typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

    // C API
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(void);
    // void CryoGlobalSymbolTable_Destroy(CryoGlobalSymbolTable symTable);

    void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);
#define printGlobalSymbolTable(symTable) CryoGlobalSymbolTable_PrintGlobalTable(symTable)

    // Class State Functions ---------------------------------------

    bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
    bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);
#define isPrimaryTable(symTable) CryoGlobalSymbolTable_GetIsPrimaryTable(symTable)
#define isDependencyTable(symTable) CryoGlobalSymbolTable_GetIsDependencyTable(symTable)

    void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
    void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);
#define setPrimaryTableStatus(symTable, isPrimary) CryoGlobalSymbolTable_SetPrimaryTableStatus(symTable, isPrimary)
#define setDependencyTableStatus(symTable, isDependency) CryoGlobalSymbolTable_SetDependencyTableStatus(symTable, isDependency)

    // Symbol Table Functions ---------------------------------------
    void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
#define initDependencySymbolTable(symTable, namespaceName) CryoGlobalSymbolTable_InitDependencyTable(symTable, namespaceName)
    void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
#define createPrimarySymbolTable(symTable, namespaceName) CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)

    void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node);
#define addNodeToSymbolTable(symTable, node) CryoGlobalSymbolTable_AddNodeToSymbolTable(symTable, node)

    // Scope Functions ---------------------------------------
    void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name);
#define EnterScope(symTable, name) CryoGlobalSymbolTable_EnterScope(symTable, name)
    void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable);
#define ExitScope(symTable) CryoGlobalSymbolTable_ExitScope(symTable)

    const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name);
#define GetScopeID(symTable, name) CryoGlobalSymbolTable_GetScopeID(symTable, name)

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

#define MAX_SYMBOLS 1024
#define MAX_DEPENDENCIES 1024

/// @brief Debugging information for the symbol table.
/// Which includes: `buildDir`, `dependencyDir`, `debugDir`, and `DBdir`.
struct DebugInfo
{
    std::string buildDir;
    std::string dependencyDir;
    std::string debugDir;
    std::string DBdir;
};

/// @brief Context for the symbol table.
/// Which includes: `isPrimary`, and `isDependency` flags.
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

        // ======================================================= //
        // Public Variables & state for the symbol table           //
        // ======================================================= //

        DebugInfo debugInfo;       // Debugging information (build, dependency, debug, and DB directories)
        TableContext tableContext; // Context & stateful flags for the symbol table (isPrimary, isDependency)

        SymbolTable *symbolTable = nullptr;            // The main entry point namespace
        SymbolTable *currentDependencyTable = nullptr; // Runtime & other dependencies
        SymbolTable **dependencyTables;                // For C interfacing (Array of dependency tables)
        SymbolTable *stdImportTable = nullptr;         // The `using` keyword table for standard library imports
        TypesTable *typeTable = nullptr;               // Global types table

        size_t dependencyCount = 0;                   // For C interfacing
        size_t dependencyCapacity = MAX_DEPENDENCIES; // For C interfacing

        std::vector<SymbolTable *> dependencyTableVector; // For C++ interfacing

        ScopeBlock *currentScope = nullptr; // The current scope block
        const char *scopeId = "null";       // The current scope ID
        size_t scopeDepth = 0;              // The current scope depth

        // ======================================================= //
        // Public Functions for interfacing with the symbol table  //
        // ======================================================= //

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

        void setCurrentDependencyTable(SymbolTable *table) { currentDependencyTable = table; }
        void setPrimaryTable(SymbolTable *table) { symbolTable = table; }

        // ======================================================= //
        // Symbol Table Management Functions                       //
        // ======================================================= //

        void createPrimaryTable(const char *namespaceName);
        void initDependencyTable(const char *namespaceName);
        void addNodeToTable(ASTNode *node);
        void completeDependencyTable(void);

        // ======================================================= //
        // Scope Management Functions                              //
        // ======================================================= //

        const char *getScopeID(void) { return scopeId; }
        size_t getScopeDepth(void) { return scopeDepth; }

        void enterScope(const char *name);
        void exitScope(void);
        const char *getScopeID(const char *name);

        void initNamepsaceScope(const char *namespaceName);

        // ======================================================= //
        // Debug Functions other than the `SymbolTableDebugger`    //
        // ======================================================= //

        void printGlobalTable(GlobalSymbolTable *table);

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
        void addSingleSymbolToTable(Symbol *symbol, SymbolTable *table);

    protected:
        ScopeBlock *createScopeBlock(const char *name, size_t depth);
        VariableSymbol *createVariableSymbol(const char *name, DataType *type, ASTNode *node, const char *scopeId);
        FunctionSymbol *createFunctionSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node);
        TypeSymbol *createTypeSymbol(const char *name, DataType *type, TypeofDataType typeOf, bool isStatic, bool isGeneric, const char *scopeId);
        PropertySymbol *createPropertySymbol(const char *name, DataType *type, ASTNode *node, ASTNode *defaultExpr, bool hasDefaultExpr, bool isStatic, const char *scopeId);
        MethodSymbol *createMethodSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility, ASTNode *node, bool isStatic, const char *scopeId);
        ExternSymbol *createExternSymbol(const char *name, DataType *returnType, DataType **paramTypes, size_t paramCount, CryoNodeType nodeType, CryoVisibilityType visibility, const char *scopeId);

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

    inline void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->printGlobalTable(
                reinterpret_cast<GlobalSymbolTable *>(symTable));
        }
    }

    // Class State Functions ---------------------------------------

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

    // Symbol Table Functions ---------------------------------------

    inline void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initDependencyTable(namespaceName);
        }
    }

    inline void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->createPrimaryTable(namespaceName);
        }
    }

    inline void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addNodeToTable(node);
        }
    }

    // Scope Functions ---------------------------------------

    inline void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->enterScope(name);
        }
    }

    inline void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->exitScope();
        }
    }

    inline const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getScopeID(name);
        }
        return nullptr;
    }

} // namespace Cryo

#endif // C API -------------------------------------------------------
#endif // GLOBAL_SYMBOL_TABLE_H
