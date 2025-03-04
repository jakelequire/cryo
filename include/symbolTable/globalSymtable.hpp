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
#include "tools/utils/c_logger.h"

    typedef struct ASTNode ASTNode;
    typedef struct DataType DataType;
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
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(const char *buildDir);
    CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create_Reaping(bool forReaping, const char *buildDir);
    void CryoGlobalSymbolTable_PrintGlobalTable(CryoGlobalSymbolTable *symTable);

    // Class State Functions ---------------------------------------

    bool CryoGlobalSymbolTable_GetIsPrimaryTable(CryoGlobalSymbolTable *symTable);
    bool CryoGlobalSymbolTable_GetIsDependencyTable(CryoGlobalSymbolTable *symTable);
    void CryoGlobalSymbolTable_SetPrimaryTableStatus(CryoGlobalSymbolTable *symTable, bool isPrimary);
    void CryoGlobalSymbolTable_SetDependencyTableStatus(CryoGlobalSymbolTable *symTable, bool isDependency);
    void CryoGlobalSymbolTable_TableFinished(CryoGlobalSymbolTable *symTable);
    void CryoGlobalSymbolTable_InitNamespace(CryoGlobalSymbolTable *symTable, const char *namespaceName);
    void CryoGlobalSymbolTable_CompleteFrontend(CryoGlobalSymbolTable *symTable);
    void CryoGlobalSymbolTable_SetLinker(CryoGlobalSymbolTable *symTable, CryoLinker *linker);
    CryoLinker *CryoGlobalSymbolTable_GetLinker(CryoGlobalSymbolTable *symTable);

    // Symbol Table Functions ---------------------------------------

    void CryoGlobalSymbolTable_InitDependencyTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
    void CryoGlobalSymbolTable_CreatePrimaryTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);

    void CryoGlobalSymbolTable_AddNodeToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node);
    void CryoGlobalSymbolTable_AddVariableToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *scopeID);
    void CryoGlobalSymbolTable_AddParamToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *functionScopeID);

    TypeOfSymbol CryoGlobalSymbolTable_GetScopeSymbolTypeFromName(CryoGlobalSymbolTable *symTable, const char *symbolName);
    TypeofDataType getTypeOfDataTypeFromName(CryoGlobalSymbolTable *symTable, const char *symbolName);
    SymbolTable *CryoGlobalSymbolTable_GetCurrentSymbolTable(CryoGlobalSymbolTable *symTable);
    SymbolTable *CryoGlobalSymbolTable_GetReapedTable(CryoGlobalSymbolTable *symTable);

    void CryoGlobalSymbolTable_HandleRootNodeImport(CryoGlobalSymbolTable *symTable, ASTNode *node);
    void CryoGlobalSymbolTable_ImportReapedTable(CryoGlobalSymbolTable *symTable, SymbolTable *reapedTable);
    TypesTable *CryoGlobalSymbolTable_GetReapedTypesTable(CryoGlobalSymbolTable *symTable);
    void CryoGlobalSymbolTable_ImportReapedTypesTable(CryoGlobalSymbolTable *symTable, TypesTable *reapedTable);
    SymbolTable *CryoGlobalSymbolTable_GetSpecificSymbolTable(CryoGlobalSymbolTable *symTable, const char *namespaceName);
    void CryoGlobalSymbolTable_ImportRuntimeSymbols(CryoGlobalSymbolTable *symTable, SymbolTable *runtimeTable);

    DataType *CryoGlobalSymbolTable_ResolveDataType(CryoGlobalSymbolTable *symTable, const char *name);
    void CryoGlobalSymbolTable_AddTypeToTable(CryoGlobalSymbolTable *symTable, TypeSymbol *typeSymbol);
    TypeSymbol *CryoGlobalSymbolTable_CreateTypeSymbol(CryoGlobalSymbolTable *symTable, const char *name, ASTNode *node, DataType *type, TypeofDataType typeOf, bool isStatic, bool isGeneric, const char *scopeId);

    void CryoGlobalSymbolTable_CleanupAndDestroySymbolTable(CryoGlobalSymbolTable *symTable);

    const char *CryoGlobalSymbolTable_GetBuildDir(CryoGlobalSymbolTable *symTable);

    // Scope Functions ---------------------------------------

    void CryoGlobalSymbolTable_EnterScope(CryoGlobalSymbolTable *symTable, const char *name);
    void CryoGlobalSymbolTable_ExitScope(CryoGlobalSymbolTable *symTable);
    const char *CryoGlobalSymbolTable_GetScopeID(CryoGlobalSymbolTable *symTable, const char *name);

    // Symbol Resolution Functions ---------------------------------------

    VariableSymbol *CryoGlobalSymbolTable_GetFrontendVariableSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID);

    ASTNode *CryoGlobalSymbolTable_FindClassProperty(CryoGlobalSymbolTable *symTable, const char *propertyName, const char *className);
    ASTNode *CryoGlobalSymbolTable_FindClassMethod(CryoGlobalSymbolTable *symTable, const char *methodName, const char *className);

    // Debug Functions ---------------------------------------

    void CryoGlobalSymbolTable_MergeDBChunks(CryoGlobalSymbolTable *symTable);
    const char *CryoGlobalSymbolTable_TypeOfSymbolToString(CryoGlobalSymbolTable *symTable, TypeOfSymbol symbolType);
    void CryoGlobalSymbolTable_LogSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol);
    const char *CryoGlobalSymbolTable_GetDependencyDirStr(CryoGlobalSymbolTable *symTable);
    void CryoGlobalSymbolTable_PrintSymbolTable(CryoGlobalSymbolTable *symTable, SymbolTable *table);
    void CryoGlobalSymbolTable_PrintTypeTable(CryoGlobalSymbolTable *symTable);

    // Declaration Functions  ---------------------------------------

    // Class Declarations
    void CryoGlobalSymbolTable_InitClassDeclaration(CryoGlobalSymbolTable *symTable, const char *className);
    void CryoGlobalSymbolTable_AddPropertyToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *property);
    void CryoGlobalSymbolTable_AddMethodToClass(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method);
    void CryoGlobalSymbolTable_CompleteClassDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *classNode, const char *className);

    // Struct Declarations
    void CryoGlobalSymbolTable_InitStructDeclaration(CryoGlobalSymbolTable *symTable, const char *structName, const char *parentName, DataType *structPtr);
    void CryoGlobalSymbolTable_InitGenericStructDeclaration(CryoGlobalSymbolTable *symTable, const char *structName, const char *parentName);
    void CryoGlobalSymbolTable_AddPropertyToStruct(CryoGlobalSymbolTable *symTable, const char *structName, ASTNode *property);
    void CryoGlobalSymbolTable_AddMethodToStruct(CryoGlobalSymbolTable *symTable, const char *className, ASTNode *method);
    void CryoGlobalSymbolTable_CompleteStructDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *structNode, const char *structName);

    // Function Declarations
    void CryoGlobalSymbolTable_InitFunctionDeclaration(CryoGlobalSymbolTable *symTable, const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType);
    void CryoGlobalSymbolTable_InitGenericFunctionDeclaration(CryoGlobalSymbolTable *symTable, const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType, GenericType **genericParams, int genericParamCount);
    void CryoGlobalSymbolTable_CompleteFunctionDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *functionNode, const char *functionName, const char *parentScopeID);
    void CryoGlobalSymbolTable_AddExternFunctionToTable(CryoGlobalSymbolTable *symTable, ASTNode *externNode, const char *namespaceScopeID);

    // Symbol Resolution Functions ---------------------------------------
    Symbol *CryoGlobalSymbolTable_GetFrontendSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID, TypeOfSymbol symbolType);
    FunctionSymbol *CryoGlobalSymbolTable_ResolveScopedFunctionSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID);
    DataType *CryoGlobalSymbolTable_GetDataTypeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol);
    Symbol *CryoGlobalSymbolTable_FindSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID);
    ASTNode *CryoGlobalSymbolTable_GetASTNodeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol);
    Symbol *CryoGlobalSymbolTable_FindMethodSymbol(CryoGlobalSymbolTable *symTable, const char *methodName, const char *className, TypeofDataType typeOfNode);

    const char *CryoGlobalSymbolTable_GetNamespace(CryoGlobalSymbolTable *symTable);

    bool CryoGlobalSymbolTable_IsStructSymbol(CryoGlobalSymbolTable *symTable, const char *name);

    // Misc Functions ---------------------------------------
    ASTNode **CryoGlobalSymbolTable_MergeAllSymbolsToASTArray(CryoGlobalSymbolTable *symTable);

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
#define CompleteFrontend(symTable) \
    CryoGlobalSymbolTable_CompleteFrontend(symTable)
