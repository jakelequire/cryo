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
    typedef struct SymbolTable SymbolTable;
    typedef struct VariableSymbol VariableSymbol;
    typedef struct FunctionSymbol FunctionSymbol;
    typedef struct ExternSymbol ExternSymbol;
    typedef struct TypeSymbol TypeSymbol;
    typedef struct PropertySymbol PropertySymbol;
    typedef struct MethodSymbol MethodSymbol;
    typedef struct ScopeBlock ScopeBlock;

    // Opaque pointer type for C
    typedef struct CryoGlobalSymbolTable_t *CryoGlobalSymbolTable;

    // C API -------------------------------------------------------
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(void);
    void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);

    // Class State Functions ---------------------------------------

    bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
    bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);

    void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
    void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);

    void CryoGlobalSymbolTable_TableFinished(CryoGlobalSymbolTable *symTable);

    // Symbol Table Functions ---------------------------------------

    void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
    void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);

    void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node);
    void CryoGlobalSymbolTable_AddVariableToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *scopeID);

    SymbolTable *CryoGlobalSymbolTable_GetCurrentSymbolTable(CryoGlobalSymbolTable *symTable);

    // Scope Functions ---------------------------------------

    void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name);
    void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable);
    const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name);

    // Symbol Resolution Functions ---------------------------------------

    VariableSymbol *CryoGlobalSymbolTable_GetFrontendVariableSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID);

// Class State Functions
#define isPrimaryTable(symTable) \
    CryoGlobalSymbolTable_GetIsPrimaryTable(symTable)
#define isDependencyTable(symTable) \
    CryoGlobalSymbolTable_GetIsDependencyTable(symTable)
#define setPrimaryTableStatus(symTable, isPrimary) \
    CryoGlobalSymbolTable_SetPrimaryTableStatus(symTable, isPrimary)
#define setDependencyTableStatus(symTable, isDependency) \
    CryoGlobalSymbolTable_SetDependencyTableStatus(symTable, isDependency)
#define TableFinished(symTable) \
    CryoGlobalSymbolTable_TableFinished(symTable)

// Symbol Table Functions
#define initDependencySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_InitDependencyTable(symTable, namespaceName)
#define createPrimarySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)
#define addNodeToSymbolTable(symTable, node) \
    CryoGlobalSymbolTable_AddNodeToSymbolTable(symTable, node)

#define AddVariableToSymbolTable(symTable, node, scopeID) \
    CryoGlobalSymbolTable_AddVariableToSymbolTable(symTable, node, scopeID)
#define GetCurrentSymbolTable(symTable) \
    CryoGlobalSymbolTable_GetCurrentSymbolTable(symTable)

// Scope Functions
#define EnterScope(symTable, name) \
    CryoGlobalSymbolTable_EnterScope(symTable, name)
#define ExitScope(symTable) \
    CryoGlobalSymbolTable_ExitScope(symTable)
#define GetScopeID(symTable, name) \
    CryoGlobalSymbolTable_GetScopeID(symTable, name)

// Symbol Resolution Functions
#define GetFrontendVariableSymbol(symTable, name, scopeID) \
    CryoGlobalSymbolTable_GetFrontendVariableSymbol(symTable, name, scopeID)

