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

#include "symbolTable/symdefs.h"

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

    void CryoGlobalSymbolTable_InitNamespace(CryoGlobalSymbolTable *symTable, const char *namespaceName);

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

    // Debug Functions ---------------------------------------

    void CryoGlobalSymbolTable_MergeDBChunks(CryoGlobalSymbolTable *symTable);

    // Declaration Functions (incomplete definitions) ---------------------------------------

    void CryoGlobalSymbolTable_InitClassDeclaration(CryoGlobalSymbolTable *symTable, const char *className);
    void CryoGlobalSymbolTable_AddPropertyToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *property);
    void CryoGlobalSymbolTable_AddMethodToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method);

    // Symbol Resolution Functions ---------------------------------------
    Symbol *CryoGlobalSymbolTable_GetFrontendSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID, TypeOfSymbol symbolType);
    FunctionSymbol *CryoGlobalSymbolTable_ResolveScopedFunctionSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID);

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
#define InitNamespace(symTable, namespaceName) \
    CryoGlobalSymbolTable_InitNamespace(symTable, namespaceName)

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
#define GetFrontendScopedFunctionSymbol(symTable, name, scopeID) \
    CryoGlobalSymbolTable_ResolveScopedFunctionSymbol(symTable, name, scopeID)

// Debug Functions
#define printGlobalSymbolTable(symTable) \
    CryoGlobalSymbolTable_PrintGlobalTable(symTable)
#define MergeDBChunks(symTable) \
    CryoGlobalSymbolTable_MergeDBChunks(symTable)

// Declaration Functions
#define InitClassDeclaration(symTable, className) \
    CryoGlobalSymbolTable_InitClassDeclaration(symTable, className)
#define AddPropertyToClass(symTable, className, property) \
    CryoGlobalSymbolTable_AddPropertyToClass(symTable, className, property)
#define AddMethodToClass(symTable, className, method) \
    CryoGlobalSymbolTable_AddMethodToClass(symTable, className, method)