#define GetBuildDir(symTable) \
    CryoGlobalSymbolTable_GetBuildDir(symTable)
#define SetGSTLinker(symTable, linker) \
    CryoGlobalSymbolTable_SetLinker(symTable, linker)
#define GetGSTLinker(symTable) \
    CryoGlobalSymbolTable_GetLinker(symTable)

// Symbol Table Functions
#define initDependencySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_InitDependencyTable(symTable, namespaceName)
#define createPrimarySymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_CreatePrimaryTable(symTable, namespaceName)
#define addNodeToSymbolTable(symTable, node) \
    CryoGlobalSymbolTable_AddNodeToSymbolTable(symTable, node)

#define AddVariableToSymbolTable(symTable, node, scopeID) \
    CryoGlobalSymbolTable_AddVariableToSymbolTable(symTable, node, scopeID)
#define AddParamToSymbolTable(symTable, node, functionScopeID) \
    CryoGlobalSymbolTable_AddParamToSymbolTable(symTable, node, functionScopeID)
#define GetCurrentSymbolTable(symTable) \
    CryoGlobalSymbolTable_GetCurrentSymbolTable(symTable)

#define GetScopeSymbolTypeFromName(symTable, symbolName) \
    CryoGlobalSymbolTable_GetScopeSymbolTypeFromName(symTable, symbolName)
#define GetTypeOfDataTypeFromName(symTable, symbolName) \
    getTypeOfDataTypeFromName(symTable, symbolName)

#define HandleRootNodeImport(symTable, node) \
    CryoGlobalSymbolTable_HandleRootNodeImport(symTable, node)

#define GetReapedTable(symTable) \
    CryoGlobalSymbolTable_GetReapedTable(symTable)
#define GetReapedTypeTable(symTable) \
    CryoGlobalSymbolTable_GetReapedTypesTable(symTable)
#define ImportReapedTable(symTable, reapedTable) \
    CryoGlobalSymbolTable_ImportReapedTable(symTable, reapedTable)
#define ImportReapedTypesTable(symTable, reapedTable) \
    CryoGlobalSymbolTable_ImportReapedTypesTable(symTable, reapedTable)