// Debug Functions
#define printGlobalSymbolTable(symTable) \
    CryoGlobalSymbolTable_PrintGlobalTable(symTable)

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
#include "symbolTable/symTableDB.hpp"

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
    std::string rootDir;
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
        enum TABLE_STATE
        {
            TABLE_UNINITIALIZED,
            TABLE_INITIALIZED,
            TABLE_IDLE,
            TABLE_IN_PROGRESS,
            TABLE_COMPLETE,
            TABLE_ERROR
        };
        enum ScopeType
        {
            GLOBAL_SCOPE = 1000,
            NAMESPACE_SCOPE,
            FUNCTION_SCOPE,
            BLOCK_SCOPE,
            CLASS_SCOPE,
            METHOD_SCOPE,
            UNKNOWN_SCOPE = -1
        };
        GlobalSymbolTable(ScopeType scopeType = GLOBAL_SCOPE)
        {
            debugInfo = getDebugInfo();
            tableContext = setDefaultContext();
            currentScopeType = scopeType;
            tableState = TABLE_INITIALIZED;
        };
        ~GlobalSymbolTable();

        // ======================================================= //
        // Public Variables & state for the symbol table           //
        // ======================================================= //

        DebugInfo debugInfo;       // Debugging information (build, dependency, debug, and DB directories)
        TableContext tableContext; // Context & stateful flags for the symbol table (isPrimary, isDependency)

        ScopeType currentScopeType = UNKNOWN_SCOPE;    // The current scope type
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
            tableState = TABLE_IN_PROGRESS;
        }
        void setIsDependencyTable(bool isDependency)
        {
            tableContext.isDependency = isDependency;
            tableContext.isPrimary = !isDependency;
            tableState = TABLE_IN_PROGRESS;
        }

        void setCurrentDependencyTable(SymbolTable *table) { currentDependencyTable = table; }
        void setPrimaryTable(SymbolTable *table) { symbolTable = table; }
        void tableFinished(void)
        {
            tableState = TABLE_COMPLETE;
            if (tableContext.isPrimary)
            {
                if (symbolTable)
                {
                    db->appendSerializedTable(symbolTable);
                }
            }
            else if (tableContext.isDependency)
            {
                if (currentDependencyTable && dependencyCount == 1)
                {
                    db->serializeSymbolTable(currentDependencyTable);
                }
            }
            else
            {
                tableState = TABLE_ERROR;
            }
        }

        // ======================================================= //
        // Symbol Table Management Functions                       //
        // ======================================================= //

        void createPrimaryTable(const char *namespaceName);
        void initDependencyTable(const char *namespaceName);
        void addNodeToTable(ASTNode *node);
        void completeDependencyTable(void);
        void addVariableToSymbolTable(ASTNode *node, const char *scopeID);

        SymbolTable *getCurrentSymbolTable(void);

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
        // Symbol Retrieval Functions                              //
        // ======================================================= //

        VariableSymbol *getFrontendVariableSymbol(const char *name, const char *scopeID);
        MethodSymbol *getFrontendMethodSymbol(const char *methodName, const char *className, const char *scopeID);

        // ======================================================= //
        // Debug Functions other than the `SymbolTableDebugger`    //
        // ======================================================= //

        void printGlobalTable(GlobalSymbolTable *table);

    private:
        TABLE_STATE tableState = TABLE_UNINITIALIZED;
        std::unique_ptr<SymbolTableDB> db = std::make_unique<SymbolTableDB>(debugInfo.rootDir);
        SymbolTableDebugger *debugger = new SymbolTableDebugger();
        DebugInfo getDebugInfo(void)
        {
            std::string rootDir = getCryoRootPath();
            std::string buildDir = rootDir + "build";
            std::string dependencyDir = buildDir + "/out/deps";
            std::string DBdir = buildDir + "/db";
            std::string debugDir = buildDir + "/debug";

            DebugInfo info = {buildDir, dependencyDir, debugDir, DBdir, rootDir};
            return info;
        }
        TableContext setDefaultContext(void) { return {false, false}; }

        Symbol *ASTNodeToSymbol(ASTNode *node);
        const char *getRootNamespace(ASTNode *root);

        // -------------------------------------------------------
        // Node Setters

        void addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table);
        void addSingleSymbolToTable(Symbol *symbol, SymbolTable *table);

    protected:
        // ======================================================= //
        // Symbol Creation Functions                               //
        // ======================================================= //

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

    inline void CryoGlobalSymbolTable_TableFinished(CryoGlobalSymbolTable *symTable)
    {
        reinterpret_cast<GlobalSymbolTable *>(symTable)->tableFinished();
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

    inline SymbolTable *CryoGlobalSymbolTable_GetCurrentSymbolTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getCurrentSymbolTable();
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_AddVariableToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *scopeID)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addVariableToSymbolTable(node, scopeID);
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

    // Symbol Resolution Functions ---------------------------------------

    inline VariableSymbol *CryoGlobalSymbolTable_GetFrontendVariableSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getFrontendVariableSymbol(name, scopeID);
        }
        return nullptr;
    }

} // namespace Cryo

#endif // C API -------------------------------------------------------
#endif // GLOBAL_SYMBOL_TABLE_H