// Symbol Resolution Functions
#define GetFrontendSymbol(symTable, name, scopeID, symbolType) \
    CryoGlobalSymbolTable_GetFrontendSymbol(symTable, name, scopeID, symbolType)

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
#define MAX_PROPERTY_COUNT 128
#define MAX_METHOD_COUNT 128

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
            TABLE_DONE,
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

        // Constructor & Destructor
        GlobalSymbolTable(ScopeType scopeType = GLOBAL_SCOPE)
        {
            debugInfo = getDebugInfo();
            tableContext = setDefaultContext();
            currentScopeType = scopeType;
            tableState = TABLE_INITIALIZED;
        };
        ~GlobalSymbolTable();

        friend class SymbolTableDebugger;
        friend class SymbolTableDB;

        //===================================================================
        // Public Member Variables
        //===================================================================

        DebugInfo debugInfo;                        // Debugging information for the symbol table (buildDir, dependencyDir, debugDir, DBdir)
        TableContext tableContext;                  // Context for the symbol table (isPrimary, isDependency)
        ScopeType currentScopeType = UNKNOWN_SCOPE; // Current scope type

        SymbolTable *symbolTable = nullptr;            // Primary symbol table
        SymbolTable *currentDependencyTable = nullptr; // Current dependency table
        SymbolTable **dependencyTables = nullptr;      // Array of dependency tables (for multiple dependencies)
        SymbolTable *stdImportTable = nullptr;         // Standard library import table
        TypesTable *typeTable = nullptr;               // Global type table
        size_t dependencyCount = 0;                    // Number of dependencies (C interface)
        size_t dependencyCapacity = MAX_DEPENDENCIES;  // Maximum number of dependencies

        std::vector<SymbolTable *> dependencyTableVector; // Vector of dependency tables (C++ interface)
        std::vector<FunctionSymbol *> globalFunctions;    // Vector of global functions
        std::vector<ExternSymbol *> externFunctions;      // Vector of external functions

        ScopeBlock *currentScope = nullptr; // Current scope block
        const char *scopeId = "null";       // Current scope ID
        size_t scopeDepth = 0;              // Current scope depth

        //===================================================================
        // Table State Management
        //===================================================================

        bool getIsPrimaryTable(void);
        bool getIsDependencyTable(void);
        void setIsPrimaryTable(bool isPrimary);
        void setIsDependencyTable(bool isDependency);
        void resetCurrentDepsTable(void);
        void setCurrentDependencyTable(SymbolTable *table);
        void setPrimaryTable(SymbolTable *table);
        void tableFinished(void);
        void initNamespace(const char *namespaceName);

        //===================================================================
        // Symbol Table Operations
        //===================================================================

        void createPrimaryTable(const char *namespaceName);
        void initDependencyTable(const char *namespaceName);
        void addNodeToTable(ASTNode *node);
        void completeDependencyTable(void);
        void addVariableToSymbolTable(ASTNode *node, const char *scopeID);
        void addGlobalFunctionToTable(FunctionSymbol *function);
        void addExternFunctionToTable(ExternSymbol *function);

        SymbolTable *getCurrentSymbolTable(void);
        Symbol *queryCurrentTable(const char *scopeID, const char *name, TypeOfSymbol symbolType);
        Symbol *querySpecifiedTable(const char *symbolName, TypeOfSymbol symbolType, SymbolTable *table);
        Symbol *getFrontendSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType);

        //===================================================================
        // Scope Management
        //===================================================================

        const char *getScopeID(void) { return scopeId; }
        size_t getScopeDepth(void) { return scopeDepth; }
        void enterScope(const char *name);
        void exitScope(void);
        const char *getScopeID(const char *name);
        void setScopeID(const char *id) { scopeId = id; }
        void initNamepsaceScope(const char *namespaceName);

        //===================================================================
        // Symbol Resolution
        //===================================================================

        VariableSymbol *getFrontendVariableSymbol(const char *name, const char *scopeID);
        MethodSymbol *getFrontendMethodSymbol(const char *methodName, const char *className, const char *scopeID);
        Symbol *resolveFunctionSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType);
        Symbol *resolveExternSymbol(const char *symbolName);
        FunctionSymbol *resolveScopedFunctionCall(const char *scopeID, const char *functionName);
        SymbolTable *findSymbolTable(const char *scopeID);
        Symbol *seekFunctionSymbolInAllTables(const char *symbolName);

        //===================================================================
        // Class Declaration Management
        //===================================================================

        void initClassDeclaration(const char *className);
        void addPropertyToClass(const char *className, ASTNode *property);
        void addMethodToClass(const char *className, ASTNode *method);

        //===================================================================
        // Debug Operations
        //===================================================================

        void printGlobalTable(GlobalSymbolTable *table);
        void mergeDBChunks(void);

    private:
        TABLE_STATE tableState = TABLE_UNINITIALIZED;
        std::unique_ptr<SymbolTableDB> db = std::make_unique<SymbolTableDB>(debugInfo.rootDir);
        SymbolTableDebugger *debugger = new SymbolTableDebugger();

        //===================================================================
        // Private Helper Functions
        //===================================================================
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
        void addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table);
        void addSingleSymbolToTable(Symbol *symbol, SymbolTable *table);
        void addSymbolToCurrentTable(Symbol *symbol);
        bool isParamSymbol(ASTNode *node);

    protected:
        //===================================================================
        // Symbol Creation
        //===================================================================
        ScopeBlock *createScopeBlock(const char *name, size_t depth);
        VariableSymbol *createVariableSymbol(const char *name, DataType *type, ASTNode *node, const char *scopeId);
        FunctionSymbol *createFunctionSymbol(const char *name, const char *parentScopeID, DataType *returnType,
                                             DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility,
                                             ASTNode *node);
        TypeSymbol *createTypeSymbol(const char *name, DataType *type, TypeofDataType typeOf,
                                     bool isStatic, bool isGeneric, const char *scopeId);
        TypeSymbol *createIncompleteTypeSymbol(const char *name, TypeofDataType typeOf);
        PropertySymbol *createPropertySymbol(ASTNode *propNode);
        MethodSymbol *createMethodSymbol(ASTNode *methodNode);
        ExternSymbol *createExternSymbol(const char *name, DataType *returnType, DataType **paramTypes,
                                         size_t paramCount, CryoNodeType nodeType, CryoVisibilityType visibility,
                                         const char *scopeId);
        Symbol *createSymbol(TypeOfSymbol symbolType, void *symbol);
        SymbolTable *createSymbolTable(const char *namespaceName);
        TypesTable *createTypeTable(const char *namespaceName);
        void processParamList(ASTNode **node, int paramCount, const char *scopeID);

        //===================================================================
        // Class Symbol Management
        //===================================================================
        Symbol *createClassDeclarationSymbol(const char *className);
        void updateClassSymbolMethods(Symbol *classSymbol, MethodSymbol *method, size_t methodCount);
        void updateClassSymbolProperties(Symbol *classSymbol, PropertySymbol *property, size_t propertyCount);
        Symbol *getClassSymbol(const char *className);
        void addClassDeclarationToTable(Symbol *classSymbol, SymbolTable *table);
        void updateClassSymbol(Symbol *classSymbol, SymbolTable *table);
    };

    // ========================================================================================== //
    // ========================================================================================== //
    // ========================================================================================== //

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

    // Debug Functions ---------------------------------------

    inline void CryoGlobalSymbolTable_MergeDBChunks(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->mergeDBChunks();
        }
    }

    // Declaration Functions ---------------------------------------

    inline void CryoGlobalSymbolTable_InitClassDeclaration(CryoGlobalSymbolTable *symTable, const char *className)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initClassDeclaration(className);
        }
    }

    inline void CryoGlobalSymbolTable_AddPropertyToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *property)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addPropertyToClass(className, property);
        }
    }

    inline void CryoGlobalSymbolTable_AddMethodToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addMethodToClass(className, method);
        }
    }

    // Symbol Resolution Functions ---------------------------------------

    inline Symbol *CryoGlobalSymbolTable_GetFrontendSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID, TypeOfSymbol symbolType)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getFrontendSymbol(name, scopeID, symbolType);
        }
        return nullptr;
    }

} // namespace Cryo

#endif // C API -------------------------------------------------------
#endif // GLOBAL_SYMBOL_TABLE_H