#define GetSpecificSymbolTable(symTable, namespaceName) \
    CryoGlobalSymbolTable_GetSpecificSymbolTable(symTable, namespaceName)

#define ResolveDataType(symTable, name) \
    CryoGlobalSymbolTable_ResolveDataType(symTable, name)
#define AddTypeToTable(symTable, typeSymbol) \
    CryoGlobalSymbolTable_AddTypeToTable(symTable, typeSymbol)
#define CreateTypeSymbol(symTable, name, node, type, typeOf, isStatic, isGeneric, scopeId) \
    CryoGlobalSymbolTable_CreateTypeSymbol(symTable, name, node, type, typeOf, isStatic, isGeneric, scopeId)

#define CleanupAndDestroySymbolTable(symTable) \
    CryoGlobalSymbolTable_CleanupAndDestroySymbolTable(symTable)

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
#define GetDataTypeFromSymbol(symTable, symbol) \
    CryoGlobalSymbolTable_GetDataTypeFromSymbol(symTable, symbol)
#define FindSymbol(symTable, name, scopeID) \
    CryoGlobalSymbolTable_FindSymbol(symTable, name, scopeID)
#define GetASTNodeFromSymbol(symTable, symbol) \
    CryoGlobalSymbolTable_GetASTNodeFromSymbol(symTable, symbol)
#define FindMethodSymbol(symTable, methodName, className, typeOfNode) \
    CryoGlobalSymbolTable_FindMethodSymbol(symTable, methodName, className, typeOfNode)

#define GetClassProperty(symTable, propertyName, className) \
    CryoGlobalSymbolTable_FindClassProperty(symTable, propertyName, className)
#define GetClassMethod(symTable, methodName, className) \
    CryoGlobalSymbolTable_FindClassMethod(symTable, methodName, className)

// Debug Functions
#define printGlobalSymbolTable(symTable) \
    CryoGlobalSymbolTable_PrintGlobalTable(symTable)
#define TypeOfSymbolToString(symTable, symbolType) \
    CryoGlobalSymbolTable_TypeOfSymbolToString(symTable, symbolType)
#define LogSymbol(symTable, symbol) \
    CryoGlobalSymbolTable_LogSymbol(symTable, symbol)
#define GetDependencyDirStr(symTable) \
    CryoGlobalSymbolTable_GetDependencyDirStr(symTable)
#define PrintSymbolTable(symTable, symbolTable) \
    CryoGlobalSymbolTable_PrintSymbolTable(symTable, symbolTable)
#define PrintTypeTable(symTable) \
    CryoGlobalSymbolTable_PrintTypeTable(symTable)

// Declaration Functions (Classes)
#define InitClassDeclaration(symTable, className) \
    CryoGlobalSymbolTable_InitClassDeclaration(symTable, className)
#define AddPropertyToClass(symTable, className, property) \
    CryoGlobalSymbolTable_AddPropertyToClass(symTable, className, property)
#define AddMethodToClass(symTable, className, method) \
    CryoGlobalSymbolTable_AddMethodToClass(symTable, className, method)
#define CompleteClassDeclaration(symTable, classNode, className) \
    CryoGlobalSymbolTable_CompleteClassDeclaration(symTable, classNode, className)

// Declaration Functions (Structs)
#define InitStructDeclaration(symTable, structName, parentNameID, structPtr) \
    CryoGlobalSymbolTable_InitStructDeclaration(symTable, structName, parentNameID, structPtr)
#define InitGenericStructDeclaration(symTable, structName, parentID) \
    CryoGlobalSymbolTable_InitGenericStructDeclaration(symTable, structName, parentID)
#define AddPropertyToStruct(symTable, structName, property) \
    CryoGlobalSymbolTable_AddPropertyToStruct(symTable, structName, property)
#define AddMethodToStruct(symTable, structName, method) \
    CryoGlobalSymbolTable_AddMethodToStruct(symTable, structName, method)
#define CompleteStructDeclaration(symTable, structNode, structName) \
    CryoGlobalSymbolTable_CompleteStructDeclaration(symTable, structNode, structName)

#define IsStructSymbol(symTable, name) \
    CryoGlobalSymbolTable_IsStructSymbol(symTable, name)

// Declaration Functions (Functions)
#define InitFunctionDeclaration(symTable, functionName, parentScopeID, params, paramCount, returnType) \
    CryoGlobalSymbolTable_InitFunctionDeclaration(symTable, functionName, parentScopeID, params, paramCount, returnType)
#define InitGenericFunctionDeclaration(symTable, functionName, parentScopeID, params, paramCount, returnType, genericParams, genericParamCount) \
    CryoGlobalSymbolTable_InitGenericFunctionDeclaration(symTable, functionName, parentScopeID, params, paramCount, returnType, genericParams, genericParamCount)
#define CompleteFunctionDeclaration(symTable, functionNode, functionName, parentScopeID) \
    CryoGlobalSymbolTable_CompleteFunctionDeclaration(symTable, functionNode, functionName, parentScopeID)
#define AddExternFunctionToTable(symTable, externNode, namespaceScopeID) \
    CryoGlobalSymbolTable_AddExternFunctionToTable(symTable, externNode, namespaceScopeID)

// Symbol Resolution Functions
#define GetFrontendSymbol(symTable, name, scopeID, symbolType) \
    CryoGlobalSymbolTable_GetFrontendSymbol(symTable, name, scopeID, symbolType)

#define GetNamespace(symTable) \
    CryoGlobalSymbolTable_GetNamespace(symTable)

// Misc Functions
#define MergeAllSymbolsToASTArray(symTable) \
    CryoGlobalSymbolTable_MergeAllSymbolsToASTArray(symTable)

#ifdef __cplusplus
} // C API

// -------------------------------------------------------------------------------------------------------------------------------------------------------

// =========================================================================================== //
// =========================================================================================== //
// === -------------------------------- C++ API Functions -------------------------------- === //
// =========================================================================================== //
// =========================================================================================== //

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
#include <algorithm>

#include "symbolTable/symdefs.h"
#include "symbolTable/debugSymbols.hpp"
#include "tools/utils/env.h" // getCryoRootPath()
#include "tools/cxx/IDGen.hpp"
#include "tools/utils/c_logger.h"

typedef struct ASTNode ASTNode;
typedef struct DataType DataType;

#define MAX_SYMBOLS 1024
#define MAX_DEPENDENCIES 1024
#define MAX_PROPERTY_COUNT 128
#define MAX_METHOD_COUNT 128
#define MAX_TYPE_SYMBOLS 1024

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

    /// @brief Global symbol table for the Cryo programming language.
    /// This class is responsible for managing the global symbol table, which includes
    /// the primary symbol table, dependency tables, and the standard library import table.
    /// The symbol table is used to store all symbols, types, and functions declared in the
    /// source code. The symbol table is also used to resolve symbols, and manage scope blocks.
    ///
    /// This class interfaces with C and C++ code, and provides a C API for interfacing.
    ///
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
        GlobalSymbolTable(const char *buildDir = nullptr, ScopeType scopeType = GLOBAL_SCOPE)
        {
            debugInfo = getDebugInfo(buildDir);
            tableContext = setDefaultContext();
            currentScopeType = scopeType;
            tableState = TABLE_INITIALIZED;
            this->typeTable = initTypeTable("global");
            this->dependencyTables = (SymbolTable **)malloc(sizeof(SymbolTable *) * MAX_DEPENDENCIES);
            // Initialize the cryo built-ins
            initCryoBuiltIns();
        };
        // This constructor is used for reaping the symbol table to a higher level symbol table manager.
        GlobalSymbolTable(bool isForReaping, const char *buildDir = nullptr, ScopeType scopeType = GLOBAL_SCOPE)
        {
            debugInfo = getDebugInfo(buildDir);
            tableContext = setDefaultContext();
            currentScopeType = scopeType;
            tableState = TABLE_INITIALIZED;
            this->typeTable = initTypeTable("global");
            this->isForReaping = isForReaping;
            this->tableContext.isPrimary = true;
            this->dependencyTables = (SymbolTable **)malloc(sizeof(SymbolTable *) * MAX_DEPENDENCIES);
        }
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
        TypesTable *typeTable = nullptr;               // Type table
        size_t dependencyCount = 0;                    // Number of dependencies (C interface)
        size_t dependencyCapacity = MAX_DEPENDENCIES;  // Maximum number of dependencies

        std::vector<SymbolTable *> dependencyTableVector;        // Vector of dependency tables (C++ interface)
        std::vector<FunctionSymbol *> globalFunctions;           // Vector of global functions                     [Unimplemented]
        std::vector<ExternSymbol *> externFunctions;             // Vector of external functions                   [Unimplemented]
        std::unordered_map<std::string, SymbolTable *> queryMap; // Map of namespace tables                        [Unimplemented]

        std::vector<std::string> importedModulePaths; // Vector of imported module file paths

        ScopeBlock *currentScope = nullptr; // Current scope block
        const char *scopeId = "null";       // Current scope ID
        size_t scopeDepth = 0;              // Current scope depth

        const char *currentNamespace = "null"; // Current namespace

        bool isFrontendComplete = false; // Flag for frontend completion
        bool isImporting = false;        // Flag for importing a module
        bool isForReaping = false;       // Flag for reaping the symbol table to a higher level symbol table manager.

        SymbolTable *reapedTable = nullptr; // Reaped symbol table

        CryoLinker *linker = nullptr; // Linker for the symbol table

        //===================================================================
        // Table State Management
        //===================================================================

        bool getIsPrimaryTable(void);                  // [C API]
        bool getIsDependencyTable(void);               // [C API]
        void setIsPrimaryTable(bool isPrimary);        // [C API]
        void setIsDependencyTable(bool isDependency);  // [C API]
        void tableFinished(void);                      // [C API]
        void initNamespace(const char *namespaceName); // [C API]
        void completeFrontend(void);                   // [C API]
        void importReapedTable(SymbolTable *table);    // [C API]
        void cleanupAndDestroy(void);                  // [C API]
        void resetCurrentDepsTable(void);
        void setCurrentDependencyTable(SymbolTable *table);
        void setPrimaryTable(SymbolTable *table);

        //===================================================================
        // Symbol Table Operations
        //===================================================================

        void createPrimaryTable(const char *namespaceName);                                              // [C API]
        void initDependencyTable(const char *namespaceName);                                             // [C API]
        void addNodeToTable(ASTNode *node);                                                              // [C API]
        void addVariableToSymbolTable(ASTNode *node, const char *scopeID);                               // [C API]
        void addParamToSymbolTable(ASTNode *node, const char *functionScopeID);                          // [C API]
        SymbolTable *getCurrentSymbolTable(void);                                                        // [C API]
        Symbol *getFrontendSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType); // [C API]
        void setLinker(CryoLinker *linker);                                                              // [C API]
        CryoLinker *getLinker(void);                                                                     // [C API]

        void addSymbolToTable(Symbol *symbol, SymbolTable *table);
        void completeDependencyTable(void);
        void addGlobalFunctionToTable(FunctionSymbol *function);
        void addExternFunctionToTable(ExternSymbol *function);
        void addNewDependencyTable(const char *namespaceName, SymbolTable *table);
        void pushNewDependencyTable(SymbolTable *table);
        void pushTypeSymbols(TypesTable *importedTypesTable);

        Symbol *queryCurrentTable(const char *scopeID, const char *name, TypeOfSymbol symbolType);
        Symbol *querySpecifiedTable(const char *symbolName, TypeOfSymbol symbolType, SymbolTable *table);
        SymbolTable *getPrimaryTable(void) { return symbolTable; }
        TypeOfSymbol getScopeSymbolTypeFromName(const char *symbolName);
        TypeofDataType getTypeOfDataTypeFromName(const char *symbolName);

        SymbolTable *getSpecificSymbolTable(const char *name);
        void importRuntimeSymbols(SymbolTable *table);
        const char *getNamespace(void);

        //===================================================================
        // Types Table / Type Management
        //===================================================================

        DataType *resolveDataType(const char *name); // [C API]
        void addTypeToTable(TypeSymbol *typeSymbol); // [C API]
        TypeSymbol *findTypeSymbol(std::string typeName);

        int addDataTypeSymbol(Symbol *symbol);
        void parseSymbolTableForTypes(SymbolTable *symbolTable);
        bool doesTypeExist(const char *name);

        void initTypeDefinition(Symbol *typeSymbol);
        void addOrUpdateType(TypeSymbol *typeSymbol);
        void completeTypeDefinition(Symbol *typeSymbol, const char *typeName);

        DataType *resolveGenericDataType(const char *baseName, DataType **typeArgs, int argCount);
        void registerGenericType(const char *name, GenericType **params, int paramCount);
        void registerGenericInstantiation(const char *baseName, DataType **typeArgs, int argCount, DataType *resultType);
        std::string createGenericInstantiationName(const char *baseName, DataType **typeArgs, int argCount);

        //===================================================================
        // Symbol Merging / Management
        //===================================================================

        SymbolTable *mergeAllSymbols(void);
        std::vector<Symbol *> mergePrimaryTable(void);
        std::vector<Symbol *> mergeAllDependencyTables(void);
        std::vector<Symbol *> mergeTwoTables(SymbolTable *table1, SymbolTable *table2);
        std::vector<Symbol *> mergeTwoSymbolVectors(std::vector<Symbol *> symbols1, std::vector<Symbol *> symbols2);
        SymbolTable *createNewSymbolTableFromSymbols(std::vector<Symbol *> symbols);
        void importReapedTypesTable(TypesTable *reapedTable);
        SymbolTable *getReapedTable(void) // [C API]
        {
            if (!isForReaping)
            {
                std::cerr << "Error: Attempting to get reaped table when not in reaping mode!" << std::endl;
            }
            return reapedTable;
        }
        TypesTable *getReapedTypesTable(void)
        {
            if (!isForReaping)
            {
                std::cerr << "Error: Attempting to get reaped type table when not in reaping mode!" << std::endl;
            }
            return typeTable;
        }

        //===================================================================
        // Imports / Exports
        //===================================================================

        void handleRootNodeImport(ASTNode *node); // [C API]

        SymbolTable *createNewImportTable(const char *namespaceName);
        void loopRootNode(ASTNode *node, SymbolTable *table, char *currentScopeID);

        //===================================================================
        // Scope Management
        //===================================================================

        void enterScope(const char *name);        // [C API]
        void exitScope(void);                     // [C API]
        const char *getScopeID(const char *name); // [C API]

        const char *getScopeID(void) { return scopeId; }
        size_t getScopeDepth(void) { return scopeDepth; }
        void setScopeID(const char *id) { scopeId = id; }
        void initNamepsaceScope(const char *namespaceName);

        void pushNewScopePair(const char *name, const char *id);
        const char *getScopeIDFromName(const char *name);
        const char *getScopeIDFromID(const char *id);

        //===================================================================
        // Symbol Resolution
        //===================================================================

        VariableSymbol *getFrontendVariableSymbol(const char *name, const char *scopeID);                   // [C API]
        FunctionSymbol *resolveScopedFunctionCall(const char *scopeID, const char *functionName);           // [C API]
        SymbolTable *findSymbolTable(const char *scopeID);                                                  // [C API]
        DataType *getDataTypeFromSymbol(Symbol *symbol);                                                    // [C API]
        ASTNode *getASTNodeFromSymbol(Symbol *symbol);                                                      // [C API]
        Symbol *findMethodSymbol(const char *methodName, const char *className, TypeofDataType typeOfNode); // [C API]
        Symbol *findSymbol(const char *symbolName, const char *scopeID);                                    // [C API]

        MethodSymbol *getFrontendMethodSymbol(const char *methodName, const char *className, const char *scopeID);
        Symbol *resolveFunctionSymbol(const char *symbolName, const char *scopeID, TypeOfSymbol symbolType);
        Symbol *resolveExternSymbol(const char *symbolName);
        Symbol *seekFunctionSymbolInAllTables(const char *symbolName);
        Symbol *seekMethodSymbolInAllTables(const char *methodName, const char *className, TypeofDataType typeOfNode);

        ASTNode *findClassProperty(const char *propertyName, const char *className);
        ASTNode *findClassMethod(const char *methodName, const char *className);

        //===================================================================
        // Misc Operations
        //===================================================================

        ASTNode **mergeAllSymbolsToASTArray(void); // [C API]

        //===================================================================
        // Debug Operations / Utilities
        //===================================================================

        void printGlobalTable(GlobalSymbolTable *table); // [C API]
        void mergeDBChunks(void);                        // [C API]
        const char *getDependencyDirStr(void);           // [C API]
        void printTypeTable(void);                       // [C API]

        const char *typeOfSymbolToString(TypeOfSymbol symbolType);
        void logSymbol(Symbol *symbol);
        void printScopeLookup(void)
        {
            for (auto &pair : scopeLookup)
            {
                // std::cout << "Scope: " << pair.first << " ID: " << pair.second << std::endl;
            }
        }

        const char *removeLastDirectoryFromPath(const char *path)
        {
            std::string pathStr = path;
            std::string::size_type pos = pathStr.find_last_of("/\\");
            return pathStr.substr(0, pos).c_str();
        }

        void logSymbolTable(SymbolTable *table) { debugger->logSymbolTable(table); }

        const char *getBuildDir(void) { return debugInfo.buildDir.c_str(); }

    private:
        TABLE_STATE tableState = TABLE_UNINITIALIZED;
        SymbolTableDebugger *debugger = new SymbolTableDebugger();
        std::vector<std::pair<std::string, std::string>> scopeLookup;

        //===================================================================
        // Cryo Built-In Functions
        //===================================================================

        void initCryoBuiltIns(void);
        void initCryoBuiltInTypes(void);

        //===================================================================
        // Private Helper Functions
        //===================================================================

        DebugInfo getDebugInfo(const char *buildDir)
        {
            std::string buildDirectory = buildDir;
            std::string rootDir = this->removeLastDirectoryFromPath(buildDir);
            std::string dependencyDir = buildDirectory + "/out/deps";
            std::string DBdir = buildDirectory + "/db";
            std::string debugDir = buildDirectory + "/debug";

            DebugInfo info = {buildDirectory, dependencyDir, debugDir, DBdir, rootDir};
            return info;
        }
        TableContext setDefaultContext(void) { return {false, false}; }

        Symbol *ASTNodeToSymbol(ASTNode *node);
        const char *getRootNamespace(ASTNode *root);
        void addSymbolsToSymbolTable(Symbol **symbols, SymbolTable *table);
        void addSingleSymbolToTable(Symbol *symbol, SymbolTable *table);
        void addSymbolToCurrentTable(Symbol *symbol);
        bool isParamSymbol(ASTNode *node);
        bool doesSymbolExist(Symbol *symbol, SymbolTable *table);
        bool doesStructSymbolExist(const char *name, SymbolTable *table);
        const char *getSymbolName(Symbol *symbol);

        TypesTable *initTypeTable(const char *namespaceName);
        Symbol *wrapSubSymbol(TypeOfSymbol symbolType, void *symbol);

    public:
        TypeSymbol *createTypeSymbol(const char *name, ASTNode *node, DataType *type,
                                     TypeofDataType typeOf, bool isStatic, bool isGeneric,
                                     const char *scopeId);

    protected:
        //===================================================================
        // Symbol Creation
        //===================================================================
        ScopeBlock *createScopeBlock(const char *name, size_t depth);
        VariableSymbol *createVariableSymbol(const char *name, DataType *type, ASTNode *node, const char *scopeId);
        FunctionSymbol *createFunctionSymbol(const char *name, const char *parentScopeID, DataType *returnType,
                                             DataType **paramTypes, size_t paramCount, CryoVisibilityType visibility,
                                             ASTNode *node);
        FunctionSymbol *createIncompleteFunctionSymbol(const char *name, const char *parentScopeID,
                                                       DataType *returnType, DataType **paramTypes, size_t paramCount);

        TypeSymbol *createIncompleteTypeSymbol(const char *name, TypeofDataType typeOf);
        PropertySymbol *createPropertySymbol(ASTNode *propNode);
        MethodSymbol *createMethodSymbol(ASTNode *methodNode);
        ExternSymbol *createExternSymbol(const char *name, DataType *returnType, DataType **paramTypes,
                                         size_t paramCount, CryoNodeType nodeType, CryoVisibilityType visibility,
                                         const char *scopeId);
        Symbol *createSymbol(TypeOfSymbol symbolType, void *symbol);
        SymbolTable *createSymbolTable(const char *namespaceName);
        TypesTable *createTypeTable(const char *namespaceName);

    public:
        //===================================================================
        // Class Symbol Management
        //===================================================================

        void initClassDeclaration(const char *className);                         // [C API]
        void addPropertyToClass(const char *className, ASTNode *property);        // [C API]
        void addMethodToClass(const char *className, ASTNode *method);            // [C API]
        void completeClassDeclaration(ASTNode *classNode, const char *className); // [C API]
        Symbol *createClassDeclarationSymbol(const char *className);
        void updateClassSymbolMethods(Symbol *classSymbol, MethodSymbol *method, size_t methodCount);
        void updateClassSymbolProperties(Symbol *classSymbol, PropertySymbol *property, size_t propertyCount);
        Symbol *getClassSymbol(const char *className);
        void addClassDeclarationToTable(Symbol *classSymbol, SymbolTable *table);
        void updateClassSymbol(Symbol *classSymbol, SymbolTable *table);

        //===================================================================
        // Struct Symbol Management
        //===================================================================

        void initStructDeclaration(const char *structName, const char *parentNameID, DataType *structPtr); // [C API]
        void initGenericStructDeclaration(const char *structName, const char *parentID);                   // [C API]
        void addPropertyToStruct(const char *structName, ASTNode *property);                               // [C API]
        void addMethodToStruct(const char *className, ASTNode *method);                                    // [C API]
        void completeStructDeclaration(ASTNode *structNode, const char *structName);                       // [C API]

        Symbol *createStructDeclarationSymbol(const char *structName, const char *parentNameID);
        Symbol *getStructSymbol(const char *structName);

        TypeSymbol *createStructDeclType(const char *structName, const char *parentNameID, DataType *structType);

        void updateStructSymbolMethods(Symbol *structSymbol, MethodSymbol *method, size_t methodCount);
        void updateStructSymbolProperties(Symbol *structSymbol, PropertySymbol *property, size_t propertyCount);

        void addStructDeclarationToTable(Symbol *structSymbol, SymbolTable *table);
        void updateStructSymbol(Symbol *structSymbol, SymbolTable *table);

        bool isStructSymbol(const char *name);

        //===================================================================
        // Function Symbol Management
        //===================================================================

        void initFunctionDeclaration(const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType);                                                            // [C API]
        void initGenericFunctionDeclaration(const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType, GenericType **genericParams, int genericParamCount); // [C API]

        void completeFunctionDeclaration(ASTNode *functionNode, const char *scopeID, const char *parentScopeID); // [C API]
        void addExternFunctionToTable(ASTNode *externNode, const char *namespaceScopeID);                        // [C API]

        void updateFunctionSymbol(Symbol *functionSymbol, SymbolTable *table);
    }; // GlobalSymbolTable

    // -------------------------------------------------------------------------------------------------------------------------------------------------------

    // =========================================================================================== //
    // =========================================================================================== //
    // === --------------------------------- C API Functions --------------------------------- === //
    // =========================================================================================== //
    // =========================================================================================== //

    // -------------------------------------------------------
    // C API Implementation
    inline CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create(const char *buildDir)
    {
        return reinterpret_cast<CryoGlobalSymbolTable *>(new GlobalSymbolTable(buildDir));
    }

    inline CryoGlobalSymbolTable *CryoGlobalSymbolTable_Create_Reaping(bool forReaping, const char *buildDir)
    {
        return reinterpret_cast<CryoGlobalSymbolTable *>(new GlobalSymbolTable(forReaping, buildDir));
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

    inline void CryoGlobalSymbolTable_CompleteFrontend(CryoGlobalSymbolTable *symTable)
    {
        reinterpret_cast<GlobalSymbolTable *>(symTable)->completeFrontend();
    }

    inline void CryoGlobalSymbolTable_SetLinker(CryoGlobalSymbolTable *symTable, CryoLinker *linker)
    {
        reinterpret_cast<GlobalSymbolTable *>(symTable)->setLinker(linker);
    }

    inline CryoLinker *CryoGlobalSymbolTable_GetLinker(CryoGlobalSymbolTable *symTable)
    {
        return reinterpret_cast<GlobalSymbolTable *>(symTable)->getLinker();
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

    inline void CryoGlobalSymbolTable_AddParamToSymbolTable(CryoGlobalSymbolTable *symTable, ASTNode *node, const char *functionScopeID)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addParamToSymbolTable(node, functionScopeID);
        }
    }

    inline TypeOfSymbol CryoGlobalSymbolTable_GetScopeSymbolTypeFromName(CryoGlobalSymbolTable *symTable, const char *symbolName)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getScopeSymbolTypeFromName(symbolName);
        }
        return UNKNOWN_SYMBOL;
    }

    inline void CryoGlobalSymbolTable_HandleRootNodeImport(CryoGlobalSymbolTable *symTable, ASTNode *node)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->handleRootNodeImport(node);
        }
    }

    inline SymbolTable *CryoGlobalSymbolTable_GetReapedTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getReapedTable();
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_ImportReapedTable(CryoGlobalSymbolTable *symTable, SymbolTable *reapedTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->importReapedTable(reapedTable);
        }
    }

    inline TypesTable *CryoGlobalSymbolTable_GetReapedTypesTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getReapedTypesTable();
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_ImportReapedTypesTable(CryoGlobalSymbolTable *symTable, TypesTable *reapedTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->importReapedTypesTable(reapedTable);
        }
    }

    inline void CryoGlobalSymbolTable_CleanupAndDestroySymbolTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->cleanupAndDestroy();
        }
    }

    inline SymbolTable *CryoGlobalSymbolTable_GetSpecificSymbolTable(CryoGlobalSymbolTable *symTable, const char *namespaceName)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getSpecificSymbolTable(namespaceName);
        }
        return nullptr;
    }

    inline const char *CryoGlobalSymbolTable_GetBuildDIr(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getBuildDir();
        }
        return nullptr;
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

    inline DataType *CryoGlobalSymbolTable_GetDataTypeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getDataTypeFromSymbol(symbol);
        }
        return nullptr;
    }

    inline Symbol *CryoGlobalSymbolTable_FindSymbol(CryoGlobalSymbolTable *symTable, const char *name, const char *scopeID)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->findSymbol(name, scopeID);
        }
        return nullptr;
    }

    inline ASTNode *CryoGlobalSymbolTable_GetASTNodeFromSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getASTNodeFromSymbol(symbol);
        }
        return nullptr;
    }

    inline Symbol *CryoGlobalSymbolTable_FindMethodSymbol(CryoGlobalSymbolTable *symTable, const char *methodName, const char *className, TypeofDataType typeOfNode)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->findMethodSymbol(methodName, className, typeOfNode);
        }
        return nullptr;
    }

    inline DataType *CryoGlobalSymbolTable_ResolveDataType(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->resolveDataType(name);
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_AddTypeToTable(CryoGlobalSymbolTable *symTable, TypeSymbol *typeSymbol)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addTypeToTable(typeSymbol);
        }
    }

    inline TypeSymbol *CryoGlobalSymbolTable_FindTypeSymbol(CryoGlobalSymbolTable *symTable, const char *typeName)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->findTypeSymbol(typeName);
        }
        return nullptr;
    }

    inline ASTNode *CryoGlobalSymbolTable_FindClassProperty(CryoGlobalSymbolTable *symTable, const char *propertyName, const char *className)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->findClassProperty(propertyName, className);
        }
        return nullptr;
    }

    inline ASTNode *CryoGlobalSymbolTable_FindClassMethod(CryoGlobalSymbolTable *symTable, const char *methodName, const char *className)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->findClassMethod(methodName, className);
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

    inline const char *CryoGlobalSymbolTable_TypeOfSymbolToString(CryoGlobalSymbolTable *symTable, TypeOfSymbol symbolType)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->typeOfSymbolToString(symbolType);
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_LogSymbol(CryoGlobalSymbolTable *symTable, Symbol *symbol)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->logSymbol(symbol);
        }
    }

    inline const char *CryoGlobalSymbolTable_GetDependencyDirStr(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getDependencyDirStr();
        }
        return nullptr;
    }

    inline void CryoGlobalSymbolTable_PrintSymbolTable(CryoGlobalSymbolTable *symTable, SymbolTable *symbolTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->logSymbolTable(symbolTable);
        }
    }

    inline void CryoGlobalSymbolTable_PrintTypeTable(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->printTypeTable();
        }
    }

    // Declaration Functions (Classes) ---------------------------------------

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

    inline void CryoGlobalSymbolTable_CompleteClassDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *classNode, const char *className)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->completeClassDeclaration(classNode, className);
        }
    }

    // Declaration Functions (Structs) ---------------------------------------

    inline void CryoGlobalSymbolTable_InitStructDeclaration(CryoGlobalSymbolTable *symTable, const char *structName, const char *parentNameID, DataType *structPtr)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initStructDeclaration(structName, parentNameID, structPtr);
        }
    }
    inline void CryoGlobalSymbolTable_InitGenericStructDeclaration(CryoGlobalSymbolTable *symTable, const char *structName, const char *parentNameID)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initGenericStructDeclaration(structName, parentNameID);
        }
    }

    inline void CryoGlobalSymbolTable_AddPropertyToStruct(CryoGlobalSymbolTable *symTable, const char *structName, ASTNode *property)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addPropertyToStruct(structName, property);
        }
    }

    inline void CryoGlobalSymbolTable_AddMethodToStruct(CryoGlobalSymbolTable *symTable, const char *structName, ASTNode *method)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addMethodToStruct(structName, method);
        }
    }

    inline void CryoGlobalSymbolTable_CompleteStructDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *structNode, const char *structName)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->completeStructDeclaration(structNode, structName);
        }
    }

    inline bool CryoGlobalSymbolTable_IsStructSymbol(CryoGlobalSymbolTable *symTable, const char *name)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->isStructSymbol(name);
        }
        return false;
    }

    // Declaration Functions (Functions) ---------------------------------------

    inline void CryoGlobalSymbolTable_InitFunctionDeclaration(CryoGlobalSymbolTable *symTable, const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initFunctionDeclaration(functionName, parentScopeID, params, paramCount, returnType);
        }
    }

    inline void CryoGlobalSymbolTable_InitGenericFunctionDeclaration(CryoGlobalSymbolTable *symTable, const char *functionName, const char *parentScopeID, ASTNode **params, size_t paramCount, DataType *returnType, GenericType **genericParams, int genericParamCount)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->initGenericFunctionDeclaration(functionName, parentScopeID, params, paramCount, returnType, genericParams, genericParamCount);
        }
    }

    inline void CryoGlobalSymbolTable_CompleteFunctionDeclaration(CryoGlobalSymbolTable *symTable, ASTNode *functionNode, const char *functionName, const char *parentScopeID)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->completeFunctionDeclaration(functionNode, functionName, parentScopeID);
        }
    }

    inline void CryoGlobalSymbolTable_AddExternFunctionToTable(CryoGlobalSymbolTable *symTable, ASTNode *externNode, const char *namespaceScopeID)
    {
        if (symTable)
        {
            reinterpret_cast<GlobalSymbolTable *>(symTable)->addExternFunctionToTable(externNode, namespaceScopeID);
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

    inline const char *CryoGlobalSymbolTable_GetNamespace(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->getNamespace();
        }
        return nullptr;
    }

    // Misc Functions ---------------------------------------

    inline ASTNode **CryoGlobalSymbolTable_MergeAllSymbolsToASTArray(CryoGlobalSymbolTable *symTable)
    {
        if (symTable)
        {
            return reinterpret_cast<GlobalSymbolTable *>(symTable)->mergeAllSymbolsToASTArray();
        }
        return nullptr;
    }

} // namespace Cryo

#endif // C API -------------------------------------------------------
#endif // GLOBAL_SYMBOL_TABLE_H
